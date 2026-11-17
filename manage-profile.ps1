param(
    [ValidateSet("Install", "Uninstall")]
    [string]$Action,
    [string]$HookPath
)

$docs = [Environment]::GetFolderPath("MyDocuments")
$profiles = @(
    Join-Path $docs "PowerShell\Microsoft.PowerShell_profile.ps1"
    Join-Path $docs "WindowsPowerShell\Microsoft.PowerShell_profile.ps1"
)

$marker = "# [PsWidget-Autohook]"
$loadLine = "if (Test-Path '$HookPath') { . '$HookPath' } $marker"

foreach ($prof in $profiles) {
    if ($Action -eq "Install") {
        $dir = Split-Path $prof -Parent
        if (-not (Test-Path $dir)) {
            New-Item -ItemType Directory -Path $dir -Force | Out-Null
        }
        if (-not (Test-Path $prof)) {
            New-Item -ItemType File -Path $prof -Force | Out-Null
        }
        $content = Get-Content $prof -Raw -ErrorAction SilentlyContinue
        if ($null -eq $content -or ($content -notmatch [regex]::Escape($marker))) {
            Add-Content -Path $prof -Value "`r`n$loadLine" -Encoding UTF8
        }
    }
    elseif ($Action -eq "Uninstall" -and (Test-Path $prof)) {
        $lines = Get-Content $prof | Where-Object { $_ -notmatch [regex]::Escape($marker) }
        Set-Content -Path $prof -Value $lines -Encoding UTF8
    }
}