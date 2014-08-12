cmake_minimum_required (VERSION 2.6)



set(XM7_RESOURCE_FONTS F-Font.ttf
                       F-Font.txt
                       F-Font_400line.ttf
                       F-Font_Symbol_Unicode.ttf
                       F-Font_Symbol_Unicode_400line.ttf
		       Sample.txt
		       )

install(FILES ${XM7_RESOURCE_FONTS} DESTINATION share/xm7/F-Font)

