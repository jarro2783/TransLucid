enable_testing()

set(RUN ${CMAKE_SOURCE_DIR}/src/tests/runbinary.sh)
set(LIBDIR
${CMAKE_BINARY_DIR}/src/libs/int:${CMAKE_BINARY_DIR}/src/libs/string)
set(TESTFILE ${CMAKE_SOURCE_DIR}/src/tltext/runtests.sh)
set(TLTEXTFILE ${CMAKE_BINARY_DIR}/src/tltext/tltext)
set(TESTPATH ${CMAKE_SOURCE_DIR}/src/tltext/tests)
set(EXAMPLESPATH ${CMAKE_SOURCE_DIR}/src/tltext/examples)

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

link_directories(${Boost_LIBRARY_DIRS})
add_executable(tltext main.cpp tltext.cpp demandhd.cpp)
target_link_libraries(tltext tlsystem ${Boost_LIBRARIES})

add_executable(matrixweb matrixweb.cpp)

if (ENABLE_TLWEB)
  add_executable(tlweb tlweb.cpp tltext.cpp demandhd.cpp)
  if(${ENABLE_TLWEB} STREQUAL "static")
    target_link_libraries(tlweb tlsystem-static -static gmpxx gmp ltdl dl)
  else()
    target_link_libraries(tlweb tlsystem)
  endif()
endif()

add_definitions(${TLCFLAGS})

add_test(blackbox ${RUN} ${LIBDIR} ${TESTFILE} ${TLTEXTFILE} ${TESTPATH})
add_test(examples ${RUN} ${LIBDIR} ${TESTFILE} ${TLTEXTFILE} ${EXAMPLESPATH})

set(EXAMPLES_OUT examples.html)

add_custom_command(OUTPUT ${EXAMPLES_OUT}
  COMMAND ${CMAKE_SOURCE_DIR}/src/tltext/build_examples.sh 
    ${EXAMPLESPATH} > ${EXAMPLES_OUT}
  DEPENDS examples/*.in
  DEPENDS build_examples.sh
)

add_custom_target(examples DEPENDS examples.html)

install(TARGETS tltext
   RUNTIME DESTINATION bin
)

install(FILES header.tl DESTINATION share/tl/tltext)
