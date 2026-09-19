cmake_minimum_required(VERSION 3.24)

project(SystemMonitoringAgent LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Declare(httplib GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git GIT_TAG v0.56.0 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(json httplib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(monitoring_core src/WindowsActivityProvider.cpp src/MetricBuffer.cpp src/HttpClient.cpp src/JsonSerializer.cpp src/Agent.cpp)
target_include_directories(monitoring_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(monitoring_core PRIVATE nlohmann_json::nlohmann_json httplib::httplib)

if(WIN32)
    target_link_libraries(monitoring_core PUBLIC user32)
endif()

add_executable(SystemMonitoringAgent src/main.cpp)
target_link_libraries(SystemMonitoringAgent PRIVATE monitoring_core)

include(CTest)

if(BUILD_TESTING)
    add_executable(WindowsActivityProviderSmokeTest tests/WindowsActivityProviderSmokeTest.cpp)
    target_link_libraries(WindowsActivityProviderSmokeTest PRIVATE monitoring_core)
    add_test(NAME WindowsActivityProviderSmokeTest COMMAND WindowsActivityProviderSmokeTest)
    set_tests_properties(WindowsActivityProviderSmokeTest PROPERTIES LABELS "smoke")

    add_executable(MetricBufferTests tests/MetricBufferTests.cpp)
    target_link_libraries(MetricBufferTests PRIVATE monitoring_core)
    add_test(NAME MetricBufferTests COMMAND MetricBufferTests)
    set_tests_properties(MetricBufferTests PROPERTIES LABELS "unit")

    add_executable(HttpClientSmokeTest tests/HttpClientSmokeTest.cpp)
    target_link_libraries(HttpClientSmokeTest PRIVATE monitoring_core httplib::httplib)
    add_test(NAME HttpClientSmokeTest COMMAND HttpClientSmokeTest)
    set_tests_properties(HttpClientSmokeTest PROPERTIES LABELS "smoke")

    add_executable(JsonSerializerTests tests/JsonSerializerTests.cpp)
    target_link_libraries(JsonSerializerTests PRIVATE monitoring_core nlohmann_json::nlohmann_json)
    add_test(NAME JsonSerializerTests COMMAND JsonSerializerTests)
    set_tests_properties(JsonSerializerTests PROPERTIES LABELS "unit")

    add_executable(AgentIntegrationTests tests/AgentIntegrationTests.cpp)
    target_link_libraries(AgentIntegrationTests PRIVATE monitoring_core httplib::httplib nlohmann_json::nlohmann_json)
    add_test(NAME AgentIntegrationTests COMMAND AgentIntegrationTests)
    set_tests_properties(AgentIntegrationTests PROPERTIES LABELS "integration" TIMEOUT 120)
endif()
