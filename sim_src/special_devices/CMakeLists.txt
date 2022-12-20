cmake_minimum_required(VERSION 3.10)

project(cpp_logic_simulation_special_device_library)

add_definitions(-std=c++14 -Wall -g -O3)

set(SPECIAL_SOURCE_DIR ${SPECIAL_DIR}/src)

set(SPECIAL_SOURCE_FILES ${SPECIAL_SOURCE_DIR}/ram.cpp
						 ${SPECIAL_SOURCE_DIR}/rom.cpp
						 ${SPECIAL_SOURCE_DIR}/uart.cpp
)

add_library(special_library SHARED ${SPECIAL_SOURCE_FILES})

target_include_directories(special_library
						   PUBLIC
						   ${SPECIAL_INCLUDE_DIR}
						   ${DEVICE_INCLUDE_DIR}
						   ${CORE_INCLUDE_DIR}
)

target_link_libraries(special_library
					  pthread
					  core_simulator
					  device_library
)
