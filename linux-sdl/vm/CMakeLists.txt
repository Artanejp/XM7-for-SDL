cmake_minimum_required (VERSION 2.6)

message("* vm")

add_library(xm7_vm
                  aluline.c
		  apalet.c
		  bubble.c
		  disasm.c
		  display.c
		  dmac.c
		  fdc.c
		  jcard.c
		  jsubsys.c
		  kanji.c
		  keyboard.c
		  maincpu.c
		  mainetc.c
		  mainmem.c
		  midi.c
		  mmr.c
		  mouse.c
		  multipag.c
		  opn.c
		  rtc.c
		  schedule.c
		  subcpu.c
		  subctrl.c
		  submem.c
		  system.c
		  tapelp.c
		  tools.c
		  ttlpalet.c
		  juliet.cpp
		  vm_sub.c
		  m6809.c
)

