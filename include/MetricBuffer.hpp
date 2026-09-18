#ifndef SYSTEM_MONITORING_AGENT_METRIC_BUFFER_HPP
#define SYSTEM_MONITORING_AGENT_METRIC_BUFFER_HPP

#include "Metric.hpp"         // for monitoring::Metric

#include <chrono>             // for std::chrono::seconds
#include <condition_variable> // for std::condition_variable
#include <cstddef>            // for std::size_t
#include <deque>              // for std::deque
#include <mutex>              // for std::mutex
#include <vector>             // for std::vector

namespace monitoring
{
    class MetricBuffer
    {
    public:
        void Push(const Metric& metric);
        bool WaitForData(std::size_t threshold, std::chrono::seconds timeout);
        std::vector<Metric> TakeBatch(std::size_t maxCount);
        void RestoreFront(std::vector<Metric>&& metrics);
        std::vector<Metric> GetAll();
        void NotifyAll();
        std::size_t Size();

    private:
        void TrimToCapacity();
        static constexpr std::size_t kMaxBufferSize{100};
        std::deque<Metric> mQueue;
        std::mutex mMutex;
        std::condition_variable mChanged;
    };
} // namespace monitoring

#endif // SYSTEM_MONITORING_AGENT_METRIC_BUFFER_HPP