if (COMMAND cmake_policy)
   cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

add_definitions(${TLCFLAGS} 
#-DSHAREDIR=\\\"${CMAKE_INSTALL_PREFIX}/share/tl\\\"
-Wall)

include_directories(${CMAKE_SOURCE_DIR}/src/libs/system/)

add_custom_target(generate_static_lexer DEPENDS 
  ${CMAKE_CURRENT_SOURCE_DIR}/tl/static_lexer.hpp)

set (SYSTEM_SOURCES
ast.cpp bestfit.cpp builtin_types.cpp charset.cpp
context.cpp dimtranslator.cpp 
equation.cpp
eval_workshops.cpp header.cpp 
hyperdatons/envhd.cpp
hyperdatons/filehd.cpp
internal_strings.cpp lexertl.cpp lexer_util.cpp 
library.cpp line_tokenizer.cpp parser.cpp
range.cpp rename.cpp system.cpp
tree_printer.cpp
tree_to_wstree.cpp types.cpp utility.cpp uuid.cpp
workshop_builder.cpp
)

add_library(tlsystem SHARED ${SYSTEM_SOURCES})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/tl/static_lexer.hpp
  COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/tl
  COMMAND ${CMAKE_BINARY_DIR}/src/generate-lexer/generate-lexer >
    ${CMAKE_CURRENT_SOURCE_DIR}/tl/static_lexer.hpp ||
    (rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/tl/static_lexer.hpp && false)
  DEPENDS ${CMAKE_BINARY_DIR}/src/generate-lexer/generate-lexer
  )

set_property(SOURCE lexertl.cpp APPEND PROPERTY OBJECT_DEPENDS
  ${CMAKE_SOURCE_DIR}/src/libs/system/tl/static_lexer.hpp)

if (BUILD_STATIC_SYSTEM)
  add_library(tlsystem-static STATIC ${SYSTEM_SOURCES})
  set_target_properties(tlsystem-static PROPERTIES OUTPUT_NAME "tlsystem")
  set_target_properties(tlsystem-static PROPERTIES PREFIX "lib")
endif()

target_link_libraries(tlsystem ltdl rt gmpxx)
INSTALL(TARGETS tlsystem
   LIBRARY DESTINATION lib
)
