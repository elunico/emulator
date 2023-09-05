cmake_minimum_required(VERSION 3.14.5)

set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_CXX_FLAGS -DUNSAFE_READ)
include_directories(include)

#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS "-Wpedantic -Wall -Wextra -g")

 set(CMAKE_BUILD_TYPE Release)
 set(CMAKE_CXX_FLAGS "-Wpedantic -Wall -Wextra -O3")
project(emulator)

add_executable(emulate src/main.cpp src/printer.cpp src/utils.cpp src/cpu.cpp src/cpu_breaker.cpp)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.0.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests test/test.cpp src/utils.cpp src/printer.cpp src/cpu.cpp src/cpu_breaker.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
