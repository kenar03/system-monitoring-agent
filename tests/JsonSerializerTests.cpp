#include "JsonSerializer.hpp" // for monitoring::JsonSerializer

#include <nlohmann/json.hpp> // for nlohmann::json

#include <chrono>    // for std::chrono
#include <iostream>  // for std::cout, std::cerr
#include <stdexcept> // for std::runtime_error
#include <string>    // for std::string
#include <vector>    // for std::vector

namespace
{

void Require(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

monitoring::Metric CreateMetric(
    const std::string& processName,
    const std::string& windowTitle,
    bool userActive)
{
    monitoring::Metric metric;

    metric.mTime = std::chrono::system_clock::now();
    metric.mProcessName = processName;
    metric.mWindowTitle = windowTitle;
    metric.mUserActive = userActive;

    return metric;
}

void TestSerializeStructure()
{
    // Подготовка: сериализатор, идентификатор агента и одна тестовая метрика.
    monitoring::JsonSerializer serializer;

    const std::string agentId = "TEST-PC";

    const std::vector<monitoring::Metric> metrics
    {
        CreateMetric(
            "browser.exe",
            "Test window",
            true)
    };

    // Фиксация допустимой нижней границы timestamp.
    const auto timestampBefore =
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();

    // Действие: сериализация пакета и обратный разбор полученной JSON-строки.
    const std::string jsonString =
        serializer.Serialize(agentId, metrics);

    const auto timestampAfter =
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();

    const nlohmann::json json =
        nlohmann::json::parse(jsonString);

    // Проверка: наличие обязательных полей верхнего уровня.
    Require(
        json.contains("agent_id"),
        "JSON must contain agent_id.");

    Require(
        json.contains("timestamp"),
        "JSON must contain timestamp.");

    Require(
        json.contains("payload"),
        "JSON must contain payload.");

    // Проверка: корректность идентификатора агента.
    Require(
        json["agent_id"] == agentId,
        "agent_id has incorrect value.");

    // Проверка: timestamp должен быть целочисленным Unix-временем.
    Require(
        json["timestamp"].is_number_integer(),
        "timestamp must be an integer.");

    const auto timestamp =
        json["timestamp"].get<long long>();

    Require(
        timestamp >= timestampBefore &&
        timestamp <= timestampAfter,
        "timestamp has incorrect value.");

    // Проверка: payload должен быть JSON-массивом с одной записью.
    Require(
        json["payload"].is_array(),
        "payload must be an array.");

    Require(
        json["payload"].size() == 1,
        "payload must contain one metric.");
}

void TestSerializeMetricValues()
{
    // Подготовка: две метрики с различными значениями полей.
    // В строках присутствуют кавычки и обратный слеш для проверки сериализации.
    monitoring::JsonSerializer serializer;

    const std::vector<monitoring::Metric> metrics
    {
        CreateMetric(
            "first.exe",
            R"(Document "First")",
            true),

        CreateMetric(
            "second.exe",
            R"(C:\Temp\Second)",
            false)
    };

    // Действие: сериализация метрик и обратный разбор JSON.
    const std::string jsonString =
        serializer.Serialize("TEST-PC", metrics);

    const nlohmann::json json =
        nlohmann::json::parse(jsonString);

    const auto& payload =
        json["payload"];

    // Проверка: количество и исходный порядок метрик должны сохраняться.
    Require(
        payload.size() == 2,
        "payload must contain two metrics.");

    Require(
        payload[0]["process_name"] == "first.exe",
        "First process_name is incorrect.");

    Require(
        payload[1]["process_name"] == "second.exe",
        "Second process_name is incorrect.");

    // Проверка: специальные символы в заголовках окон должны сохраняться
    // после сериализации и обратного разбора JSON.
    Require(
        payload[0]["window_title"] == R"(Document "First")",
        "First window_title is incorrect.");

    Require(
        payload[1]["window_title"] == R"(C:\Temp\Second)",
        "Second window_title is incorrect.");

    // Проверка: логические значения активности должны сохранять свой тип и значение.
    Require(
        payload[0]["user_active"] == true,
        "First user_active is incorrect.");

    Require(
        payload[1]["user_active"] == false,
        "Second user_active is incorrect.");

    // Проверка: время метрики должно быть строкой формата
    // YYYY-MM-DD HH:MM:SS.
    Require(
        payload[0]["time"].is_string(),
        "Metric time must be a string.");

    const std::string time =
        payload[0]["time"].get<std::string>();

    Require(
        time.size() == 19,
        "Metric time has incorrect format.");

    Require(
        time[4] == '-' &&
        time[7] == '-' &&
        time[10] == ' ' &&
        time[13] == ':' &&
        time[16] == ':',
        "Metric time has incorrect format.");
}

void TestSerializeEmptyPayload()
{
    // Подготовка: сериализатор и пустой набор метрик.
    monitoring::JsonSerializer serializer;
    const std::vector<monitoring::Metric> metrics;

    // Действие: сериализация пакета без метрик.
    const std::string jsonString =
        serializer.Serialize("TEST-PC", metrics);

    const nlohmann::json json =
        nlohmann::json::parse(jsonString);

    // Проверка: payload должен существовать и оставаться пустым JSON-массивом.
    Require(
        json["payload"].is_array(),
        "payload must be an array.");

    Require(
        json["payload"].empty(),
        "payload must be empty.");
}

template<typename TestFunction>
bool RunTest(
    const std::string& name,
    TestFunction testFunction)
{
    try
    {
        testFunction();

        std::cout
            << "[PASS] "
            << name
            << '\n';

        return true;
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "[FAIL] "
            << name
            << ": "
            << exception.what()
            << '\n';

        return false;
    }
}

} // namespace

int main()
{
    int failedTests = 0;

    failedTests += !RunTest(
        "Serialize structure",
        TestSerializeStructure);

    failedTests += !RunTest(
        "Serialize metric values",
        TestSerializeMetricValues);

    failedTests += !RunTest(
        "Serialize empty payload",
        TestSerializeEmptyPayload);

    if (failedTests == 0)
    {
        std::cout
            << "\nAll JsonSerializer tests passed.\n";

        return 0;
    }

    std::cerr
        << "\n"
        << failedTests
        << " JsonSerializer test(s) failed.\n";

    return 1;
}