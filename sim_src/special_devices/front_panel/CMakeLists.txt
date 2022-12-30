cmake_minimum_required(VERSION 3.10)

project(front_panel_library)

add_definitions(-std=c++14 -g -O3)

set(FLTK_DIR /home/sebsikora/Documents/cpp_projects/fltk/fltk-1.3.8/build)
set(FP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FP_SOURCES
	${FP_SRC_DIR}/fp_components.cpp
)
set(FP_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(FLTK REQUIRED NO_MODULE)

add_executable(fp_demo fp_demo.cpp ${FP_SOURCES})

target_include_directories(fp_demo PUBLIC ${FLTK_INCLUDE_DIRS} ${FP_INC_DIR})

target_link_libraries(fp_demo fltk)
