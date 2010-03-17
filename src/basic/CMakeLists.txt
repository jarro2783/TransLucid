if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

link_libraries(tlinterpreter ${TLLIBS} ${Boost_LIBRARIES})

add_executable(single single.cpp)
add_definitions(${TLCFLAGS})

add_executable(echo echo.cpp)

add_executable(typewriter typewriter.cpp)
