enable_testing()

set(RUN ${CMAKE_SOURCE_DIR}/src/tests/runbinary.sh)
set(LIBDIR
${CMAKE_BINARY_DIR}/src/libs/int:${CMAKE_BINARY_DIR}/src/libs/string)
set(TESTFILE ${CMAKE_SOURCE_DIR}/src/tltext/runtests.sh)
set(TLTEXTFILE ${CMAKE_BINARY_DIR}/src/tltext/tltext)
set(TESTPATH ${CMAKE_SOURCE_DIR}/src/tltext/tests)

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

link_directories(${Boost_LIBRARY_DIRS})
add_executable(tltext main.cpp tltext.cpp demandhd.cpp)
target_link_libraries(tltext tlsystem ${Boost_LIBRARIES})

add_executable(tlweb tlweb.cpp tltext.cpp demandhd.cpp)
target_link_libraries(tlweb tlsystem)

add_definitions(${TLCFLAGS})

add_test(blackbox ${RUN} ${LIBDIR} ${TESTFILE} ${TLTEXTFILE} ${TESTPATH})

install(TARGETS tltext
   RUNTIME DESTINATION bin
)

install(FILES header.tl DESTINATION share/tl/tltext)
