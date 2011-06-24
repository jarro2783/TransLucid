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

set(TLCFLAGS -Wall -std=gnu++0x -DPACKAGE_VERSION=\"0.0.35\"
  -DBOOST_SPIRIT_UNICODE -DHAVE_CONFIG_H -DBOOST_MPL_LIMIT_LIST_SIZE=30
  -DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS)
set(TLLIBS -std=gnu++0x)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
"${CMAKE_INSTALL_PREFIX}/share/tl")

set(CMAKE_BUILD_TYPE build)

set (Boost_USE_STATIC_LIBS OFF)
set (Boost_ADDITIONAL_VERSIONS 1.46.1 1.46.0)
set (Boost_USE_MULTITHREADED ON)
find_package(Boost 1.45.0 REQUIRED COMPONENTS program_options)

if (Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
endif()

INCLUDE_DIRECTORIES(src/include ${CMAKE_BINARY_DIR})

set(PREFIX \"${CMAKE_INSTALL_PREFIX}\")
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

add_subdirectory(src)
