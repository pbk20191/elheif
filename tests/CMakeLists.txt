cmake_minimum_required(VERSION 3.14)
include(FetchContent)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(elheif-test test.cpp)
add_dependencies(elheif-test elheif)
target_link_libraries(elheif-test PRIVATE elheif GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(elheif-test)