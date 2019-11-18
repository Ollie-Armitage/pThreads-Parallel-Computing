cmake_minimum_required(VERSION 3.15)
project(Parallel_Computing C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -pthread)
find_package(Threads)




add_executable(Parallel_Computing main.c)

target_link_libraries(Parallel_Computing m)