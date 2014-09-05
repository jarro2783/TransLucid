add_subdirectory(detail)
add_subdirectory(static)
add_subdirectory(types)
add_subdirectory(tyinf)

FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURREN_TSOURCE_DIR}/juice/*.hpp")
INSTALL(FILES ${files} DESTINATION include/tl)
