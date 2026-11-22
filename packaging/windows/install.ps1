# Zephyr Terminal Windows Offline Installer (PowerShell)
param (
    [string]$InstallDir = "$env:LOCALAPPDATA\Programs\ZephyrTerminal"
)

Write-Host "==============================================" -ForegroundColor Cyan
Write-Host "    Zephyr Terminal Windows Installer       " -ForegroundColor Cyan
Write-Host "==============================================" -ForegroundColor Cyan

New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
New-Item -ItemType Directory -Force -Path "$InstallDir\bin" | Out-Null

Copy-Item -Path ".\build\zephyr-shell.exe" -Destination "$InstallDir\bin\zephyr-shell.exe" -ErrorAction SilentlyContinue
Copy-Item -Path ".\build\zephyr.exe" -Destination "$InstallDir\bin\zephyr.exe" -ErrorAction SilentlyContinue
Copy-Item -Path ".\zephyr.desktop" -Destination "$InstallDir\zephyr.desktop" -ErrorAction SilentlyContinue

# Add to User PATH if not present
$UserPath = [Environment]::GetEnvironmentVariable("PATH", "User")
if ($UserPath -notlike "*$InstallDir\bin*") {
    [Environment]::SetEnvironmentVariable("PATH", "$UserPath;$InstallDir\bin", "User")
    Write-Host "-> Added $InstallDir\bin to User PATH." -ForegroundColor Green
}

Write-Host "`nZephyr Terminal successfully installed on Windows!" -ForegroundColor Green
Write-Host "You can now run 'zephyr-shell' or 'zephyr' in PowerShell/CMD/Terminal." -ForegroundColor Yellow

