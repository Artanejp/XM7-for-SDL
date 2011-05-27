#
# FM-7 EMULATOR "XM7"
#
# Copyright (C) 1999-2009 ÇoÇhÅD(yasushi@tanaka.net)
# Copyright (C) 2001-2009 Ryu Takegami
#
# [ ROMÉZÅ[Éo makefile ]
#

AS09		= AS63.EXE
AS09FLAGS	= -y -8
AS86		= NASMW.EXE
AS86FLAGS	= -f bin

AS86_BIN	= SAVEROM.COM
AS86_OBJS	= $(AS86_BIN)
AS86_SRCS	= ${AS86_OBJS:.COM=.X86}
AS09_BIN	= SAVEROM.BIN
AS09_OBJS	= IPL.O SAVEROM.O
AS09_SRCS	= ${AS09_OBJS:.O=.ASM}
AS86_BIN2	= SAVEROM2.COM
AS86_OBJS2	= $(AS86_BIN2)
AS86_SRCS2	= ${AS86_OBJS2:.COM=.X86}
AS09_BIN2	= SAVEROM2.BIN
AS09_OBJS2	= IPL.O SAVEROM2.O
AS09_SRCS2	= ${AS09_OBJS2:.O=.ASM}
AS86_BIN7	= SAVEROM7.COM
AS86_OBJS7	= $(AS86_BIN7)
AS86_SRCS7	= ${AS86_OBJS7:.COM=.X86}
AS09_BIN7	= SAVEROM7.BIN
AS09_OBJS7	= IPL.O SAVEROM7.O
AS09_SRCS7	= ${AS09_OBJS7:.O=.ASM}
AS86_BINJ	= SAVEJDIC.COM
AS86_OBJSJ	= $(AS86_BIN7)
AS86_SRCSJ	= ${AS86_OBJS7:.COM=.X86}
AS09_BINJ	= SAVEJDIC.BIN
AS09_OBJSJ	= IPL.O SAVEJDIC.O
AS09_SRCSJ	= ${AS09_OBJSJ:.O=.ASM}

TARGET : $(AS09_BIN) $(AS09_BIN2) $(AS09_BIN7) $(AS09_BINJ) $(AS86_BIN) $(AS86_BIN2) $(AS86_BIN7) $(AS86_BINJ)

$(AS09_BIN) : $(AS09_OBJS)
		copy /b IPL.O+SAVEROM.O $(AS09_BIN)
$(AS09_BIN2) : $(AS09_OBJS2)
		copy /b IPL.O+SAVEROM2.O $(AS09_BIN2)
$(AS09_BIN7) : $(AS09_OBJS7)
		copy /b IPL.O+SAVEROM7.O $(AS09_BIN7)
$(AS09_BINJ) : $(AS09_OBJSJ)
		copy /b IPL.O+SAVEJDIC.O $(AS09_BINJ)

saverom.o : saverom.asm
		$(AS09) $(AS09FLAGS) saverom.asm -o$&.o -dMODEL=1 -dDISKTYPE=1
saverom2.o : saverom.asm
		$(AS09) $(AS09FLAGS) saverom.asm -o$&.o -dMODEL=2 -dDISKTYPE=2
saverom7.o : saverom.asm
		$(AS09) $(AS09FLAGS) saverom.asm -o$&.o -dMODEL=0 -dDISKTYPE=1
savejdic.o : savejdic.asm
		$(AS09) $(AS09FLAGS) savejdic.asm -o$&.o -dDISKTYPE=1

saverom.com : saverom.x86
		$(AS86) $(AS86FLAGS) saverom.x86 -o$&.com -dMODEL=1
saverom2.com : saverom.x86
		$(AS86) $(AS86FLAGS) saverom.x86 -o$&.com -dMODEL=2
saverom7.com : saverom.x86
		$(AS86) $(AS86FLAGS) saverom.x86 -o$&.com -dMODEL=7
savejdic.com : saverom.x86
		$(AS86) $(AS86FLAGS) saverom.x86 -o$&.com -dMODEL=3

.asm.o :
		$(AS09) $(AS09FLAGS) $&.asm -o$&.o
.x86.obj :
		$(AS86) $(AS86FLAGS) $&.x86
