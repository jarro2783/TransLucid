cmake_minimum_required(VERSION 2.6)
project(TransLucid)

enable_testing()

find_package(PkgConfig REQUIRED)

set(TLCFLAGS -Wall -std=gnu++0x)
set(TLLIBS -std=gnu++0x)

set(CMAKE_BUILD_TYPE build)

set (Boost_USE_STATIC_LIBS OFF)
set (Boost_ADDITIONAL_VERSIONS 1.41 1.41.0)
set (Boost_USE_MULTITHREADED ON)
set (CMAKE_MODULE_PATH /home/jarrydb)
set (Boost_DEBUG TRUE)
find_package(Boost 1.41.0 REQUIRED COMPONENTS program_options)

if (Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
endif()

INCLUDE_DIRECTORIES(src/include)

add_subdirectory(src)
