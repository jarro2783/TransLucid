if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_executable(tl main.cpp directory_system.cpp)
target_link_libraries(tl tlsystem ${TLLIBS} ${Boost_LIBRARIES})
add_definitions(${TLCFLAGS})

install(TARGETS tl
   RUNTIME DESTINATION bin
)
