#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <string_view>
#include <thread>

#include "system_monitoring_agent/version.hpp"

namespace
{

std::atomic_bool stop_requested{false};

void handle_signal(int)
{
    stop_requested.store(true);
}

void print_help(std::string_view executable_name)
{
    std::cout << "Usage: " << executable_name << " [--help] [--version]\n"
              << "\n"
              << "Runs the system monitoring agent scaffold until Ctrl+C.\n";
}

void print_version()
{
    std::cout << SYSTEM_MONITORING_AGENT_NAME << " " << SYSTEM_MONITORING_AGENT_VERSION << "\n";
}

} // namespace

int main(int argc, char* argv[])
{
    const std::string_view executable_name = argc > 0 ? argv[0] : "system_monitoring_agent";

    if (argc > 2)
    {
        std::cerr << "Too many arguments.\n";
        print_help(executable_name);
        return 2;
    }

    if (argc == 2)
    {
        const std::string_view argument = argv[1];
        if (argument == "--help" || argument == "-h")
        {
            print_help(executable_name);
            return 0;
        }
        if (argument == "--version" || argument == "-v")
        {
            print_version();
            return 0;
        }

        std::cerr << "Unknown argument: " << argument << "\n";
        print_help(executable_name);
        return 2;
    }

    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    std::cout << SYSTEM_MONITORING_AGENT_NAME << " " << SYSTEM_MONITORING_AGENT_VERSION
              << " started. Press Ctrl+C to stop.\n";

    using namespace std::chrono_literals;
    while (!stop_requested.load())
    {
        std::this_thread::sleep_for(250ms);
    }

    std::cout << "Shutdown requested. Exiting cleanly.\n";
    return 0;
}
