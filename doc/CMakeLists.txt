
if (DOT STREQUAL "DOT-NOTFOUND")
  set (HAVE_DOT "NO")
else()
  set (HAVE_DOT "YES")
endif()

if (DOXYGEN STREQUAL "DOXYGEN-NOTFOUND")
  add_custom_target(doc echo doxygen not installed, documentation not built)
else()
  add_custom_target(doc doxygen)
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/doc/Doxyfile.in 
    ${CMAKE_BINARY_DIR}/doc/Doxyfile)
endif()
