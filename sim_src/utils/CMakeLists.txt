cmake_minimum_required(VERSION 3.10)

project(cpp_logic_simulation_utils_library)

add_definitions(-std=c++14 -Wall -g -O3)

set(UTILS_SOURCE_DIR ${UTILS_DIR}/src)

set(UTILS_SOURCE_FILES ${UTILS_SOURCE_DIR}/utils.cpp
						${UTILS_SOURCE_DIR}/strnatcmp.cpp
)

add_library(utils_library SHARED ${UTILS_SOURCE_FILES})

target_include_directories(utils_library
							PUBLIC
							${UTILS_INCLUDE_DIR}
							${CORE_INCLUDE_DIR}
)
