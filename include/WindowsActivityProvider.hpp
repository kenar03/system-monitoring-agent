#ifndef SYSTEM_MONITORING_AGENT_WINDOWS_ACTIVITY_PROVIDER_HPP
#define SYSTEM_MONITORING_AGENT_WINDOWS_ACTIVITY_PROVIDER_HPP

#include <chrono> // for std::chrono::seconds
#include <string> // for std::string

namespace monitoring
{
    class WindowsActivityProvider
    {
    public:
        std::string GetProcessName() const;
        std::string GetWindowTitle() const;
        bool WasUserActive(std::chrono::seconds interval) const;
    };
} // namespace monitoring

#endif // SYSTEM_MONITORING_AGENT_WINDOWS_ACTIVITY_PROVIDER_HPP