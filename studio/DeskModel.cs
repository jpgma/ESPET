using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Espet.Studio;

public sealed class DeskModel : INotifyPropertyChanged
{
    string _status = "no board";
    string _heap = "—";
    string _ship = "—";
    string _gapMean = "—";
    string _log = "";

    public event PropertyChangedEventHandler? PropertyChanged;

    public string Status
    {
        get => _status;
        private set => Set(ref _status, value);
    }

    public string Heap
    {
        get => _heap;
        private set => Set(ref _heap, value);
    }

    public string Ship
    {
        get => _ship;
        private set => Set(ref _ship, value);
    }

    public string GapMean
    {
        get => _gapMean;
        private set => Set(ref _gapMean, value);
    }

    public string Log
    {
        get => _log;
        private set => Set(ref _log, value);
    }

    public void Apply(BoardSnapshot snap)
    {
        Status = snap.Status;
        Heap = snap.Heap;
        Ship = snap.Ship;
        GapMean = snap.GapMean;
        Log = snap.Log;
    }

    void Set(ref string field, string value, [CallerMemberName] string? name = null)
    {
        if (field == value)
        {
            return;
        }

        field = value;
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }
}
