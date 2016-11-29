cmake_minimum_required(VERSION 2.8.12.2)
SET(PROJECT_NAME furries)

#set project
project(${PROJECT_NAME})

#variables for directories
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_CXX_DIR ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_EXT_DIR ${PROJECT_SOURCE_DIR}/external)

if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
		message( FATAL_ERROR "Please make a build directory" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()


set(MAYA_INCLUDE_DIR "" CACHE PATH "Maya include directory")
set(MAYA_LIB_DIR "" CACHE PATH "maya library")

#compiler flags
SET(CMAKE_CXX_FLAGS "-std=c++11")

if(CMAKE_COMPILER_IS_GNUCXX)
add_definitions(
	-g
	-Wall
)
endif()

add_subdirectory(${PROJECT_CXX_DIR})
include_directories(${MAYA_INCLUDE_DIR})
include_directories(${PROJECT_INCLUDE_DIR})
link_directories(${MAYA_LIB_DIR})


set( EXT_DEPS
	OpenMaya
	Foundation
)

if (APPLE)
    add_definitions(-DOSMac_)
endif()


add_library(${PROJECT_NAME} SHARED ${MAIN_SOURCE})
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".bundle")
elseif(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".mll")
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY LINK_FLAGS "/export:initializePlugin /export:uninitializePlugin")
endif()
add_dependencies(${PROJECT_NAME} ${EXT_DEPS})
target_link_libraries(${PROJECT_NAME} ${EXT_DEPS})
