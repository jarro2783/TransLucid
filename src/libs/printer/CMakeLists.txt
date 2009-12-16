add_library(printer SHARED printer.cpp)
add_definitions(${TLCFLAGS})
install(TARGETS printer
   LIBRARY DESTINATION share/tl
)
