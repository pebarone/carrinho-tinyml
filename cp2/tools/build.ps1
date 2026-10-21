param(
    [ValidateSet('collection', 'inference')]
    [string]$Mode = 'collection',
    [ValidateSet('real', 'synthetic')]
    [string]$Model = 'real'
)

$ErrorActionPreference = 'Stop'

$repoDir = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$sketchDir = Join-Path $repoDir 'cp2\firmware\carrinho'
$modelArchive = Join-Path $repoDir 'cp2\model\edge-impulse-arduino-v1.zip'
$libraryRoot = Join-Path $repoDir 'cp2\model\library'
$libraryDir = Join-Path $libraryRoot 'cp2-carrinho-ultrassom_inferencing'
$modelHeader = Join-Path $sketchDir 'model_config.h'
$inferenceHeaderName = 'cp2-carrinho-ultrassom_inferencing.h'
$manifestPath = Join-Path $repoDir 'cp2\model\export.json'
if ($Model -eq 'real') {
    $modelArchive = Join-Path $repoDir 'cp2\model\edge-impulse-arduino-real-v1-float32.zip'
    $manifestPath = Join-Path $repoDir 'cp2\model\export-real-v1-float32.json'
    $libraryRoot = Join-Path $repoDir 'cp2\model\library\real-v1-float32'
    $libraryDir = Join-Path $libraryRoot 'cp2-carrinho-real-v1_inferencing'
    $inferenceHeaderName = 'cp2-carrinho-real-v1_inferencing.h'
}
$outputName = if ($Mode -eq 'collection') { 'collection' } elseif ($Model -eq 'real') { 'tinyml-real' } else { 'tinyml' }
$outputDir = Join-Path $repoDir "build\$outputName"
$firmwarePath = Join-Path $outputDir 'carrinho.ino.bin'
$elfPath = Join-Path $outputDir 'carrinho.ino.elf'

if (-not (Get-Command arduino-cli -ErrorAction SilentlyContinue)) {
    throw 'arduino-cli nao foi encontrado no PATH.'
}

if ($Mode -eq 'inference') {
    $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
    $archiveHash = (Get-FileHash -LiteralPath $modelArchive -Algorithm SHA256).Hash
    if ($archiveHash -ne $manifest.sha256) {
        throw "O ZIP do modelo difere do SHA-256 registrado em $manifestPath."
    }
    if (-not (Test-Path -LiteralPath $libraryDir)) {
        if (-not (Test-Path -LiteralPath $modelArchive)) {
            throw "Falta o arquivo versionado da biblioteca Edge Impulse: $modelArchive"
        }

        New-Item -ItemType Directory -Path $libraryRoot -Force | Out-Null
        Expand-Archive -LiteralPath $modelArchive -DestinationPath $libraryRoot -Force
    }

    $inferenceHeader = Join-Path $libraryDir "src\$inferenceHeaderName"
    if (-not (Test-Path -LiteralPath $inferenceHeader)) {
        throw "A biblioteca Edge Impulse local nao contem o header esperado: $inferenceHeader"
    }

    if ($Model -eq 'synthetic' -and -not (Test-Path -LiteralPath $modelHeader)) {
        Copy-Item -LiteralPath (Join-Path $sketchDir 'model_config.example.h') -Destination $modelHeader
    }
}

New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $repoDir 'build\wokwi') -Force | Out-Null
$collectionFlag = if ($Mode -eq 'collection') { 1 } else { 0 }
$syntheticFlag = if ($Model -eq 'synthetic') { 1 } else { 0 }
$buildProperty = "compiler.cpp.extra_flags=-DCOLLECTION_MODE=$collectionFlag -DCP2_USE_SYNTHETIC_MODEL=$syntheticFlag"
$compileArgs = @(
    '--fqbn', 'esp32:esp32:esp32',
    '--output-dir', $outputDir,
    '--build-property', $buildProperty
)
if ($Mode -eq 'inference') {
    $compileArgs += @('--libraries', $libraryRoot)
}
$compileArgs += $sketchDir

& arduino-cli compile @compileArgs
if ($LASTEXITCODE -ne 0) {
    throw "arduino-cli compile falhou com codigo $LASTEXITCODE no modo $Mode."
}

if (-not (Test-Path -LiteralPath $firmwarePath) -or -not (Test-Path -LiteralPath $elfPath)) {
    throw "A compilacao terminou sem gerar os arquivos esperados em $outputDir."
}

Write-Output "Modo: $Mode"
Write-Output "Modelo: $Model"
Write-Output "Firmware: $firmwarePath"
Write-Output "ELF: $elfPath"
