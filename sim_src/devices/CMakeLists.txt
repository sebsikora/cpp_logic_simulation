cmake_minimum_required(VERSION 3.10)

project(cpp_logic_simulation_device_library)

add_definitions(-std=c++14 -Wall -g -O3)

set(DEVICE_SOURCE_DIR ${DEVICE_DIR}/src)

set(DEVICE_SOURCE_FILES ${DEVICE_SOURCE_DIR}/devices.cpp
						${DEVICE_SOURCE_DIR}/n_bit_sr_latch.cpp
						${DEVICE_SOURCE_DIR}/quad_sr_latch.cpp
						${DEVICE_SOURCE_DIR}/sr_latch.cpp
						${DEVICE_SOURCE_DIR}/jk_ff_static.cpp
						${DEVICE_SOURCE_DIR}/game_of_life.cpp
)

add_library(device_library SHARED ${DEVICE_SOURCE_FILES})

target_include_directories(device_library
							PUBLIC
							${DEVICE_INCLUDE_DIR}
							${CORE_INCLUDE_DIR}
)

target_link_libraries(device_library
						core_simulator
)
