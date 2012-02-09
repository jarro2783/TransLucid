macro(GettextTranslate)

  message(STATUS "gettext translate")

  if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/POTFILES.in)
    message(FATAL_ERROR "There is no POTFILES.in in this directory")
  endif()

  if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Makevars)
    message(FATAL_ERROR "There is no Makevars in this directory")
  endif()

  file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/Makevars makevars
    REGEX "^[^=]+=(.*)$"
  )

  foreach(makevar ${makevars})
    message(STATUS "makevar: " ${makevar})
    string(REGEX REPLACE "^([^= ]+) =[ ]?(.*)$" "\\1" MAKEVAR_KEY ${makevar})
    string(REGEX REPLACE "^([^= ]+) =[ ]?(.*)$" "\\2" 
      MAKEVAR_${MAKEVAR_KEY} ${makevar})
    message(STATUS split: ${MAKEVAR_KEY}=${MAKEVAR_${MAKEVAR_KEY}})
  endforeach()
  message(STATUS DOMAIN=${MAKEVAR_DOMAIN})

#string(REGEX MATCH "^[^=]+=(.*)$" parsed_variables ${makevars})

endmacro()
