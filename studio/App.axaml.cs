using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using System.Windows.Input;

namespace Espet.Studio;

public partial class App : Application
{
    public override void Initialize()
    {
        AvaloniaXamlLoader.Load(this);
    }

    public override void OnFrameworkInitializationCompleted()
    {
        if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
        {
            desktop.ShutdownMode = ShutdownMode.OnExplicitShutdown;

            var link = new BoardLink();
            var window = new DeskWindow();
            link.Updated += snap => Dispatcher.UIThread.Post(() => window.Model.Apply(snap));
            desktop.Exit += (_, _) => link.Dispose();

            var icons = new TrayIcons
            {
                new TrayIcon
                {
                    ToolTipText = "ESPET",
                    Icon = MakeIcon(),
                    Command = new ActionCommand(() => ToggleDesk(window)),
                    Menu = new NativeMenu
                    {
                        new NativeMenuItem("Show")
                        {
                            Command = new ActionCommand(() => ShowDesk(window, link)),
                        },
                        new NativeMenuItem("Release port")
                        {
                            Command = new ActionCommand(link.Release),
                        },
                        new NativeMenuItem("Quit")
                        {
                            Command = new ActionCommand(() => Quit(desktop, window, link)),
                        },
                    },
                },
            };
            TrayIcon.SetIcons(this, icons);

            link.Start();
            ShowDesk(window, link);
        }

        base.OnFrameworkInitializationCompleted();
    }

    static void ToggleDesk(DeskWindow window)
    {
        if (window.IsVisible)
        {
            window.Hide();
            return;
        }

        window.Show();
        window.Park();
        window.Activate();
    }

    static void ShowDesk(DeskWindow window, BoardLink link)
    {
        link.Resume();
        window.Show();
        window.Park();
        window.Activate();
    }

    static void Quit(IClassicDesktopStyleApplicationLifetime desktop, DeskWindow window, BoardLink link)
    {
        window.AllowClose();
        link.Dispose();
        window.Close();
        desktop.Shutdown();
    }

    static WindowIcon MakeIcon()
    {
        var bmp = new RenderTargetBitmap(new PixelSize(32, 32), new Vector(96, 96));
        using (var ctx = bmp.CreateDrawingContext())
        {
            ctx.FillRectangle(new SolidColorBrush(Color.Parse("#141414")), new Rect(0, 0, 32, 32));
            ctx.FillRectangle(new SolidColorBrush(Color.Parse("#f2f2f2")), new Rect(8, 8, 16, 16));
        }

        return new WindowIcon(bmp);
    }

    sealed class ActionCommand : ICommand
    {
        readonly Action _action;

        public ActionCommand(Action action) => _action = action;

        public event EventHandler? CanExecuteChanged
        {
            add { }
            remove { }
        }

        public bool CanExecute(object? parameter) => true;

        public void Execute(object? parameter) => _action();
    }
}
