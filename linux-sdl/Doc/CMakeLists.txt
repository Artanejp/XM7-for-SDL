cmake_minimum_required (VERSION 2.6)


find_program(MAKEINFO_EXECUTABLE makeinfo)

set(info_in ${CMAKE_CURRENT_SOURCE_DIR}/xm7.ja.texinfo)
set(info_out ${CMAKE_CURRENT_BINARY_DIR}/xm7-sdl.ja.info)

add_custom_command(OUTPUT ${info_out}
                   COMMAND ${MAKEINFO_EXECUTABLE} -o ${info_out} ${info_in}
		   DEPENDS ${info_in}
		   COMMENT "Creating Info file ${info_out}"
		   VERBATIM)

add_custom_target(info ALL DEPENDS ${info_out})
	
install(FILES ${info_out} DESTINATION share/info)

