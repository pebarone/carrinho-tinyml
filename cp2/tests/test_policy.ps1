$ErrorActionPreference = 'Stop'

$dataRootOutput = & arduino-cli config get directories.data
if ($LASTEXITCODE -ne 0) {
  throw 'Could not read the Arduino CLI data directory.'
}
$dataRoot = ($dataRootOutput | Out-String).Trim()
$toolRoot = Join-Path $dataRoot 'packages\esp32\tools\esp-x32'
if (-not (Test-Path -LiteralPath $toolRoot -PathType Container)) {
  throw "ESP32 Xtensa toolchain was not found under Arduino data directory: $toolRoot"
}

$compiler = Get-ChildItem -LiteralPath $toolRoot -Filter 'xtensa-esp-elf-g++.exe' -File -Recurse |
  Sort-Object FullName -Descending |
  Select-Object -First 1 -ExpandProperty FullName
if (-not $compiler) {
  throw "xtensa-esp-elf-g++.exe was not found under: $toolRoot"
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$checks = @(
  (Join-Path $PSScriptRoot 'firmware_policy_compile.cpp'),
  (Join-Path $repoRoot 'cp1\tests\threshold_policy_compile.cpp')
)

foreach ($source in $checks) {
  if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
    throw "Policy compile check is missing: $source"
  }
  Write-Host "Checking $source"
  & $compiler -std=gnu++17 -Wall -Wextra -Werror -fsyntax-only $source
  if ($LASTEXITCODE -ne 0) {
    throw "Policy compile check failed: $source"
  }
}

Write-Host 'All firmware policy assertions passed.'
