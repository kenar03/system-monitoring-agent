#include "WindowsActivityProvider.hpp" // for monitoring::WindowsActivityProvider

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // for Win32 window functions
#include <filesystem> // for std::filesystem::path
#include <string>     // for std::string, std::wstring

namespace monitoring
{
    namespace
    {
        std::string WideStringToUtf8(const std::wstring& value)
        {
            if (value.empty())
            {
                return {};
            }

            const int requiredSize = WideCharToMultiByte(
                CP_UTF8,
                0,
                value.data(),
                static_cast<int>(value.size()),
                nullptr,
                0,
                nullptr,
                nullptr);

            if (requiredSize <= 0)
            {
                return {};
            }

            std::string result(static_cast<std::size_t>(requiredSize), '\0');

            const int convertedCharacters = WideCharToMultiByte(
                CP_UTF8,
                0,
                value.data(),
                static_cast<int>(value.size()),
                result.data(),
                requiredSize,
                nullptr,
                nullptr);

            if (convertedCharacters <= 0)
            {
                return {};
            }

            return result;
        }

    } // namespace

    std::string WindowsActivityProvider::GetProcessName() const
    {
        HWND hwnd = GetForegroundWindow();

        if (hwnd == nullptr)
        {
            return {};
        }

        DWORD processId = 0;
        GetWindowThreadProcessId(hwnd, &processId);

        if (processId == 0)
        {
            return {};
        }

        HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);

        if (processHandle == nullptr)
        {
            return {};
        }

        std::wstring processPath(32768, L'\0');
        DWORD pathSize = static_cast<DWORD>(processPath.size());
        const BOOL success = QueryFullProcessImageNameW(processHandle, 0, processPath.data(), &pathSize);

        CloseHandle(processHandle);

        if (success == FALSE)
        {
            return {};
        }

        processPath.resize(static_cast<std::size_t>(pathSize));

        const std::filesystem::path path(processPath);

        return WideStringToUtf8(path.filename().wstring());
    }

    std::string WindowsActivityProvider::GetWindowTitle() const
    {
        HWND hwnd = GetForegroundWindow();

        if (hwnd == nullptr)
        {
            return {};
        }

        const int titleLength = GetWindowTextLengthW(hwnd);

        if (titleLength <= 0)
        {
            return {};
        }

        std::wstring windowTitle(static_cast<std::size_t>(titleLength) + 1, L'\0');

        const int copiedCharacters = GetWindowTextW(hwnd, windowTitle.data(), static_cast<int>(windowTitle.size()));

        if (copiedCharacters <= 0)
        {
            return {};
        }

        windowTitle.resize(static_cast<std::size_t>(copiedCharacters));

        return WideStringToUtf8(windowTitle);
    }

    bool WindowsActivityProvider::WasUserActive(std::chrono::seconds interval) const
    {
        LASTINPUTINFO lastInputInfo{};
        lastInputInfo.cbSize = sizeof(LASTINPUTINFO);

        if (GetLastInputInfo(&lastInputInfo) == FALSE)
        {
            return false;
        }

        const DWORD elapsedMilliseconds = GetTickCount() - lastInputInfo.dwTime;
        const auto intervalMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(interval);

        return elapsedMilliseconds <= intervalMilliseconds.count();
    }
} // namespace monitoring