cmake_minimum_required(VERSION 3.14)
project(mini_redis)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.0)
FetchContent_MakeAvailable(Catch2)

add_executable(tests tests/test_hash_table.cpp src/hash_table.cpp) #executables and the source files. Create a runnable program named tests, and compile it using these specific .cpp files.
target_include_directories(tests PRIVATE include) #where to find out header files (.h or .hpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain) #any external libraries we need