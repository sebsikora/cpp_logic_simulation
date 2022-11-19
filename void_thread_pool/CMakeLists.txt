cmake_minimum_required(VERSION 3.10)

project(void_thread_pool)

add_definitions(-std=c++14 -Wall -g -O3)

find_package(Threads REQUIRED)

set(SOURCES void_thread_pool.cpp
			void_thread_pool.hpp
)

add_library(thread_pool SHARED ${SOURCES})

target_link_libraries(thread_pool pthread)
