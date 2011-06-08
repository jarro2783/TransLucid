if (COMMAND cmake_policy)
   cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

add_definitions(${TLCFLAGS} 
#-DSHAREDIR=\\\"${CMAKE_INSTALL_PREFIX}/share/tl\\\"
-Wall)
add_library(tlsystem SHARED 
ast.cpp bestfit.cpp builtin_types.cpp
compiled_functors.cpp dimtranslator.cpp 
equation.cpp equation_parser.cpp expr_compiler.cpp 
expr_parser.cpp header.cpp lexer.cpp lexer_util.cpp library.cpp 
line_tokenizer.cpp parser.cpp printer.cpp
range.cpp system.cpp translator.cpp tree_printer.cpp
tree_to_wstree.cpp tuple_parser.cpp types.cpp utility.cpp
)

target_link_libraries(tlsystem ltdl rt gmpxx)
INSTALL(TARGETS tlsystem
   LIBRARY DESTINATION lib
)
