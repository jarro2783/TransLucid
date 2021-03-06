enable_testing()

add_subdirectory(po)

set(RUN ${CMAKE_SOURCE_DIR}/src/tests/runbinary.sh)
set(LIBDIR
${CMAKE_BINARY_DIR}/src/libs/int:${CMAKE_BINARY_DIR}/src/libs/string)
set(TESTFILE ${CMAKE_SOURCE_DIR}/src/tltext/runtests.sh)
set(TLTEXTFILE ${CMAKE_BINARY_DIR}/src/tltext/tltext)
set(TESTPATH ${CMAKE_SOURCE_DIR}/src/tltext/tests)
set(EXAMPLESPATH ${CMAKE_SOURCE_DIR}/src/tltext/examples)
set(CACHEPATH ${CMAKE_SOURCE_DIR}/src/tltext/caching)

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_executable(tltext main.cpp tltext.cpp demandhd.cpp)
target_link_libraries(tltext tlsystem)

add_executable(matrixweb matrixweb.cpp)

if (ENABLE_TLWEB)
  add_executable(tlweb tlweb.cpp tltext.cpp demandhd.cpp)
  if(${ENABLE_TLWEB} STREQUAL "static")
    target_link_libraries(tlweb tlsystem-static -static gmpxx gmp ltdl dl
      ${TL_LIBS})
  else()
    target_link_libraries(tlweb tlsystem)
  endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TLCFLAGS}")

add_definitions(-DTRANSLATE_DOMAIN=\"tltext\" 
    -DLOCALEDIR=\"${LOCALEDIR}\" -DENABLE_NLS)

add_test(blackbox ${RUN} ${LIBDIR} ${TESTFILE} ${TLTEXTFILE} ${TESTPATH})
add_test(examples ${RUN} ${LIBDIR} ${TESTFILE} ${TLTEXTFILE} ${EXAMPLESPATH})
#add_test(caching ${RUN} ${LIBDIR} ${TESTFILE} ${TLTEXTFILE} ${CACHEPATH}
#  --cache)

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
install(FILES header-tyinf.tl DESTINATION share/tl/tltext)
