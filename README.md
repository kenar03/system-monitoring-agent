# system-monitoring-agent

Prototype C++ console agent for collecting user activity metrics and sending
them to a demo HTTP endpoint.

## Requirements

- CMake 3.24 or newer
- Visual Studio 2022 with the C++ desktop workload, or another C++17 compiler
- VS Code with the recommended C/C++ and CMake Tools extensions

## Build

Configure and build the default Debug preset:

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
```

The executable is produced under:

```text
build/msvc-debug/bin/Debug/system_monitoring_agent.exe
```

## Run

```powershell
.\scripts\run.cmd
```

The current application is a scaffold. It stays alive like a background agent
until Ctrl+C and supports:

```powershell
.\scripts\run.cmd --version
.\build\msvc-debug\bin\Debug\system_monitoring_agent.exe --help
.\build\msvc-debug\bin\Debug\system_monitoring_agent.exe --version
```

## Debug

Open the folder in VS Code and use the `Debug agent (MSVC)` launch
configuration. It builds the Debug preset before starting the executable.

## Test

```powershell
ctest --preset windows-msvc-debug
```
