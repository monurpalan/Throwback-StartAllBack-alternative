<#
.SYNOPSIS
  Starts Throwback in every theme and language, captures screenshots and checks
  that the bar reserves its screen space and shuts down cleanly.

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File tools\smoke-test.ps1
#>
param(
  [string]$Exe = "$PSScriptRoot\..\build\msvc\Debug\throwback.exe",
  [string]$OutDir = "$PSScriptRoot\..\build\smoke"
)
$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System; using System.Runtime.InteropServices;
public static class SmokeNative {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
  [DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern IntPtr FindWindow(string cls, string title);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hwnd, out RECT rect);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hwnd, uint msg, IntPtr w, IntPtr l);
  [DllImport("user32.dll")] public static extern bool SystemParametersInfo(int action, int param, ref RECT rect, int flags);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hwnd);
  [DllImport("user32.dll")] public static extern void mouse_event(uint flags, int x, int y, uint data, UIntPtr extra);
  [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, UIntPtr extra);
  public static RECT WorkArea() { var r = new RECT(); SystemParametersInfo(0x30, 0, ref r, 0); return r; }
}
'@

function Find-Bar {
  $hwnd = [SmokeNative]::FindWindow("ThrowbackTaskbar", [NullString]::Value)
  if ($hwnd -eq [IntPtr]::Zero) { return $null }  # IntPtr.Zero is truthy in PowerShell
  $hwnd
}

function Save-Screen([string]$Path, [int]$X, [int]$Y, [int]$Width, [int]$Height) {
  $bitmap = New-Object System.Drawing.Bitmap $Width, $Height
  $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
  $graphics.CopyFromScreen($X, $Y, 0, 0, $bitmap.Size)
  $bitmap.Save($Path)
  $graphics.Dispose(); $bitmap.Dispose()
}

if (-not (Test-Path $Exe)) { throw "Build first: $Exe not found" }
if (Find-Bar) { throw "Throwback is already running; close it first" }
New-Item -ItemType Directory -Force $OutDir | Out-Null
$before = [SmokeNative]::WorkArea()
$failures = @()

foreach ($theme in "dark", "light") {
  foreach ($lang in "en", "tr") {
    $name = "$theme-$lang"
    $process = Start-Process $Exe -ArgumentList "--theme=$theme", "--lang=$lang" -PassThru
    Start-Sleep -Seconds 3
    $bar = Find-Bar
    if (-not $bar) { $failures += "${name}: bar window not found"; continue }

    $rect = New-Object SmokeNative+RECT
    [void][SmokeNative]::GetWindowRect($bar, [ref]$rect)
    $height = $rect.Bottom - $rect.Top
    $work = [SmokeNative]::WorkArea()
    if ($work.Top -lt $rect.Bottom -and $work.Bottom -gt $rect.Top) {
      $failures += "${name}: work area overlaps the bar"
    }
    Save-Screen "$OutDir\$name-bar.png" $rect.Left $rect.Top ($rect.Right - $rect.Left) $height

    # Right-click the clock: opens the bar menu with a translated item.
    $clockX = $rect.Right - [int]($height * 1.5)
    [void][SmokeNative]::SetCursorPos($clockX, $rect.Top + [int]($height / 2))
    [SmokeNative]::mouse_event(0x8, 0, 0, 0, [UIntPtr]::Zero)
    [SmokeNative]::mouse_event(0x10, 0, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 700
    Save-Screen "$OutDir\$name-menu.png" ($clockX - 300) $rect.Top 420 ($height + 80)
    [SmokeNative]::keybd_event(0x1B, 0, 0, [UIntPtr]::Zero)
    [SmokeNative]::keybd_event(0x1B, 0, 2, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 300

    # Click the orb: the start menu opens next to the bar.
    [void][SmokeNative]::SetCursorPos($rect.Left + [int]($height * 0.7), $rect.Top + [int]($height / 2))
    [SmokeNative]::mouse_event(0x2, 0, 0, 0, [UIntPtr]::Zero)
    [SmokeNative]::mouse_event(0x4, 0, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 2000
    $menu = [SmokeNative]::FindWindow("ThrowbackStartMenu", [NullString]::Value)
    if ($menu -eq [IntPtr]::Zero -or -not [SmokeNative]::IsWindowVisible($menu)) {
      $failures += "${name}: start menu did not open"
    } else {
      $menuRect = New-Object SmokeNative+RECT
      [void][SmokeNative]::GetWindowRect($menu, [ref]$menuRect)
      Save-Screen "$OutDir\$name-start.png" $menuRect.Left $menuRect.Top `
        ($menuRect.Right - $menuRect.Left) ($menuRect.Bottom - $menuRect.Top)
    }
    [SmokeNative]::keybd_event(0x1B, 0, 0, [UIntPtr]::Zero)
    [SmokeNative]::keybd_event(0x1B, 0, 2, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 300

    [void][SmokeNative]::PostMessage($bar, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero)  # WM_CLOSE
    if (-not $process.WaitForExit(5000)) {
      $failures += "${name}: did not exit after WM_CLOSE"
      Stop-Process -Id $process.Id -Force
    } elseif ($process.ExitCode -ne 0) {
      $failures += "${name}: exit code $($process.ExitCode)"
    }
    Start-Sleep -Milliseconds 500
    $after = [SmokeNative]::WorkArea()
    if ($after.Top -ne $before.Top -or $after.Bottom -ne $before.Bottom) {
      $failures += "${name}: work area not restored"
    }
    Write-Host "checked $name"
  }
}

Write-Host "Screenshots: $((Resolve-Path $OutDir).Path)"
if ($failures.Count) {
  $failures | ForEach-Object { Write-Host "FAIL $_" }
  exit 1
}
Write-Host "Smoke test passed"
