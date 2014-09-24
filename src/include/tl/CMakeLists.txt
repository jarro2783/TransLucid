add_subdirectory(detail)
add_subdirectory(static)
add_subdirectory(types)
add_subdirectory(tyinf)
add_subdirectory(juice)

FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
INSTALL(FILES ${files} DESTINATION include/tl/juice)
