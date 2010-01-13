cmake_minimum_required(VERSION 2.6)
project(TransLucid)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)

set(TLCFLAGS ${GLIBMM_CFLAGS} -Wall)
set(TLLIBS ${GLIBMM_LDFLAGS})

set (Boost_USE_STATIC_LIBS OFF)
set (Boost_ADDITIONAL_VERSIONS 1.39 1.39.0)
set (Boost_USE_MULTITHREADED ON)
find_package(Boost 1.39.0 REQUIRED COMPONENTS program_options)

if (Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
endif()

INCLUDE_DIRECTORIES(src/include)

add_subdirectory(src)
