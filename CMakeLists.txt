cmake_minimum_required(VERSION 3.18)
project(WiSpy)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(WiSpy main.cpp exec.cpp exec.h)
target_link_libraries(WiSpy Threads::Threads)