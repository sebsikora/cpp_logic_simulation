cmake_minimum_required(VERSION 3.10)

project(cpp_logic_simulation_demos)

add_definitions(-std=c++14 -Wall -g -O3)

set(DEMOS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/demos_src)
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sim_src/core)
set(DEVICE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sim_src/devices)
set(SPECIAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sim_src/special_devices)
set(THREAD_POOL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/void_thread_pool)
set(UTILS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sim_src/utils)

set(CORE_INCLUDE_DIR ${CORE_DIR}/include)
set(DEVICE_INCLUDE_DIR ${DEVICE_DIR}/include)
set(SPECIAL_INCLUDE_DIR ${SPECIAL_DIR}/include)
set(UTILS_INCLUDE_DIR ${UTILS_DIR}/include)

set(COMMON_INCLUDE_DIRS ${CORE_INCLUDE_DIR}
						${DEVICE_INCLUDE_DIR}
						${SPECIAL_INCLUDE_DIR}
						${THREAD_POOL_DIR}
						${UTILS_INCLUDE_DIR}
)

add_subdirectory(${THREAD_POOL_DIR})
add_subdirectory(${CORE_DIR})
add_subdirectory(${DEVICE_DIR})
add_subdirectory(${SPECIAL_DIR})
add_subdirectory(${UTILS_DIR})

link_libraries(core_simulator device_library utils_library)	# All the demos need to link to the simulator library...

add_executable(jk_ff_demo ${DEMOS_DIR}/jk_ff_demo.cpp)
add_executable(4_bit_counter ${DEMOS_DIR}/4_bit_counter_demo.cpp)
add_executable(4_bit_counter_static ${DEMOS_DIR}/4_bit_counter_static_demo.cpp)
add_executable(n_bit_counter_demo ${DEMOS_DIR}/n_bit_counter_demo.cpp)
add_executable(n_bit_counter_asc_demo ${DEMOS_DIR}/n_bit_counter_asc_demo.cpp)
add_executable(n_bit_decoder_demo ${DEMOS_DIR}/n_bit_decoder_demo.cpp)
add_executable(sr_latch_demo ${DEMOS_DIR}/sr_latch_demo.cpp)
add_executable(sr_latch_demo_2 ${DEMOS_DIR}/sr_latch_demo_2.cpp)
add_executable(timing_test_3_static ${DEMOS_DIR}/timing_test_3_static.cpp)
add_executable(timing_test_3 ${DEMOS_DIR}/timing_test_3.cpp)
add_executable(game_of_life_cell_demo ${DEMOS_DIR}/game_of_life_cell_demo.cpp)
add_executable(game_of_life_cell_descision_demo ${DEMOS_DIR}/gol_cell_descision_demo.cpp)
add_executable(uart_demo ${DEMOS_DIR}/uart_demo.cpp)

include_directories(${COMMON_INCLUDE_DIRS})

target_link_libraries(uart_demo special_library)
