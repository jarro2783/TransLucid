add_subdirectory(detail)
add_subdirectory(types)

FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
INSTALL(FILES ${files} DESTINATION include/tl)
