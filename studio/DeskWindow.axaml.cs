using Avalonia;
using Avalonia.Controls;

namespace Espet.Studio;

public partial class DeskWindow : Window
{
    bool _allowClose;

    public DeskWindow()
    {
        Model = new DeskModel();
        DataContext = Model;
        InitializeComponent();
    }

    public DeskModel Model { get; }

    public void AllowClose() => _allowClose = true;

    protected override void OnOpened(EventArgs e)
    {
        base.OnOpened(e);
        Park();
    }

    public void Park()
    {
        var screen = Screens?.Primary;
        if (screen == null)
        {
            return;
        }

        var area = screen.WorkingArea;
        var scale = screen.Scaling;
        var w = (int)Math.Ceiling(Width * scale);
        var h = (int)Math.Ceiling(Height * scale);
        const int margin = 12;
        Position = new PixelPoint(area.Right - w - margin, area.Bottom - h - margin);
    }

    protected override void OnClosing(WindowClosingEventArgs e)
    {
        if (!_allowClose)
        {
            e.Cancel = true;
            Hide();
        }

        base.OnClosing(e);
    }
}
