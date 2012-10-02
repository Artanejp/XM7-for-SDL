*
*	FM-7 EMULATOR "XM7"
*
*	Copyright (C) 1999-2012 �o�h�D(yasushi@tanaka.net)
*	Copyright (C) 2001-2012 Ryu Takegami
*
*	[ JIS78 ROM�Z�[�o(XM7 V3�Ή�) ]
*
*	�Ώۋ@��E�������f�B�A�^�C�v�ɉ����ăA�Z���u�����ɃI�v�V�����w�肷�邱��
*		-dDISKTYPE=0	360KB(2D)  �ʏ�t�H�[�}�b�g
*		-dDISKTYPE=1	720KB(2DD) 1�g���b�N�����t�H�[�}�b�g
*		-dDISKTYPE=2	720KB(2DD) �ʏ�t�H�[�}�b�g
*		-dFBLOADM		DISK BASIC�p
*

	if	defined(FBLOADM)
				ORG		$0FE0
	else
				ORG		$1000
	endif

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
RS232D			EQU		$FD06			* USART �f�[�^���W�X�^
RS232S			EQU		$FD07			* USART �X�e�[�^�X���W�X�^
RS232C			EQU		RS232S			* USART �R�}���h���W�X�^
RSBAUD			EQU		$FD0B			* RS-232C �{�[���[�g���W�X�^
RSENABLE		EQU		$FD0C			* RS-232C �C�l�[�u�����W�X�^
BANKSEL			EQU		$FD0F			* ROM/��RAM�Z���N�g���W�X�^
INITDIS			EQU		$FD10			* �C�j�V�G�[�^ROM �f�B�Z�[�u�����W�X�^
SUBSTAT			EQU		$FD12			* �T�u���[�h�X�e�[�^�X���W�X�^
SUBBANK			EQU		$FD13			* �T�u�o���N���W�X�^
FDCSTAT			EQU		$FD18			* FDC �X�e�[�^�X���W�X�^
FDCCMD			EQU		FDCSTAT			* FDC �R�}���h���W�X�^
FDCTRACK		EQU		$FD19			* FDC �g���b�N���W�X�^
FDCSECTOR		EQU		$FD1A			* FDC �Z�N�^���W�X�^
FDCDATA			EQU		$FD1B			* FDC �f�[�^���W�X�^
FDDSIDE			EQU		$FD1C			* FDD �T�C�h���W�X�^
FDDDRIVE		EQU		$FD1D			* FDD �h���C�u���W�X�^
FDDMODE			EQU		$FD1E			* FDD ���[�h���W�X�^
FDDDRQ			EQU		$FD1F			* FDC DRQ/IRQ���W�X�^
KNJ1ADR			EQU		$FD20			* ��ꐅ������ �A�h���X���W�X�^
KNJ1DATA		EQU		$FD22			* ��ꐅ������ �f�[�^���W�X�^
KNJ2ADR			EQU		$FD2C			* ��񐅏����� �A�h���X���W�X�^
KNJ2DATA		EQU		$FD2E			* ��񐅏����� �f�[�^���W�X�^
DICBANK			EQU		$FD2E			* ����ROM �o���N���W�X�^
MMR				EQU		$FD80			* MMR �������}�b�s���O���W�X�^
MSGR			EQU		$FD90			* MMR �Z�O�����g���W�X�^
MMSR			EQU		$FD93			* MMR ���[�h�Z���N�g���W�X�^
MSSW1			EQU		$FD94			* ���[�h�Z���N�g�X�C�b�`1 (40/20)
MSSW2			EQU		$FD95			* ���[�h�Z���N�g�X�C�b�`2 (EX)

*
*	�萔��` (�T�uI/O)
*
SUBMODEL4		EQU		$D405			* �T�u���[�h���W�X�^ (L4)
CRTDSP			EQU		$D408			* CRT ON/OFF���W�X�^
VACC			EQU		$D409			* VRAM�A�N�Z�X�t���O
ALUCMD			EQU		$D410			* �_�����Z�R�}���h���W�X�^
ALUCOL			EQU		$D411			* �_�����Z�J���[���W�X�^
ALUMASK			EQU		$D412			* �_�����Z�}�X�N���W�X�^
ALUDIS			EQU		$D41B			* �_�����Z�o���N�f�B�Z�[�u�����W�X�^
KENCDAT			EQU		$D431			* �L�[�G���R�[�_�f�[�^���W�X�^
KENCSTA			EQU		$D432			* �L�[�G���R�[�_�X�e�[�^�X���W�X�^

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
M_KANJI			EQU		4			* FM77AV40EX/SX(����ROM)
D_NORM2D		EQU		0			* 2D (360KB) �ʏ�t�H�[�}�b�g
D_SKIP2DD		EQU		1			* 2DD(720KB) �P�g���b�N�����t�H�[�}�b�g
D_NORM2DD		EQU		2			* 2DD(720KB) �ʏ�t�H�[�}�b�g

*
*	���@CMT I/F�o�R��������p �]���v���O����
*
	if	defined(FBLOADM)
ENTRY			ORCC	#$50
				LEAX	<START,PCR
				LDU		#$1000
				LDY		#((PRGEND-START)|1)+1
TFRLOOP			LDD		,X++
				STD		,U++
				LEAY	-2,Y
				BNE		TFRLOOP
				JMP		$1000

				ORG		$1000
	endif

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
				FCC		/SAVEKNJ /
				FCC		/ V1.35  /
				FCC		"Copyright 1999 �o�h�D/ 2003-2012 Ryu Takegami"
				FCB		$0D,$0A,$00

*
*	���C���v���O����
*
				ORG		START+$80
MAIN			ORCC	#$50
	if	defined(FBLOADM)
				LDS		#STACK
	else
				LEAS	STACK,PCR
	endif

* �n�[�h�E�F�A������
	if	defined(RSDRIVER)
				LBSR	RS_INIT
	endif
				LBSR	INIT
				LBSR	FM77AV_INIT

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

* ���C�����Z�[�u����
MAINSAVE		LBSR	KANJI

* ����(���b�Z�[�W�\���E���[�v)
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
INIT1			CLR	,X+
				DECA
				BNE	INIT1
* ��ʏ�����
				LEAX	SUBINIT_CMD,PCR
				LDB		#$09
				LBSR	SUB_CMD
				LEAX	SUBCONS_CMD,PCR
				LDB		#$02
				LBSR	SUB_CMD
* FM77AV�`�F�b�N
				LBSR	IS_SUPPORT2DD
				BCC		INIT2
				LEAX	FM77AV_MSG,PCR
				LBSR	PRINT
				BRA		*
INIT2			RTS

*--[ ROM�Z�[�o(JIS78��������ROM) ]--------------------------------------------

*
*	�g���T�u�V�X�e��ROM��JIS78��������ROM(AV40EX/SX only)
*
KANJI			CLR		INITDIS
				LDA		$6B0E
				LDB		$6B10
				CMPD	#$3431
				BNE		KANJI2

* �g��ROM�}�b�s���O�E�Z���N�g
				LDD		#$2E88
				STA		MMR+4
				STB		MSSW2

				LBSR	CLS
				LEAX	KANJI_MSG,PCR
				LBSR	PUT_STRING
				LEAX	KANJI_FN,PCR
				LDD		#$80
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
* 64�񃋁[�v
				LDA		#$40
KANJI1			PSHS	A
				STA		DICBANK
				LDX		#$4000
				LDU		#4
				LBSR	FILE_WRITE
				PULS	A
				INCA
				CMPA	#$60
				BLO		KANJI1
* NEXT
KANJI2			LDD		#$3402
				STA		MMR+4
				CLR		DICBANK
				CLR		MSSW2
				STB		INITDIS
				RTS

*--[ �@�픻�� ]---------------------------------------------------------------

*
*	FM77AV����
*
*	RESULT: CY	FM-7(ON),FM77AV(OFF)
*
IS_FM77AV		PSHS	A
* �W��RAM�֐؂�ւ��A$00����������
				LDA		#$02
				STA		INITDIS
				CLR		$6000
* �C�j�V�G�[�^ROM�֐؂�ւ��A�ǂݏo���`�F�b�N
				CLR		INITDIS
				TST		$6000
				BEQ		IS_FM77AV1
* �Y�ꂸ�ɕW��RAM�ɖ߂��Ă���
				STA		INITDIS
				PULS	A
				ANDCC	#$FE
				RTS
* ������͖߂��K�v�͂Ȃ��̂������...
IS_FM77AV1		STA		INITDIS
				PULS	A
				ORCC	#$01
				RTS

*
*	2DD�Ή��@����
*
*	RESULT: CY	2DD���Ή�(ON),2DD�Ή�(OFF)
*
IS_SUPPORT2DD	PSHS	A
* �W��RAM�֐؂�ւ��A$00����������
				LDA		#$02
				STA		INITDIS
				CLR		$6B0E
* �C�j�V�G�[�^ROM�֐؂�ւ��A�ǂݏo���`�F�b�N
				CLR		INITDIS
				LDA		$6B0E
				CMPA	#$34
				BEQ		IS_SUPPORT2DD1
				CMPA	#$32
				BNE		IS_SUPPORT2DD2
* �Y�ꂸ�ɕW��RAM�ɖ߂��Ă���
IS_SUPPORT2DD1	LDA		#$02
				STA		INITDIS
				PULS	A
				ANDCC	#$FE
				RTS
* �Y�ꂸ�ɕW��RAM�ɖ߂��Ă���
IS_SUPPORT2DD2	LDA		#$02
				STA		INITDIS
				PULS	A
				ORCC	#$01
				RTS

*--[ FM77AV ]-----------------------------------------------------------------

*
*	FM77AV������
*
FM77AV_INIT		LBSR	MMR_INIT
				LBSR	SUB_HALT
				LBSR	MMR_VRAM
* �L�[�G���R�[�_�ɑ΂��A�X�L�����R�[�h���w��
				LDA		#$00
				STA		KENCDAT
FM77AV_INIT1	LDA		KENCSTA
				LSRA
				BCC		FM77AV_INIT1
				LDA		#$02
				STA		KENCDAT
FM77AV_INIT2	LDA		KENCSTA
				LSRA
				BCC		FM77AV_INIT2
				LBSR	MMR_BASIC
				RTS

*
*	MMR������
*
*	USEREG: A,B,X
*
MMR_INIT		CLR		MMSR
				CLR		MSSW1
				CLR		MSSW2
				LDB		#3
MMR_INIT1		STB		MSGR
				LDX		#MMR+0
				LDA		#$30
MMR_INIT2		STA		,X+
				INCA
				CMPA	#$40
				BCS		MMR_INIT2
				DECB
				BPL		MMR_INIT1
* VRAM�p�Z�O�����g(SEGMENT 1)������
				LDB		#1
				STB		MSGR
				LDX		#MMR+8
				LDB		#4
				LDA		#$10
MMR_INIT3		STA		,X+
				INCA
				DECB
				BNE		MMR_INIT3
				LDA		#$1D
				STA		MMR+$D
*
				CLR		MSGR
				LDA		#$80
				STA		MMSR
				RTS

*
*	MMR�ݒ�(VRAM�A�N�Z�X)
*
*	USEREG: A
*
MMR_VRAM		STA		BANKSEL
				LDA		#1
				STA		MSGR
* �_�����ZON
				LDA		#$08
				STA		ALUDIS
				LDA		#$80
				STA		ALUCMD
				RTS

*
*	MMR�ݒ�(F-BASIC ROM)
*
MMR_BASIC		EQU		*
				CLR		MSGR
				TST		BANKSEL
				RTS

*
*	�X�y�[�X�҂�
*
KEY_SPACE		LDA		KEYDAT
* BREAK�R�[�h�����`�F�b�N
* ���̃\�[�X�ł͉����L�[�����󉟂����Ȃ��Ɠ��삵�Ȃ��悤�Ȃ̂�
				CMPA	#$57+$80
				BEQ		KEY_SPACE
				CMPA	#$58+$80
				BEQ		KEY_SPACE
				CMPA	#$35+$80
				BEQ		KEY_SPACE
* �������܂ő҂�
KEY_SPACE1		LDA		KEYDAT
				BMI		KEY_SPACE1
				CMPA	#$57
				BEQ		KEY_SPACE2
				CMPA	#$58
				BEQ		KEY_SPACE2
				CMPA	#$35
				BNE		KEY_SPACE1
* �������܂ő҂�
KEY_SPACE2		LDA		KEYDAT
				BPL		KEY_SPACE2
				RTS

*--[ �����o��(FM77AV) ]-------------------------------------------------------

*
*	������\��
*
*	PARAM.: X	������
*			U	VRAM�A�h���X
*
PUT_STRING		LBSR	MMR_VRAM
PUT_STRING1		LDA		,X+
				BEQ		PUT_STRING4
				CMPA	#$11
				BEQ		PUT_STRING2
				CMPA	#$12
				BEQ		PUT_STRING3
				LDB		,X+
				LBSR	PUT_KANJI
				LEAU	2,U
				BRA		PUT_STRING1
* �F�w��
PUT_STRING2		LDA		,X+
				STA		<COLOR
				BRA		PUT_STRING1
* ���W�w��(8dot�P��)
PUT_STRING3		LDA		1,X
				LDB		#$A0
				MUL
				LSLB
				ROLA
				LSLB
				ROLA
				ORB		,X
				LEAX	2,X
				TFR		D,U
				BRA		PUT_STRING1
* �I��
PUT_STRING4		LBSR	MMR_BASIC
				RTS

*
*	�����\��
*
*	PARAM.: D	SHIFT JIS
*			U	VRAM�A�h���X
*
PUT_KANJI		PSHS	U,X
				LBSR	SHIFT_TO_JIS
				LEAU	KANJI_BUF,PCR
				LBSR	GET_KANJI
				LEAX	KANJI_BUF,PCR
				LDU		2,S
				LEAU	$8000,U
* 16x12 �N���A
				CLR		ALUCOL
				CLR		ALUMASK
				LDB		#12
PUT_KANJI1		TST		,U
				TST		1,U
				LEAU	$50,U
				DECB
				BNE		PUT_KANJI1
				LEAU	-$3C0,U
* 16x12 �`��
				LDA		<COLOR
				STA		ALUCOL
				LDB		#12
PUT_KANJI2		LDA		,X+
				STA		ALUMASK
				TST		,U
				LDA		,X+
				STA		ALUMASK
				TST		1,U
				LEAU	$50,U
				DECB
				BNE		PUT_KANJI2
				PULS	X,U,PC

*
*	�����p�^�[���擾
*
*	PARAM.: D	JIS
*			U	�o�b�t�@(32�o�C�g)
*
GET_KANJI		PSHS	X,B,A
* RA0-RA4
				PSHS	B,A
				LSLB
				ROLA
				LSLB
				ROLA
				LSLB
				ROLA
				LSLB
				ROLA
				ANDA	#$01
				STD		,S
* RA5-RA7
				LDA		2,S
				LSLA
				ANDA	#$0E
				ORA		,S
				STA		,S
* RA8
				LDD		2,S
				ANDA	#$70
				ANDB	#$60
				CMPD	#$2060
				BEQ		GET_KANJI1
				LDA		2,S
				LSLA
				ANDA	#$10
				ORA		,S
				STA		,S
				BRA		GET_KANJI2
GET_KANJI1		LDA		#$10
				ORA		,S
				STA		,S
* RA9,RA10,RA11
GET_KANJI2		LDD		2,S
				LSRB
				LSRB
				LSRB
				LSRB
				LSRB
				ANDB	#$03
				PSHS	B
				LSRA
				LSRA
				ANDA	#$1C
				ORA		,S+
				LEAX	GET_KANJI_TBL,PCR
				CLRB
GET_KANJI3		CMPA	,X+
				BEQ		GET_KANJI4
				INCB
				CMPB	#$08
				BCS		GET_KANJI3
				CLRB
GET_KANJI4		TFR		B,A
				LSLA
				LSLA
				LSLA
				LSLA
				LSLA
				ORA		,S
				LDB		1,S
				TFR		D,X
				LEAS	4,S
* ����ROM�A�h���X=X
				LDA		#4
				PSHS	A
GET_KANJI5		STX		KNJ1ADR
				LDD		KNJ1DATA
				STD		,U
				LEAX	1,X
				STX		KNJ1ADR
				LDD		KNJ1DATA
				ORA		,U
				ORB		1,U
				COMA
				COMB
				STD		,U++
				LEAX	1,X
				STX		KNJ1ADR
				LDD		KNJ1DATA
				COMA
				COMB
				STD		,U++
				LEAX	1,X
				STX		KNJ1ADR
				LDD		KNJ1DATA
				COMA
				COMB
				STD		,U++
				LEAX	1,X
				DEC		,S
				BNE		GET_KANJI5
				PULS	A,X,PC

*
*	SHIFT JIS��JIS�ϊ�
*
*	PARAM.: D	SHIFT JIS
*	RESULT: D	JIS
*
SHIFT_TO_JIS	PSHS	A
				ADDA	,S+
				SUBB	#$1F
				BMI		SHIFT_TO_JIS1
				CMPB	#$61
				ADCB	#$DE
SHIFT_TO_JIS1	ADDD	#$1FA1
				ANDA	#$7F
				ANDB	#$7F
				RTS

*
* ��ʃN���A
*
* USEREG: A,B,X,Y,U
*
CLS				LBSR	MMR_VRAM
				CLR		ALUCOL
				CLR		ALUMASK
				LDU		#$8000
				LDB		#200
				PSHS	B
CLS1			PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B,X,Y
				PULU	A,B
				DEC		,S
				BNE		CLS1
				PULS	B
				LBRA	MMR_BASIC

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
*	������o�� (FM77AV�p)
*
*	PARAM.: X	������
*	USEREG: A,B,X,U
*
PRINT			TFR	X,U
* �������𐔂���
				CLRB
PRINT1			TST		,U+
				BEQ		PRINT2
				INCB
				BRA		PRINT1
* �o�b�t�@�ɃZ�b�g
PRINT2			LEAU	BUFFER,PCR
				LDA		#$03
				STA		,U+
				STB		,U+
PRINT3			LDA		,X+
				STA		,U+
				DECB
				BNE		PRINT3
* ���s
				LEAX	BUFFER,PCR
				LDB		1,X
				INCB
				INCB
				LBRA	SUB_CMD

*--[ DOS ]--------------------------------------------------------------------

	include		diskdrv.asm

*--[ DATA ]-------------------------------------------------------------------

*
*	�T�uCPU �R�}���h (FM77AV)
*
SUBINIT_CMD		FCB		$01,$00,40,25,0,25,0,1,0
SUBCONS_CMD		FCB		$0C,$06
INKEY_CMD		FCB		$29,$01

*
*	����ROM�ϊ��e�[�u��
*
GET_KANJI_TBL	FCB		$09,$0A,$0D,$0E
				FCB		$0F,$11,$12,$13

*
*	�t�@�C���l�[��
*
KANJI_FN		FCC		/KANJI   ROM/,0

*
*	�\�����b�Z�[�W(�N����)
*
FM77AV_MSG		FCB		$12,$0A,$0B
				FCC		"FM77AV40EX/SX ONLY!",0

	if	(defined(FBLOADM))
TITLE_MSG		FCB		$11,$07,$12,$17,$06
				FCC		/�w�l�V�@�q�n�l�Z�[�u���[�e�B���e�B/
				FCB		$12,$1C,$08
				FCC		/�i�b�j�@�P�X�X�X�@�o�h�D/
				FCB		$12,$14,$0a
				FCC		/�i�b�j�@�Q�O�O�R�`�Q�O�P�Q�@�������݂肤/,0
TITLE_MSG2		FCC		$12,$12,$10
	endif
				FCC		/�h���C�u�O�Ƀu�����N�Q�c�c�f�B�X�N���Z�b�g����/,0
FORMAT_MSG		FCB		$11,$07,$12,$1B,$09
				FCC		/�_���t�H�[�}�b�g���Ă��܂�/,0

*
* �\�����b�Z�[�W(ROM�Z�[�u��)
*
KANJI_MSG		FCB		$11,$07,$12,$0F,$09
				FCC		/��P���������q�n�l�i�i�h�r�V�W�j�@���Z�[�u���Ă��܂�/,0

*
* �\�����b�Z�[�W(�G���[)
*
NOTREADY_MSG	FCB		$11,$02,$12,$19,$10
				FCC		/�f�B�X�N���Z�b�g����Ă��܂���/,0
WRPROTECT_MSG	FCB		$11,$02,$12,$1A,$10
				FCC		/���C�g�v���e�N�g����Ă��܂�/,0
SEEKERR_MSG		FCB		$11,$02,$12,$20,$10
				FCC		/�V�[�N�G���[�ł�/,0
READERR_MSG		FCB		$11,$02,$12,$1F,$10
				FCC		/�ǂݍ��݃G���[�ł�/,0
WRITEERR_MSG	FCB		$11,$02,$12,$1F,$10
				FCC		/�������݃G���[�ł�/,0
SPACE_MSG		FCB		$11,$04,$12,$1A,$12
				FCC		/�X�y�[�X/
				FCB		$11,$07
				FCC		/�L�[�������Ă�������/,0
CLEAR_MSG		FCB		$11,$07,$12,$19,$10
				FCC		/�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@/
				FCB		$12,$1A,$12
				FCC		/�@�@�@�@�@�@�@�@�@�@�@�@�@�@/,0
COMPLETE_MSG	FCB		$11,$07,$12,$22,$09
				FCC		/�I�����܂���/,0

*
* �v���O�����I��
*
PRGEND			EQU		*
				END