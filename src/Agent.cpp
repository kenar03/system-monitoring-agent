#include "Agent.hpp" // for monitoring::Agent

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // for GetComputerNameA, MAX_COMPUTERNAME_LENGTH

#include <chrono>  // for std::chrono::steady_clock
#include <mutex>   // for std::unique_lock
#include <utility> // for std::move
#include <fstream> // for std::ofstream
#include <iostream> // for std::cerr

namespace monitoring
{

namespace
{
    std::string GetAgentId()
    {
        char computerName[MAX_COMPUTERNAME_LENGTH + 1]{};
        DWORD size = static_cast<DWORD>(sizeof(computerName));

        if (GetComputerNameA(computerName, &size) == FALSE)
        {
            return "unknown-agent";
        }

        return std::string(computerName, size);
    }

    } // namespace

    Agent::Agent(std::string endpoint) : mAgentId(GetAgentId()), mEndpoint(std::move(endpoint))
    {
    }

    Metric Agent::CollectMetric() const
    {
        Metric metric;
        metric.mTime = std::chrono::system_clock::now();
        metric.mProcessName = mActivityProvider.GetProcessName();
        metric.mWindowTitle = mActivityProvider.GetWindowTitle();
        metric.mUserActive = mActivityProvider.WasUserActive(kCollectionInterval);
        return metric;
    }

    void Agent::CollectorLoop()
    {
        auto nextCollectionTime = std::chrono::steady_clock::now();

        while (!mStopRequested.load())
        {
            mBuffer.Push(CollectMetric());
            nextCollectionTime += kCollectionInterval;

            std::unique_lock<std::mutex> lock(mStopMutex);
            mStopCondition.wait_until(lock, nextCollectionTime, [this] { return mStopRequested.load(); });
        }
    }

    void Agent::Start()
    {
        mStopRequested.store(false);
        mCollectorThread = std::thread(&Agent::CollectorLoop, this);
        mSenderThread = std::thread(&Agent::SenderLoop, this);
    }

    void Agent::RequestStop()
    {
        mStopRequested.store(true);
        mBuffer.NotifyAll();
        mStopCondition.notify_all();
    }

    void Agent::Stop()
    {
        RequestStop();

        if (mCollectorThread.joinable())
        {
            mCollectorThread.join();
        }

        if (mSenderThread.joinable())
        {
            mSenderThread.join();
        }

        SaveBackup();
    }

    void Agent::SenderLoop()
    {
        bool retryPending = false;

        while (!mStopRequested.load())
        {
            if (retryPending)
            {
                std::unique_lock<std::mutex> lock(mStopMutex);
                mStopCondition.wait_for(lock, kSendInterval, [this] { return mStopRequested.load(); });
            }
            else
            {
                mBuffer.WaitForData(kBatchSize, kSendInterval, mStopRequested);
            }

            if (mStopRequested.load())
            {
                break;
            }

            auto batch = mBuffer.TakeBatch(kBatchSize);

            if (batch.empty())
            {
                retryPending = false;
                continue;
            }

            if (mStopRequested.load())
            {
                mBuffer.RestoreFront(std::move(batch));
                break;
            }

            const std::string json = mJsonSerializer.Serialize(mAgentId, batch);

            if (mStopRequested.load())
            {
                mBuffer.RestoreFront(std::move(batch));
                break;
            }

            const bool sent = mHttpClient.PostJson(mEndpoint, json);

            if (!sent)
            {
                mBuffer.RestoreFront(std::move(batch));
                retryPending = true;
            }
            else
            {
                retryPending = false;
            }
        }
    }

    void Agent::SaveBackup()
    {
        const auto metrics = mBuffer.GetAll();

        if (metrics.empty())
        {
            return;
        }

        const std::string json = mJsonSerializer.Serialize(mAgentId, metrics);

        std::ofstream file("backup.json", std::ios::trunc);

        if (!file)
        {
            std::cerr << "Failed to open backup.json.\n";
            return;
        }

        file << json;
    }
} // namespace monitoring
