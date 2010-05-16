if (COMMAND cmake_policy)
   cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

add_definitions(${TLCFLAGS} 
#-DSHAREDIR=\\\"${CMAKE_INSTALL_PREFIX}/share/tl\\\"
-Wall)
add_library(tlsystem SHARED system.cpp expr_parser.cpp 
library.cpp dimtranslator.cpp types.cpp
range.cpp parser.cpp
builtin_types.cpp equation.cpp
header_type.cpp cache.cpp utility.cpp 
compiled_functors.cpp expr_compiler.cpp valuehd.cpp printer.cpp
container.cpp set_types.cpp translator.cpp ast.cpp
tuple_parser.cpp bestfit.cpp)

#add_precompiled_header(tlsystem ${PROJECT_SOURCE_DIR}/src/include/tl/expr_parser.hpp)

#link_directories(${GLIBMM_LIBRARY_DIRS})
#target_link_libraries(tlsystem ${GLIBMM_LIBRARIES})
target_link_libraries(tlsystem ${GLIBMM_LDFLAGS} ltdl rt gmpxx)
INSTALL(TARGETS tlsystem
   LIBRARY DESTINATION lib
)
