enable_testing()

set(RUN ${CMAKE_SOURCE_DIR}/src/tests/runbinary.sh)
set(LIBDIR ${CMAKE_BINARY_DIR}/src/libs/int)
set(TESTFILE ${CMAKE_SOURCE_DIR}/src/tlcore/runtests.sh)
set(TLCOREFILE ${CMAKE_BINARY_DIR}/src/tlcore/tlcore)
set(TESTPATH ${CMAKE_SOURCE_DIR}/src/tlcore/tests)

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

link_directories(${Boost_LIBRARY_DIRS})
add_executable(tlcore main.cpp tlcore.cpp)
target_link_libraries(tlcore tlsystem ${Boost_LIBRARIES})
add_definitions(${TLCFLAGS})

add_test(blackbox ${RUN} ${LIBDIR} ${TESTFILE} ${TLCOREFILE} ${TESTPATH})

install(TARGETS tlcore
   RUNTIME DESTINATION bin
)
