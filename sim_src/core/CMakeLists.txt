cmake_minimum_required(VERSION 3.10)

project(cpp_logic_simulation_core_simulator)

add_definitions(-std=c++14 -Wall -g -O3)

set(CORE_SOURCE_DIR ${CORE_DIR}/src)

set(CORE_SOURCE_FILES ${CORE_SOURCE_DIR}/c_clock.cpp
						${CORE_SOURCE_DIR}/c_comp.cpp
						${CORE_SOURCE_DIR}/c_device.cpp
						${CORE_SOURCE_DIR}/c_gate.cpp
						${CORE_SOURCE_DIR}/c_gates.cpp
						${CORE_SOURCE_DIR}/c_probe.cpp
						${CORE_SOURCE_DIR}/c_sim.cpp
						${CORE_SOURCE_DIR}/c_monitor.cpp
)

add_library(core_simulator SHARED ${CORE_SOURCE_FILES})

target_include_directories(core_simulator
							PUBLIC
							${CORE_INCLUDE_DIR}
							${THREAD_POOL_DIR}
							${UTILS_INCLUDE_DIR}
)

target_link_libraries(core_simulator
						thread_pool
						pthread
)
