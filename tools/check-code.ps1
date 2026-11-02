<#
.SYNOPSIS
  Code health limits, run in CI and locally:
  - source files at most 600 lines, test files at most 1000;
  - files that cannot be split are listed, with a reason, in
    tools/file-size-allowlist.txt, and that list may only shrink;
  - functions at most 120 lines and cognitive complexity 25 (clang-tidy).

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File tools\check-code.ps1
#>
param([string]$ClangTidy = "clang-tidy")
$ErrorActionPreference = "Stop"
$root = Resolve-Path "$PSScriptRoot\.."
$failures = @()

# Lowering this is welcome; raising it needs a very good reason in review.
$MaxAllowlistEntries = 0
$SourceLimit = 600
$TestLimit = 1000

# --- File sizes ------------------------------------------------------------
$allowlistPath = Join-Path $PSScriptRoot "file-size-allowlist.txt"
$allowed = @{}
foreach ($line in Get-Content $allowlistPath) {
  $entry = $line.Trim()
  if ($entry -eq "" -or $entry.StartsWith("#")) { continue }
  $parts = $entry -split "\s+#\s*", 2
  if ($parts.Count -lt 2 -or $parts[1].Trim() -eq "") {
    $failures += "allowlist entry without a reason: $entry"
  }
  $allowed[$parts[0].Trim()] = $true
}
if ($allowed.Count -gt $MaxAllowlistEntries) {
  $failures += "file-size allowlist has $($allowed.Count) entries; it may only shrink (max $MaxAllowlistEntries)"
}

$files = Get-ChildItem (Join-Path $root "src"), (Join-Path $root "tests") -Recurse -Include *.cpp, *.h
foreach ($file in $files) {
  $relative = $file.FullName.Substring($root.Path.Length + 1).Replace("\", "/")
  $lines = (Get-Content $file.FullName | Measure-Object -Line).Lines
  $limit = if ($relative.StartsWith("tests/")) { $TestLimit } else { $SourceLimit }
  if ($lines -gt $limit -and -not $allowed.ContainsKey($relative)) {
    $failures += "${relative}: $lines lines (limit $limit)"
  }
}

# --- Function size and complexity -----------------------------------------
$flags = @("-std=c++20", "-I$root\src", "-DUNICODE", "-D_UNICODE", "-DWIN32_LEAN_AND_MEAN", "-DNOMINMAX")
foreach ($file in Get-ChildItem (Join-Path $root "src") -Recurse -Include *.cpp) {
  $output = & $ClangTidy --quiet $file.FullName -- @flags 2>&1
  if ($LASTEXITCODE -ne 0) {
    $failures += ($output | Where-Object { $_ -match "error:" } | ForEach-Object { "$_" })
  }
}

if ($failures.Count) {
  $failures | ForEach-Object { Write-Host "FAIL $_" }
  exit 1
}
Write-Host "Code checks passed ($($files.Count) files)"
