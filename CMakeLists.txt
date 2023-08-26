cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_CXX_FLAGS -DUNSAFE_READ)
include_directories(include)

set(CMAKE_BUILD_TYPE Debug)

# set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_CXX_FLAGS -O3)
project(emulator)

add_executable(emulate main.cpp printer.cpp utils.cpp cpu.cpp cpu_breaker.cpp)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.0.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests test.cpp utils.cpp printer.cpp cpu.cpp cpu_breaker.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
