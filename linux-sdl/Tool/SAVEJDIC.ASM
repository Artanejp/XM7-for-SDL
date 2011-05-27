*
*	FM-7 EMULATOR "XM7"
*
*	Copyright (C) 1999-2009 �o�h�D(yasushi@tanaka.net)
*	Copyright (C) 2001-2009 Ryu Takegami
*
*	[ ���{��T�u�V�X�e�� ROM�Z�[�o(XM7 V1.1�Ή�) ]
*
*	�Ώۋ@��E�������f�B�A�^�C�v�ɉ����ăA�Z���u�����ɃI�v�V�����w�肷�邱��
*		-dDISKTYPE=0	360KB(2D)  �ʏ�t�H�[�}�b�g
*		-dDISKTYPE=1	720KB(2DD) 1�g���b�N�����t�H�[�}�b�g
*		-dDISKTYPE=2	720KB(2DD) �ʏ�t�H�[�}�b�g
*

				ORG		$1000

*
*	�萔��` (���C���V�X�e��)
*
STACK			EQU		$0F00			* �X�^�b�N�ŏI
BUFFER			EQU		$0F80			* �ėp�o�b�t�@
KANJI_BUF		EQU		$0FE8			* �����o�b�t�@
DISK_BUF		EQU		$2800			* �f�B�X�N�o�b�t�@
SHARED			EQU		$FC80			* ���LRAM

*
*	�萔��` (���C��I/O)
*
KEYDAT			EQU		$FD01			* �L�[�f�[�^���W�X�^
SUBMODE			EQU		$FD04			* �T�u�V�X�e�����[�h���W�X�^
SUBSIN			EQU		$FD05			* �T�u�V�X�e���C���^�t�F�[�X���W�X�^
BANKSEL			EQU		$FD0F			* ROM/��RAM�Z���N�g���W�X�^
INITDIS			EQU		$FD10			* �C�j�V�G�[�gROM �f�B�Z�[�u�����W�X�^
FDCSTAT			EQU		$FD18			* FDC �X�e�[�^�X���W�X�^
FDCCMD			EQU		FDCSTAT			* FDC �R�}���h���W�X�^
FDCTRACK		EQU		$FD19			* FDC �g���b�N���W�X�^
FDCSECTOR		EQU		$FD1A			* FDC �Z�N�^���W�X�^
FDCDATA			EQU		$FD1B			* FDC �f�[�^���W�X�^
FDDSIDE			EQU		$FD1C			* FDD �T�C�h���W�X�^
FDDDRIVE		EQU		$FD1D			* FDD �h���C�u���W�X�^
FDDMODE			EQU		$FD1E			* FDD ���[�h���W�X�^
FDDDRQ			EQU		$FD1F			* FDC DRQ/IRQ���W�X�^
JSYNCR			EQU		$FD28			* ���{��T�u �������W�X�^(R)
JRCBR			EQU		$FD29			* ���{��T�u RCB���W�X�^(R)
JSYNCW			EQU		$FD2A			* ���{��T�u �������W�X�^(W)
JRCBW			EQU		$FD2B			* ���{��T�u RCB���W�X�^(W)
MMR				EQU		$FD80			* MMR �������}�b�s���O���W�X�^
MSGR			EQU		$FD90			* MMR �Z�O�����g���W�X�^
MMSR			EQU		$FD93			* MMR ���[�h�Z���N�g���W�X�^

*
*	�萔��` (�T�uI/O)
*
SUBMODEL4		EQU		$D405			* �T�u���[�h���W�X�^ (L4)
CRTDSP			EQU		$D408			* CRT ON/OFF���W�X�^
VACC			EQU		$D409			* VRAM�A�N�Z�X�t���O

*
*	�萔��` (���{��T�u�V�X�e��)
*
JSRCBAREA		EQU		$9F00			* ���C���Ƃ�RCB�ʐM�G���A
JSDICBNK		EQU		$9FFF			* �����o���N���W�X�^

*
*	���[�N�G���A��`
*
RS_FLAG			EQU		$00			* �V���A���o�̓t���O
DRIVE			EQU		$01			* �A�N�Z�X�h���C�u
TRACK			EQU		$02			* �A�N�Z�X�g���b�N
DRIVE0_T		EQU		$03			* �h���C�u0�̃g���b�N
DRIVE1_T		EQU		$04			* �h���C�u1�̃g���b�N
SIDE			EQU		$05			* �A�N�Z�X�T�C�h
SECTOR			EQU		$06			* �A�N�Z�X�Z�N�^
SECTORS			EQU		$07			* �A���Z�N�^��
CLUSTER			EQU		$08			* �t�@�C���擪�N���X�^
CLUSTERS		EQU		$0A			* �g�[�^���N���X�^��(1�N���X�^1KB)
LASTBYTE		EQU		$0C			* �ŏI�N���X�^�̃o�C�g��
COLOR			EQU		$0E			* �����J���[�R�[�h
TYPE			EQU		$0F			* FDC�R�}���h�^�C�v
C_TRACK			EQU		$10			* �J�����g�g���b�N
C_SIDE			EQU		$11			* �J�����g�T�C�h
C_SECTOR		EQU		$12			* �J�����g�Z�N�^
C_SECTORS		EQU		$13			* �J�����g�Z�N�^��
FM77_FLAG		EQU		RS_FLAG		* FM-77�t���O
L4_FLAG			EQU		COLOR		* 400���C���t���O (L4)

*
*	�A�Z���u��������`
*
M_FM7			EQU		0			* FM-7�V���[�Y�S�@��
M_FM77AV		EQU		1			* FM77AV�V���[�Y�S�@��
M_FM77AV40		EQU		2			* FM77AV40/EX/SX
D_NORM2D		EQU		0			* 2D (360KB) �ʏ�t�H�[�}�b�g
D_SKIP2DD		EQU		1			* 2DD(720KB) �P�g���b�N�����t�H�[�}�b�g
D_NORM2DD		EQU		2			* 2DD(720KB) �ʏ�t�H�[�}�b�g

*
*	�v���O�����X�^�[�g
*
START			BRA		MAIN

*
*	�v���O�����w�b�_
*
HEADER			FCC		/TINY-DOS(mini) for FM-7  /
				FCC		/Copyright 1991-1992,1999 by �o�h�D/
				FCB		$0D,$0A,$00

HEADER2			EQU		*
				FCC		/SAVEJDIC V1.02  /
				FCC		"Copyright 1999 �o�h�D/ 2006 Ryu Takegami"
				FCB		$0D,$0A,$00

*
*	���C���v���O����
*
				ORG		START+$80
MAIN			ORCC	#$50
				LEAS	STACK,PCR

* �n�[�h�E�F�A������
				LBSR	INIT
				LBSR	JRESET
				LBSR	FM7_INIT

* �N���^�C�g���\��
	if	(DISKTYPE==D_NORM2D)||(defined(FBLOADM))
				LEAX	TITLE_MSG,PCR
				LBSR	PUT_STRING
				LEAX	TITLE_MSG2,PCR
				LBSR	PUT_STRING
				LEAX	SPACE_MSG,PCR
				LBSR	PUT_STRING
				LBSR	KEY_SPACE
	endif

* �f�B�X�N�t�H�[�}�b�g
				LBSR	DISK_RST
				LBSR	MS_FORMAT

* ���{��T�u���Z�[�u����
MAINSAVE		LBSR	JSUBMON
				LBSR	DICROM

* ����(���b�Z�[�W�\���E���[�v)
				LBSR	JRESET
				LBSR	CLS
				LEAX	COMPLETE_MSG,PCR
				LBSR	PUT_STRING
				BRA		*


*
* ������
*
INIT			LEAX	START,PCR
				LEAX	-$100,X
				TFR		X,D
				TFR		A,DP
* ���[�N�N���A
				CLRA
INIT1			CLR		,X+
				DECA
				BNE		INIT1
* ��ʏ�����
				LEAX	SUBINIT_CMD,PCR
				LDB		#$09
				LBSR	SUB_CMD
				LEAX	SUBCONS_CMD,PCR
				LDB		#$02
				LBSR	SUB_CMD
* ���{��ʐM�J�[�h�`�F�b�N
				LBSR	JSUBCHK
				BCC		INIT2
				LEAX	NOTFOUND_MSG,PCR
				LBSR	PUT_STRING
				BRA		*
INIT2			RTS

*--[ ROM�Z�[�o(���{��T�u�V�X�e��) ]------------------------------------------

*
*	���{��T�u�V�X�e��ROM
*
JSUBMON			LBSR	CLS
				LEAX	JSUBMON_MSG,PCR
				LBSR	PUT_STRING
				LEAX	JSUBMON_FN,PCR
				LDD		#16
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				CLRA
				LDX		#$C000
				LDY		#$4000
				LDU		#$4000
				LBSR	JTFR
				LDX		#$4000
				LDU		#16
				LBSR	FILE_WRITE
				RTS

*
*	����ROM
*
DICROM			LBSR	CLS
				LEAX	DICROM_MSG,PCR
				LBSR	PUT_STRING
				LEAX	DICROM_FN,PCR
				LDD		#$100
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE

* 64�񃋁[�v
				CLRA
DICROM1			PSHS	A
				LDX		#$A000
				LDY		#$4000
				LDU		#$1000
				LBSR	JTFR
				LDX		#$4000
				LDU		#4
				LBSR	FILE_WRITE
				PULS	A
				INCA
				CMPA	#$40
				BCS		DICROM1
				RTS

*--[ ���{��ʐM�J�[�h���䕔 v1.01  coded by RyuTak 2006/07/01 ]---------------

*
*	���{��ʐM�J�[�h�F���`�F�b�N
*
*	out
*		Cflag=1	�J�[�h�����݂��Ȃ�(���Ԃ�)
*
JSUBCHK			LDA		JSYNCR
				BMI		JCHKNOCARD
				ANDCC	#$FE
				RTS
JCHKNOCARD		ORCC	#$01
				RTS

*
*	���{��T�u�V�X�e����~
*
JSUBHALT		PSHS	A
JSUBHALT1		LDA		JSYNCR
				BMI		JSUBHALT1
				CLR		JSYNCW
				PULS	A,PC

*
*	���{��T�u�V�X�e���n��
*
JSUBRUN			PSHS	A
				LDA		#$80
				STA		JSYNCW
				PULS	A,PC

*
*	���{��T�u�V�X�e�����Z�b�g
*
JRESET			PSHS	A
				LBSR	JSUBHALT
				LDA		#$0C
				STA		JRCBW
				LBSR	JSUBRUN
				PULS	A,PC

*
*	���{��T�u�V�X�e���ւ̃R�}���h���M
*
JSENDBLK		CLR		JSYNCW
JSENDBLK1		LDA		,X+
				STA		JRCBW
				DECB
				BNE		JSENDBLK1
				LBSR	JSUBRUN
				RTS

*
*	���{��T�u�V�X�e������̃f�[�^�]��(yamamoto ver.)
*
*	in
*		A = ���{��T�u������ROM�o���N�i���o
*		X = ���{��T�u���]�����A�h���X
*		Y = ���C��CPU ���]����A�h���X
*		U = �]���T�C�Y(�蔲���Ȃ̂�128�o�C�g�P�ʂ�)
*
JTFR			PSHS	D,X,Y,U,CC,DP
				PSHS	A
* YAMAMOTO�R�}���h�g�p�̂��߈�U�_�~�[�R�}���h�𑗐M
				PSHS	X
				LDB		#JDUMMYCMDEND-JDUMMYCMD
				LEAX	JDUMMYCMD,PCR
				LBSR	JSENDBLK
* �]���v���O�����𑗐M
				LDB		#JSENDPRGEND-JSENDPRG
				LEAX	JSENDPRG,PCR
				LBSR	JSENDBLK
				PULS	X
* �]���v�����鎫��ROM�o���N�i���o�E�A�h���X�𑗐M
JTFRLOOP1		LBSR	JSUBHALT
				LDA		,S
				STA		JRCBW
				TFR		X,D
				STA		JRCBW
				STB		JRCBW
				LBSR	JSUBRUN
				LBSR	JSUBHALT
* 128�o�C�g�P�ʂœ]�����s��
				LDB		#128
JTFRLOOP3		LDA		JRCBR
				STA		,Y+
				DECB
				BNE		JTFRLOOP3
				LEAX	128,X
				LEAU	-128,U
				CMPU	#$0000
				BNE		JTFRLOOP1
* �]���X�g�b�v���߂𑗐M
				LEAS	1,S
				LBSR	JSUBHALT
				LDA		#$FF
				STA		JRCBW
				LBSR	JSUBRUN
				LBSR	JSUBHALT
				PULS	D,X,Y,U,CC,DP,PC

*
*	�N�����ɑ��M����_�~�[YAMAMOTO�R�}���h
*	  (��������Ȃ��ƈȌ��YAMAMOTO�R�}���h�����퓮�삵�Ȃ�)
*
JDUMMYCMD		FCB		$80
				FCC		/YAMAMOTO/
				FCB		$00
JDUMMYCMDEND	EQU		*

*
*	�f�[�^�]���p�v���O����
*	 (���{��T�u�V�X�e���� $9E00-�ɓ]�������)
*
JSENDPRG		FCB		$80
				FCC		/YAMAMOTO/
				FCB		$01
				FDB		JSRCBAREA+$14
				FDB		$9E00
				FDB		JSENDPRGEND-JSENDPRG1
				FCB		$03
				FDB		$9E00
				FCB		$00
* ���C��������̃f�[�^���M��҂�
JSENDPRG1		CLR		$9FFF
				LDX		#10
JSENDPRG2		MUL
				LEAX	-1,X
				BNE		JSENDPRG2
* ��M�f�[�^��$FF�Ȃ�]���X�g�b�v����
* ��M�f�[�^��$FF�ȊO�Ȃ炻�̃o���N(����5bit)�̃f�[�^��128�o�C�g�]��
* (����ROM�ȊO�̗̈�ł̓o���N���͊֌W�Ȃ�)
				LDA		JSRCBAREA
				CMPA	#$FF
				BEQ		JSENDPRG4
				ORA		#$80
				STA		$9FFF
				LDX		JSRCBAREA+1
				LDY		#JSRCBAREA
				LDB		#128
JSENDPRG3		LDA		,X+
				STA		,Y+
				DECB
				BNE		JSENDPRG3
				BRA		JSENDPRG1
* YAMAMOTO�R�}���h�����A�h���X�𒲐����Ď����A�N�Z�X���[�`���ɔ��
JSENDPRG4		LDX		#JSRCBAREA+9
				STX		$800C
				JMP		[$FFFE]
JSENDPRGEND		EQU		*

*--[ FM-7 ]-------------------------------------------------------------------

*
*	FM-7������ (77�`�F�b�N�t��)
*
FM7_INIT		TST		BANKSEL
				LDD		#$0201
				STA		INITDIS
* �u�[�gROM�̗�RAM���g�p�ł��邩�m�F (�g�p�ł���Ȃ�77)
				STB		MMSR
				LDB		$FE02
				CLR		$FE02
				LDA		$FE02
				STA		<FM77_FLAG
				STB		$FE02
				CLR		MMSR
				RTS

*
*	�X�y�[�X�҂�
*
KEY_SPACE		EQU		*
KEY_SPACE1		LEAX	INKEY_CMD,PCR
				LDB		#2
				LBSR	SUB_CMD
				LBSR	SUB_HALT
				LDB		#$80
				STB		SHARED
				LDD		SHARED+3
				CLR		SUBSIN
				TSTB
				BEQ		KEY_SPACE1
				CMPA	#$20
				BNE		KEY_SPACE1
				RTS

*
*	�X�y�[�X���̓`�F�b�N
*
KEY_CHKSP		LEAX	INKEY_CHK_CMD,PCR
				LDB		#2
				LBSR	SUB_CMD
				LBSR	SUB_HALT
				LDB		#$80
				STB		SHARED
				LDD		SHARED+3
				CLR		SUBSIN
				TSTB
				BEQ		KEY_CHKSP1
				CMPA	#$20
				BNE		KEY_CHKSP1
				FCB		$86
KEY_CHKSP1		CLRA
				RTS

*
*	��ʃN���A
*
CLS				LEAX	CLS_CMD,PCR
				LDB		#CLS_CMDe-CLS_CMD
				LBRA	SUB_CMD

*
*	�T�uCPU ��ʃN���A�v���O����
*
CLS_CMD			FCB		$3F
				FCC		/YAMAUCHI/
				FCB		$93,$D3,$8F,$90
*
CLSENT			LDU		#$0000+80*200
				BSR		CLS1
				LDU		#$4000+80*200
				BSR		CLS1
				LDU		#$8000+80*200
CLS1			TST		VACC
				LDB		#100
				PSHS	B
				LDX		#0
				TFR		X,D
				TFR		X,Y
CLS2			PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D,X,Y
				PSHU	D
				DEC		,S
				BNE		CLS2
				CLR		VACC
				PULS	B,PC
CLS_CMDe		EQU		*

*--[ �T�uCPU ]----------------------------------------------------------------

*
*	�T�uCPU HALT
*
*	USEREG: A
*
SUB_HALT		LDA		SUBSIN
				BMI		SUB_HALT
				LDA		#$80
				STA		SUBSIN
SUB_HALT1		LDA		SUBSIN
				BPL		SUB_HALT1
				RTS

*
*	�T�uCPU �R�}���h�o��
*
*	PARAM.: X	�R�}���h�f�[�^($FC82�`)
*			B	���M����
*	USEREG: A,B,X,U
*
SUB_CMD			LBSR	SUB_HALT
* �f�[�^��������
				LDU		#SHARED
				CLR		,U+
				CLR		,U+
SUB_CMD1		LDA		,X+
				STA		,U+
				DECB
				BNE		SUB_CMD1
* �T�uCPU ���s
				CLR		SUBSIN
				RTS

*
*	������o�� (FM-7�p,ANK��p)
*
*	PARAM.: X	������
*	USEREG: A,B,X,U
*
PUT_STRING		EQU		*
PRINT			TFR		X,U
* �������𐔂���
				CLRB
PRINT0			LEAU	BUFFER,PCR
				LDA		#$03
				CLRB
				STA		,U+
				STB		,U+
PRINT1			LDA		,X+
				TSTA
				BEQ		PRINT2
				STA		,U+
				INCB
				BRA		PRINT1
PRINT2			PSHS	A,X
				LEAX	BUFFER,PCR
				STB		1,X
				INCB
				INCB
				LBSR	SUB_CMD
				PULS	A,X
				RTS

*--[ DOS ]--------------------------------------------------------------------

*
*	�f�B�X�N���Z�b�g
*
DISK_RST		LBSR	CLS
				LDA		#$10		* �_���h���C�u�ƕ����h���C�u�𓯈�ɐݒ�
				STA		FDDMODE		* (�h���C�u0)
				LDA		#$15		* ����
				STA		FDDMODE		* (�h���C�u1)
	if	(DISKTYPE==D_NORM2DD)
				CLR		FDDMODE
	else
				LDA		#$40
				STA		FDDMODE
	endif
				LDA		#$80
				STA		FDDDRIVE
				CLR		<DRIVE
DISK_RST1		LBSR	RESTORE
				BCC		DISK_RST2
				LBSR	DISK_ERR
				BRA		DISK_RST1
DISK_RST2		RTS

*
*	MS-DOS 360KB/720KB�f�B�X�N
*	�t�H�[�}�b�g����э쐬
*
MS_FORMAT		LBSR	CLS
				LEAX	FORMAT_MSG,PCR
				LBSR	PUT_STRING
				CLR		<DRIVE

	if	(DISKTYPE==D_NORM2D)||(DISKTYPE==D_SKIP2DD)||(defined(FBLOADM))
				CLR		<TRACK
* �t�H�[�}�b�g
MS_FORMAT1		CLR		<SIDE
MS_FORMAT2		LBSR	FORMAT
				BCC		MS_FORMAT3
				LBSR	DISK_ERR
				BRA		MS_FORMAT2
* ���̃T�C�h
MS_FORMAT3		INC		<SIDE
				LDA		<SIDE
				CMPA	#$02
				BCS		MS_FORMAT2
* ���̃g���b�N
				INC		<TRACK
				LDA		<TRACK
*	if	(DISKTYPE==D_NORM2DD)
*				CMPA	#80
*	else
				CMPA	#40
*	endif
				BCS		MS_FORMAT1
	endif

* �g���b�N0
				LBSR	MS_T0MAKE
				CLR		<TRACK
				CLR		<SIDE
MS_FORMAT4		LEAX	DISK_BUF,PCR
				LDA		#$01
				STA		<SECTOR
				LDA		#$09
				STA		<SECTORS
				LBSR	WRITEM
				BCC		MS_FORMAT5
				LBSR	DISK_ERR
				BRA		MS_FORMAT4
* �g���b�N1
MS_FORMAT5		LBSR	MS_T1MAKE
				INC	<SIDE
MS_FORMAT6		LEAX	DISK_BUF,PCR
				LDA		#$01
				STA		<SECTOR
				LDA		#$03
				STA		<SECTORS
				LBSR	WRITEM
				BCC		MS_FORMAT7
				LBSR	DISK_ERR
				BRA		MS_FORMAT6
MS_FORMAT7		RTS

*
*	�g���b�N0 �f�[�^�쐬
*
MS_T0MAKE		LEAX	DISK_BUF,PCR
	if	(DISKTYPE==D_NORM2D)
				LEAU	DPB_360,PCR
	else
				LEAU	DPB_720,PCR
	endif
				LDB		#64
MS_T0MAKE1		LDA		,U+
				STA		,X+
				DECB
				BNE		MS_T0MAKE1
				LDB		#192
MS_T0MAKE2		CLR		,X+
				DECB
				BNE		MS_T0MAKE2
MS_T0MAKE3		CLR		,X+
				DECB
				BNE		MS_T0MAKE3
* FAT
				LBSR	MS_MAKE_FAT
				LBSR	MS_MAKE_FAT
* �f�B���N�g��
	if	(DISKTYPE==D_NORM2D)
				LDB		#$40
	else
				LDB		#32
	endif
				PSHS	B
MS_T0MAKE4		LBSR	MS_MAKE_DIR
				DEC		,S
				BNE		MS_T0MAKE4
				PULS	B,PC

*
*	�g���b�N�P �f�[�^�쐬
*
MS_T1MAKE		LEAX	DISK_BUF,PCR
	if	(DISKTYPE==D_NORM2D)
				LDB		#48
	else
				LDB		#80
	endif
				PSHS	B
MS_T1MAKE1		LBSR	MS_MAKE_DIR
				DEC		,S
				BNE		MS_T1MAKE1
				PULS	B,PC

	if	(DISKTYPE==D_NORM2D)

*
*	FAT�쐬(2D)
*
*	PARAM.: X	�f�[�^�o�b�t�@
*	USEREG: A,B
*
MS_MAKE_FAT		LDA		#$FD
				STA		,X+
				LDA		#$FF
				STA		,X+
				STA		,X+
				LDB		#$FD
				CLRA
MS_MAKE_FAT1	STA		,X+
				DECB
				BNE		MS_MAKE_FAT1
MS_MAKE_FAT2	STA		,X+
				DECB
				BNE		MS_MAKE_FAT2
MS_MAKE_FAT3	STA		,X+
				DECB
				BNE		MS_MAKE_FAT3
MS_MAKE_FAT4	STA		,X+
				DECB
				BNE		MS_MAKE_FAT4
				RTS

	elsif	(DISKTYPE==D_SKIP2DD)

*
*	FAT�쐬(�g���b�N����2DD)
*
*	PARAM.: X	�f�[�^�o�b�t�@
*	USEREG: A,B
*
MS_MAKE_FAT	TFR	X,U
* FAT�I���A�h���X���X�^�b�N�Ƀv�b�V��
				LEAY	$600,X
				PSHS	Y
* FAT�f�[�^�o�b�t�@��������
MS_MAKE_FAT1	CLR		,U+
				CMPU	,S
				BNE		MS_MAKE_FAT1
		 		LDD		#$F9FF
				STA		,X
				STB		1,X
				STB		2,X
* ��V�����_��s�ǃN���X�^�Ƃ���FAT���쐬
				LEAS	-2,S
				LDD		#0
				LDU		#2
				STU		,S
MS_MAKE_FAT2	LBSR	SET_FAT
				LEAU	1,U
				DEC		1,S
				BNE		MS_MAKE_FAT2
MS_MAKE_FAT3	LDY		#9
				STY		,S
				EORA	#$0F
				EORB	#$F7
MS_MAKE_FAT4	LBSR	SET_FAT
				LEAU	1,U
				CMPU	#715
				BHS		MS_MAKE_FAT5
				DEC		1,S
				BNE		MS_MAKE_FAT4
				BRA		MS_MAKE_FAT3
MS_MAKE_FAT5	LEAS	2,S
				PULS	X
				RTS

	else

*
*	FAT�쐬(2DD)
*
*	PARAM.: X	�f�[�^�o�b�t�@
*	USEREG: A,B
*
MS_MAKE_FAT		LDA		#$F9
				STA		,X+
				LDA		#$FF
				STA		,X+
				STA		,X+
				LDB		#$FD
MS_MAKE_FAT1	CLR		,X+
				DECB
				BNE		MS_MAKE_FAT1
MS_MAKE_FAT2	CLR		,X+
				DECB
				BNE		MS_MAKE_FAT2
MS_MAKE_FAT3	CLR		,X+
				DECB
				BNE		MS_MAKE_FAT3
MS_MAKE_FAT4	CLR		,X+
				DECB
				BNE		MS_MAKE_FAT4
MS_MAKE_FAT5	CLR		,X+
				DECB
				BNE		MS_MAKE_FAT5
MS_MAKE_FAT6	CLR		,X+
				DECB
				BNE		MS_MAKE_FAT6
				RTS

	endif

*
*	�k���f�B���N�g���쐬
*
*	PARAM.: X	�f�[�^�o�b�t�@
*	USEREG: A,B
*
MS_MAKE_DIR		CLR		,X+
				LDB		#$1F
				LDA		#$E5
MS_MAKE_DIR1	STA		,X+
				DECB
				BNE		MS_MAKE_DIR1
				RTS

*
*	�t�@�C���쐬
*
*	PARAM.: X	�t�@�C���l�[��(8+3)
*	CLUSTERS,LASTBYTE�͗\�߃Z�b�g���Ă���
*
FILE_CREATE		PSHS	X
				CLR	<TRACK
				CLR	<SIDE
* �擪�g���b�N�ǂݍ���
FILE_CREATE1	LDA		#$01
				STA		<SECTOR
				LDA		#$09
				STA		<SECTORS
				LEAX	DISK_BUF,PCR
				LBSR	READM
				BCC		FILE_CREATE2
				LBSR	DISK_ERR
				BRA		FILE_CREATE1
* �f�[�^�ݒ�
FILE_CREATE2	LEAX	DISK_BUF+$200,PCR
				LBSR	MAKE_FAT
	if	(DISKTYPE==D_NORM2D)
				LEAX	DISK_BUF+$A00,PCR
	else
				LEAX	DISK_BUF+$E00,PCR
	endif
				PULS	U
				LBSR	MAKE_DIR
* �擪�g���b�N��������
FILE_CREATE3	LDA		#$01
				STA		<SECTOR
				LDA		#$09
				STA		<SECTORS
				LEAX	DISK_BUF,PCR
				LBSR	WRITEM
				BCC		FILE_CREATE4
				LBSR	DISK_ERR
				BRA		FILE_CREATE3
* C,H,R���쐬���Ă���
FILE_CREATE4	LDD		<CLUSTER
				LBSR	CLUS_TO_CHR
				RTS

*
*	FAT�ݒ�
*
*	PARAM.: X	��1FAT�擪�A�h���X
*	USEREG: A,B,X,Y,U
*
MAKE_FAT		LDU		#$0002
* �󂢂Ă���N���X�^���T�[�`����
MAKE_FAT1		LBSR	GET_FAT
				CMPD	#$0000
				BEQ		MAKE_FAT2
				LEAU	1,U
				BRA		MAKE_FAT1
* �擪FAT�ԍ��m��
MAKE_FAT2		STU		<CLUSTER
				LDD		<CLUSTERS
				CMPD	#$0001
				BEQ		MAKE_FAT4
				TFR		D,Y
				LEAY	-1,Y
* ���̋󂫃N���X�^���T�[�`����
MAKE_FAT3A		PSHS	U
				LEAU	1,U
MAKE_FAT30		LBSR	GET_FAT
				CMPD	#$0000
				BEQ		MAKE_FAT31
				LEAU	1,U
				BRA		MAKE_FAT30
MAKE_FAT31		TFR		U,D
				PULS	U
* ���������󂫃N���X�^��ݒ肷��
MAKE_FAT3		BSR		SET_FAT
				TFR		D,U
				LEAY	-1,Y
				BNE		MAKE_FAT3A
* �ŏI�N���X�^�́A$FFF���Z�b�g
MAKE_FAT4		LDD		#$0FFF
				BSR		SET_FAT
				RTS

*
*	FAT�f�[�^�擾
*
*	PARAM.: X	FAT�擪�A�h���X
*			U	�N���X�^�ԍ�
*	RESULT: D	FAT�f�[�^(12bit)
*
GET_FAT			TFR		U,D
				LSRA
				RORB
				BCS		GET_FAT_ODD
* �����p�^�[��
GET_FAT_EVEN	PSHS	D
				ADDD	,S
				ADDD	,S
				LEAS	2,S
				LDD		D,X
				EXG		A,B
				ANDA	#$0F
				RTS
* ��p�^�[��
GET_FAT_ODD		PSHS	D
				ADDD	,S
				ADDD	,S
				LEAS	2,S
				ADDD	#1
				LDD		D,X
				EXG		A,B
				LSRA
				RORB
				LSRA
				RORB
				LSRA
				RORB
				LSRA
				RORB
				RTS

*
*	FAT�f�[�^�Z�b�g
*
*	PARAM.: X	��1FAT�擪�A�h���X
*			U	�N���X�^�ԍ�
*			D	FAT�f�[�^(12bit)
*
SET_FAT			PSHS	D,X
				PSHS	D
				TFR		U,D
				LSRA
				RORB
				BCS		SET_FAT_ODD
* �����p�^�[��
SET_FAT_EVEN	PSHS	D
				ADDD	,S
				ADDD	,S
				LEAS	2,S
				LEAX	D,X
* ��ɑ|�����Ă���
				LDB		1,X
				ANDB	#$F0
				STB		1,X
* �X�^�b�N������o���āAOR
				PULS	D
				EXG		A,B
				STA		,X
				ORB		1,X
				STB		1,X
* ��2FAT
	if	(DISKTYPE==D_NORM2D)
				STD		$400,X
	else
				STD		$600,X
	endif
				PULS	D,X,PC
* ��p�^�[��
SET_FAT_ODD	PSHS	D
				ADDD	,S
				ADDD	,S
				LEAS	2,S
				ADDD	#1
				LEAX	D,X
* ��ɑ|�����Ă���
				LDA		,X
				ANDA	#$0F
				STA		,X
* �X�^�b�N������o���āAOR
				PULS	D
				LSLB
				ROLA
				LSLB
				ROLA
				LSLB
				ROLA
				LSLB
				ROLA
				EXG		A,B
				ORA		,X
				STA		,X
				STB		1,X
* ��2FAT
	if	(DISKTYPE==D_NORM2D)
				STD		$400,X
	else
				STD		$600,X
	endif
				PULS	D,X,PC

*
*	�f�B���N�g���쐬
*
*	PARAM.: X	�f�B���N�g���G���A�擪�A�h���X
*			U	�t�@�C���l�[��(8+3)
*	USEREG: A,B,X,U
*
MAKE_DIR		LDA		,X
				BEQ		MAKE_DIR1
				CMPA	#$E5
				BEQ		MAKE_DIR1
				LEAX	32,X
				BRA		MAKE_DIR
* �t�@�C���l�[�����R�s�[
MAKE_DIR1		LDB		#11
MAKE_DIR2		LDA		,U+
				STA		,X+
				DECB
				BNE		MAKE_DIR2
* �A�g���r���[�g
				LDA		#$20
				STA		,X+
* ���g�p�̈�̓N���A
				LDB		#10
MAKE_DIR3		CLR		,X+
				DECB
				BNE		MAKE_DIR3
* ����
				LDD		#$0060
				STD		,X++
				LDD		#$1427
				STD		,X++
* �擪�N���X�^
				LDD		<CLUSTER
				EXG		A,B
				STD		,X++
* �t�@�C���T�C�Y
				CLR		,X
				CLR		1,X
				CLR		2,X
				CLR		3,X
				LDD		<CLUSTERS
				LDU		<LASTBYTE
				CMPU	#$0000
				BEQ		MAKE_DIR4
				SUBD	#$0001
MAKE_DIR4	LSLB
				ROLA
				LSLB
				ROLA
				EXG		A,B
				STD		1,X
* �]��o�C�g����
				LDD		<LASTBYTE
				STB		,X
				ADDA	1,X
				BCS		MAKE_DIR5
				STA		1,X
				RTS
MAKE_DIR5		STA		1,X
				INC		2,X
				RTS

*
*	�N���X�^�ԍ����J�����gC,H,R
*
*	PARAM.: D	�N���X�^�ԍ�
*
CLUS_TO_CHR		PSHS	X
				TFR		D,X
		if	(DISKTYPE>=D_SKIP2DD)
				LEAX	1,X
		endif
				CLR		<C_TRACK
				CLR		<C_SIDE
				LDA		#$09
				STA		<C_SECTOR
* ���[�v
CLUS_TO_CHR1	LDA		<C_SECTOR
				INCA
				INCA
				STA		<C_SECTOR
				CMPA	#$0A
				BCS		CLUS_TO_CHR2
* ���̃T�C�h
				SUBA	#$09
				STA		<C_SECTOR
				INC		<C_SIDE
				LDA		<C_SIDE
				CMPA	#$01
				BEQ		CLUS_TO_CHR2
* ���̃g���b�N
				CLR		<C_SIDE
				INC		<C_TRACK
* NEXT
CLUS_TO_CHR2	LEAX	-1,X
				BNE		CLUS_TO_CHR1
	if	(DISKTYPE==D_SKIP2DD)
				LSR		<C_TRACK		�U2DD�p�g���b�N�ԍ��␳
	endif
				PULS	X,PC

*
*	�t�@�C����������
*
*	PARAM.: X	�o�b�t�@�A�h���X
*			U	�������ݍő�T�C�Y(KB)
*
FILE_WRITE		CMPU	<CLUSTERS
				BCS		FILE_WRITE1
				LDU		<CLUSTERS
* U=���񏑂����ރN���X�^��
FILE_WRITE1		PSHS	U
				LDD		<CLUSTERS
				SUBD	,S
				STD		<CLUSTERS
				LDD		,S
				ADDD	,S
				STD		,S
* ,S=���񏑂����ރZ�N�^��
FILE_WRITE2		LDB		#10
				SUBB	<C_SECTOR
				CLRA
				CMPD	,S
				BCS		FILE_WRITE4
				LDB		1,S
* B=���̃g���b�N�ŏ����Z�N�^
FILE_WRITE4		STB		<C_SECTORS
				LDA		<C_TRACK
				STA		<TRACK
				LDA		<C_SIDE
				STA		<SIDE
				LDA		<C_SECTOR
				STA		<SECTOR
				LDA		<C_SECTORS
				STA		<SECTORS
				PSHS	X
				LBSR	WRITEM
				TSTA
				BCC		FILE_WRITE5
* �������݃G���[
				LBSR	DISK_ERR
				PULS	X
				BRA		FILE_WRITE2
* NEXT
FILE_WRITE5		LEAS	2,S
				CLRA
				LDB		<C_SECTORS
				PSHS	D
				LDD		2,S
				SUBD	,S++
				STD		,S
* C,H,R
				LDD		<C_SIDE
				ADDB	<C_SECTORS
				CMPB	#$0A
				BCS		FILE_WRITE6
				LDB		#$01
				INCA
				CMPA	#$01
				BEQ		FILE_WRITE6
				CLRA
				INC		<C_TRACK
* �I���`�F�b�N
FILE_WRITE6		STD		<C_SIDE
				LDD		,S
				BNE		FILE_WRITE2
				PULS	U,PC

*
*	�f�B�X�N�G���[
*
*	PARAM.: A	FDC�X�e�[�^�X
*	USEREG: A,B,X,U
*
DISK_ERR		LEAX	NOTREADY_MSG,PCR
				BITA	#$80
				BNE		DISK_ERR1
				LEAX	WRPROTECT_MSG,PCR
				BITA	#$40
				BNE		DISK_ERR1
				LDA		<TYPE
				LEAX	SEEKERR_MSG,PCR
				TSTA
				BEQ		DISK_ERR1
				LEAX	READERR_MSG,PCR
				DECA
				BEQ		DISK_ERR1
				LEAX	WRITEERR_MSG,PCR
* �\��
DISK_ERR1		LBSR	PUT_STRING
				LEAX	SPACE_MSG,PCR
				LBSR	PUT_STRING
* �L�[�҂�
				LBSR	KEY_SPACE
* �I��
				LEAX	CLEAR_MSG,PCR
				LBSR	PUT_STRING
				LBSR	RESTORE
				RTS

*--[ FDC�h���C�o ]------------------------------------------------------------

*
*	FDC ���X�g�A
*
*	RESULT: A	Type1 �X�e�[�^�X
*			CY	�G���[�t���O
*
RESTORE			LDA		<DRIVE
				ORA		#$80
				STA		FDDDRIVE
* BUSY�҂�
RESTORE1		LDA		FDCSTAT
				LSRA
				BCS		RESTORE1
* ���X�g�A�R�}���h��������
				CLR		<TYPE
				LDA		#$08
				STA		FDCCMD
* �I���҂�
RESTORE2		LDA		FDDDRQ
				BITA	#$40
				BEQ		RESTORE2
* �X�e�[�^�X
				LDA		FDCSTAT
				ANDA	#$D8
				BEQ		RESTORE3
				ORCC	#$01
				RTS
* �g���b�N0���L��
RESTORE3		TST		<DRIVE
				BNE		RESTORE4
				CLR		<DRIVE0_T
				RTS
RESTORE4		CLR		<DRIVE1_T
				RTS

*
*	FDC �V�[�N
*
*	RESULT: A	Type1 �X�e�[�^�X
*			CY	�G���[�t���O
*	USEREG: B
*
SEEK			LDA		<DRIVE
				ORA		#$80
				STA		FDDDRIVE
* �g���b�N���W�X�^�֌��݂̃g���b�N����������
				TST		<DRIVE
				BNE		SEEK1
				LDA		<DRIVE0_T
				STA		FDCTRACK
				BRA		SEEK2
SEEK1			LDA		<DRIVE1_T
				STA		FDCTRACK
* BUSY�҂�
SEEK2			LDA		FDCSTAT
				LSRA
				BCS		SEEK2
* �V�[�N�R�}���h��������
				LDA		<TRACK
				STA		FDCDATA
				CLR		<TYPE
				LDA		#$18
				STA		FDCCMD
* �I���҂�
SEEK3			LDA		FDDDRQ
				BITA	#$40
				BEQ		SEEK3
* �X�e�[�^�X
				LDA		FDCSTAT
				ANDA	#$98
				BEQ		SEEK4
				ORCC	#$01
				RTS
* �g���b�N�Z�[�u
SEEK4			TST		<DRIVE
				BNE		SEEK5
				LDB		<TRACK
				STB		<DRIVE0_T
				ANDCC	#$FE
				RTS
SEEK5			LDB		<TRACK
				STB		<DRIVE1_T
				ANDCC	#$FE
				RTS

	if	(DISKTYPE==D_NORM2D)

*
*	FDC �t�H�[�}�b�g(2D)
*
*	RESULT: A	Type3 �X�e�[�^�X
*			CY	�G���[�t���O
*
FORMAT			LBSR	SEEK
				BCC		FORMAT1
				RTS
FORMAT1			LBSR	FORMAT_MAKE
				LEAU	DISK_BUF,PCR
				LDA		<TRACK
				STA		FDCTRACK
				LDA		#$01
				STA		FDCSECTOR
				LDA		<SIDE
				STA		FDDSIDE
* BUSY�҂�
FORMAT2			LDA		FDCSTAT
				LSRA
				BCS		FORMAT2
* �R�}���h���s
				LDA		#$02
				STA		<TYPE
				LDA		#$F0
				STA		FDCCMD
* �f�[�^�]��
FORMAT3			LDA		FDDDRQ
				BPL		FORMAT4
				LDA		,U+
				STA		FDCDATA
				BRA		FORMAT3
FORMAT4			BITA	#$40
				BEQ		FORMAT3
* �]���I��
				LDA		FDCSTAT
				ANDA	#$E8
				BEQ		FORMAT5
				ORCC	#$01
				RTS
FORMAT5			CLRA
				RTS

*
*	FDC�t�H�[�}�b�g �f�[�^�쐬
*
*	USEREG: A,B,U
*
FORMAT_MAKE		LEAU	DISK_BUF,PCR
				LBSR	FM_PRE
				LDA		#$01
				STA		<SECTOR
FORMAT_MAKE1	LBSR	FM_ID
				LBSR	FM_DATA
				INC		<SECTOR
				LDA		<SECTOR
				CMPA	#$0A
				BCS		FORMAT_MAKE1
				LBSR	FM_POST
				RTS

*
*	FDC�t�H�[�}�b�g �v���A���u���쐬
*
*	USEREG: A,B,U
*
FM_PRE			LDD		#$4E50
				LBSR	FM_SUB
				LDD		#$000C
				LBSR	FM_SUB
* INDEX�}�[�N
				LDA		#$F6
				STA		,U+
				STA		,U+
				STA		,U+
				LDA		#$FC
				STA		,U+
* GAP1
				LDD		#$4E32
				LBSR	FM_SUB
				RTS

*
*	FDC�t�H�[�}�b�g ID�쐬
*
*	USEREG: A,B,U
*
FM_ID			LDD		#$000C
				LBSR	FM_SUB
* ID�}�[�N
				LDA		#$F5
				STA		,U+
				STA		,U+
				STA		,U+
				LDA		#$FE
				STA		,U+
* C,H,R,N
				LDA		<TRACK
				STA		,U+
				LDA		<SIDE
				STA		,U+
				LDA		<SECTOR
				STA		,U+
				LDA		#$02
				STA		,U+
* CRC
				LDA		#$F7
				STA		,U+
* GAP2
				LDD		#$4E16
				LBSR	FM_SUB
				RTS

*
*	FDC�t�H�[�}�b�g �f�[�^���쐬
*
*	USEREG: A,B,U
*
FM_DATA			LDD		#$000C
				LBSR	FM_SUB
* �f�[�^�}�[�N
				LDA		#$F5
				STA		,U+
				STA		,U+
				STA		,U+
				LDA		#$FB
				STA		,U+
* �f�[�^
				LDD		#$0000
				LBSR	FM_SUB
				LDD		#$0000
				LBSR	FM_SUB
* CRC
				LDA		#$F7
				STA		,U+
* GAP3
				LDD		#$4E54
				LBSR	FM_SUB
				RTS

*
*	FDC�t�H�[�}�b�g �|�X�g�A���u�����쐬
*
*	USEREG: A,B,U
*
FM_POST			LDD		#$4E00
				LBSR	FM_SUB
				LDD		#$4E90
				LBSR	FM_SUB
				RTS

*
*	FDC�t�H�[�}�b�g �T�u
*
*	PARAM.: A	�������݃f�[�^
*			B	�J��Ԃ���
*
FM_SUB			STA		,U+
				DECB
				BNE		FM_SUB
				RTS

*	elsif	(defined(FORMAT2DD)||defined(FBLOADM))&&(DISKTYPE==D_SKIP2DD)
	elsif	(DISKTYPE==D_SKIP2DD)

*
*	FDC �t�H�[�}�b�g (�g���b�N����2DD)
*
*	RESULT: A	Type3 �X�e�[�^�X
*			CY	�G���[�t���O
*
FORMAT			LBSR	SEEK
				BCC		FORMAT1
				RTS
FORMAT1			LBSR	FORMAT_MAKE
				LEAU	DISK_BUF,PCR
				LDA		<TRACK
				STA		FDCTRACK
				LDA		#$01
				STA		FDCSECTOR
				LDA		<SIDE
				STA		FDDSIDE
* BUSY�҂�
FORMAT2			LDA		FDCSTAT
				LSRA
				BCS		FORMAT2
* �R�}���h���s
				LDA		#$02
				STA		<TYPE
				LDA		#$F0
				STA		FDCCMD
* �f�[�^�]��
FORMAT3			LDA		FDDDRQ
				BPL		FORMAT4
				LDA		,U+
				STA		FDCDATA
				BRA		FORMAT3
FORMAT4			BITA	#$40
				BEQ		FORMAT3
* �]���I��
				LDA		FDCSTAT
				ANDA	#$E8
				BEQ		FORMAT5
				ORCC	#$01
				RTS
FORMAT5			CLRA
				RTS

*
*	FDC�t�H�[�}�b�g �f�[�^�쐬
*
*	USEREG: A,B,U
*
FORMAT_MAKE		LEAU	DISK_BUF,PCR
				LBSR	FM_PRE
				LDA		#$01
				STA		<SECTOR
FORMAT_MAKE1	LBSR	FM_ID
				LBSR	FM_DATA
				INC		<SECTOR
				LDA		<SECTOR
				CMPA	#$0A
				BCS		FORMAT_MAKE1
				LBSR	FM_POST
				RTS

*
*	FDC�t�H�[�}�b�g �v���A���u���쐬
*
*	USEREG: A,B,U
*
FM_PRE			LDD		#$4E50
				LBSR	FM_SUB
				LDD		#$000C
				LBSR	FM_SUB
* INDEX�}�[�N
				LDA		#$F6
				STA		,U+
				STA		,U+
				STA		,U+
				LDA		#$FC
				STA		,U+
* GAP1
				LDD		#$4E32
				LBSR	FM_SUB
				RTS

*
*	FDC�t�H�[�}�b�g ID�쐬
*
*	USEREG: A,B,U
*
FM_ID			LDD		#$000C
				LBSR	FM_SUB
* ID�}�[�N
				LDA		#$F5
				STA		,U+
				STA		,U+
				STA		,U+
				LDA		#$FE
				STA		,U+
* C,H,R,N
				LDA		<TRACK
				LSLA		�U2DD�p
				STA		,U+
				LDA		<SIDE
				STA		,U+
				LDA		<SECTOR
				STA		,U+
				LDA		#$02
				STA		,U+
* CRC
				LDA		#$F7
				STA		,U+
* GAP2
				LDD		#$4E16
				LBSR	FM_SUB
				RTS

*
*	FDC�t�H�[�}�b�g �f�[�^���쐬
*
*	USEREG: A,B,U
*
FM_DATA			LDD		#$000C
				LBSR	FM_SUB
* �f�[�^�}�[�N
				LDA		#$F5
				STA		,U+
				STA		,U+
				STA		,U+
				LDA		#$FB
				STA		,U+
* �f�[�^
				LDD		#$0000
				LBSR	FM_SUB
				LDD		#$0000
				LBSR	FM_SUB
* CRC
				LDA		#$F7
				STA		,U+
* GAP3
				LDD		#$4E54
				LBSR	FM_SUB
				RTS

*
*	FDC�t�H�[�}�b�g �|�X�g�A���u�����쐬
*
*	USEREG: A,B,U
*
FM_POST			LDD		#$4E00
				LBSR	FM_SUB
				LDD		#$4E90
				LBSR	FM_SUB
				RTS

*
*	FDC�t�H�[�}�b�g �T�u
*
*	PARAM.: A	�������݃f�[�^
*			B	�J��Ԃ���
*
FM_SUB			STA		,U+
				DECB
				BNE		FM_SUB
				RTS

	elsif	(defined(FBLOADM)||defined(FORMAT2DD))&&(DISKTYPE==D_NORM2DD)

*
*	FDC �t�H�[�}�b�g (2DD)
*
*	RESULT: A	Type3 �X�e�[�^�X
*			CY	�G���[�t���O
*
FORMAT			LBSR	SEEK
				BCC		FORMAT1
				RTS
FORMAT1			LBSR	FORMAT_MAKE
				LEAU	DISK_BUF,PCR
				LDA		<TRACK
				STA		FDCTRACK
				LDA		#$01
				STA		FDCSECTOR
				LDA		<SIDE
				STA		FDDSIDE
* BUSY�҂�
FORMAT2			LDA		FDCSTAT
				LSRA
				BCS		FORMAT2
* �R�}���h���s
				LDA		#$02
				STA		<TYPE
				LDA		#$F0
				STA		FDCCMD
* �f�[�^�]��
FORMAT3			LDA		FDDDRQ
				BPL		FORMAT4
				LDA		,U+
				STA		FDCDATA
				BRA		FORMAT3
FORMAT4			BITA	#$40
				BEQ		FORMAT3
* �]���I��
				LDA		FDCSTAT
				ANDA	#$E8
				BEQ		FORMAT5
				ORCC	#$01
				RTS
FORMAT5			CLRA
				RTS

*
*	FDC�t�H�[�}�b�g �f�[�^�쐬
*
*	USEREG: A,B,U
*
FORMAT_MAKE		LEAU	DISK_BUF,PCR
				LBSR	FM_PRE
				LDA		#$01
				STA		<SECTOR
FORMAT_MAKE1	LBSR	FM_ID
				LBSR	FM_DATA
				INC		<SECTOR
				LDA		<SECTOR
				CMPA	#$0A
				BCS		FORMAT_MAKE1
				LBSR	FM_POST
				RTS

*
*	FDC�t�H�[�}�b�g �v���A���u���쐬
*
*	USEREG: A,B,U
*
FM_PRE			LDD		#$4E50
				LBSR	FM_SUB
				LDD		#$000C
				LBSR	FM_SUB
* INDEX�}�[�N
				LDA		#$F6
				STA		,U+
				STA		,U+
				STA		,U+
				LDA		#$FC
				STA		,U+
* GAP1
				LDD		#$4E32
				LBSR	FM_SUB
				RTS

*
*	FDC�t�H�[�}�b�g ID�쐬
*
*	USEREG: A,B,U
*
FM_ID			LDD		#$000C
				LBSR	FM_SUB
* ID�}�[�N
				LDA		#$F5
				STA		,U+
				STA		,U+
				STA		,U+
				LDA		#$FE
				STA		,U+
* C,H,R,N
				LDA		<TRACK
				STA		,U+
				LDA		<SIDE
				STA		,U+
				LDA		<SECTOR
				STA		,U+
				LDA		#$02
				STA		,U+
* CRC
				LDA		#$F7
				STA		,U+
* GAP2
				LDD		#$4E16
				LBSR	FM_SUB
				RTS

*
*	FDC�t�H�[�}�b�g �f�[�^���쐬
*
*	USEREG: A,B,U
*
FM_DATA			LDD		#$000C
				LBSR	FM_SUB
* �f�[�^�}�[�N
				LDA		#$F5
				STA		,U+
				STA		,U+
				STA		,U+
				LDA		#$FB
				STA		,U+
* �f�[�^
				LDD		#$0000
				LBSR	FM_SUB
				LDD		#$0000
				LBSR	FM_SUB
* CRC
				LDA		#$F7
				STA		,U+
* GAP3
				LDD		#$4E54
				LBSR	FM_SUB
				RTS

*
*	FDC�t�H�[�}�b�g �|�X�g�A���u�����쐬
*
*	USEREG: A,B,U
*
FM_POST			LDD		#$4E00
				LBSR	FM_SUB
				LDD		#$4E90
				LBSR	FM_SUB
				RTS

*
*	FDC�t�H�[�}�b�g �T�u
*
*	PARAM.: A	�������݃f�[�^
*			B	�J��Ԃ���
*
FM_SUB			STA		,U+
				DECB
				BNE		FM_SUB
				RTS

	endif

*
*	FDC �Z�N�^�ǂݍ���(�Z�N�^�A��)
*
*	PARAM.: X	�f�[�^�o�b�t�@
*	RESULT: A	Type2 �X�e�[�^�X
*			CY	�G���[�t���O
*
READM			LBSR	SEEK
				BCC		READM1
				RTS
READM1			LDA		<TRACK
	if	(DISKTYPE==D_SKIP2DD)
				LSLA
	endif
				STA		FDCTRACK
				LDA		<SECTOR
				STA		FDCSECTOR
				LDA		<SIDE
				STA		FDDSIDE
* BUSY�҂�
READM2			LDA		FDCSTAT
				LSRA
				BCS		READM2
* �R�}���h���s
				LDA		#$01
				STA		<TYPE
				LDA		#$80
				STA		FDCCMD
* �f�[�^�]��
READM3			LDA		FDDDRQ
				BPL		READM4
				LDA		FDCDATA
				STA		,X+
				BRA		READM3
READM4			BITA	#$40
				BEQ		READM3
* �]���I��
				LDA		FDCSTAT
				ANDA	#$BC
				BEQ		READM5
				ORCC	#$01
				RTS
* ���̃Z�N�^��
READM5			INC		<SECTOR
				DEC		<SECTORS
				BNE		READM1
				RTS

*
*	FDC �Z�N�^��������(�Z�N�^�A��)
*
*	PARAM.: X	�f�[�^�o�b�t�@
*	RESULT: A	Type2 �X�e�[�^�X
*			CY	�G���[�t���O
*
WRITEM			LBSR	SEEK
				BCC		WRITEM1
				RTS
WRITEM1			LDA		<TRACK
	if	(DISKTYPE==D_SKIP2DD)
				LSLA
	endif
				STA		FDCTRACK
				LDA		<SECTOR
				STA		FDCSECTOR
				LDA		<SIDE
				STA		FDDSIDE
* BUSY�҂�
WRITEM2			LDA		FDCSTAT
				LSRA
				BCS		WRITEM2
* �R�}���h���s
				LDA		#$02
				STA		<TYPE
				LDA		#$A0
				STA		FDCCMD
* �f�[�^�]��
WRITEM3			LDA		FDDDRQ
				BPL		WRITEM4
				LDA		,X+
				STA		FDCDATA
				BRA		WRITEM3
WRITEM4			BITA	#$40
				BEQ		WRITEM3
* �]���I��
				LDA		FDCSTAT
				ANDA	#$FC
				BEQ		WRITEM5
				ORCC	#$01
				RTS
* ���̃Z�N�^��
WRITEM5			INC		<SECTOR
				DEC		<SECTORS
				BNE		WRITEM1
				RTS

	if	(DISKTYPE==D_NORM2D)

*
* DPB (360KB,$40�o�C�g)
*
DPB_360			FCB		$EB,$FE,$90			* �W�����v���[�`��
				FCC		/MSDOS5.0/			* OEM NAME
				FCB		$00,$02				* �o�C�g��/�Z�N�^
				FCB		$02					* �Z�N�^��/�N���X�^
				FCB		$01,$00				* �\��Z�N�^��
				FCB		$02					* FAT��
				FCB		$70,$00				* ���[�g�f�B���N�g���ő吔
				FCB		$D0,$02				* �����Z�N�^��
				FCB		$FD					* ���f�B�AID
				FCB		$02,$00				* FAT�Z�N�^��
				FCB		$09,$00				* �Z�N�^��/�g���b�N
				FCB		$02					* �w�b�h��
				FCB		$00,$00,$00,$00
				FCB		$00,$00,$00,$00
				FCB		$00,$00,$00,$00
				FCB		$00,$00,$00,$00
				FCC		/NO NAME    /		* �{�����[�����x��
				FCC		/FAT12   /
				FCB		$00,$00				* �\��

	else

*
* DPB (720KB,$40�o�C�g)
*
DPB_720			FCB		$EB,$FE,$90			* �W�����v���[�`��
				FCC		/MSDOS5.0/			* OEM NAME
				FCB		$00,$02				* �o�C�g��/�Z�N�^
				FCB		$02					* �Z�N�^��/�N���X�^
				FCB		$01,$00				* �\��Z�N�^��
				FCB		$02					* FAT��
				FCB		$70,$00				* ���[�g�f�B���N�g���ő吔
				FCB		$A0,$05				* �����Z�N�^��
				FCB		$F9					* ���f�B�AID
				FCB		$03,$00				* FAT�Z�N�^��
				FCB		$09,$00				* �Z�N�^��/�g���b�N
				FCB		$02					* �w�b�h��
				FCB		$00,$00,$00,$00
				FCB		$00,$00,$00,$00
				FCB		$00,$00,$00,$00
				FCB		$00,$00,$00,$00
				FCC		/NO NAME    /		* �{�����[�����x��
				FCC		/FAT12   /
				FCB		$00,$00				* �\��

	endif

*--[ DATA ]-------------------------------------------------------------------

*
*	�T�uCPU �R�}���h (FM-7)
*
SUBINIT_CMD		FCB		$01,$00,40,25,0,25,0,1,0
SUBCONS_CMD		FCB		$0C,$06
INKEY_CMD		FCB		$29,$03
INKEY_CHK_CMD	FCB		$29,$00

*
*	�t�@�C���l�[��
*
JSUBMON_FN		FCC		/JSUBMON ROM/,0
DICROM_FN		FCC		/DICROM  ROM/,0

*
*	�\�����b�Z�[�W(�N����)
*
NOTFOUND_MSG	FCB		$11,$02,$12,$04,$0B
				FCC		/Japanese Sub System Unavailable./,0
FORMAT_MSG		FCB		$11,$07,$12,$0A,$09
				FCC		/Disk formatting .../,0
	if	(defined(FBLOADM)||(DISKTYPE==D_NORM2D))
TITLE_MSG		FCB		$11,$07,$12,$04,$06
				FCC		/XM7 ROM SAVE UTILITY for FM77-101/
				FCB		$12,$02,$08
				FCC		"(C) 1999 PI. / 2003-2010 Ryu Takegami",0
TITLE_MSG2		FCB		$11,$04,$12,$06,$10
				FCC		/Set blank disk to drive 0 and/,0
	endif

*
* �\�����b�Z�[�W(ROM�Z�[�u��)
*
JSUBMON_MSG		FCB		$11,$07,$12,$03,$09
				FCC		/Saving Japanese Sub Monitor ROM .../,0
DICROM_MSG		FCB		$11,$07,$12,$07,$09
				FCC		/Saving Dictionary ROM .../,0

*
* �\�����b�Z�[�W(�G���[)
*
NOTREADY_MSG	FCB		$11,$02,$12,$0C,$10
				FCC		/Drive not ready./,0
WRPROTECT_MSG	FCB		$11,$02,$12,$0A,$10
				FCC		/Disk write protected./,0
SEEKERR_MSG		FCB		$11,$02,$12,$0F,$10
				FCC		/Seek error./,0
READERR_MSG		FCB		$11,$02,$12,$0F,$10
				FCC		/Read error./,0
WRITEERR_MSG	FCB		$11,$02,$12,$0F,$10
				FCC		/Write error./,0
SPACE_MSG		FCB		$11,$04,$12,$0D,$12
				FCC		/Push SPACE key./,0
CLEAR_MSG		FCB		$11,$07,$12,$05,$10
				FCC		/                     /
				FCB		$12,$0d,$12
				FCC		/               /,0
COMPLETE_MSG	FCB		$11,$07,$12,$0F,$09
				FCC		/Completed./,0

*
* �v���O�����I��
*
PRGEND			EQU		*
				END