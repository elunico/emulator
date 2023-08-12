cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

set(CMAKE_BUILD_TYPE Debug)

project(emulator)

add_executable(emulate emulate.cpp)
