cmake_minimum_required(VERSION 3.10)

project(cpp_logic_simulation_demos)

add_definitions(-std=c++14 -Wall -g -O3)

set(DEMOS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/demos_src)

set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sim_src/core)
set(DEVICE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sim_src/devices)
set(GOL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sim_src/devices/game_of_life)
set(THREAD_POOL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/void_thread_pool)
set(UTILS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sim_src/utils)

set(DEVICE_SOURCES ${DEVICE_DIR}/devices.cpp
				   ${DEVICE_DIR}/n_bit_sr_latch.cpp
				   ${DEVICE_DIR}/quad_sr_latch.cpp
				   ${DEVICE_DIR}/sr_latch.cpp
				   ${DEVICE_DIR}/jk_ff_static.cpp
)

set(GOL_SOURCES ${GOL_DIR}/game_of_life.cpp)

set(UTILS_SOURCES ${UTILS_DIR}/strnatcmp.cpp
				  ${UTILS_DIR}/utils.cpp
)

set(COMMON_SOURCES ${DEVICE_SOURCES}
				   ${UTILS_SOURCES}
)

set(COMMON_INCLUDE_DIRS ${CORE_DIR}/include
						${DEVICE_DIR}
						${THREAD_POOL_DIR}
						${UTILS_DIR}
)

add_subdirectory(${THREAD_POOL_DIR})
add_subdirectory(${CORE_DIR})

link_libraries(core_simulator)	# All the demos need to link to the simulator library...

add_executable(jk_ff_demo ${COMMON_SOURCES} ${DEMOS_DIR}/jk_ff_demo.cpp)
add_executable(4_bit_counter ${COMMON_SOURCES} ${DEMOS_DIR}/4_bit_counter_demo.cpp)
add_executable(4_bit_counter_static ${COMMON_SOURCES} ${DEMOS_DIR}/4_bit_counter_static_demo.cpp)
add_executable(sr_latch_demo ${COMMON_SOURCES} ${DEMOS_DIR}/sr_latch_demo.cpp)
add_executable(sr_latch_demo_2 ${COMMON_SOURCES} ${DEMOS_DIR}/sr_latch_demo_2.cpp)
add_executable(timing_test_3_static ${COMMON_SOURCES} ${DEMOS_DIR}/timing_test_3_static.cpp)
add_executable(timing_test_3 ${COMMON_SOURCES} ${DEMOS_DIR}/timing_test_3.cpp)
add_executable(game_of_life_cell_demo ${COMMON_SOURCES} ${GOL_SOURCES} ${DEMOS_DIR}/game_of_life_cell_demo.cpp)

include_directories(${COMMON_INCLUDE_DIRS})
target_include_directories(game_of_life_cell_demo PUBLIC ${GOL_DIR})
