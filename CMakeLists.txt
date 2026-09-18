cmake_minimum_required(VERSION 3.20)

project(SystemMonitoringAgent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Production code used by both the application and tests.
add_library(monitoring_core
    src/WindowsActivityProvider.cpp
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