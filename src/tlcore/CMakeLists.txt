if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

link_directories(${Boost_LIBRARY_DIRS})
add_executable(tlcore main.cpp tlcore.cpp)
target_link_libraries(tlcore tlsystem ${Boost_LIBRARIES})
add_definitions(${TLCFLAGS})

install(TARGETS tlcore
   RUNTIME DESTINATION bin
)
