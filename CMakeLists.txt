cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

set(CMAKE_BUILD_TYPE Debug)

# set(CMAKE_CXX_FLAGS -O3)

project(emulator)

add_executable(emulate emulate.cpp printer.cpp)
