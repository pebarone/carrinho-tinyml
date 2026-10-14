$ErrorActionPreference = 'Stop'

$projectDir = $PSScriptRoot
$outputDir = Join-Path $projectDir '.wokwi-firmware'
$sketchDir = Join-Path $projectDir 'sketch'
$firmwarePath = Join-Path $outputDir 'sketch.ino.bin'
$elfPath = Join-Path $outputDir 'sketch.ino.elf'

arduino-cli compile --fqbn esp32:esp32:esp32 --output-dir $outputDir $sketchDir
if ($LASTEXITCODE -ne 0) {
    throw "arduino-cli compile falhou com codigo $LASTEXITCODE"
}
if (-not (Test-Path -LiteralPath $firmwarePath) -or -not (Test-Path -LiteralPath $elfPath)) {
    throw 'A compilacao terminou sem gerar o firmware e o ELF esperados.'
}

$requiredFiles = @('sketch.ino.bin', 'sketch.ino.elf')
Get-ChildItem -File -LiteralPath $outputDir |
    Where-Object { $_.Name -notin $requiredFiles } |
    ForEach-Object { Remove-Item -LiteralPath $_.FullName }

Write-Output "Firmware: $firmwarePath"
Write-Output "ELF: $elfPath"
