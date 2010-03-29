enable_testing()

set(RUN ${CMAKE_SOURCE_DIR}/src/tests/runbinary.sh)
set(LIBDIR ${CMAKE_BINARY_DIR}/src/libs/int)

link_libraries(tlinterpreter ${TLLIBS} ${Boost_LIBRARIES})

add_executable(expressions expressions.cpp)
add_executable(equations equations.cpp)
add_executable(printing printing.cpp)
add_definitions(${TLCFLAGS})

add_test(expressions ${RUN} ${LIBDIR} expressions)
add_test(equations ${RUN} ${LIBDIR} equations)
add_test(printing printing)
