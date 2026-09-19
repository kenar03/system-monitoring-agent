#include "HttpClient.hpp" // for monitoring::HttpClient

#include <httplib.h> // for httplib::Server

#include <iostream> // for std::cout, std::cerr
#include <string>   // for std::string
#include <thread>   // for std::thread

int main()
{
    // Подготовка: локальный HTTP-сервер и ожидаемое тело запроса.
    httplib::Server server;

    const std::string expectedBody =
        R"({"message":"test"})";

    bool requestReceived = false;
    bool bodyCorrect = false;
    bool contentTypeCorrect = false;

    server.Post(
        "/",
        [&](const httplib::Request& request,
            httplib::Response& response)
        {
            requestReceived = true;

            bodyCorrect =
                request.body == expectedBody;

            contentTypeCorrect =
                request.get_header_value("Content-Type") ==
                "application/json";

            response.status = 200;
        });

    const int port =
        server.bind_to_any_port("127.0.0.1");

    if (port <= 0)
    {
        std::cerr << "Failed to bind test server.\n";
        return 1;
    }

    std::thread serverThread(
        [&server]()
        {
            server.listen_after_bind();
        });

    // Действие: отправка JSON через тестируемый HttpClient.
    monitoring::HttpClient client;

    const std::string endpoint =
        "http://127.0.0.1:" + std::to_string(port);

    const bool result =
        client.PostJson(endpoint, expectedBody);

    server.stop();
    serverThread.join();

    // Проверка: запрос должен быть успешно отправлен и принят сервером.
    if (!result)
    {
        std::cerr << "PostJson returned false.\n";
        return 1;
    }

    if (!requestReceived)
    {
        std::cerr << "Server did not receive POST request.\n";
        return 1;
    }

    if (!bodyCorrect)
    {
        std::cerr << "Request body is incorrect.\n";
        return 1;
    }

    if (!contentTypeCorrect)
    {
        std::cerr << "Content-Type is incorrect.\n";
        return 1;
    }

    std::cout << "HttpClient smoke test passed.\n";

    return 0;
}