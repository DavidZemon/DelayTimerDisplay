cmake_minimum_required(VERSION 3.3)
find_package(PropWare 3.0 REQUIRED)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")
project(DelayTimerDisplay)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(MODEL lmm)
create_simple_executable(DelayTimerDisplay main.cpp)
#target_compile_definitions(DelayTimerDisplay PUBLIC DEBUG)
