#include "MetricBuffer.hpp" // for monitoring::MetricBuffer

#include <mutex>   // for std::lock_guard
#include <utility> // for std::move

namespace monitoring
{
    void MetricBuffer::Push(const Metric& metric)
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);

            mQueue.push_back(metric);
            TrimToCapacity();
        }

        mChanged.notify_one();
    }

    void MetricBuffer::TrimToCapacity()
    {
        while (mQueue.size() > kMaxBufferSize)
        {
            mQueue.pop_front();
        }
    }

    std::size_t MetricBuffer::Size()
    {
        std::lock_guard<std::mutex> lock(mMutex);

        return mQueue.size();
    }

    std::vector<Metric> MetricBuffer::GetAll()
    {
        std::lock_guard<std::mutex> lock(mMutex);

        return std::vector<Metric>(mQueue.begin(), mQueue.end());
    }

    std::vector<Metric> MetricBuffer::TakeBatch(std::size_t maxCount)
    {
        std::lock_guard<std::mutex> lock(mMutex);

        std::vector<Metric> batch;
        batch.reserve(maxCount);

        while (!mQueue.empty() && batch.size() < maxCount)
        {
            batch.push_back(std::move(mQueue.front()));
            mQueue.pop_front();
        }

        return batch;
    }

    void MetricBuffer::RestoreFront(std::vector<Metric>&& metrics)
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);

            for (auto iterator = metrics.rbegin(); iterator != metrics.rend(); ++iterator)
            {
                mQueue.push_front(std::move(*iterator));
            }

            TrimToCapacity();
        }

        mChanged.notify_one();
    }

    void MetricBuffer::NotifyAll()
    {
        mChanged.notify_all();
    }

    bool MetricBuffer::WaitForData(std::size_t threshold, std::chrono::seconds timeout)
    {
        std::unique_lock<std::mutex> lock(mMutex);

        return mChanged.wait_for(lock, timeout, [this, threshold] { return mQueue.size() >= threshold; });
    }
} // namespace monitoring