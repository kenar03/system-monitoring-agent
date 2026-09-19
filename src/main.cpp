#include "Agent.hpp" // for monitoring::Agent

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // for SetConsoleCtrlHandler

#include <atomic>   // for std::atomic_bool
#include <chrono>   // for std::chrono::milliseconds
#include <csignal>  // for SIGINT, SIGTERM, std::signal, std::sig_atomic_t
#include <iostream> // for std::cout, std::cerr
#include <thread>   // for std::this_thread::sleep_for

namespace
{
    std::atomic_bool gConsoleStopRequested{false};
    volatile std::sig_atomic_t gSignalStopRequested = 0;

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

int main()
{
    if (SetConsoleCtrlHandler(ConsoleHandler, TRUE) == FALSE)
    {
        std::cerr << "Failed to register console handler.\n";
        return 1;
    }

    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    monitoring::Agent agent("http://localhost:8080");
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