add_library(int SHARED libint.cpp libint.hpp)
add_definitions(${TLCFLAGS})
target_link_libraries(int gmpxx gmp)
install(TARGETS int
   LIBRARY DESTINATION share/tl
)
