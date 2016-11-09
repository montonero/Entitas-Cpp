
# Copyright (c) 2016 Juan Delgado (JuDelCo)
# License: MIT License
# MIT License web page: https://opensource.org/licenses/MIT

# ----------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.0)

if(NOT CONFIGURED_ONCE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Debug Release RelWithDebInfo MinSizeRel" FORCE)
	set(BUILD_CPU_ARCH   "x64"   CACHE STRING "x86 x64")
endif()

if( NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug" AND
	NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release" AND
	NOT ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo" AND
	NOT ${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
	message(FATAL_ERROR "Bad CMAKE_BUILD_TYPE variable value (Debug Release RelWithDebInfo MinSizeRel)")
endif()

if( NOT ${BUILD_CPU_ARCH} STREQUAL "x86" AND
	NOT ${BUILD_CPU_ARCH} STREQUAL "x64")
	message(FATAL_ERROR "Bad BUILD_CPU_ARCH variable value (x86 x64)")
endif()

message(STATUS "CMAKE_BUILD_CONFIG: ${CMAKE_BUILD_TYPE} (${BUILD_CPU_ARCH})")

# -------------------------------------------------------------------------------------------------

set(CMAKE_CXX_FLAGS         "-Wall -Werror -fmax-errors=5 -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG   "-gdwarf-2 -Og -DDEBUG_ON") # -g
set(CMAKE_CXX_FLAGS_RELEASE "-s -O2")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})

set(EXECUTABLE_NAME main_${CMAKE_BUILD_TYPE}_${BUILD_CPU_ARCH})
file(GLOB_RECURSE ENTITAS_SRC entitas/*.*pp)

if(${BUILD_CPU_ARCH} STREQUAL "x64")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
endif()

# -------------------------------------------------------------------------------------------------

project(EntitasPP CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${EXECUTABLE_NAME}
	main.cpp
	${ENTITAS_SRC}
)

# -------------------------------------------------------------------------------------------------

set(CONFIGURED_ONCE TRUE CACHE INTERNAL "Flag - CMake has configured at least once")
