#include "WindowsActivityProvider.hpp" // for monitoring::WindowsActivityProvider

#include <chrono>   // for std::chrono::seconds
#include <iostream> // for std::cout
#include <thread>   // for std::this_thread::sleep_for
#include <Windows.h> // for SetConsoleOutputCP, CP_UTF8

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    monitoring::WindowsActivityProvider provider;

    std::cout << "Switch between windows now.\n";

    for (int i = 0; i < 10; ++i)
    {
        std::this_thread::sleep_for(
            std::chrono::seconds{1});

        std::cout
            << "Process: " << provider.GetProcessName()
            << " | Title: " << provider.GetWindowTitle()
            << '\n';
    }

    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}