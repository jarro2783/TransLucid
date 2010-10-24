add_library(string SHARED libstring.cpp libstring.hpp)
add_definitions(${TLCFLAGS})
install(TARGETS string
   LIBRARY DESTINATION share/tl
)
