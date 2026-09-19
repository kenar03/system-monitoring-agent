#ifndef SYSTEM_MONITORING_AGENT_AGENT_HPP
#define SYSTEM_MONITORING_AGENT_AGENT_HPP

#include "HttpClient.hpp"              // for monitoring::HttpClient
#include "JsonSerializer.hpp"          // for monitoring::JsonSerializer
#include "Metric.hpp"                  // for monitoring::Metric
#include "MetricBuffer.hpp"            // for monitoring::MetricBuffer
#include "WindowsActivityProvider.hpp" // for monitoring::WindowsActivityProvider

#include <atomic>             // for std::atomic_bool
#include <chrono>             // for std::chrono::seconds
#include <condition_variable> // for std::condition_variable
#include <cstddef>            // for std::size_t
#include <mutex>              // for std::mutex
#include <string>             // for std::string
#include <thread>             // for std::thread

namespace monitoring
{
    class Agent
    {
    public:
        explicit Agent(std::string endpoint);

        void Start();
        void RequestStop();
        void Stop();

    private:
        void CollectorLoop();
        void SenderLoop();

        Metric CollectMetric() const;
        void SaveBackup();

        static constexpr std::chrono::seconds kCollectionInterval{5};
        static constexpr std::chrono::seconds kSendInterval{30};
        static constexpr std::size_t kBatchSize{10};

        WindowsActivityProvider mActivityProvider;
        MetricBuffer mBuffer;
        JsonSerializer mJsonSerializer;
        HttpClient mHttpClient;

        std::thread mCollectorThread;
        std::thread mSenderThread;

        std::atomic_bool mStopRequested{false};
        std::mutex mStopMutex;
        std::condition_variable mStopCondition;

        std::string mAgentId;
        std::string mEndpoint;
    };
} // namespace monitoring

#endif // SYSTEM_MONITORING_AGENT_AGENT_HPP
