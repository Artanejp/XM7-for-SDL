cmake_minimum_required (VERSION 2.6)

message("* debugger")


add_library(xm7_debugger
                       	dbg_breakpoint.cpp
			dbg_disasm09.cpp
			dbg_memdump.cpp
			memaccess.cpp
)