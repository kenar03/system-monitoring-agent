#include "JsonSerializer.hpp" // for monitoring::JsonSerializer

#include <nlohmann/json.hpp> // for nlohmann::json

#include <chrono>  // for std::chrono
#include <ctime>   // for std::tm, std::time_t
#include <iomanip> // for std::put_time
#include <sstream> // for std::ostringstream

namespace monitoring
{
    namespace
    {
        std::string FormatTime(const std::chrono::system_clock::time_point& timestamp)
        {
            const std::time_t timeT = std::chrono::system_clock::to_time_t(timestamp);

            std::tm localTime{};

            if (localtime_s(&localTime, &timeT) != 0)
            {
                return {};
            }

            std::ostringstream stream;

            stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");

            return stream.str();
        }
    } // namespace

    std::string JsonSerializer::Serialize(const std::string& agentId, const std::vector<Metric>& metrics) const
    {
        nlohmann::json json;

        json["agent_id"] = agentId;

        const auto now = std::chrono::system_clock::now();
        const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

        json["timestamp"] = timestamp;

        nlohmann::json payload = nlohmann::json::array();

        for (const auto& metric : metrics)
        {
            nlohmann::json metricJson;

            metricJson["time"] = FormatTime(metric.mTime);
            metricJson["process_name"] = metric.mProcessName;
            metricJson["window_title"] = metric.mWindowTitle;
            metricJson["user_active"] = metric.mUserActive;

            payload.push_back(metricJson);
        }

        json["payload"] = payload;

        return json.dump();
    }
} // namespace monitoring