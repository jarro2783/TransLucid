#   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice
#
#   This file is part of TransLucid.
#
#   TransLucid is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3, or (at your option)
#   any later version.
#
#   TransLucid is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with TransLucid; see the file COPYING.  If not see
#   <http://www.gnu.org/licenses/>. 

cmake_minimum_required(VERSION 2.6)
project(TransLucid)

enable_testing()

try_compile(GCC_IS_CORRECT_VERSION ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/checks/version.cpp)

if (NOT GCC_IS_CORRECT_VERSION)
   message(FATAL_ERROR "g++ version 4.5.0 or greater is required")
endif()

find_package(PkgConfig REQUIRED)

set(TLCFLAGS -Wall -std=gnu++0x)
set(TLLIBS -std=gnu++0x)

set(CMAKE_BUILD_TYPE build)

set (Boost_USE_STATIC_LIBS OFF)
set (Boost_ADDITIONAL_VERSIONS 1.42 1.42.0 1.43 1.43.0)
set (Boost_USE_MULTITHREADED ON)
set (CMAKE_MODULE_PATH /home/jarrydb)
find_package(Boost 1.42.0 REQUIRED COMPONENTS program_options)

if (Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
endif()

INCLUDE_DIRECTORIES(src/include)

add_subdirectory(src)
