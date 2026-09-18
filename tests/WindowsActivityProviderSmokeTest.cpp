#include "WindowsActivityProvider.hpp" // for monitoring::WindowsActivityProvider

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // for MultiByteToWideChar, CP_UTF8, MB_ERR_INVALID_CHARS

#include <chrono>   // for std::chrono::seconds
#include <iostream> // for std::cout, std::cerr
#include <string>   // for std::string


namespace
{

bool IsValidUtf8(const std::string& value)
{
    if (value.empty())
    {
        return true;
    }

    const int characterCount = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0);

    return characterCount > 0;
}

} // namespace


int main()
{
    monitoring::WindowsActivityProvider provider;

    bool failed = false;

    const std::string processName = provider.GetProcessName();

    if (processName.empty())
    {
        std::cerr << "[FAIL] Process name is empty.\n";

        failed = true;
    }
    else
    {
        std::cout << "[PASS] Process name was retrieved.\n";
    }

    if (processName.find('\\') != std::string::npos ||
        processName.find('/') != std::string::npos)
    {
        std::cerr << "[FAIL] Process name contains a path.\n";

        failed = true;
    }
    else
    {
        std::cout << "[PASS] Process name contains only the file name.\n";
    }

    const std::string windowTitle = provider.GetWindowTitle();

    if (windowTitle.empty())
    {
        std::cerr << "[FAIL] Window title is empty.\n";

        failed = true;
    }
    else
    {
        std::cout << "[PASS] Window title was retrieved.\n";
    }

    if (!IsValidUtf8(windowTitle))
    {
        std::cerr << "[FAIL] Window title is not valid UTF-8.\n";

        failed = true;
    }
    else
    {
        std::cout << "[PASS] Window title is valid UTF-8.\n";
    }

    const bool userActive = provider.WasUserActive(std::chrono::seconds{60});

    if (!userActive)
    {
        std::cerr << "[FAIL] Recent user activity was not detected.\n";

        failed = true;
    }
    else
    {
        std::cout << "[PASS] Recent user activity was detected.\n";
    }

    return failed ? 1 : 0;
}