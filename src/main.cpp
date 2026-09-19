#include "Agent.hpp" // for monitoring::Agent

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // for SetConsoleCtrlHandler

#include <atomic>   // for std::atomic_bool
#include <chrono>   // for std::chrono::milliseconds
#include <csignal>  // for SIGINT, SIGTERM, std::signal, std::sig_atomic_t
#include <iostream> // for std::cout, std::cerr
#include <string>   // for std::string
#include <string_view> // for std::string_view
#include <thread>   // for std::this_thread::sleep_for

namespace
{
    constexpr std::string_view kDefaultEndpoint = "http://localhost:8080";

    std::atomic_bool gConsoleStopRequested{false};
    volatile std::sig_atomic_t gSignalStopRequested = 0;

    void PrintUsage(std::string_view executableName)
    {
        std::cout
            << "Usage: " << executableName << " [--endpoint URL] [--help] [--version]\n"
            << "\n"
            << "Runs the system monitoring agent until Ctrl+C.\n";
    }

    BOOL WINAPI ConsoleHandler(DWORD signal)
    {
        if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT || signal == CTRL_CLOSE_EVENT)
        {
            gConsoleStopRequested.store(true);
            return TRUE;
        }

        return FALSE;
    }

    void SignalHandler(int)
    {
        gSignalStopRequested = 1;
    }
} // namespace

int main(int argc, char* argv[])
{
    std::string endpoint{kDefaultEndpoint};
    const std::string_view executableName = argc > 0 ? argv[0] : "SystemMonitoringAgent";

    for (int index = 1; index < argc; ++index)
    {
        const std::string_view argument = argv[index];

        if (argument == "--help" || argument == "-h")
        {
            PrintUsage(executableName);
            return 0;
        }

        if (argument == "--version" || argument == "-v")
        {
            std::cout << "SystemMonitoringAgent 0.1.0\n";
            return 0;
        }

        if (argument == "--endpoint")
        {
            if (index + 1 >= argc)
            {
                std::cerr << "--endpoint requires a URL.\n";
                return 2;
            }

            endpoint = argv[++index];
            continue;
        }

        std::cerr << "Unknown argument: " << argument << '\n';
        PrintUsage(executableName);
        return 2;
    }

    if (SetConsoleCtrlHandler(ConsoleHandler, TRUE) == FALSE)
    {
        std::cerr << "Failed to register console handler.\n";
        return 1;
    }

    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    monitoring::Agent agent(endpoint);
    agent.Start();

    std::cout << "System Monitoring Agent started. Press Ctrl+C to stop.\n";

    while (!gConsoleStopRequested.load() && gSignalStopRequested == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{100});
    }

    std::cout << "Stopping agent...\n";

    agent.Stop();

    std::cout << "Agent stopped.\n";
    return 0;
}
