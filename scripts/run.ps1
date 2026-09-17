param(
    [ValidateSet('Debug', 'Release')]
    [string]$Config = 'Debug',

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$AgentArgs
)

$ErrorActionPreference = 'Stop'

$Root = Resolve-Path (Join-Path $PSScriptRoot '..')
$Preset = if ($Config -eq 'Release') { 'windows-msvc-release' } else { 'windows-msvc-debug' }
$BuildDir = if ($Config -eq 'Release') { 'msvc-release' } else { 'msvc-debug' }

cmake --preset $Preset
cmake --build --preset $Preset

$Executable = Join-Path $Root "build/$BuildDir/bin/$Config/system_monitoring_agent.exe"
if (-not (Test-Path $Executable)) {
    throw "Executable was not found: $Executable"
}

& $Executable @AgentArgs
exit $LASTEXITCODE
