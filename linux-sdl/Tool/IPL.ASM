*
*	FM-7 EMULATOR "XM7"
*
*	Copyright (C) 1999-2009 �o�h�D(yasushi@tanaka.net)
*	Copyright (C) 2001-2009 Ryu Takegami
*
*	[ ���[�e�B���e�BIPL ]
*

				ORG		$0100

*
*	�萔��`
*
PRGENT			EQU		$1000		* ���[�h����v���O�����̐擪�A�h���X
BEEP			EQU		$FD03		* BEEP���䃌�W�X�^
BANKSEL			EQU		$FD0F		* ROM/��RAM�Z���N�g���W�X�^
FDDMODE			EQU		$FD1E		* FDD ���[�h���W�X�^
DREAD			EQU		$FE08		* �f�B�X�N���[�h�G���g��
RCBDBA			EQU		2			* RCB �f�[�^�o�b�t�@�A�h���X
RCBTRK			EQU		4			* RCB �g���b�N�i���o
RCBSCT			EQU		5			* RCB �Z�N�^�i���o
RCBSID			EQU		6			* RCB �T�C�h�i���o
RCBUNT			EQU		7			* RCB ���j�b�g�i���o

*
* �v���O�����X�^�[�g
*
START			BRA		MAIN
				NOP

*
* DPB (720KB 2DD)
*
				ORG		START+3
				FCC		/MSDOS5.0/
				FCB		$00,$02,$02,$01,$00
				FCB		$02,$70,$00,$A0,$05
				FCB		$F9
				FCB		$03,$00,$09,$00,$02
				FCB		$00,$00,$00,$00,$00
				FCB		$00,$00,$00,$00
				FCB		$00,$00
				FCB		$00,$00
				FCB		$00,$00,$00
				FCC		/NO NAME    /
				FCC		/FAT12   /

*
* ���C��
*
MAIN			ORCC	#$50
* ������
				LDS		#$0F00
				LDA		#$FD
				TFR		A,DP
				STA		<BANKSEL
				CLR		<FDDMODE
* �{�̓ǂݍ���
				LDB		#$08
LOOP			LEAX	<RCB,PCR
				PSHS	B
				JSR		DREAD
				PULS	B
				TSTA
				BNE		ERROR
* ���̃Z�N�^��
				INC		RCBDBA,X
				INC		RCBDBA,X
				INC		RCBSCT,X
				DECB
				BNE		LOOP

*
* �W�����v
*
				JMP		PRGENT

*
* �ǂݍ��݃G���[
*
ERROR			LDA		#$81
				STA		<BEEP
				BRA		*

*
* RCB
*
RCB				FCB		$0A,$00		* DREAD
				FDB		PRGENT		* �o�b�t�@�A�h���X
				FCB		$00			* �g���b�N
				FCB		$02			* �Z�N�^
				FCB		$00			* �T�C�h
				FCB		$00			* �h���C�u

*
* ���܂�f�[�^
*
OMAKE			ORG		START+$80
				FCC		/���̃v���O������XM7�̂��߂�/
				FCC		/���N(1999�N)�ɍ쐬�������̂ł��B/
				FCC		/�����Ƃ��āAMS-DOS���ʌ݊���/
				FCC		/�ȒP�ȃt�@�C���V�X�e���������Ă���/
				FCC		/Win9x�Ō݊����̂��߃T�|�[�g����Ă���A360KB/
				FCC		/���f�B�A�̍쐬�y�уt�@�C����������/
				FCC		/���ł��܂��B�Ȃ�/
				FCC		/�J���ɂ�MS-DOS��ŃN���X�A�Z���u��/
				FCC		/AS63.EXE���g�p���܂����B�g���₷��/
				FCC		/�A�Z���u����FSW�Œ񋟒��������/
				FCC		/�̕��Ɋ��ӂ��܂��B/
				FCC		/1999.7.31 written by �o�h�D/
				FCC		/(ytanaka@ipc-tokai.or.jp)/

*
* �v���O�����I��
*
				END