cmake_minimum_required(VERSION 3.14)
project(mini_redis)
set(CMAKE_CXX_STANDARD 17)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)
FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.0)
FetchContent_MakeAvailable(Catch2)

add_library(core STATIC
  src/hash_table.cpp
  src/expiring_store.cpp
  src/lru_index.cpp)
target_include_directories(core PUBLIC include)

add_executable(tests
  tests/test_hash_table.cpp
  tests/test_expiring_store.cpp
  tests/test_lru.cpp)
target_link_libraries(tests PRIVATE core Catch2::Catch2WithMain)

add_executable(bench bench/bench.cpp)
target_link_libraries(bench PRIVATE core)

# Add command_handler to your library sources
add_library(miniredis_lib 
    src/hash_table.cpp 
    src/lru_index.cpp 
    src/expiring_store.cpp
    src/command_handler.cpp
)

# TELL THE LIBRARY WHERE THE HEADERS ARE 
target_include_directories(miniredis_lib PUBLIC include)

# Build the main CLI executable
add_executable(redis_cli src/main.cpp)
target_link_libraries(redis_cli miniredis_lib)
target_include_directories(redis_cli PRIVATE include)

# Build the command handler test
add_executable(test_command_handler tests/test_command_handler.cpp)
target_link_libraries(test_command_handler miniredis_lib)
target_include_directories(test_command_handler PRIVATE include)

# Register the new test with CTest
add_test(NAME TestCommandHandler COMMAND test_command_handler)