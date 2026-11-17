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
