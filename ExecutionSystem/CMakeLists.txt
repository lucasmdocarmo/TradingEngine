cmake_minimum_required(VERSION 3.15)
project(HighFrequencyExecutionSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Dependencies
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

# Check for nlohmann_json
find_package(nlohmann_json QUIET)
if (NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, attempting to use FetchContent...")
    include(FetchContent)
    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
    FetchContent_MakeAvailable(json)
endif()

# Includes
include_directories(include)

# Executable
add_executable(execution_engine src/main.cpp)

target_link_libraries(execution_engine PRIVATE 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    Boost::system 
    Boost::thread
    nlohmann_json::nlohmann_json
)

# Compiler warnings
if(MSVC)
    target_compile_options(execution_engine PRIVATE /W4)
else()
    target_compile_options(execution_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()
