using System.IO.Ports;
using System.Management;
using System.Text.RegularExpressions;

namespace Espet.Studio;

public readonly record struct BoardSnapshot(
    string Status,
    string Heap,
    string Ship,
    string GapMean,
    string Log);

public sealed class BoardLink : IDisposable
{
    const int LogLimit = 6;

    static readonly Regex Ansi = new(@"\u001B\[[0-9;]*[A-Za-z]", RegexOptions.Compiled);
    static readonly Regex ComName = new(@"\((COM\d+)\)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
    static readonly Regex HeapLine = new(@"free internal\s+(\d+)", RegexOptions.Compiled);
    static readonly Regex ShipLine = new(@"ship\s+(\d+)\s+us", RegexOptions.Compiled);
    static readonly Regex MeanLine = new(@"gap min\s+\d+\s+max\s+\d+\s+mean\s+(\d+)", RegexOptions.Compiled);

    readonly object _gate = new();
    readonly Queue<string> _log = new();
    Thread? _thread;
    SerialPort? _port;
    string? _heap;
    string? _ship;
    string? _gapMean;
    volatile bool _stop;
    volatile bool _paused;

    public event Action<BoardSnapshot>? Updated;

    public void Start()
    {
        _thread = new Thread(Loop) { IsBackground = true, Name = "board-link" };
        _thread.Start();
    }

    public void Release()
    {
        _paused = true;
        lock (_gate)
        {
            ClearStats();
        }

        ClosePort();
        Publish("port released");
    }

    public void Resume()
    {
        _paused = false;
    }

    public void Dispose()
    {
        _stop = true;
        ClosePort();
        _thread?.Join(TimeSpan.FromSeconds(2));
    }

    void Loop()
    {
        while (!_stop)
        {
            if (_paused)
            {
                Thread.Sleep(200);
                continue;
            }

            var port = OpenPort();
            if (port == null)
            {
                TryOpen();
                Thread.Sleep(400);
                continue;
            }

            try
            {
                var line = port.ReadLine();
                Accept(line);
            }
            catch (TimeoutException)
            {
            }
            catch (Exception)
            {
                if (_stop || _paused)
                {
                    continue;
                }

                ClosePort();
                lock (_gate)
                {
                    ClearStats();
                }

                Publish("no board");
            }
        }
    }

    void TryOpen()
    {
        if (_paused || _stop)
        {
            return;
        }

        List<string> ports;
        try
        {
            ports = FindPorts();
        }
        catch
        {
            Publish("no board");
            return;
        }

        if (ports.Count == 0)
        {
            Publish("no board");
            return;
        }

        if (ports.Count > 1)
        {
            Publish("more than one Espressif port");
            return;
        }

        var name = ports[0];
        var port = new SerialPort(name, 115200)
        {
            DtrEnable = false,
            RtsEnable = false,
            Handshake = Handshake.None,
            ReadTimeout = 500,
            NewLine = "\n",
            Encoding = System.Text.Encoding.UTF8,
        };

        try
        {
            port.Open();
            port.DtrEnable = false;
            port.RtsEnable = false;
            try
            {
                port.DiscardInBuffer();
            }
            catch (Exception)
            {
            }
        }
        catch (Exception ex) when (ex is UnauthorizedAccessException || IsBusy(ex))
        {
            port.Dispose();
            Publish($"{name} is in use");
            return;
        }
        catch (Exception)
        {
            port.Dispose();
            Publish("no board");
            return;
        }

        lock (_gate)
        {
            if (_paused || _stop)
            {
                port.Dispose();
                return;
            }

            ClearStats();
            _port = port;
        }

        Publish($"connected · {name}");
    }

    void Accept(string raw)
    {
        var line = Ansi.Replace(raw, "").Trim().TrimEnd('\r');
        if (line.Length == 0)
        {
            return;
        }

        BoardSnapshot snap;
        lock (_gate)
        {
            _log.Enqueue(line);
            while (_log.Count > LogLimit)
            {
                _log.Dequeue();
            }

            var heap = HeapLine.Match(line);
            if (heap.Success)
            {
                _heap = heap.Groups[1].Value;
            }

            var ship = ShipLine.Match(line);
            if (ship.Success)
            {
                _ship = ship.Groups[1].Value + " us";
            }

            var mean = MeanLine.Match(line);
            if (mean.Success)
            {
                _gapMean = mean.Groups[1].Value + " us";
            }

            var portName = _port?.PortName;
            var status = portName == null ? "no board" : $"connected · {portName}";
            snap = Capture(status);
        }

        Updated?.Invoke(snap);
    }

    SerialPort? OpenPort()
    {
        lock (_gate)
        {
            return _port;
        }
    }

    void ClosePort()
    {
        SerialPort? port;
        lock (_gate)
        {
            port = _port;
            _port = null;
        }

        if (port == null)
        {
            return;
        }

        try
        {
            port.Close();
        }
        catch (Exception)
        {
        }

        try
        {
            port.Dispose();
        }
        catch (Exception)
        {
        }
    }

    void ClearStats()
    {
        _heap = null;
        _ship = null;
        _gapMean = null;
        _log.Clear();
    }

    void Publish(string status)
    {
        BoardSnapshot snap;
        lock (_gate)
        {
            snap = Capture(status);
        }

        Updated?.Invoke(snap);
    }

    BoardSnapshot Capture(string status)
    {
        return new BoardSnapshot(
            status,
            Blank(_heap),
            Blank(_ship),
            Blank(_gapMean),
            string.Join('\n', _log));
    }

    static string Blank(string? value) => string.IsNullOrEmpty(value) ? "—" : value;

    static bool IsBusy(Exception ex)
    {
        var message = ex.Message;
        return message.Contains("access", StringComparison.OrdinalIgnoreCase)
            || message.Contains("use", StringComparison.OrdinalIgnoreCase);
    }

    static List<string> FindPorts()
    {
        var found = new List<string>();
        using var searcher = new ManagementObjectSearcher(
            "SELECT Name, DeviceID FROM Win32_PnPEntity WHERE DeviceID LIKE '%VID_303A%'");
        using var results = searcher.Get();
        foreach (var obj in results)
        {
            using var device = (ManagementObject)obj;
            var id = device["DeviceID"] as string;
            var name = device["Name"] as string;
            if (id == null || name == null)
            {
                continue;
            }

            if (!id.Contains("VID_303A", StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }

            var match = ComName.Match(name);
            if (!match.Success)
            {
                continue;
            }

            var port = match.Groups[1].Value.ToUpperInvariant();
            if (!found.Contains(port))
            {
                found.Add(port);
            }
        }

        return found;
    }
}
