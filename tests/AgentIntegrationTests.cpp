#include "Agent.hpp" // for monitoring::Agent

#include <httplib.h>      // for httplib::Server
#include <nlohmann/json.hpp> // for nlohmann::json

#include <chrono>             // for std::chrono
#include <condition_variable> // for std::condition_variable
#include <filesystem>         // for std::filesystem
#include <iostream>           // for std::cout, std::cerr
#include <mutex>              // for std::mutex
#include <stdexcept>          // for std::runtime_error
#include <string>             // for std::string
#include <thread>             // for std::thread
#include <vector>             // for std::vector
#include <fstream>            // for std::ifstream

namespace
{
    void Require(bool condition, const std::string& message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    void TestAgentSendsMetrics()
    {
        std::filesystem::remove("backup.json");

        httplib::Server server;
        std::mutex mutex;
        std::condition_variable receivedCondition;
        std::string receivedBody;
        bool received{false};

        server.Post("/", [&](const httplib::Request& request, httplib::Response& response)
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                receivedBody = request.body;
                received = true;
            }

            response.status = 200;
            response.set_content("OK", "text/plain");
            receivedCondition.notify_one();
        });

        const int port = server.bind_to_any_port("127.0.0.1");
        Require(port > 0, "Failed to bind test HTTP server.");

        std::thread serverThread([&server] { server.listen_after_bind(); });

        monitoring::Agent agent("http://127.0.0.1:" + std::to_string(port));
        agent.Start();

        {
            std::unique_lock<std::mutex> lock(mutex);
            const bool requestReceived = receivedCondition.wait_for(lock, std::chrono::seconds{40}, [&received] { return received; });
            Require(requestReceived, "Agent did not send metrics within 40 seconds.");
        }

        agent.Stop();
        server.stop();

        if (serverThread.joinable())
        {
            serverThread.join();
        }

        const auto json = nlohmann::json::parse(receivedBody);

        Require(json.contains("agent_id"), "JSON does not contain agent_id.");
        Require(json["agent_id"].is_string(), "agent_id must be a string.");
        Require(!json["agent_id"].get<std::string>().empty(), "agent_id must not be empty.");

        Require(json.contains("timestamp"), "JSON does not contain timestamp.");
        Require(json["timestamp"].is_number_integer(), "timestamp must be an integer.");

        Require(json.contains("payload"), "JSON does not contain payload.");
        Require(json["payload"].is_array(), "payload must be an array.");
        Require(!json["payload"].empty(), "payload must contain metrics.");
        Require(json["payload"].size() <= 10, "payload must contain no more than 10 metrics.");

        for (const auto& metric : json["payload"])
        {
            Require(metric.contains("time"), "Metric does not contain time.");
            Require(metric.contains("process_name"), "Metric does not contain process_name.");
            Require(metric.contains("window_title"), "Metric does not contain window_title.");
            Require(metric.contains("user_active"), "Metric does not contain user_active.");

            Require(metric["time"].is_string(), "time must be a string.");
            Require(metric["process_name"].is_string(), "process_name must be a string.");
            Require(metric["window_title"].is_string(), "window_title must be a string.");
            Require(metric["user_active"].is_boolean(), "user_active must be a boolean.");
        }

        std::filesystem::remove("backup.json");
    }

    void TestAgentRetriesAfterServerError()
    {
        std::filesystem::remove("backup.json");

        httplib::Server server;
        std::mutex mutex;
        std::condition_variable receivedCondition;
        std::vector<std::string> receivedBodies;
        int requestCount{0};

        server.Post("/", [&](const httplib::Request& request, httplib::Response& response)
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                receivedBodies.push_back(request.body);
                ++requestCount;
            }

            response.status = requestCount == 1 ? 500 : 200;
            response.set_content(requestCount == 1 ? "Error" : "OK", "text/plain");
            receivedCondition.notify_one();
        });

        const int port = server.bind_to_any_port("127.0.0.1");
        Require(port > 0, "Failed to bind test HTTP server.");

        std::thread serverThread([&server] { server.listen_after_bind(); });

        monitoring::Agent agent("http://127.0.0.1:" + std::to_string(port));
        agent.Start();

        {
            std::unique_lock<std::mutex> lock(mutex);
            const bool retried = receivedCondition.wait_for(lock, std::chrono::seconds{70}, [&requestCount] { return requestCount >= 2; });
            Require(retried, "Agent did not retry after failed request.");
        }

        agent.Stop();
        server.stop();

        if (serverThread.joinable())
        {
            serverThread.join();
        }

        Require(receivedBodies.size() >= 2, "Expected at least two requests.");

        const auto firstJson = nlohmann::json::parse(receivedBodies[0]);
        const auto secondJson = nlohmann::json::parse(receivedBodies[1]);

        const auto& firstPayload = firstJson["payload"];
        const auto& secondPayload = secondJson["payload"];

        Require(!firstPayload.empty(), "First payload must not be empty.");
        Require(secondPayload.size() >= firstPayload.size(), "Retried payload lost metrics.");

        for (std::size_t i = 0; i < firstPayload.size(); ++i)
        {
            Require(firstPayload[i] == secondPayload[i], "Failed metrics were not restored correctly.");
        }

        std::filesystem::remove("backup.json");
    }

    void TestAgentSavesBackupOnStop()
    {
        std::filesystem::remove("backup.json");

        monitoring::Agent agent("http://127.0.0.1:1");
        agent.Start();

        std::this_thread::sleep_for(std::chrono::seconds{6});
        agent.Stop();

        Require(std::filesystem::exists("backup.json"), "backup.json was not created.");

        std::ifstream file("backup.json");
        Require(file.is_open(), "Failed to open backup.json.");

        nlohmann::json json;
        file >> json;
        file.close();

        Require(json.contains("agent_id"), "Backup does not contain agent_id.");
        Require(json.contains("timestamp"), "Backup does not contain timestamp.");
        Require(json.contains("payload"), "Backup does not contain payload.");
        Require(json["payload"].is_array(), "Backup payload must be an array.");
        Require(!json["payload"].empty(), "Backup payload must contain unsent metrics.");

        std::filesystem::remove("backup.json");
    }
} // namespace

int main()
{
    try
    {
        TestAgentSendsMetrics();
        std::cout << "[PASS] Agent sends metrics\n";

        TestAgentRetriesAfterServerError();
        std::cout << "[PASS] Agent retries after server error\n";

        TestAgentSavesBackupOnStop();
        std::cout << "[PASS] Agent saves backup on stop\n";

        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "[FAIL] " << exception.what() << '\n';
        return 1;
    }
}