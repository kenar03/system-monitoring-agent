cmake_minimum_required(VERSION 3.20)

project(SystemMonitoringAgent LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.56.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(
    json
    httplib
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Production code used by both the application and tests.
add_library(monitoring_core
    src/WindowsActivityProvider.cpp
    src/MetricBuffer.cpp
    src/HttpClient.cpp
    src/JsonSerializer.cpp
)

target_include_directories(monitoring_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(WIN32)
    target_link_libraries(monitoring_core
        PUBLIC
            user32
    )
endif()

# Main application.
add_executable(SystemMonitoringAgent
    src/main.cpp
)

target_link_libraries(SystemMonitoringAgent
    PRIVATE
        monitoring_core
)

target_link_libraries(monitoring_core
    PRIVATE
        nlohmann_json::nlohmann_json
        httplib::httplib
)

# Tests.
enable_testing()

add_executable(WindowsActivityProviderSmokeTest
    tests/WindowsActivityProviderSmokeTest.cpp
)

target_link_libraries(WindowsActivityProviderSmokeTest
    PRIVATE
        monitoring_core
)

add_test(
    NAME WindowsActivityProviderSmokeTest
    COMMAND WindowsActivityProviderSmokeTest
)

set_tests_properties(
    WindowsActivityProviderSmokeTest
    PROPERTIES
        LABELS "smoke"
)

add_executable(MetricBufferTests
    tests/MetricBufferTests.cpp
)

target_link_libraries(MetricBufferTests
    PRIVATE
        monitoring_core
)

add_test(
    NAME MetricBufferTests
    COMMAND MetricBufferTests
)

set_tests_properties(
    MetricBufferTests
    PROPERTIES
        LABELS "unit"
)

add_executable(HttpClientSmokeTest
    tests/HttpClientSmokeTest.cpp
)

target_link_libraries(HttpClientSmokeTest
    PRIVATE
        monitoring_core
        httplib::httplib
)

add_test(
    NAME HttpClientSmokeTest
    COMMAND HttpClientSmokeTest
)

set_tests_properties(
    HttpClientSmokeTest
    PROPERTIES
        LABELS "smoke"
)

add_executable(JsonSerializerTests
    tests/JsonSerializerTests.cpp
)

target_link_libraries(JsonSerializerTests
    PRIVATE
        monitoring_core
        nlohmann_json::nlohmann_json
)

add_test(
    NAME JsonSerializerTests
    COMMAND JsonSerializerTests
)

set_tests_properties(
    JsonSerializerTests
    PROPERTIES
        LABELS "unit"
)