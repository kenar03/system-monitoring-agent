param(
    [ValidateSet('Debug', 'Release')]
    [string]$Config = 'Debug',

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$AgentArgs
)

$ErrorActionPreference = 'Stop'

$Root = Resolve-Path (Join-Path $PSScriptRoot '..')
$BuildDir = Join-Path $Root 'build'

cmake -S $Root -B $BuildDir
cmake --build $BuildDir --config $Config

$Executable = Join-Path $BuildDir "$Config/SystemMonitoringAgent.exe"
if (-not (Test-Path $Executable)) {
    throw "Executable was not found: $Executable"
}

& $Executable @AgentArgs
exit $LASTEXITCODE
