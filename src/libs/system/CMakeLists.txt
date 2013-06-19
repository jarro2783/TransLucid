add_subdirectory(lex)
add_subdirectory(po)

if (COMMAND cmake_policy)
   cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

add_definitions(${TLCFLAGS} 
#-DSHAREDIR=\\\"${CMAKE_INSTALL_PREFIX}/share/tl\\\"
-DTRANSLATE_DOMAIN=\"libtl\" -DLOCALEDIR=\"${LOCALEDIR}\"
-DENABLE_NLS
-Wall)

include_directories(${CMAKE_SOURCE_DIR}/src/libs/system/)

add_custom_target(generate_static_lexer DEPENDS 
  ${CMAKE_CURRENT_SOURCE_DIR}/lex/static_lexer.hpp)

set (SYSTEM_SOURCES
assignment.cpp ast.cpp bestfit.cpp builtin_types.cpp cache.cpp charset.cpp
chi.cpp context.cpp datadef.cpp dependencies.cpp dimtranslator.cpp 
equation.cpp
eval_workshops.cpp
function.cpp
hyperdatons/arrayhd.cpp
hyperdatons/envhd.cpp
hyperdatons/filehd.cpp
internal_strings.cpp lexertl.cpp lexer_util.cpp 
library.cpp line_tokenizer.cpp opdef.cpp parser.cpp
range.cpp region.cpp rename.cpp semantic_transform.cpp 
system.cpp system_util.cpp
tree_printer.cpp tree_rewriter.cpp
tree_to_wstree.cpp 
tyinf/constraint_graph.cpp tyinf/type_inference.cpp
types/function_type.cpp types/numbers.cpp types/union.cpp
types.cpp utility.cpp uuid.cpp
workshop_builder.cpp
)

link_directories(${ICU_LIBRARY_DIRS})

add_library(tlsystem SHARED ${SYSTEM_SOURCES})

set_target_properties(tlsystem PROPERTIES
  SOVERSION 4
  VERSION 4.0.0
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/lex/static_lexer.hpp
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/lex/generate-lexer >
    ${CMAKE_CURRENT_SOURCE_DIR}/lex/static_lexer.hpp ||
    (rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/lex/static_lexer.hpp && false)
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/lex/generate-lexer
  )

set_property(SOURCE lexertl.cpp APPEND PROPERTY OBJECT_DEPENDS
  ${CMAKE_SOURCE_DIR}/src/libs/system/lex/static_lexer.hpp)

if (BUILD_STATIC_SYSTEM)
  add_library(tlsystem-static STATIC ${SYSTEM_SOURCES})
  set_target_properties(tlsystem-static PROPERTIES OUTPUT_NAME "tlsystem")
  set_target_properties(tlsystem-static PROPERTIES PREFIX "lib")
endif()

target_link_libraries(tlsystem ltdl rt gmpxx gmp ${ICU_LIBRARIES} -Wl,-O1)
INSTALL(TARGETS tlsystem
   LIBRARY DESTINATION lib
)
