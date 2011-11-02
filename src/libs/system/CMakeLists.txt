if (COMMAND cmake_policy)
   cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

add_definitions(${TLCFLAGS} 
#-DSHAREDIR=\\\"${CMAKE_INSTALL_PREFIX}/share/tl\\\"
-Wall)

set (SYSTEM_SOURCES
ast.cpp ast-new.cpp bestfit.cpp builtin_types.cpp charset.cpp
context.cpp dimtranslator.cpp 
equation.cpp equation_parser.cpp
eval_workshops.cpp expr_parser.cpp header.cpp 
hyperdatons/envhd.cpp
hyperdatons/filehd.cpp
internal_strings.cpp lexer.cpp lexertl.cpp lexer_util.cpp 
library.cpp line_tokenizer.cpp parser.cpp
range.cpp rename.cpp system.cpp translator.cpp tree_printer.cpp
tree_to_wstree.cpp tuple_parser.cpp types.cpp utility.cpp uuid.cpp
workshop_builder.cpp
)

add_library(tlsystem SHARED ${SYSTEM_SOURCES})

if (BUILD_STATIC_SYSTEM)
  add_library(tlsystem-static STATIC ${SYSTEM_SOURCES})
  set_target_properties(tlsystem-static PROPERTIES OUTPUT_NAME "tlsystem")
  set_target_properties(tlsystem-static PROPERTIES PREFIX "lib")
endif()

target_link_libraries(tlsystem ltdl rt gmpxx)
INSTALL(TARGETS tlsystem
   LIBRARY DESTINATION lib
)
