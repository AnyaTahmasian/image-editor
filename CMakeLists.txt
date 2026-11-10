cmake_minimum_required(VERSION 3.16)
project(imgedit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(IMGEDIT_SANITIZE "Build with AddressSanitizer and UBSan" OFF)

# The library with all the image code. The CLI, tests and benchmark link to it.
add_library(imgedit_lib
  src/image.cpp
  src/adjust.cpp
  src/filters.cpp
  src/transform.cpp
  src/cli.cpp
)
target_include_directories(imgedit_lib PUBLIC include)
# stb is third-party code, mark it SYSTEM so its warnings don't show up
target_include_directories(imgedit_lib SYSTEM PRIVATE third_party/stb)

if(MSVC)
  target_compile_options(imgedit_lib PRIVATE /W4)
else()
  target_compile_options(imgedit_lib PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
endif()

if(IMGEDIT_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
  target_compile_options(imgedit_lib PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(imgedit_lib PUBLIC -fsanitize=address,undefined)
endif()

add_executable(imgedit src/main.cpp)
target_link_libraries(imgedit PRIVATE imgedit_lib)

add_executable(imgedit_bench bench/bench.cpp)
target_link_libraries(imgedit_bench PRIVATE imgedit_lib)

enable_testing()
add_executable(imgedit_tests
  tests/main.cpp
  tests/test_image.cpp
  tests/test_adjust.cpp
  tests/test_filters.cpp
  tests/test_transform.cpp
  tests/test_cli.cpp
)
target_link_libraries(imgedit_tests PRIVATE imgedit_lib)
add_test(NAME imgedit_tests COMMAND imgedit_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
