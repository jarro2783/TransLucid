if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

link_libraries(tlsystem ${TLLIBS} ${Boost_LIBRARIES})

add_executable(single single.cpp)
add_definitions(${TLCFLAGS})

add_executable(echo echo.cpp)

add_executable(typewriter typewriter.cpp)

add_executable(fact1 fact1.cpp)
