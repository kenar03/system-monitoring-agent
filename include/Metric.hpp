#ifndef SYSTEM_MONITORING_AGENT_METRIC_HPP
#define SYSTEM_MONITORING_AGENT_METRIC_HPP

#include <chrono> // for std::chrono::system_clock::time_point
#include <string> // for std::string

namespace monitoring
{

struct Metric
{
    std::chrono::system_clock::time_point mTime{};
    std::string mProcessName;
    std::string mWindowTitle;
    bool mUserActive{false};
};

} // namespace monitoring

#endif // SYSTEM_MONITORING_AGENT_METRIC_HPP