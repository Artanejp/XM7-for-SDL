cmake_minimum_required (VERSION 2.6)


if(GETTEXT_FOUND)
     add_custom_target(messages.mo ALL 
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/messages.po
		     COMMAND  ${GETTEXT_MSGFMT_EXECUTABLE}
		              ${CMAKE_CURRENT_SOURCE_DIR}/messages.po
		              -o ${CMAKE_CURRENT_BINARY_DIR}/messages.mo
		     )
     install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/messages.po
                   ${CMAKE_CURRENT_BINARY_DIR}/messages.mo
             DESTINATION share/xm7/C)
endif()
