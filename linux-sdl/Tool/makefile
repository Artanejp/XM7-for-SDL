#
# FM-7 EMULATOR "XM7"
#
# Copyright (C) 1999-2012 ÇoÇhÅD(yasushi@tanaka.net)
# Copyright (C) 2001-2012 Ryu Takegami
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
AS86_OBJSJ	= $(AS86_BINJ)
AS86_SRCSJ	= ${AS86_OBJSJ:.COM=.X86}
AS09_BINJ	= SAVEJDIC.BIN
AS09_OBJSJ	= IPL.O SAVEJDIC.O
AS09_SRCSJ	= ${AS09_OBJSJ:.O=.ASM}
AS86_BINK	= SAVEKNJ.COM
AS86_OBJSK	= $(AS86_BINK)
AS86_SRCSK	= ${AS86_OBJSK:.COM=.X86}
AS09_BINK	= SAVEKNJ.BIN
AS09_OBJSK	= IPL.O SAVEKNJ.O
AS09_SRCSK	= ${AS09_OBJSK:.O=.ASM}
AS09_BINC	= SAVEROM.2B0
AS09_OBJSC	= SAVEROM.CO
AS09_SRCSC	= ${AS09_OBJSC:.CO=.ASM}
AS09_BIN2C	= SAVEROM2.2B0
AS09_OBJS2C	= SAVEROM2.CO
AS09_SRCS2C	= ${AS09_OBJS2C:.CO=.ASM}
AS09_BIN7C	= SAVEROM7.2B0
AS09_OBJS7C	= SAVEROM7.CO
AS09_SRCS7C	= ${AS09_OBJS7C:.CO=.ASM}
AS09_BINJC	= SAVEJDIC.2B0
AS09_OBJSJC	= SAVEJDIC.CO
AS09_SRCSJC	= ${AS09_OBJSKC:.CO=.ASM}
AS09_BINKC	= SAVEKNJ.2B0
AS09_OBJSKC	= SAVEKNJ.CO
AS09_SRCSKC	= ${AS09_OBJSKC:.CO=.ASM}

TARGET : $(AS09_BIN) $(AS09_BIN2) $(AS09_BIN7) $(AS09_BINJ) $(AS09_BINK) $(AS86_BIN) $(AS86_BIN2) $(AS86_BIN7) $(AS86_BINJ) $(AS86_BINK) $(AS09_BINC) $(AS09_BIN2C) $(AS09_BIN7C) $(AS09_BINJC) $(AS09_BINKC)
		copy /y DUMMY.D77 SAVER.D77
		FMWRITE.BAT

$(AS09_BIN) : $(AS09_OBJS)
		copy /by IPL.O+SAVEROM.O $(AS09_BIN)
$(AS09_BIN2) : $(AS09_OBJS2)
		copy /by IPL.O+SAVEROM2.O $(AS09_BIN2)
$(AS09_BIN7) : $(AS09_OBJS7)
		copy /by IPL.O+SAVEROM7.O $(AS09_BIN7)
$(AS09_BINJ) : $(AS09_OBJSJ)
		copy /by IPL.O+SAVEJDIC.O $(AS09_BINJ)
$(AS09_BINK) : $(AS09_OBJSK)
		copy /by IPL.O+SAVEKNJ.O $(AS09_BINK)
$(AS09_BINC) : $(AS09_OBJSC)
		copy /by SAVEROM.2BH+SAVEROM.CO $(AS09_BINC)
$(AS09_BIN2C) : $(AS09_OBJS2C)
		copy /by SAVEROM2.2BH+SAVEROM2.CO $(AS09_BIN2C)
$(AS09_BIN7C) : $(AS09_OBJS7C)
		copy /by SAVEROM7.2BH+SAVEROM7.CO $(AS09_BIN7C)
$(AS09_BINJC) : $(AS09_OBJSJC)
		copy /by SAVEJDIC.2BH+SAVEJDIC.CO $(AS09_BINJC)
$(AS09_BINKC) : $(AS09_OBJSKC)
		copy /by SAVEKNJ.2BH+SAVEKNJ.CO $(AS09_BINKC)

saverom.o : saverom.asm
		$(AS09) $(AS09FLAGS) saverom.asm -o$&.o -dMODEL=1 -dDISKTYPE=1
saverom2.o : saverom.asm
		$(AS09) $(AS09FLAGS) saverom.asm -o$&.o -dMODEL=2 -dDISKTYPE=2
saverom7.o : saverom.asm
		$(AS09) $(AS09FLAGS) saverom.asm -o$&.o -dMODEL=0 -dDISKTYPE=1
savejdic.o : savejdic.asm
		$(AS09) $(AS09FLAGS) savejdic.asm -o$&.o -dDISKTYPE=1
saveknj.o : saveknj.asm
		$(AS09) $(AS09FLAGS) saveknj.asm -o$&.o -dMODEL=4 -dDISKTYPE=2
saverom.co : saverom.asm
		$(AS09) $(AS09FLAGS) saverom.asm -o$&.co -dMODEL=1 -dDISKTYPE=1 -dFBLOADM -k
saverom2.co : saverom.asm
		$(AS09) $(AS09FLAGS) saverom.asm -o$&.co -dMODEL=2 -dDISKTYPE=2 -dFBLOADM -k
saverom7.co : saverom.asm
		$(AS09) $(AS09FLAGS) saverom.asm -o$&.co -dMODEL=0 -dDISKTYPE=1 -dFBLOADM -k
savejdic.co : savejdic.asm
		$(AS09) $(AS09FLAGS) savejdic.asm -o$&.co -dDISKTYPE=1 -dFBLOADM -k
saveknj.co : saveknj.asm
		$(AS09) $(AS09FLAGS) saveknj.asm -o$&.co -dMODEL=4 -dDISKTYPE=2 -dFBLOADM -k
saverom.com : saverom.x86
		$(AS86) $(AS86FLAGS) saverom.x86 -o$&.com -dMODEL=1
saverom2.com : saverom.x86
		$(AS86) $(AS86FLAGS) saverom.x86 -o$&.com -dMODEL=2
saverom7.com : saverom.x86
		$(AS86) $(AS86FLAGS) saverom.x86 -o$&.com -dMODEL=7
savejdic.com : saverom.x86
		$(AS86) $(AS86FLAGS) saverom.x86 -o$&.com -dMODEL=3
saveknj.com : saverom.x86
		$(AS86) $(AS86FLAGS) saverom.x86 -o$&.com -dMODEL=4

.asm.o :
		$(AS09) $(AS09FLAGS) $&.asm -o$&.o
.asm.co :
		$(AS09) $(AS09FLAGS) $&.asm -o$&.co
.x86.obj :
		$(AS86) $(AS86FLAGS) $&.x86
