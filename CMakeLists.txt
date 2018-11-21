cmake_minimum_required(VERSION 3.11)
find_package(PropWare 3.0 REQUIRED)

project(DelayTimerDisplay)

set(MODEL lmm)
create_simple_executable(DelayTimerDisplay main.cpp)
target_compile_definitions(DelayTimerDisplay PUBLIC DEBUG)
