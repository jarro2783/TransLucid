enable_testing()

link_libraries(tlinterpreter ${TLLIBS} ${Boost_LIBRARIES})

add_executable(expressions expressions.cpp)
add_executable(equations equations.cpp)
add_executable(printing printing.cpp)
add_definitions(${TLCFLAGS})

add_test(expressions runbinary.sh "" expressions)
add_test(equations equations)
add_test(printing printing)
