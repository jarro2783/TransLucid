#   Copyright (C) 2009--2013 Jarryd Beck
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
project(tl)

set(VERSION "0.4.0")
set(builddir ${CMAKE_BINARY_DIR}/doc)
set(abs_top_srcdir ${CMAKE_SOURCE_DIR})

set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE TRUE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")
include(GettextTranslate)

include(CheckCXXCompilerFlag)

enable_testing()

execute_process(COMMAND
  ${CMAKE_SOURCE_DIR}/checks/generate_gcc_min_version_check
  OUTPUT_FILE ${CMAKE_BINARY_DIR}/version.cpp)

try_compile(GCC_IS_CORRECT_VERSION ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/version.cpp
  COMPILE_DEFINITIONS "-std=c++11"
)

if (NOT GCC_IS_CORRECT_VERSION)
   message(FATAL_ERROR "g++ version 4.7.0 or greater is required")
endif()

#find_package(Gettext REQUIRED)

find_package(PkgConfig REQUIRED)

set(LOCALEDIR ${CMAKE_INSTALL_PREFIX}/share/locale)

CHECK_CXX_COMPILER_FLAG(-Wno-unused-local-typedefs HAS_NO_UNUSED_LOCAL)
if (HAS_NO_UNUSED_LOCAL)
  set(W_NO_UNUSED_LOCAL -Wno-unused-local-typedefs)
endif()

set(TLCFLAGS "-Wall ${W_NO_UNUSED_LOCAL} -std=c++11 -DPACKAGE_VERSION=\\\"${VERSION}\\\" -DHAVE_CONFIG_H")

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
"${CMAKE_INSTALL_PREFIX}/share/tl")

pkg_check_modules(ICU REQUIRED icu-uc)
if (NOT ICU_FOUND)
  message(FATAL_ERROR "ICU not found")
endif()
set(TLLIBS -std=c++11 ${ICU_LDFLAGS})

# do we build tlweb
#option (ENABLE_TLWEB "Build the tlweb application" OFF)

if (ENABLE_TLWEB)
  if (${ENABLE_TLWEB} STREQUAL "static")
    set(BUILD_STATIC_SYSTEM ON)
  endif()
endif()

# look for documentation requirements
find_program(DOXYGEN doxygen)
find_program(DOT dot)

if (DOT STREQUAL "DOT-NOTFOUND")
  message(WARNING "documentation will be built without graphs")
endif()

#INCLUDE_DIRECTORIES(BEFORE src/include ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_FLAGS "-I${CMAKE_SOURCE_DIR}/src/include -I${CMAKE_BINARY_DIR} ${CMAKE_CXX_FLAGS}")

set(PREFIX \"${CMAKE_INSTALL_PREFIX}\")
set(HEADERDIR \"${CMAKE_INSTALL_PREFIX}/share/tl/tltext\")
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

add_subdirectory(src)
add_subdirectory(doc)
