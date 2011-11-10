enable_testing()

set(RUN ${CMAKE_SOURCE_DIR}/src/tests/runbinary.sh)
set(LIBDIR ${CMAKE_BINARY_DIR}/src/libs/int)
set(BINDIR .)

link_libraries(tlsystem ${TLLIBS} ${Boost_LIBRARIES})
add_definitions(${TLCFLAGS})

include_directories(${CMAKE_SOURCE_DIR}/src/libs/system)
#add_executable(expressions expressions.cpp)
#add_executable(equations equations.cpp)
#add_executable(printing printing.cpp)
#add_executable(iterator iterator.cpp)
#add_executable(hyperdatons hyperdatons.cpp)
#add_executable(iterinput iterinput.cpp)

add_executable(variant variant.cpp)
add_executable(parser parser.cpp)
add_executable(system system.cpp)
add_executable(uuid uuid.cpp)

#add_test(iterator ./iterator)
#add_test(printing ./printing)
add_test(variant ./variant)
add_test(parser ./parser)
add_test(system ./system)
add_test(uuid ./uuid)
