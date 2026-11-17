# Visualizer Project

This project provides a visualization tool for PowerShell environments.

## Features

- Visualizes system data and processes
- Provides a graphical interface for monitoring
- Supports custom visualization hooks via PowerShell scripts

## Setup

### Prerequisites

- Windows operating system
- PowerShell 5.1 or later
- .NET Framework (for main.exe)

### Installation

1. Clone or download this repository
2. Navigate to the project directory
3. Run the main application by executing `main.exe`

### Autostart Configuration

To configure the application to start automatically when Windows starts, run the following PowerShell command:

```powershell
Set-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run" -Name "PsWidget" -Value "{path}\main.exe"
```

Replace `{path}` with the actual path to your Visualizer installation directory.

### Profile Configuration

Add the following to your PowerShell profile to enable visualization hooks:

```powershell
if (Test-Path "{path}\pswidget-hook.ps1") { 
    . "{path}\pswidget-hook.ps1"
    Write-Host "Visualization [OK]" -ForegroundColor Green
}
```

Again, replace `{path}` with your actual installation path.

## Usage

1. Launch the application via the autostart or manually
2. The visualization interface will appear
3. Customize visualizations by editing the `pswidget-hook.ps1` file

## Notes

- Ensure the path to `main.exe` and `pswidget-hook.ps1` is correct for your installation
- The visualization hook script must be properly formatted PowerShell code
- Administrative privileges may be required for some visualization features

## License

## Mouse Controls

- **Left Click & Drag**: Move the widget horizontally.
- **Double Left Click**: Reset the widget's position to default.
- **Right Click**: Close the application.

## Progress bar capture

If you want to have progress bar displayed paste this into console, again providing real path: `$code = 'Add-Type -TypeDefinition ''using System; using System.IO; using System.IO.Pipes; using System.Runtime.InteropServices; using System.Text; using System.Text.RegularExpressions; using System.Threading; public static class VisLiveWatcher { [StructLayout(LayoutKind.Sequential)] public struct COORD { public short X; public short Y; public COORD(short x, short y) { X = x; Y = y; } } [StructLayout(LayoutKind.Sequential)] public struct SMALL_RECT { public short Left, Top, Right, Bottom; } [StructLayout(LayoutKind.Sequential)] public struct CONSOLE_SCREEN_BUFFER_INFO { public COORD dwSize; public COORD dwCursorPosition; public short wAttributes; public SMALL_RECT srWindow; public COORD dwMaximumWindowSize; } [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)] static extern IntPtr CreateFileW(string lpFileName, uint dwDesiredAccess, uint dwShareMode, IntPtr lpSecurityAttributes, uint dwCreationDisposition, uint dwFlagsAndAttributes, IntPtr hTemplateFile); [DllImport("kernel32.dll", SetLastError = true)] static extern bool GetConsoleScreenBufferInfo(IntPtr hConsoleOutput, out CONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo); [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)] static extern bool ReadConsoleOutputCharacterW(IntPtr hConsoleOutput, [Out] char[] lpCharacter, uint nLength, COORD dwReadCoord, out uint lpNumberOfCharsRead); public static volatile bool IsActive = false; private static Thread _worker; private static int _pid; private static string _lastSent = ""; private static readonly Regex PromptRegex = new Regex(@"^(\(.+\)\s*)?PS [A-Za-z]:\\.*>", RegexOptions.Compiled); private static readonly Regex VisCmdRegex = new Regex(@"^vis(\s|$)", RegexOptions.Compiled | RegexOptions.IgnoreCase); private static readonly Regex MultiSpaceRegex = new Regex(@"\s{2,}", RegexOptions.Compiled); public static bool Start(int pid) { _pid = pid; IsActive = true; _lastSent = ""; SendPipe("FOCUS", ""); bool ok = SendPipe("OUT", "Connected (PID: " + pid + ")"); if (_worker == null || !_worker.IsAlive) { _worker = new Thread(Loop); _worker.IsBackground = true; _worker.Start(); } return ok; } public static void Stop() { IsActive = false; } public static void NotifyEnter() { if (IsActive) { _lastSent = ""; SendPipe("FOCUS", ""); } } public static bool SendPipe(string type, string text) { try { using (var pipe = new NamedPipeClientStream(".", "PsLastLinePipe", PipeDirection.Out)) { pipe.Connect(25); using (var writer = new StreamWriter(pipe, new UTF8Encoding(false))) { writer.AutoFlush = true; writer.WriteLine(_pid + "|" + type + "|" + text); } } return true; } catch { return false; } } static string ReadRow(IntPtr hCon, short row, short width) { if (row < 0 || width <= 0) return ""; char[] buf = new char[width]; uint read = 0; if (!ReadConsoleOutputCharacterW(hCon, buf, (uint)width, new COORD(0, row), out read) || read == 0) return ""; return new string(buf, 0, (int)read).Trim(); } static bool IsIgnored(string line) { if (string.IsNullOrWhiteSpace(line)) return true; if (PromptRegex.IsMatch(line)) return true; if (VisCmdRegex.IsMatch(line)) return true; if (line.StartsWith("[vis]")) return true; return false; } static void Loop() { IntPtr hCon = CreateFileW("CONOUT$", 0xC0000000u, 3u, IntPtr.Zero, 3u, 0u, IntPtr.Zero); while (true) { if (!IsActive) { Thread.Sleep(200); continue; } try { CONSOLE_SCREEN_BUFFER_INFO csbi; if (GetConsoleScreenBufferInfo(hCon, out csbi)) { short curY = csbi.dwCursorPosition.Y; short width = csbi.dwSize.X; short minRow = (short)Math.Max(0, curY - 15); for (short r = curY; r >= minRow; r--) { string line = ReadRow(hCon, r, width); if (!IsIgnored(line)) { string compact = MultiSpaceRegex.Replace(line, " "); if ((compact.Contains("%") || compact.StartsWith("#")) && r > 0) { string prev = ReadRow(hCon, (short)(r - 1), width); if (!IsIgnored(prev)) { compact = MultiSpaceRegex.Replace(prev, " ") + " " + compact; } } if (compact != _lastSent) { _lastSent = compact; SendPipe("OUT", compact); } break; } } } } catch {} Thread.Sleep(1`
