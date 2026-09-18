#include "MetricBuffer.hpp" // for monitoring::MetricBuffer, monitoring::Metric

#include <chrono>    // for std::chrono
#include <iostream>  // for std::cout, std::cerr
#include <stdexcept> // for std::runtime_error
#include <string>    // for std::string, std::to_string
#include <thread>    // for std::thread, std::this_thread::sleep_for
#include <utility>   // for std::move
#include <vector>    // for std::vector

namespace
{

monitoring::Metric CreateMetric(int id)
{
    monitoring::Metric metric;

    metric.mTime = std::chrono::system_clock::now();
    metric.mProcessName = "process_" + std::to_string(id);
    metric.mWindowTitle = "window_" + std::to_string(id);
    metric.mUserActive = true;

    return metric;
}

void Require(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void TestPushAndSize()
{
    // Подготовка: пустой буфер и одна тестовая метрика.
    monitoring::MetricBuffer buffer;
    const monitoring::Metric metric = CreateMetric(1);

    // Действие: добавление метрики в буфер.
    buffer.Push(metric);

    // Проверка: размер буфера должен увеличиться до одной записи.
    Require(
        buffer.Size() == 1,
        "Push must increase buffer size.");
}

void TestTakeBatch()
{
    // Подготовка: три метрики, добавленные в известном порядке.
    monitoring::MetricBuffer buffer;

    buffer.Push(CreateMetric(1));
    buffer.Push(CreateMetric(2));
    buffer.Push(CreateMetric(3));

    // Действие: извлечение двух самых старых метрик.
    const std::vector<monitoring::Metric> batch =
        buffer.TakeBatch(2);

    // Проверка: количество, удаление из буфера и сохранение порядка.
    Require(
        batch.size() == 2,
        "TakeBatch must return requested number of metrics.");

    Require(
        buffer.Size() == 1,
        "Taken metrics must be removed from buffer.");

    Require(
        batch[0].mProcessName == "process_1",
        "First metric has incorrect order.");

    Require(
        batch[1].mProcessName == "process_2",
        "Second metric has incorrect order.");
}

void TestRestoreFront()
{
    // Подготовка: в буфере находятся метрики 4 и 5,
    // отдельно подготовлена ранее извлечённая пачка 1, 2, 3.
    monitoring::MetricBuffer buffer;

    buffer.Push(CreateMetric(4));
    buffer.Push(CreateMetric(5));

    std::vector<monitoring::Metric> metrics;
    metrics.push_back(CreateMetric(1));
    metrics.push_back(CreateMetric(2));
    metrics.push_back(CreateMetric(3));

    // Действие: возврат пачки в начало буфера.
    buffer.RestoreFront(std::move(metrics));

    // Проверка: итоговый порядок должен быть 1, 2, 3, 4, 5.
    const std::vector<monitoring::Metric> allMetrics =
        buffer.GetAll();

    Require(
        allMetrics.size() == 5,
        "RestoreFront must restore all metrics.");

    for (std::size_t i = 0; i < allMetrics.size(); ++i)
    {
        const std::string expected =
            "process_" + std::to_string(i + 1);

        Require(
            allMetrics[i].mProcessName == expected,
            "RestoreFront changed metric order.");
    }
}

void TestCapacity()
{
    // Подготовка: пустой буфер.
    monitoring::MetricBuffer buffer;

    // Действие: добавление 101 метрики при максимальной ёмкости 100.
    for (int i = 0; i <= 100; ++i)
    {
        buffer.Push(CreateMetric(i));
    }

    // Проверка: размер ограничен 100 записями,
    // самая старая запись удалена, самая новая сохранена.
    Require(
        buffer.Size() == 100,
        "Buffer size must not exceed 100.");

    const std::vector<monitoring::Metric> metrics =
        buffer.GetAll();

    Require(
        metrics.front().mProcessName == "process_1",
        "Oldest metric must be removed when capacity is exceeded.");

    Require(
        metrics.back().mProcessName == "process_100",
        "Newest metric must remain in buffer.");
}

void TestWaitForDataThresholdReached()
{
    // Подготовка: буфер с двумя метриками.
    monitoring::MetricBuffer buffer;

    buffer.Push(CreateMetric(1));
    buffer.Push(CreateMetric(2));

    // Действие: ожидание порога, который уже достигнут.
    const bool result =
        buffer.WaitForData(
            2,
            std::chrono::seconds{0});

    // Проверка: ожидание должно завершиться успешно.
    Require(
        result,
        "WaitForData must return true when threshold is reached.");
}

void TestWaitForDataTimeout()
{
    // Подготовка: пустой буфер.
    monitoring::MetricBuffer buffer;

    // Действие: ожидание одной записи с нулевым таймаутом.
    const bool result =
        buffer.WaitForData(
            1,
            std::chrono::seconds{0});

    // Проверка: при недостигнутом пороге должен быть возвращён false.
    Require(
        !result,
        "WaitForData must return false on timeout.");
}

void TestWaitForDataWakesAfterPush()
{
    // Подготовка: пустой буфер и отдельный поток,
    // ожидающий появления хотя бы одной записи.
    monitoring::MetricBuffer buffer;
    bool waitResult = false;

    std::thread waiter(
        [&buffer, &waitResult]()
        {
            waitResult = buffer.WaitForData(
                1,
                std::chrono::seconds{2});
        });

    std::this_thread::sleep_for(
        std::chrono::milliseconds{50});

    // Действие: добавление записи, достигающей требуемого порога.
    buffer.Push(CreateMetric(1));

    waiter.join();

    // Проверка: ожидающий поток должен завершить ожидание успешно.
    Require(
        waitResult,
        "WaitForData must wake after Push reaches threshold.");
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
        "Push and Size",
        TestPushAndSize);

    failedTests += !RunTest(
        "TakeBatch",
        TestTakeBatch);

    failedTests += !RunTest(
        "RestoreFront",
        TestRestoreFront);

    failedTests += !RunTest(
        "Capacity",
        TestCapacity);

    failedTests += !RunTest(
        "WaitForData threshold",
        TestWaitForDataThresholdReached);

    failedTests += !RunTest(
        "WaitForData timeout",
        TestWaitForDataTimeout);

    failedTests += !RunTest(
        "WaitForData wake after Push",
        TestWaitForDataWakesAfterPush);

    if (failedTests == 0)
    {
        std::cout << "\nAll MetricBuffer tests passed.\n";
        return 0;
    }

    std::cerr
        << "\n"
        << failedTests
        << " MetricBuffer test(s) failed.\n";

    return 1;
}
