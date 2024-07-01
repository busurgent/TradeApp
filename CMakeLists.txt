cmake_minimum_required(VERSION 3.10)

project(TradingSystem)

add_executable(server server.cpp)
add_executable(client client.cpp)

enable_testing()

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(TradingServerTests test_trading.cpp)

target_link_libraries(TradingServerTests gtest gtest_main)

add_test(NAME TradingServerTests COMMAND TradingServerTests)

