param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('collection', 'inference')]
    [string]$Mode,

    [Parameter(Mandatory = $true)]
    [ValidateSet('collection', 'blocked', 'free', 'emergency', 'approach')]
    [string]$Scenario
)

$ErrorActionPreference = 'Stop'

$requiredMode = if ($Scenario -eq 'collection') { 'collection' } else { 'inference' }
if ($Mode -ne $requiredMode) {
    throw "O cenario '$Scenario' deve ser usado com o modo '$requiredMode'."
}

$repoDir = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$projectDir = Join-Path $repoDir 'cp2\wokwi'
$diagramPath = Join-Path $projectDir 'diagram.json'
$scenarioPath = Join-Path $projectDir "scenarios\$Scenario.yaml"
$outputName = if ($Mode -eq 'collection') { 'collection' } else { 'tinyml' }
$outputDir = Join-Path $repoDir 'build\wokwi'
$runRoot = Join-Path $projectDir '.runs'

if (-not (Test-Path -LiteralPath $scenarioPath)) {
    throw "Arquivo de cenario nao encontrado: $scenarioPath"
}

& (Join-Path $PSScriptRoot 'build.ps1') -Mode $Mode
if ($LASTEXITCODE -ne 0) {
    throw "A compilacao para o modo $Mode falhou."
}

$wokwiCommand = Get-Command wokwi-cli -ErrorAction SilentlyContinue
if (-not $wokwiCommand) {
    throw 'wokwi-cli nao foi encontrado no PATH. Instale pelo guia oficial em https://docs.wokwi.com/wokwi-ci/cli-installation.'
}
if (-not (Test-Path Env:WOKWI_CLI_TOKEN)) {
    throw 'Defina WOKWI_CLI_TOKEN com um token de CI do Wokwi antes de executar o CLI. A licenca da extensao do VS Code nao substitui esse token.'
}

New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
New-Item -ItemType Directory -Path $runRoot -Force | Out-Null
$runDir = Join-Path $runRoot "run-$([guid]::NewGuid().ToString('N'))"
$resolvedRunRoot = (Resolve-Path $runRoot).Path.TrimEnd([IO.Path]::DirectorySeparatorChar) + [IO.Path]::DirectorySeparatorChar
if (-not $runDir.StartsWith($resolvedRunRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'O diretorio temporario da simulacao saiu da pasta cp2/wokwi/.runs.'
}

try {
    New-Item -ItemType Directory -Path $runDir | Out-Null
    $diagram = Get-Content -LiteralPath $diagramPath -Raw | ConvertFrom-Json
    $distance = switch ($Scenario) {
        'collection' { '100' }
        'blocked' { '15' }
        'free' { '120' }
        'emergency' { '5' }
        'approach' { '120' }
    }
    ($diagram.parts | Where-Object { $_.id -eq 'range' }).attrs.distance = $distance
    $stagedDiagramPath = Join-Path $runDir 'diagram.json'
    $diagramJson = $diagram | ConvertTo-Json -Depth 32
    [IO.File]::WriteAllText($stagedDiagramPath, $diagramJson, [Text.UTF8Encoding]::new($false))
    Copy-Item -LiteralPath $scenarioPath -Destination (Join-Path $runDir 'scenario.yaml')

    $relativeFirmware = "../../../../build/$outputName/carrinho.ino.bin"
    $relativeElf = "../../../../build/$outputName/carrinho.ino.elf"
    @"
[wokwi]
version = 1
firmware = "$relativeFirmware"
elf = "$relativeElf"
"@ | ForEach-Object { [IO.File]::WriteAllText((Join-Path $runDir 'wokwi.toml'), $_, [Text.UTF8Encoding]::new($false)) }

    $logPath = Join-Path $outputDir "$Mode-$Scenario.serial.log"
    $vcdPath = Join-Path $outputDir "$Mode-$Scenario.vcd"
    & $wokwiCommand.Source $runDir --scenario 'scenario.yaml' --serial-log-file $logPath --vcd-file $vcdPath --timeout 15000
    if ($LASTEXITCODE -ne 0) {
        throw "wokwi-cli falhou com codigo $LASTEXITCODE no cenario $Scenario."
    }

    Write-Output "Log serial: $logPath"
    Write-Output "Logic analyzer: $vcdPath"
}
finally {
    if (Test-Path -LiteralPath $runDir) {
        Remove-Item -LiteralPath $runDir -Recurse -Force
    }
}
