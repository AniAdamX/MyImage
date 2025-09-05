cmake_minimum_required(VERSION 4.1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(MyImage)

file(GLOB SRC_FILES
	"${PROJECT_SOURCE_DIR}/*.h"
	"${PROJECT_SOURCE_DIR}/*.hpp"
	"${PROJECT_SOURCE_DIR}/*.cpp"
	"${PROJECT_SOURCE_DIR}/*.cc")

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})
