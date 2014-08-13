cmake_minimum_required (VERSION 2.6)

find_package(ImageMagick)
set(XM7_RESOURCE_ICONS 2dd.ico
                       app_2d.ico
                       app_2d1.ico
                       app_2d2.ico
                       app_2d5.ico
                       app_2dd1.ico
                       app_2dd2.ico
                       app_351.ico
                       app_352.ico
                       app_5in.ico
                       app_av.ico
                       app_d77.ico
                       app_d771.ico
                       app_d772.ico
                       app_d775.ico
                       app_d77g.ico
                       app_ex.ico
                       app_sx.ico
                       app_t77.ico
                       app_t771.ico
                       app_vfd.ico
                       app_vfd1.ico
                       app_vfd2.ico
                       app_vfd5.ico
		       fujitsu.ico
		       fujitsu2.ico
		       tamori.ico
		       ten40ex.ico
		       ten40ex2.ico
		       ten40sx.ico
		       ten77av.ico
		       tenfm7.ico
		       tenfm77.ico
		       tenfm8.ico
		       xm7.ico
		       )
set(XM7_RESOURCE_PNGS "")


if(ImageMagick_convert_FOUND)
   foreach(loop_var_src ${XM7_RESOURCE_ICONS})
     string(REPLACE .ico .png loop_var_target ${loop_var_src})
     set(XM7_RESOURCE_PNGS "${XM7_RESOURCE_PNGS} ${loop_var_target}")
     add_custom_target(${loop_var_target} ALL 
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${loop_var_src}
		     COMMAND  ${ImageMagick_convert_EXECUTABLE}
		     ${CMAKE_CURRENT_SOURCE_DIR}/${loop_var_src}
		     ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${loop_var_target}
		     )
     install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${loop_var_target} DESTINATION share/xm7)
     set_directory_properties(PROPERTIES 
                              ADDITIONAL_MAKE_CLEAN_FILES 
                              ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${loop_var_target})
   endforeach(loop_var_src)
endif()
install(FILES xm7_mani.xml  DESTINATION share/xm7)




add_subdirectory(F-Font)