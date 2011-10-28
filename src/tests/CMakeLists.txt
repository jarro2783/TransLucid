enable_testing()

set(RUN ${CMAKE_SOURCE_DIR}/src/tests/runbinary.sh)
set(LIBDIR ${CMAKE_BINARY_DIR}/src/libs/int)
set(BINDIR .)

link_libraries(tlsystem ${TLLIBS} ${Boost_LIBRARIES})

#add_executable(expressions expressions.cpp)
#add_executable(equations equations.cpp)
#add_executable(printing printing.cpp)
#add_executable(iterator iterator.cpp)
#add_executable(hyperdatons hyperdatons.cpp)
#add_executable(iterinput iterinput.cpp)
add_executable(line_tokenizer line_tokenizer.cpp)

add_executable(system system.cpp)
add_executable(lexer lexer.cpp)
add_executable(variant variant.cpp)

add_definitions(${TLCFLAGS})

#add_test(iterator ./iterator)
#add_test(expressions ${RUN} ${LIBDIR} ./expressions)
#add_test(equations ${RUN} ${LIBDIR} ./equations)
#add_test(printing ./printing)
add_test(system ./system)
add_test(line_tokenizer ./line_tokenizer)
add_test(lexer ./lexer)
add_test(variant ./variant)
