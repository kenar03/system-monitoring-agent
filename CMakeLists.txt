cmake_minimum_required(VERSION 3.24)

project(
    system_monitoring_agent
    VERSION 0.1.0
    DESCRIPTION "Prototype system monitoring agent"
    LANGUAGES CXX
)

option(AGENT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/system_monitoring_agent/version.hpp"
    @ONLY
)

add_executable(system_monitoring_agent
    src/main.cpp
)

target_include_directories(system_monitoring_agent
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

target_compile_features(system_monitoring_agent PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(system_monitoring_agent PRIVATE /W4 /permissive- /utf-8)
    if(AGENT_WARNINGS_AS_ERRORS)
        target_compile_options(system_monitoring_agent PRIVATE /WX)
    endif()
else()
    target_compile_options(system_monitoring_agent PRIVATE -Wall -Wextra -Wpedantic)
    if(AGENT_WARNINGS_AS_ERRORS)
        target_compile_options(system_monitoring_agent PRIVATE -Werror)
    endif()
endif()

set_property(TARGET system_monitoring_agent PROPERTY VS_DEBUGGER_WORKING_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

include(CTest)
if(BUILD_TESTING)
    add_test(
        NAME agent_prints_version
        COMMAND system_monitoring_agent --version
    )
endif()
