if (COMMAND cmake_policy)
   cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

add_definitions(${TLCFLAGS} -DSHAREDIR="${CMAKE_INSTALL_PREFIX}/share/tl"
-Wall)
add_library(tlinterpreter SHARED interpreter.cpp interpreter_expr.cpp 
interpreter_tuple.cpp interpreter_constant.cpp
library.cpp dimtranslator.cpp types.cpp
range.cpp expr.cpp parser.cpp
builtin_types.cpp equation.cpp
header_type.cpp cache.cpp utility.cpp 
compiled_functors.cpp expr_compiler.cpp consthd.cpp printer.cpp
container.cpp set_types.cpp)
#link_directories(${GLIBMM_LIBRARY_DIRS})
#target_link_libraries(tlinterpreter ${GLIBMM_LIBRARIES})
target_link_libraries(tlinterpreter ${GLIBMM_LDFLAGS} ltdl rt gmpxx)
INSTALL(TARGETS tlinterpreter
   LIBRARY DESTINATION lib
)
