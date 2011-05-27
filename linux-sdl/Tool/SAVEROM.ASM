*
*	FM-7 EMULATOR "XM7"
*
*	Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
*	Copyright (C) 2001-2010 Ryu Takegami
*
*	[ ROMセーバ(XM7 V1/V2/V3対応) ]
*
*	対象機種・生成メディアタイプに応じてアセンブル時にオプション指定すること
*		-dMODEL=0		FM-7シリーズ全機種   (SAVEROM7)
*		-dMODEL=1		FM77AVシリーズ全機種 (SAVEROM)
*		-dMODEL=2		FM77AV40/EX/SX       (SAVEROM2)
*		-dDISKTYPE=0	360KB(2D)  通常フォーマット
*		-dDISKTYPE=1	720KB(2DD) 1トラック抜きフォーマット
*		-dDISKTYPE=2	720KB(2DD) 通常フォーマット
*		-dFBLOADM		DISK BASIC用
*

	if	defined(FBLOADM)
				ORG		$0FE0
	else
				ORG		$1000
	endif

*
*	定数定義 (メインシステム)
*
STACK			EQU		$0F00			* スタック最終
BUFFER			EQU		$0F80			* 汎用バッファ
KANJI_BUF		EQU		$0FE8			* 漢字バッファ
DISK_BUF		EQU		$2800			* ディスクバッファ
SHARED			EQU		$FC80			* 共有RAM

*
*	定数定義 (メインI/O)
*
KEYDAT			EQU		$FD01			* キーデータレジスタ
SUBMODE			EQU		$FD04			* サブシステムモードレジスタ
SUBSIN			EQU		$FD05			* サブシステムインタフェースレジスタ
RS232D			EQU		$FD06			* USART データレジスタ
RS232S			EQU		$FD07			* USART ステータスレジスタ
RS232C			EQU		RS232S			* USART コマンドレジスタ
RSBAUD			EQU		$FD0B			* RS-232C ボーレートレジスタ
RSENABLE		EQU		$FD0C			* RS-232C イネーブルレジスタ
BANKSEL			EQU		$FD0F			* ROM/裏RAMセレクトレジスタ
INITDIS			EQU		$FD10			* イニシエートROM ディセーブルレジスタ
SUBSTAT			EQU		$FD12			* サブモードステータスレジスタ
SUBBANK			EQU		$FD13			* サブバンクレジスタ
FDCSTAT			EQU		$FD18			* FDC ステータスレジスタ
FDCCMD			EQU		FDCSTAT			* FDC コマンドレジスタ
FDCTRACK		EQU		$FD19			* FDC トラックレジスタ
FDCSECTOR		EQU		$FD1A			* FDC セクタレジスタ
FDCDATA			EQU		$FD1B			* FDC データレジスタ
FDDSIDE			EQU		$FD1C			* FDD サイドレジスタ
FDDDRIVE		EQU		$FD1D			* FDD ドライブレジスタ
FDDMODE			EQU		$FD1E			* FDD モードレジスタ
FDDDRQ			EQU		$FD1F			* FDC DRQ/IRQレジスタ
KNJ1ADR			EQU		$FD20			* 第一水準漢字 アドレスレジスタ
KNJ1DATA		EQU		$FD22			* 第一水準漢字 データレジスタ
KNJ2ADR			EQU		$FD2C			* 第二水準漢字 アドレスレジスタ
KNJ2DATA		EQU		$FD2E			* 第二水準漢字 データレジスタ
DICBANK			EQU		$FD2E			* 辞書ROM バンクレジスタ
MMR				EQU		$FD80			* MMR メモリマッピングレジスタ
MSGR			EQU		$FD90			* MMR セグメントレジスタ
MMSR			EQU		$FD93			* MMR モードセレクトレジスタ
MSSW1			EQU		$FD94			* モードセレクトスイッチ1 (40/20)
MSSW2			EQU		$FD95			* モードセレクトスイッチ2 (EX)

*
*	定数定義 (サブI/O)
*
SUBMODEL4		EQU		$D405			* サブモードレジスタ (L4)
CRTDSP			EQU		$D408			* CRT ON/OFFレジスタ
VACC			EQU		$D409			* VRAMアクセスフラグ
ALUCMD			EQU		$D410			* 論理演算コマンドレジスタ
ALUCOL			EQU		$D411			* 論理演算カラーレジスタ
ALUMASK			EQU		$D412			* 論理演算マスクレジスタ
ALUDIS			EQU		$D41B			* 論理演算バンクディセーブルレジスタ
KENCDAT			EQU		$D431			* キーエンコーダデータレジスタ
KENCSTA			EQU		$D432			* キーエンコーダステータスレジスタ

*
*	ワークエリア定義
*
RS_FLAG			EQU		$00			* シリアル出力フラグ
DRIVE			EQU		$01			* アクセスドライブ
TRACK			EQU		$02			* アクセストラック
DRIVE0_T		EQU		$03			* ドライブ0のトラック
DRIVE1_T		EQU		$04			* ドライブ1のトラック
SIDE			EQU		$05			* アクセスサイド
SECTOR			EQU		$06			* アクセスセクタ
SECTORS			EQU		$07			* 連続セクタ数
CLUSTER			EQU		$08			* ファイル先頭クラスタ
CLUSTERS		EQU		$0A			* トータルクラスタ数(1クラスタ1KB)
LASTBYTE		EQU		$0C			* 最終クラスタのバイト数
COLOR			EQU		$0E			* 漢字カラーコード
TYPE			EQU		$0F			* FDCコマンドタイプ
C_TRACK			EQU		$10			* カレントトラック
C_SIDE			EQU		$11			* カレントサイド
C_SECTOR		EQU		$12			* カレントセクタ
C_SECTORS		EQU		$13			* カレントセクタ数
FM77_FLAG		EQU		RS_FLAG		* FM-77フラグ
L4_FLAG			EQU		COLOR		* 400ラインフラグ (L4)

*
*	アセンブル条件定義
*
M_FM7			EQU		0			* FM-7シリーズ全機種
M_FM77AV		EQU		1			* FM77AVシリーズ全機種
M_FM77AV40		EQU		2			* FM77AV40/EX/SX
D_NORM2D		EQU		0			* 2D (360KB) 通常フォーマット
D_SKIP2DD		EQU		1			* 2DD(720KB) １トラック抜きフォーマット
D_NORM2DD		EQU		2			* 2DD(720KB) 通常フォーマット

*
*	実機強制動作用(謎) 転送プログラム
*
	if	defined(FBLOADM)
entry			orcc	#$50
				leax	<start,pcr
				ldu		#$1000
				ldy		#((prgend-start)|1)+1
tfrloop			ldd		,x++
				std		,u++
				leay	-2,y
				bne		tfrloop
				jmp		$1000

				org		$1000
	endif

*
*	プログラムスタート
*
START			BRA		MAIN

*
*	プログラムヘッダ
*
HEADER			FCC		/TINY-DOS(mini) for FM-7  /
				FCC		/Copyright 1991-1992,1999 by ＰＩ．/
				FCB		$0D,$0A,$00

HEADER2			EQU		*
	if	(MODEL==M_FM77AV)
				FCC		/SAVEROM /
	elsif	(MODEL==M_FM77AV40)
				FCC		/SAVEROM2/
	else
				FCC		/SAVEROM7/
	endif
				FCC		/ V1.33  /
				FCC		"Copyright 1999 ＰＩ．/ 2003-2010 Ryu Takegami"
				FCB		$0D,$0A,$00

*
*	メインプログラム
*
				ORG		START+$80
MAIN			ORCC	#$50
	if	defined(FBLOADM)
				LDS		#STACK
	else
				LEAS	STACK,PCR
	endif

* ハードウェア初期化
	if	defined(RSDRIVER)
				LBSR	RS_INIT
	endif
				LBSR	INIT
	if	(MODEL>=M_FM77AV)
				LBSR	FM77AV_INIT
	else
				LBSR	FM7_INIT
	endif

* 起動タイトル表示
	if	(DISKTYPE==D_NORM2D)||(defined(FBLOADM))
				LEAX	TITLE_MSG,PCR
				LBSR	PUT_STRING
				LEAX	TITLE_MSG2,PCR
				LBSR	PUT_STRING
				LEAX	SPACE_MSG,PCR
				LBSR	PUT_STRING
				LBSR	KEY_SPACE
	endif

* ディスクフォーマット
				LBSR	DISK_RST
				LBSR	MS_FORMAT

* メイン側セーブ処理
MAINSAVE		LBSR	FBASIC
				LBSR	BOOT_BAS
				LBSR	BOOT_DOS
	if	(MODEL==M_FM7)
				LBSR	BOOT_1MB
				LBSR	BOOT_MMR
	endif
	if	(MODEL>=M_FM77AV)
				LBSR	INITIATE
	endif
				LBSR	KANJI
	if	(MODEL==M_FM77AV40)
				LBSR	KANJI_2
				LBSR	DICCHK
				LBCS	SUBSAVE
				LBSR	DICROM
	endif

* サブ側セーブ処理
SUBSAVE			EQU		*
	if	(MODEL>=M_FM77AV)
				LBSR	SUBSYS_A
				LBSR	SUBSYS_B
				LBSR	SUBSYS_C
				LBSR	SUBSYS_CG
	else
				LBSR	SUBSYS_C
				LBSR	SUBSYS_L4
	endif
	if	(MODEL==M_FM77AV40)
				LBSR	EXTSUB
				LBSR	BOOT_MMR
	endif

* 完了(メッセージ表示・ループ)
				LBSR	CLS
				LEAX	COMPLETE_MSG,PCR
				LBSR	PUT_STRING
				BRA		*


*
* 初期化
*
INIT			LEAX	START,PCR
				LEAX	-$100,X
				TFR		X,D
				TFR		A,DP
* ワーククリア
				CLRA
INIT1			CLR	,X+
				DECA
				BNE	INIT1
* 画面初期化
				LEAX	SUBINIT_CMD,PCR
				LDB		#$09
				LBSR	SUB_CMD
				LEAX	SUBCONS_CMD,PCR
				LDB		#$02
				LBSR	SUB_CMD
* FM77AVチェック
	if	(MODEL==M_FM77AV)
				LBSR	IS_FM77AV
	elsif	(MODEL==M_FM77AV40)
				LBSR	IS_SUPPORT2DD
	endif
	if	(MODEL>=M_FM77AV)
				BCC		INIT2
				LEAX	FM77AV_MSG,PCR
				LBSR	PRINT
				BRA		*
	endif
INIT2			RTS

*--[ ROMセーバ(全機種共通部) ]------------------------------------------------

*
*	F-BASIC ROM
*
FBASIC			LBSR	CLS
				LEAX	FBASIC_MSG,PCR
				LBSR	PUT_STRING
				LEAX	FBASIC_FN,PCR
				LDD		#31
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDX		#$8000
				LDU		#31
				LBSR	FILE_WRITE
				RTS

*
*	KANJI ROM (第一水準)
*
KANJI			CLRA
				CLRB
				STD		KNJ1ADR
				LDD		KNJ1DATA
				CMPD	#$FFFF
				BEQ		KANJI3

				LBSR	CLS
				LEAX	KANJI_MSG,PCR
				LBSR	PUT_STRING
				LEAX	KANJI_FN,PCR
				LDD		#$80
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
* 8回ループ
				LDX		#$0000
KANJI1			LDU		#$4000
KANJI2			STX		KNJ1ADR
				LDD		KNJ1DATA
				STD		,U++
				LEAX	1,X
				CMPU	#$8000
				BNE		KANJI2
* セーブ
				PSHS	X
				LDX		#$4000
				LDU		#16
				LBSR	FILE_WRITE
				PULS	X
* NEXT
				CMPX	#$0000
				BNE		KANJI1
KANJI3			RTS

*--[ ROMセーバ(FM-7シリーズ) ]------------------------------------------------

	if	(MODEL==M_FM7)

*
*	BOOT(BASIC) ROM
*
BOOT_BAS		LBSR	CLS
				LBSR	IS_FM77AV
				BCC		BOOT_BAS6
BOOT_BAS1		LDX		$FE03
				CMPX	#$FE61
				BEQ		BOOT_BAS5
				LDA		FM77_FLAG
				BEQ		BOOT_BAS2
				LEAX	BOOTB_DIPSW_MSG,PCR
				BRA		BOOT_BAS3
BOOT_BAS2		LEAX	BOOTB_DIP77_MSG,PCR
BOOT_BAS3		LBSR	PRINT
BOOT_BAS4		LBSR	KEY_CHKSP
				BNE		BOOT_BAS5
				LDX		$FE03
				CMPX	#$FE61
				BNE		BOOT_BAS4
BOOT_BAS5		LBSR	CLS
BOOT_BAS6		LEAX	BOOTBAS_MSG,PCR
				LBSR	PRINT
				LEAX	BOOTBAS_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LBSR	IS_FM77AV
				BCC		BOOT_BAS7
				LDX		#$FE00
				BRA		BOOT_BAS8
BOOT_BAS7		CLR		INITDIS
				LDX		#$7800
BOOT_BAS8		LDU		#1
				LBSR	FILE_WRITE
				LDA		#$02
				STA		INITDIS
				RTS

*
*	BOOT(DOS) ROM
*
BOOT_DOS		LBSR	CLS
				LBSR	IS_FM77AV
				BCC		BOOT_DOS6
BOOT_DOS1		LDX		$FE03
				CMPX	#$FE65
				BEQ		BOOT_DOS5
				LDA		FM77_FLAG
				BEQ		BOOT_DOS2
				LEAX	BOOTD_DIPSW_MSG,PCR
				BRA		BOOT_DOS3
BOOT_DOS2		LEAX	BOOTD_DIP77_MSG,PCR
BOOT_DOS3		LBSR	PRINT
BOOT_DOS4		LBSR	KEY_CHKSP
				BNE		BOOT_DOS5
				LDX		$FE03
				CMPX	#$FE65
				BNE		BOOT_DOS4
BOOT_DOS5		LBSR	CLS
BOOT_DOS6		LEAX	BOOTDOS_MSG,PCR
				LBSR	PRINT
				LEAX	BOOTDOS_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LBSR	IS_FM77AV
				BCC		BOOT_DOS7
				LDX		#$FE00
				BRA		BOOT_DOS8
BOOT_DOS7		CLR		INITDIS
				LDX		#$7A00
BOOT_DOS8		LDU		#1
				LBSR	FILE_WRITE
				LDA		#$02
				STA		INITDIS
				RTS

*
*	BOOT(1MB DOS) ROM
*
BOOT_1MB		LBSR	CLS
				LBSR	IS_FM77AV
				BCC		BOOT_1MB4
				TST		<FM77_FLAG
				BNE		BOOT_1MB4
BOOT_1MB1		LDX		$FE03
				CMPX	#$FE8D
				BEQ		BOOT_1MB3
				LEAX	BOOTS_DIP77_MSG,PCR
				LBSR	PRINT
BOOT_1MB2		LBSR	KEY_CHKSP
				BNE		BOOT_1MB3
				LDX		$FE03
				CMPX	#$FE8D
				BNE		BOOT_1MB2
BOOT_1MB3		LBSR	CLS
				LEAX	BOOT1MB_MSG,PCR
				LBSR	PRINT
				LEAX	BOOT1MB_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDX		#$FE00
				LDU		#1
				LBSR	FILE_WRITE
BOOT_1MB4		RTS

*
*	BOOT(MMR $3F BANK) ROM
*
BOOT_MMR		LBSR	CLS
				LBSR	IS_FM77AV
				BCC		BOOT_MMR2
				TST		<FM77_FLAG
				BNE		BOOT_MMR2
BOOT_MMR1		LEAX	BOOTMMR_MSG,PCR
				LBSR	PRINT
				LEAX	BOOTMMR_FN,PCR
				LDD		#$3F80
				STA		MMR+4
				STB		MMSR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDX		#$4E00
				LDU		#1
				LBSR	FILE_WRITE
				LDD		#$3400
				STA		MMR+4
				STB		MMSR
BOOT_MMR2		RTS

*
*	SUBSYSTEM (TYPE-C) ROM
*
SUBSYS_C		LBSR	CLS
				LEAX	SUBSYSC_MSG,PCR
				LBSR	PRINT
				LEAX	SUBSYSC_FN,PCR
				LDD		#10
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
* サブモニタ転送
				LDX		#$D800
				LDY		#$4800
SUBSYS_C1		PSHS	X,Y
				STX		SRCADR,PCR
				LEAX	TFR_CMD,PCR
				LDB		#TFR_CMDe-TFR_CMD
				LBSR	SUB_CMD
				PULS	X,Y
				LBSR	SUB_HALT
				LDU		#$FCC0
				LDB		#$40
SUBSYS_C2		LDA		,U+
				STA		,Y+
				DECB
				BNE		SUBSYS_C2
				LDA		#$80
				STA		SHARED
				CLR		SUBSIN
				LEAX	$40,X
				BNE		SUBSYS_C1
* セーブ
				LDX		#$4800
				LDU		#10
				LBSR	FILE_WRITE
				RTS

*
*	SUBSYSTEM (400LINE) ROM
*
SUBSYS_L4		LDA		<FM77_FLAG
				BNE		SUBSYS_L4_4
* 400ラインカード搭載チェック
				LDA		SUBMODE
				BITA	#$10
				BNE		SUBSYS_L4_4
* 400ラインモードに移行
				LBSR	INITL4
*
				LBSR	CLS
				LEAX	SUBSYSL4_MSG,PCR
				LBSR	PRINT
				LEAX	SUBSYSL4_FN,PCR
				LDD		#18
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
* 裏RAMをセレクト
				STA		BANKSEL
* サブモニタ前半転送(転送コマンドはタイプC用と共用)
				LDX		#$9800
				LDY		#$4000
SUBSYS_L4_1		PSHS	X,Y
				STX		SRCADR,PCR
				LEAX	TFR_CMD,PCR
				LDB		#TFR_CMDe-TFR_CMD
				LBSR	SUB_CMD
				PULS	X,Y
				LBSR	SUB_HALT
				LDU		#$FCC0
				LDB		#$40
SUBSYS_L4_2		LDA		,U+
				STA		,Y+
				DECB
				BNE		SUBSYS_L4_2
				LDA		#$80
				STA		SHARED
				CLR		SUBSIN
				LEAX	$40,X
				BEQ		SUBSYS_L4_3
				CMPX	#$C000
				BNE		SUBSYS_L4_1
				LDX		#$E000
				BRA		SUBSYS_L4_1
* セーブ
SUBSYS_L4_3		LDX		#$4000
				LDU		#18
				LBSR	FILE_WRITE
* F-BASIC ROMをセレクト
				TST		BANKSEL
* 200ラインモードに戻す
				LBSR	INITL2
SUBSYS_L4_4		RTS

*
* サブモニタ転送用メンテナンスコマンド
*
TFR_CMD			FCB		$3F,$59,$41,$4D,$41,$55,$43,$48,$49
				FCB		$91
SRCADR			FDB		$0000,$D3C0,$0040
				FCB		$90
TFR_CMDe		EQU		*

	else

*--[ ROMセーバ(FM77AVシリーズ共通部) ]----------------------------------------

*
* BOOT(BASIC) ROM
*
BOOT_BAS		LBSR	CLS
				LEAX	BOOT_MSG,PCR
				LBSR	PUT_STRING
				LEAX	BOOTBAS_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDX		#$7800
				LDU		#1
				CLR		INITDIS
				LBSR	FILE_WRITE
				LDA		#$02
				STA		INITDIS
				RTS

*
* BOOT(DOS) ROM
*
BOOT_DOS		LEAX	BOOTDOS_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDX		#$7A00
				LDU		#1
				CLR		INITDIS
				LBSR	FILE_WRITE
				LDA		#$02
				STA		INITDIS
				RTS

*
* SUBSYSTEM (TYPE-C) ROM
*
SUBSYS_C		CLR		SUBBANK
				CLR		SUBSIN
				LBSR	SUB_HALT
				LBSR	CLS
				LEAX	SUBSYSC_MSG,PCR
				LBSR	PUT_STRING
				LEAX	SUBSYSC_FN,PCR
				LDD		#10
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDD		#$1D1E
				STD		MMR+4
				INCB
				STB		MMR+6
				LDX		#$4800
				LDU		#10
				LBSR	FILE_WRITE
				LDD		#$3435
				STD		MMR+4
				INCB
				STB		MMR+6
				RTS

*
* INITIATE ROM
*
INITIATE		LBSR	CLS
				LEAX	INITIATE_MSG,PCR
				LBSR	PUT_STRING
				LEAX	INITIATE_FN,PCR
				LDD		#8
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDX		#$6000
				LDU		#8
				CLR		INITDIS
				LBSR	FILE_WRITE
				LDA		#$02
				STA		INITDIS
				RTS

*
* SUBSYSTEM (TYPE-A) ROM
*
SUBSYS_A		LDA		#$01
				STA		SUBBANK
				CLR		SUBSIN
				LBSR	SUB_HALT
				LBSR	CLS
				LEAX	SUBSYSA_MSG,PCR
				LBSR	PUT_STRING
				LEAX	SUBSYSA_FN,PCR
				LDD		#8
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDD		#$1E1F
				STD		MMR+4
				LDX		#$4000
				LDU		#8
				LBSR	FILE_WRITE
				LDD		#$3435
				STD		MMR+4
				RTS

*
* SUBSYSTEM (TYPE-B) ROM
*
SUBSYS_B		LDA		#$02
				STA		SUBBANK
				CLR		SUBSIN
				LBSR	SUB_HALT
				CLR		SUBSTAT
				LBSR	CLS
				LEAX	SUBSYSB_MSG,PCR
				LBSR	PUT_STRING
				LEAX	SUBSYSB_FN,PCR
				LDD		#8
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDD		#$1E1F
				STD		MMR+4
				LDX		#$4000
				LDU		#8
				LBSR	FILE_WRITE
				LDD		#$3435
				STD		MMR+4
				RTS

*
* SUBSYSTEM (CG) ROM
*
SUBSYS_CG		CLR		SUBBANK
				CLR		SUBSIN
				LBSR	SUB_HALT
				LBSR	CLS
				LEAX	SUBSYSCG_MSG,PCR
				LBSR	PUT_STRING
				LEAX	SUBSYSCG_FN,PCR
				LDD		#8
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDA		#$1D
				STA		MMR+4
* ４回書き込み
				CLRA
SUBSYS_CG1		PSHS	A
				STA		$4430
				LDX		#$4800
				LDU		#2
				LBSR	FILE_WRITE
				PULS	A
				INCA
				CMPA	#$04
				BCS		SUBSYS_CG1
* 終了
				LDA		#$34
				STA		MMR+4
				RTS

	endif

*--[ ROMセーバ(FM77AV40系固有部) ]--------------------------------------------

	if	(MODEL==M_FM77AV40)

*
*	KANJI ROM (第２水準,チェック処理込み)
*
KANJI_2			EQU		*
* 第2水準漢字ROM搭載チェック(対AV20系用)
				CLRA
				CLRB
				STD		KNJ2ADR
				LDD		KNJ2DATA
				CMPD	#$FFFF
				BEQ		KANJI2_3

				LBSR	CLS
				LEAX	KANJI2_MSG,PCR
				LBSR	PUT_STRING
				LEAX	KANJI2_FN,PCR
				LDD		#$80
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
* 8回ループ
				LDX		#$0000
KANJI2_1		LDU		#$4000
KANJI2_2		STX		KNJ2ADR
				LDD		KNJ2DATA
				STD		,U++
				LEAX	1,X
				CMPU	#$8000
				BNE		KANJI2_2
* セーブ
				PSHS	X
				LDX		#$4000
				LDU		#16
				LBSR	FILE_WRITE
				PULS	X
* NEXT
				CMPX	#$0000
				BNE		KANJI2_1
KANJI2_3		RTS

*
*	辞書ROM存在チェック
*
*	RESULT: CY	なし(ON),あり(OFF)
*
DICCHK			PSHS	D
				LDD		#$2E40
				STA		MMR+4
				STB		DICBANK
				CLR		MSSW2
				LDB		#$34

* バンク切り換えが行われたかチェックする
				CLR		$4080
				LDA		$4080
				CLR		DICBANK
				CMPA	$4080
				BEQ		DICCHK2

* 辞書ROMあり
DICCHK1			STB		MMR+4
				PULS	D
				ANDCC	#$FE
				RTS

* 辞書ROMなし
DICCHK2			STB		MMR+4
				PULS	D
				ORCC	#$01
				RTS

*
*	辞書ROM
*
DICROM			LDD		#$2E40
				STA		MMR+4

* 辞書ROMマッピング・セレクト
				STB		DICBANK
				CLR		MSSW2

				LBSR	CLS
				LEAX	DICROM_MSG,PCR
				LBSR	PUT_STRING
				LEAX	DICROM_FN,PCR
				LDD		#$100
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE

* 64回ループ
				LDA		#$40
DICROM1			PSHS	A
				STA		DICBANK
				LDX		#$4000
				LDU		#4
				LBSR	FILE_WRITE
				PULS	A
				INCA
				BPL		DICROM1

* NEXT
DICEXIT			LDD		#$3400
				STA		MMR+4
				STB		DICBANK
				RTS

*
*	拡張サブシステムROM(AV40EX/SX only)
*
EXTSUB			CLR		INITDIS
				LDA		$6B0E
				LDB		$6B10
				CMPD	#$3431
				BNE		EXTSUB2

* 拡張ROMマッピング・セレクト
				LDD		#$2E88
				STA		MMR+4
				STB		MSSW2

				LBSR	CLS
				LEAX	EXTSUB_MSG,PCR
				LBSR	PUT_STRING
				LEAX	EXTSUB_FN,PCR
				LDD		#$30
				STD		<CLUSTERS
				LDD		#1024
				STD		<LASTBYTE
				LBSR	FILE_CREATE
* 12回ループ
				LDA		#$60
EXTSUB1			PSHS	A
				STA		DICBANK
				LDX		#$4000
				LDU		#4
				LBSR	FILE_WRITE
				PULS	A
				INCA
				CMPA	#$6C
				BLO		EXTSUB1
* NEXT
EXTSUB2			LDD		#$3402
				STA		MMR+4
				CLR		DICBANK
				CLR		MSSW2
				STB		INITDIS
				RTS

*
*	FM-77のBOOT(MMR $3F BANK)相当ROM
*
BOOT_MMR		CLR		INITDIS
				LDA		$6B0E
				LDB		$6B10
				CMPD	#$3431
				BNE		BOOT_MMR2

* 拡張ROMマッピング・セレクト
				LDD		#$2E88
				STA		MMR+4
				STB		MSSW2

				LBSR	CLS
				LEAX	BOOTMMR_MSG,PCR
				LBSR	PUT_STRING
				LEAX	BOOTMMR_FN,PCR
				LDD		#1
				STD		<CLUSTERS
				LDD		#512
				STD		<LASTBYTE
				LBSR	FILE_CREATE
				LDA		#$7F
				STA		DICBANK
				LDX		#$4E00
				LDU		#4
				LBSR	FILE_WRITE
* NEXT
BOOT_MMR2		LDD		#$3402
				STA		MMR+4
				CLR		DICBANK
				CLR		MSSW2
				STB		INITDIS
				RTS

	endif

*--[ 機種判定 ]---------------------------------------------------------------

*
*	FM77AV判定
*
*	RESULT: CY	FM-7(ON),FM77AV(OFF)
*
IS_FM77AV		PSHS	A
* 標準RAMへ切り替え、$00を書き込む
				LDA		#$02
				STA		INITDIS
				CLR		$6000
* イニシエートROMへ切り替え、読み出しチェック
				CLR		INITDIS
				TST		$6000
				BEQ		IS_FM77AV1
* 忘れずに標準RAMに戻しておく
				STA		INITDIS
				PULS	A
				ANDCC	#$FE
				RTS
* こちらは戻す必要はないのだけれど...
IS_FM77AV1		STA		INITDIS
				PULS	A
				ORCC	#$01
				RTS

*
*	2DD対応機判定
*
*	RESULT: CY	2DD未対応(ON),2DD対応(OFF)
*
IS_SUPPORT2DD	PSHS	A
* 標準RAMへ切り替え、$00を書き込む
				LDA		#$02
				STA		INITDIS
				CLR		$6B0E
* イニシエートROMへ切り替え、読み出しチェック
				CLR		INITDIS
				LDA		$6B0E
				CMPA	#$34
				BEQ		IS_SUPPORT2DD1
				CMPA	#$32
				BNE		IS_SUPPORT2DD2
* こちらは戻す必要はないのだけれど...
IS_SUPPORT2DD1	LDA		#$02
				STA		INITDIS
				PULS	A
				ANDCC	#$FE
				RTS
* 忘れずに標準RAMに戻しておく
IS_SUPPORT2DD2	LDA		#$02
				STA		INITDIS
				PULS	A
				ORCC	#$01
				RTS

*--[ FM-7 ]-------------------------------------------------------------------

	if	(MODEL==M_FM7)

*
*	FM-7初期化 (77チェック付き)
*
FM7_INIT		LBSR	MMR_INIT
				TST		BANKSEL
				LDD		#$0201
				STA		INITDIS
* ブートROMの裏RAMが使用できるか確認 (使用できるなら77)
				STB		MMSR
				LDB		$FE02
				CLR		$FE02
				LDA		$FE02
				STA		<FM77_FLAG
				STB		$FE02
				CLR		MMSR
				RTS

*
*	スペース待ち
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
*	スペース入力チェック
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
*	画面クリア
*
CLS				EQU		*
				TST		L4_FLAG
				BEQ		CLS_L2
* 400ライン時はERASEコマンドを利用(ただの手抜き)
				LEAX	CLSL4_CMD,PCR
				LDB		#3
				LBRA	SUB_CMD
* 200ライン時は独自プログラムで消去
CLS_L2			EQU		*
				LEAX	CLS_CMD,PCR
				LDB		#CLS_CMDe-CLS_CMD
				LBRA	SUB_CMD

*
*	サブCPU 画面クリアプログラム
*
CLS_CMD			FCB		$3F,$59,$41,$4D,$41,$55,$43,$48,$49
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

*
*	サブCPU 画面クリアコマンド (L4)
*
CLSL4_CMD		FCB		$02,$00,$00

	endif

*--[ FM77AV ]-----------------------------------------------------------------

	if	(MODEL>=M_FM77AV)

*
*	FM77AV初期化
*
FM77AV_INIT		LBSR	MMR_INIT
				LBSR	SUB_HALT
				LBSR	MMR_VRAM
* キーエンコーダに対し、スキャンコードを指定
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
*	MMR初期化
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
* VRAM用セグメント(SEGMENT 1)初期化
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
*	MMR設定(VRAMアクセス)
*
*	USEREG: A
*
MMR_VRAM		STA		BANKSEL
				LDA		#1
				STA		MSGR
* 論理演算ON
				LDA		#$08
				STA		ALUDIS
				LDA		#$80
				STA		ALUCMD
				RTS

*
*	MMR設定(F-BASIC ROM)
*
MMR_BASIC		EQU		*
				CLR		MSGR
				TST		BANKSEL
				RTS

*
*	スペース待ち
*
KEY_SPACE		LDA		KEYDAT
* BREAKコード発生チェック
* 元のソースでは何かキーを一回空押ししないと動作しないようなので
				CMPA	#$57+$80
				BEQ		KEY_SPACE
				CMPA	#$58+$80
				BEQ		KEY_SPACE
				CMPA	#$35+$80
				BEQ		KEY_SPACE
* 押されるまで待つ
KEY_SPACE1		LDA		KEYDAT
				BMI		KEY_SPACE1
				CMPA	#$57
				BEQ		KEY_SPACE2
				CMPA	#$58
				BEQ		KEY_SPACE2
				CMPA	#$35
				BNE		KEY_SPACE1
* 離されるまで待つ
KEY_SPACE2		LDA		KEYDAT
				BPL		KEY_SPACE2
				RTS

	endif

*--[ FM-77L4 ]----------------------------------------------------------------

	if	(MODEL==M_FM7)

*
*	MMR初期化
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
*
				RTS

*
* 400ラインモード初期化
*
INITL4			LDA		SUBMODE
				BITA	#$10
				LBNE	INITL4_1
				LDA		#$FF
				STA		L4_FLAG
				LBSR	SUBCHG
* 画面初期化
				LEAX	SUBINIT_L4_CMD,PCR
				LDB		#$08
				LBSR	SUB_CMD
				LEAX	SUBCONS_L4_CMD,PCR
				LDB		#$03
				LBSR	SUB_CMD
* パレット初期化
				LBSR	INITL2_2
				LDD		#$000F
				STD		$FD38
INITL4_1		RTS

*
* サブシステムモード切り換え
*
SUBCHG			BSR		SUBCHG3
				LDD		#$40E8
				STA		SUBSIN
				CLR		SUBSIN
SUBCHG1			LDA		SUBSIN
				BMI		SUBCHG1
				STB		SUBMODE
SUBCHG2			LDA		SUBSIN
				BPL		SUBCHG2
SUBCHG3			LDA		#$E0
				STA		SUBMODE
				LEAX	INITL4_CMD,PCR
				LDB		#INITL4_CMDe-INITL4_CMD
				LBRA	SUB_CMD

*
* 200ラインモード初期化
*
INITL2			LDA		SUBMODE
				BITA	#$10
				LBNE	INITL2_2
				LDD		#$00CC
				STA		L4_FLAG
				STB		SUBMODE
INITL2_1		LDA		SUBSIN
				BPL		INITL2_1
* 画面初期化
				LEAX	SUBINIT_CMD,PCR
				LDB		#$09
				LBSR	SUB_CMD
				LEAX	SUBCONS_CMD,PCR
				LDB		#$02
				LBSR	SUB_CMD
* パレット初期化 (インテンシティビットは考慮しない)
INITL2_2		LDA		#$07
				LDX		#$FD40
INITL2_3		STA		,-X
				DECA
				BPL		INITL2_3
INITL2_4		RTS

	endif

*--[ 漢字出力(FM77AV) ]-------------------------------------------------------

	if	(MODEL>=M_FM77AV)

*
*	文字列表示
*
*	PARAM.: X	文字列
*			U	VRAMアドレス
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
* 色指定
PUT_STRING2		LDA		,X+
				STA		<COLOR
				BRA		PUT_STRING1
* 座標指定(8dot単位)
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
* 終了
PUT_STRING4		LBSR	MMR_BASIC
				RTS

*
*	漢字表示
*
*	PARAM.: D	SHIFT JIS
*			U	VRAMアドレス
*
PUT_KANJI		PSHS	U,X
				LBSR	SHIFT_TO_JIS
				LEAU	KANJI_BUF,PCR
				LBSR	GET_KANJI
				LEAX	KANJI_BUF,PCR
				LDU		2,S
				LEAU	$8000,U
* 16x12 クリア
				CLR		ALUCOL
				CLR		ALUMASK
				LDB		#12
PUT_KANJI1		TST		,U
				TST		1,U
				LEAU	$50,U
				DECB
				BNE		PUT_KANJI1
				LEAU	-$3C0,U
* 16x12 描画
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
*	漢字パターン取得
*
*	PARAM.: D	JIS
*			U	バッファ(32バイト)
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
* 漢字ROMアドレス=X
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
*	SHIFT JIS→JIS変換
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
* 画面クリア
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
	endif

*--[ サブCPU ]----------------------------------------------------------------

*
*	サブCPU HALT
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
*	サブCPU コマンド出力
*
*	PARAM.: X	コマンドデータ($FC82～)
*			B	送信長さ
*	USEREG: A,B,X,U
*
SUB_CMD			LBSR	SUB_HALT
* データ書き込み
				LDU		#SHARED
				CLR		,U+
				CLR		,U+
SUB_CMD1		LDA		,X+
				STA		,U+
				DECB
				BNE		SUB_CMD1
* サブCPU 実行
				CLR		SUBSIN
				RTS

	if	(MODEL==M_FM7)

*
*	文字列出力 (FM-7用,ANK専用)
*
*	PARAM.: X	文字列
*	USEREG: A,B,X,U
*
PUT_STRING		EQU		*
PRINT			TFR		X,U
* 文字数を数える
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

	else

*
*	文字列出力 (FM77AV用)
*
*	PARAM.: X	文字列
*	USEREG: A,B,X,U
*
PRINT			TFR	X,U
* 文字数を数える
				CLRB
PRINT1			TST		,U+
				BEQ		PRINT2
				INCB
				BRA		PRINT1
* バッファにセット
PRINT2			LEAU	BUFFER,PCR
				LDA		#$03
				STA		,U+
				STB		,U+
PRINT3			LDA		,X+
				STA		,U+
				DECB
				BNE		PRINT3
* 実行
				LEAX	BUFFER,PCR
				LDB		1,X
				INCB
				INCB
				LBRA	SUB_CMD

	endif

*--[ RS-232Cドライバ ]--------------------------------------------------------

	if	defined(RSDRIVER)
*
*	RS-232C初期化
*	(FM77AV20/40以降)
*
RS_INIT			PSHS	A,B
				LDB		#$02
RS_INIT0		LDA		#$01
				STA		RSENABLE
* 内部リセットをかけ、待つ
				LDA		#$40
				STA		RS232C
				CLRA
RS_INIT1		DECA
				BNE		RS_INIT1
* パリティなし, 8bit長, ストップビット1
				LDA		#$4E
				STA		RS232C
				LDA		#$11
				STA		RS232C
* 9600bps
				LDA		#$0C
				STA		RSBAUD
* なぜか２回実行しないと動作しない
				DECB
				BNE		RS_INIT0
* フラグON
				DECB
				STB		<RS_FLAG
				PULS	A,B,PC

*
*	RS-232C 1バイト出力
*
*	PARAM.: A	送信データ
*
RS_PUTC			TST		<RS_FLAG
				BEQ		RS_PUTC2
				PSHS	A
* 送信レディ待ち
RS_PUTC1		LDA		RS232S
				LSRA
				BCC		RS_PUTC1
* データ送信
				PULS	A
				STA		RS232D
RS_PUTC2		RTS

*
*	RS-232C 文字列出力(改行つき)
*
*	PARAM.: Y	送信文字列
*
RS_PUTS			PSHS	A
RS_PUTS1		LDA		,Y+
				BEQ		RS_PUTS2
				BSR		RS_PUTC
				BRA		RS_PUTS1
RS_PUTS2		LDA		#$0D
				BSR		RS_PUTC
				LDA		#$0A
				BSR		RS_PUTC
				PULS	A,PC

*
*	RS-232C デバッグ
*
RS_DEBUG		PSHS	CC,A,B,DP,X,Y,U
* CC
				LEAX	DEBUG_DBG+3,PCR
				LDA		,S
				LBSR	SET2HEX
* A
				LEAX	DEBUG_DBG+8,PCR
				LDA		1,S
				LBSR	SET2HEX
* B
				LEAX	DEBUG_DBG+13,PCR
				LDA		2,S
				LBSR	SET2HEX
* DP
				LEAX	DEBUG_DBG+19,PCR
				LDA		3,S
				LBSR	SET2HEX
* X
				LEAX	DEBUG_DBG+24,PCR
				LDD		4,S
				LBSR	SET4HEX
* Y
				LEAX	DEBUG_DBG+31,PCR
				LDD		6,S
				LBSR	SET4HEX
* U
				LEAX	DEBUG_DBG+38,PCR
				LDD		8,S
				LBSR	SET4HEX
* S
				LEAX	DEBUG_DBG+45,PCR
				TFR		S,D
				ADDD	#12
				LBSR	SET4HEX
* PC
				LEAX	DEBUG_DBG+53,PCR
				LDD		10,S
				LBSR	SET4HEX
* 出力
				LEAY	DEBUG_DBG,PCR
				BSR		RS_PUTS
				PULS	CC,A,B,DP,X,Y,U,PC

*
*	１６進４桁セット
*
*	PARAM.: D	データ
*			X	バッファ
*
SET4HEX			BSR		SET2HEX
				TFR		B,A

*
*	１６進２桁セット
*
*	PARAM.: A	データ
*			X	バッファ
*
SET2HEX			PSHS	A
				LSRA
				LSRA
				LSRA
				LSRA
				BSR		SET1HEX
				PULS	A
				ANDA	#$0F

*
*	１６進１桁セット
*
*	PARAM.: A	データ
*			X	バッファ
*
SET1HEX			ADDA	#$30
				CMPA	#$3A
				BCS		SET1HEX1
				ADDA	#$07
SET1HEX1		STA		,X+
				RTS

	endif

*--[ DOS ]--------------------------------------------------------------------

*
*	ディスクリセット
*
DISK_RST		LBSR	CLS
				LDA		#$10		* 論理ドライブと物理ドライブを同一に設定
				STA		FDDMODE		* (ドライブ0)
				LDA		#$15		* 同上
				STA		FDDMODE		* (ドライブ1)
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
*	MS-DOS 360KB/720KBディスク
*	フォーマットおよび作成
*
MS_FORMAT		LBSR	CLS
				LEAX	FORMAT_MSG,PCR
				LBSR	PUT_STRING
				CLR		<DRIVE

	if	(DISKTYPE==D_NORM2D)||(DISKTYPE==D_SKIP2DD)||(defined(FBLOADM))
				CLR		<TRACK
* フォーマット
MS_FORMAT1		CLR		<SIDE
MS_FORMAT2		LBSR	FORMAT
				BCC		MS_FORMAT3
				LBSR	DISK_ERR
				BRA		MS_FORMAT2
* 次のサイド
MS_FORMAT3		INC		<SIDE
				LDA		<SIDE
				CMPA	#$02
				BCS		MS_FORMAT2
* 次のトラック
				INC		<TRACK
				LDA		<TRACK
*	if	(DISKTYPE==D_NORM2DD)
*				CMPA	#80
*	else
				CMPA	#40
*	endif
				BCS		MS_FORMAT1
	endif

* トラック0
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
* トラック1
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
*	トラック0 データ作成
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
* ディレクトリ
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
*	トラック１ データ作成
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
*	FAT作成(2D)
*
*	PARAM.: X	データバッファ
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
*	FAT作成(トラック抜き2DD)
*
*	PARAM.: X	データバッファ
*	USEREG: A,B
*
MS_MAKE_FAT	TFR	X,U
* FAT終了アドレスをスタックにプッシュ
				LEAY	$600,X
				PSHS	Y
* FATデータバッファを初期化
MS_MAKE_FAT1	CLR		,U+
				CMPU	,S
				BNE		MS_MAKE_FAT1
		 		LDD		#$F9FF
				STA		,X
				STB		1,X
				STB		2,X
* 奇数シリンダを不良クラスタとするFATを作成
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
*	FAT作成(2DD)
*
*	PARAM.: X	データバッファ
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
*	ヌルディレクトリ作成
*
*	PARAM.: X	データバッファ
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
*	ファイル作成
*
*	PARAM.: X	ファイルネーム(8+3)
*	CLUSTERS,LASTBYTEは予めセットしておく
*
FILE_CREATE		PSHS	X
				CLR	<TRACK
				CLR	<SIDE
* 先頭トラック読み込み
FILE_CREATE1	LDA		#$01
				STA		<SECTOR
				LDA		#$09
				STA		<SECTORS
				LEAX	DISK_BUF,PCR
				LBSR	READM
				BCC		FILE_CREATE2
				LBSR	DISK_ERR
				BRA		FILE_CREATE1
* データ設定
FILE_CREATE2	LEAX	DISK_BUF+$200,PCR
				LBSR	MAKE_FAT
	if	(DISKTYPE==D_NORM2D)
				LEAX	DISK_BUF+$A00,PCR
	else
				LEAX	DISK_BUF+$E00,PCR
	endif
				PULS	U
				LBSR	MAKE_DIR
* 先頭トラック書き込み
FILE_CREATE3	LDA		#$01
				STA		<SECTOR
				LDA		#$09
				STA		<SECTORS
				LEAX	DISK_BUF,PCR
				LBSR	WRITEM
				BCC		FILE_CREATE4
				LBSR	DISK_ERR
				BRA		FILE_CREATE3
* C,H,Rを作成しておく
FILE_CREATE4	LDD		<CLUSTER
				LBSR	CLUS_TO_CHR
				RTS

*
*	FAT設定
*
*	PARAM.: X	第1FAT先頭アドレス
*	USEREG: A,B,X,Y,U
*
MAKE_FAT		LDU		#$0002
* 空いているクラスタをサーチする
MAKE_FAT1		LBSR	GET_FAT
				CMPD	#$0000
				BEQ		MAKE_FAT2
				LEAU	1,U
				BRA		MAKE_FAT1
* 先頭FAT番号確定
MAKE_FAT2		STU		<CLUSTER
				LDD		<CLUSTERS
				CMPD	#$0001
				BEQ		MAKE_FAT4
				TFR		D,Y
				LEAY	-1,Y
* 次の空きクラスタをサーチする
MAKE_FAT3A		PSHS	U
				LEAU	1,U
MAKE_FAT30		LBSR	GET_FAT
				CMPD	#$0000
				BEQ		MAKE_FAT31
				LEAU	1,U
				BRA		MAKE_FAT30
MAKE_FAT31		TFR		U,D
				PULS	U
* 発見した空きクラスタを設定する
MAKE_FAT3		BSR		SET_FAT
				TFR		D,U
				LEAY	-1,Y
				BNE		MAKE_FAT3A
* 最終クラスタは、$FFFをセット
MAKE_FAT4		LDD		#$0FFF
				BSR		SET_FAT
				RTS

*
*	FATデータ取得
*
*	PARAM.: X	FAT先頭アドレス
*			U	クラスタ番号
*	RESULT: D	FATデータ(12bit)
*
GET_FAT			TFR		U,D
				LSRA
				RORB
				BCS		GET_FAT_ODD
* 偶数パターン
GET_FAT_EVEN	PSHS	D
				ADDD	,S
				ADDD	,S
				LEAS	2,S
				LDD		D,X
				EXG		A,B
				ANDA	#$0F
				RTS
* 奇数パターン
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
*	FATデータセット
*
*	PARAM.: X	第1FAT先頭アドレス
*			U	クラスタ番号
*			D	FATデータ(12bit)
*
SET_FAT			PSHS	D,X
				PSHS	D
				TFR		U,D
				LSRA
				RORB
				BCS		SET_FAT_ODD
* 偶数パターン
SET_FAT_EVEN	PSHS	D
				ADDD	,S
				ADDD	,S
				LEAS	2,S
				LEAX	D,X
* 先に掃除しておく
				LDB		1,X
				ANDB	#$F0
				STB		1,X
* スタックから取り出して、OR
				PULS	D
				EXG		A,B
				STA		,X
				ORB		1,X
				STB		1,X
* 第2FAT
	if	(DISKTYPE==D_NORM2D)
				STD		$400,X
	else
				STD		$600,X
	endif
				PULS	D,X,PC
* 奇数パターン
SET_FAT_ODD	PSHS	D
				ADDD	,S
				ADDD	,S
				LEAS	2,S
				ADDD	#1
				LEAX	D,X
* 先に掃除しておく
				LDA		,X
				ANDA	#$0F
				STA		,X
* スタックから取り出して、OR
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
* 第2FAT
	if	(DISKTYPE==D_NORM2D)
				STD		$400,X
	else
				STD		$600,X
	endif
				PULS	D,X,PC

*
*	ディレクトリ作成
*
*	PARAM.: X	ディレクトリエリア先頭アドレス
*			U	ファイルネーム(8+3)
*	USEREG: A,B,X,U
*
MAKE_DIR		LDA		,X
				BEQ		MAKE_DIR1
				CMPA	#$E5
				BEQ		MAKE_DIR1
				LEAX	32,X
				BRA		MAKE_DIR
* ファイルネームをコピー
MAKE_DIR1		LDB		#11
MAKE_DIR2		LDA		,U+
				STA		,X+
				DECB
				BNE		MAKE_DIR2
* アトリビュート
				LDA		#$20
				STA		,X+
* 未使用領域はクリア
				LDB		#10
MAKE_DIR3		CLR		,X+
				DECB
				BNE		MAKE_DIR3
* 時間
				LDD		#$0060
				STD		,X++
				LDD		#$1427
				STD		,X++
* 先頭クラスタ
				LDD		<CLUSTER
				EXG		A,B
				STD		,X++
* ファイルサイズ
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
* 余りバイト処理
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
*	クラスタ番号→カレントC,H,R
*
*	PARAM.: D	クラスタ番号
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
* ループ
CLUS_TO_CHR1	LDA		<C_SECTOR
				INCA
				INCA
				STA		<C_SECTOR
				CMPA	#$0A
				BCS		CLUS_TO_CHR2
* 次のサイド
				SUBA	#$09
				STA		<C_SECTOR
				INC		<C_SIDE
				LDA		<C_SIDE
				CMPA	#$01
				BEQ		CLUS_TO_CHR2
* 次のトラック
				CLR		<C_SIDE
				INC		<C_TRACK
* NEXT
CLUS_TO_CHR2	LEAX	-1,X
				BNE		CLUS_TO_CHR1
	if	(DISKTYPE==D_SKIP2DD)
				LSR		<C_TRACK		偽2DD用トラック番号補正
	endif
				PULS	X,PC

*
*	ファイル書き込み
*
*	PARAM.: X	バッファアドレス
*			U	書き込み最大サイズ(KB)
*
FILE_WRITE		CMPU	<CLUSTERS
				BCS		FILE_WRITE1
				LDU		<CLUSTERS
* U=今回書き込むクラスタ数
FILE_WRITE1		PSHS	U
				LDD		<CLUSTERS
				SUBD	,S
				STD		<CLUSTERS
				LDD		,S
				ADDD	,S
				STD		,S
* ,S=今回書き込むセクタ数
FILE_WRITE2		LDB		#10
				SUBB	<C_SECTOR
				CLRA
				CMPD	,S
				BCS		FILE_WRITE4
				LDB		1,S
* B=このトラックで書くセクタ
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
* 書き込みエラー
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
* 終了チェック
FILE_WRITE6		STD		<C_SIDE
				LDD		,S
				BNE		FILE_WRITE2
				PULS	U,PC

*
*	ディスクエラー
*
*	PARAM.: A	FDCステータス
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
* 表示
DISK_ERR1		LBSR	PUT_STRING
				LEAX	SPACE_MSG,PCR
				LBSR	PUT_STRING
* キー待ち
				LBSR	KEY_SPACE
* 終了
				LEAX	CLEAR_MSG,PCR
				LBSR	PUT_STRING
				LBSR	RESTORE
				RTS

*--[ FDCドライバ ]------------------------------------------------------------

*
*	FDC リストア
*
*	RESULT: A	Type1 ステータス
*			CY	エラーフラグ
*
RESTORE			LDA		<DRIVE
				ORA		#$80
				STA		FDDDRIVE
* BUSY待ち
RESTORE1		LDA		FDCSTAT
				LSRA
				BCS		RESTORE1
* リストアコマンド書き込み
				CLR		<TYPE
				LDA		#$08
				STA		FDCCMD
* 終了待ち
RESTORE2		LDA		FDDDRQ
				BITA	#$40
				BEQ		RESTORE2
* ステータス
				LDA		FDCSTAT
				ANDA	#$D8
				BEQ		RESTORE3
				ORCC	#$01
				RTS
* トラック0を記憶
RESTORE3		TST		<DRIVE
				BNE		RESTORE4
				CLR		<DRIVE0_T
				RTS
RESTORE4		CLR		<DRIVE1_T
				RTS

*
*	FDC シーク
*
*	RESULT: A	Type1 ステータス
*			CY	エラーフラグ
*	USEREG: B
*
SEEK			LDA		<DRIVE
				ORA		#$80
				STA		FDDDRIVE
* トラックレジスタへ現在のトラックを書き込む
				TST		<DRIVE
				BNE		SEEK1
				LDA		<DRIVE0_T
				STA		FDCTRACK
				BRA		SEEK2
SEEK1			LDA		<DRIVE1_T
				STA		FDCTRACK
* BUSY待ち
SEEK2			LDA		FDCSTAT
				LSRA
				BCS		SEEK2
* シークコマンド書き込み
				LDA		<TRACK
				STA		FDCDATA
				CLR		<TYPE
				LDA		#$18
				STA		FDCCMD
* 終了待ち
SEEK3			LDA		FDDDRQ
				BITA	#$40
				BEQ		SEEK3
* ステータス
				LDA		FDCSTAT
				ANDA	#$98
				BEQ		SEEK4
				ORCC	#$01
				RTS
* トラックセーブ
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
*	FDC フォーマット(2D)
*
*	RESULT: A	Type3 ステータス
*			CY	エラーフラグ
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
* BUSY待ち
FORMAT2			LDA		FDCSTAT
				LSRA
				BCS		FORMAT2
* コマンド実行
				LDA		#$02
				STA		<TYPE
				LDA		#$F0
				STA		FDCCMD
* データ転送
FORMAT3			LDA		FDDDRQ
				BPL		FORMAT4
				LDA		,U+
				STA		FDCDATA
				BRA		FORMAT3
FORMAT4			BITA	#$40
				BEQ		FORMAT3
* 転送終了
				LDA		FDCSTAT
				ANDA	#$E8
				BEQ		FORMAT5
				ORCC	#$01
				RTS
FORMAT5			CLRA
				RTS

*
*	FDCフォーマット データ作成
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
*	FDCフォーマット プリアンブル作成
*
*	USEREG: A,B,U
*
FM_PRE			LDD		#$4E50
				LBSR	FM_SUB
				LDD		#$000C
				LBSR	FM_SUB
* INDEXマーク
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
*	FDCフォーマット ID作成
*
*	USEREG: A,B,U
*
FM_ID			LDD		#$000C
				LBSR	FM_SUB
* IDマーク
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
*	FDCフォーマット データ部作成
*
*	USEREG: A,B,U
*
FM_DATA			LDD		#$000C
				LBSR	FM_SUB
* データマーク
				LDA		#$F5
				STA		,U+
				STA		,U+
				STA		,U+
				LDA		#$FB
				STA		,U+
* データ
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
*	FDCフォーマット ポストアンブル部作成
*
*	USEREG: A,B,U
*
FM_POST			LDD		#$4E00
				LBSR	FM_SUB
				LDD		#$4E90
				LBSR	FM_SUB
				RTS

*
*	FDCフォーマット サブ
*
*	PARAM.: A	書き込みデータ
*			B	繰り返し数
*
FM_SUB			STA		,U+
				DECB
				BNE		FM_SUB
				RTS

*	elsif	(defined(FORMAT2DD)||defined(FBLOADM))&&(DISKTYPE==D_SKIP2DD)
	elsif	(DISKTYPE==D_SKIP2DD)

*
*	FDC フォーマット (トラック抜き2DD)
*
*	RESULT: A	Type3 ステータス
*			CY	エラーフラグ
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
* BUSY待ち
FORMAT2			LDA		FDCSTAT
				LSRA
				BCS		FORMAT2
* コマンド実行
				LDA		#$02
				STA		<TYPE
				LDA		#$F0
				STA		FDCCMD
* データ転送
FORMAT3			LDA		FDDDRQ
				BPL		FORMAT4
				LDA		,U+
				STA		FDCDATA
				BRA		FORMAT3
FORMAT4			BITA	#$40
				BEQ		FORMAT3
* 転送終了
				LDA		FDCSTAT
				ANDA	#$E8
				BEQ		FORMAT5
				ORCC	#$01
				RTS
FORMAT5			CLRA
				RTS

*
*	FDCフォーマット データ作成
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
*	FDCフォーマット プリアンブル作成
*
*	USEREG: A,B,U
*
FM_PRE			LDD		#$4E50
				LBSR	FM_SUB
				LDD		#$000C
				LBSR	FM_SUB
* INDEXマーク
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
*	FDCフォーマット ID作成
*
*	USEREG: A,B,U
*
FM_ID			LDD		#$000C
				LBSR	FM_SUB
* IDマーク
				LDA		#$F5
				STA		,U+
				STA		,U+
				STA		,U+
				LDA		#$FE
				STA		,U+
* C,H,R,N
				LDA		<TRACK
				LSLA		偽2DD用
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
*	FDCフォーマット データ部作成
*
*	USEREG: A,B,U
*
FM_DATA			LDD		#$000C
				LBSR	FM_SUB
* データマーク
				LDA		#$F5
				STA		,U+
				STA		,U+
				STA		,U+
				LDA		#$FB
				STA		,U+
* データ
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
*	FDCフォーマット ポストアンブル部作成
*
*	USEREG: A,B,U
*
FM_POST			LDD		#$4E00
				LBSR	FM_SUB
				LDD		#$4E90
				LBSR	FM_SUB
				RTS

*
*	FDCフォーマット サブ
*
*	PARAM.: A	書き込みデータ
*			B	繰り返し数
*
FM_SUB			STA		,U+
				DECB
				BNE		FM_SUB
				RTS

	elsif	(defined(FBLOADM)||defined(FORMAT2DD))&&(DISKTYPE==D_NORM2DD)

*
*	FDC フォーマット (2DD)
*
*	RESULT: A	Type3 ステータス
*			CY	エラーフラグ
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
* BUSY待ち
FORMAT2			LDA		FDCSTAT
				LSRA
				BCS		FORMAT2
* コマンド実行
				LDA		#$02
				STA		<TYPE
				LDA		#$F0
				STA		FDCCMD
* データ転送
FORMAT3			LDA		FDDDRQ
				BPL		FORMAT4
				LDA		,U+
				STA		FDCDATA
				BRA		FORMAT3
FORMAT4			BITA	#$40
				BEQ		FORMAT3
* 転送終了
				LDA		FDCSTAT
				ANDA	#$E8
				BEQ		FORMAT5
				ORCC	#$01
				RTS
FORMAT5			CLRA
				RTS

*
*	FDCフォーマット データ作成
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
*	FDCフォーマット プリアンブル作成
*
*	USEREG: A,B,U
*
FM_PRE			LDD		#$4E50
				LBSR	FM_SUB
				LDD		#$000C
				LBSR	FM_SUB
* INDEXマーク
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
*	FDCフォーマット ID作成
*
*	USEREG: A,B,U
*
FM_ID			LDD		#$000C
				LBSR	FM_SUB
* IDマーク
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
*	FDCフォーマット データ部作成
*
*	USEREG: A,B,U
*
FM_DATA			LDD		#$000C
				LBSR	FM_SUB
* データマーク
				LDA		#$F5
				STA		,U+
				STA		,U+
				STA		,U+
				LDA		#$FB
				STA		,U+
* データ
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
*	FDCフォーマット ポストアンブル部作成
*
*	USEREG: A,B,U
*
FM_POST			LDD		#$4E00
				LBSR	FM_SUB
				LDD		#$4E90
				LBSR	FM_SUB
				RTS

*
*	FDCフォーマット サブ
*
*	PARAM.: A	書き込みデータ
*			B	繰り返し数
*
FM_SUB			STA		,U+
				DECB
				BNE		FM_SUB
				RTS

	endif

*
*	FDC セクタ読み込み(セクタ連続)
*
*	PARAM.: X	データバッファ
*	RESULT: A	Type2 ステータス
*			CY	エラーフラグ
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
* BUSY待ち
READM2			LDA		FDCSTAT
				LSRA
				BCS		READM2
* コマンド実行
				LDA		#$01
				STA		<TYPE
				LDA		#$80
				STA		FDCCMD
* データ転送
READM3			LDA		FDDDRQ
				BPL		READM4
				LDA		FDCDATA
				STA		,X+
				BRA		READM3
READM4			BITA	#$40
				BEQ		READM3
* 転送終了
				LDA		FDCSTAT
				ANDA	#$BC
				BEQ		READM5
				ORCC	#$01
				RTS
* 次のセクタへ
READM5			INC		<SECTOR
				DEC		<SECTORS
				BNE		READM1
				RTS

*
*	FDC セクタ書き込み(セクタ連続)
*
*	PARAM.: X	データバッファ
*	RESULT: A	Type2 ステータス
*			CY	エラーフラグ
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
* BUSY待ち
WRITEM2			LDA		FDCSTAT
				LSRA
				BCS		WRITEM2
* コマンド実行
				LDA		#$02
				STA		<TYPE
				LDA		#$A0
				STA		FDCCMD
* データ転送
WRITEM3			LDA		FDDDRQ
				BPL		WRITEM4
				LDA		,X+
				STA		FDCDATA
				BRA		WRITEM3
WRITEM4			BITA	#$40
				BEQ		WRITEM3
* 転送終了
				LDA		FDCSTAT
				ANDA	#$FC
				BEQ		WRITEM5
				ORCC	#$01
				RTS
* 次のセクタへ
WRITEM5			INC		<SECTOR
				DEC		<SECTORS
				BNE		WRITEM1
				RTS

	if	(DISKTYPE==D_NORM2D)

*
* DPB (360KB,$40バイト)
*
DPB_360			FCB		$EB,$FE,$90			* ジャンプルーチン
				FCC		/MSDOS5.0/			* OEM NAME
				FCB		$00,$02				* バイト数/セクタ
				FCB		$02					* セクタ数/クラスタ
				FCB		$01,$00				* 予約セクタ数
				FCB		$02					* FAT数
				FCB		$70,$00				* ルートディレクトリ最大数
				FCB		$D0,$02				* 物理セクタ数
				FCB		$FD					* メディアID
				FCB		$02,$00				* FATセクタ数
				FCB		$09,$00				* セクタ数/トラック
				FCB		$02					* ヘッド数
				FCB		$00,$00,$00,$00
				FCB		$00,$00,$00,$00
				FCB		$00,$00,$00,$00
				FCB		$00,$00,$00,$00
				FCC		/NO NAME    /		* ボリュームラベル
				FCC		/FAT12   /
				FCB		$00,$00				* 予備

	else

*
* DPB (720KB,$40バイト)
*
DPB_720			FCB		$EB,$FE,$90			* ジャンプルーチン
				FCC		/MSDOS5.0/			* OEM NAME
				FCB		$00,$02				* バイト数/セクタ
				FCB		$02					* セクタ数/クラスタ
				FCB		$01,$00				* 予約セクタ数
				FCB		$02					* FAT数
				FCB		$70,$00				* ルートディレクトリ最大数
				FCB		$A0,$05				* 物理セクタ数
				FCB		$F9					* メディアID
				FCB		$03,$00				* FATセクタ数
				FCB		$09,$00				* セクタ数/トラック
				FCB		$02					* ヘッド数
				FCB		$00,$00,$00,$00
				FCB		$00,$00,$00,$00
				FCB		$00,$00,$00,$00
				FCB		$00,$00,$00,$00
				FCC		/NO NAME    /		* ボリュームラベル
				FCC		/FAT12   /
				FCB		$00,$00				* 予備

	endif

*--[ DATA ]-------------------------------------------------------------------

	if	(MODEL>=M_FM77AV)

*
*	サブCPU コマンド (FM77AV)
*
SUBINIT_CMD		FCB		$01,$00,40,25,0,25,0,1,0
SUBCONS_CMD		FCB		$0C,$06
INKEY_CMD		FCB		$29,$01

*
*	漢字ROM変換テーブル
*
GET_KANJI_TBL	FCB		$09,$0A,$0D,$0E
				FCB		$0F,$11,$12,$13

	else

*
*	サブCPU コマンド (FM-7)
*
SUBINIT_CMD		FCB		$01,$00,40,25,0,25,0,1,0
SUBCONS_CMD		FCB		$0C,$06
INKEY_CMD		FCB		$29,$03
INKEY_CHK_CMD	FCB		$29,$00

*
*	サブCPU コマンド (FM-77L4)
*
SUBINIT_L4_CMD	FCB		$01,$07,40,25,0,25,0,1
SUBCONS_L4_CMD	FCB		$0C,$0E,$D6
INITL4_CMD		FCB		$3F,$59,$41,$4D,$41,$55,$43,$48,$49
				FCB		$93,$D3,$8F,$90
				STA		$D408
				LDA		SUBMODEL4
				ANDA	#$FD
				STA		SUBMODEL4
				RTS
INITL4_CMDe		EQU		*

	endif

*
*	ファイルネーム
*
FBASIC_FN		FCC		/FBASIC30ROM/,0
BOOTBAS_FN		FCC		/BOOT_BASROM/,0
BOOTDOS_FN		FCC		/BOOT_DOSROM/,0
KANJI_FN		FCC		/KANJI   ROM/,0
SUBSYSC_FN		FCC		/SUBSYS_CROM/,0

	if	(MODEL>=M_FM77AV)

INITIATE_FN		FCC		/INITIATEROM/,0
SUBSYSA_FN		FCC		/SUBSYS_AROM/,0
SUBSYSB_FN		FCC		/SUBSYS_BROM/,0
SUBSYSCG_FN		FCC		/SUBSYSCGROM/,0

	endif

	if	(MODEL==M_FM77AV40)

KANJI2_FN		FCC		/KANJI2  ROM/,0
DICROM_FN		FCC		/DICROM  ROM/,0
EXTSUB_FN		FCC		/EXTSUB  ROM/,0
BOOTMMR_FN		FCC		/BOOT_MMRROM/,0

	endif

	if	(MODEL==M_FM7)

BOOT1MB_FN		FCC		/BOOT_1MBROM/,0
BOOTMMR_FN		FCC		/BOOT_MMRROM/,0
SUBSYSL4_FN		FCC		/SUBSYSL4ROM/,0

	endif

*
*	表示メッセージ(起動時)
*
	if (MODEL==M_FM77AV)&&(DISKTYPE!=D_NORM2DD)
FM77AV_MSG		FCB		$12,12,11
				FCC		"FM77AV SERIES ONLY!",0
	elsif	((MODEL==M_FM77AV40)||(DISKTYPE==D_NORM2DD))
FM77AV_MSG		FCB		$12,$0A,$0B
				FCC		"FM77AV20/40/EX/SX ONLY!",0
	endif

	if	(MODEL>=M_FM77AV)&&(defined(FBLOADM)||(DISKTYPE==D_NORM2D))
TITLE_MSG		FCB		$11,$07,$12,$17,$06
				FCC		/ＸＭ７　ＲＯＭセーブユーティリティ/
				FCB		$12,$1C,$08
				FCC		/（Ｃ）　１９９９　ＰＩ．/
				FCB		$12,$14,$0a
				FCC		/（Ｃ）　２００３～２０１０　たけがみりう/,0
TITLE_MSG2		FCC		$12,$12,$10
	endif
	if	(MODEL>=M_FM77AV)&&defined(FBLOADM)&&(DISKTYPE==D_NORM2DD)
				FCC		/ドライブ０にブランク２ＤＤディスクをセットして/,0
	elsif	(MODEL>=M_FM77AV)&&defined(FBLOADM)&&(DISKTYPE!=D_NORM2DD)
				FCC		/ドライブ０にブランク２Ｄディスクをセットして/,0
	endif
	if	(MODEL>=M_FM77AV)&&(defined(FBLOADM)||(DISKTYPE==D_NORM2D))
FORMAT_MSG		FCB		$11,$07,$12,$1D,$09
				FCC		/フォーマットしています/,0
	elsif	(MODEL>=M_FM77AV)
FORMAT_MSG		FCB		$11,$07,$12,$1B,$09
				FCC		/論理フォーマットしています/,0
	elsif	(MODEL==M_FM7)&&(defined(FBLOADM)||(DISKTYPE==D_NORM2D))
TITLE_MSG		FCB		$11,$07,$12,$06,$06
				FCC		/XM7 ROM SAVE UTILITY for FM-7/
				FCB		$12,$02,$08
				FCC		"(C) 1999 PI. / 2003-2010 Ryu Takegami",0
TITLE_MSG2		FCB		$11,$04,$12,$06,$10
				FCC		/Set blank disk to drive 0 and/,0
FORMAT_MSG		FCB		$11,$07,$12,$0A,$09
				FCC		/Disk formatting .../,0
	elsif	(MODEL==M_FM7)
FORMAT_MSG		FCB		$11,$07,$12,$0A,$09
				FCC		/Disk formatting .../,0
	endif

*
* 表示メッセージ(ROMセーブ時)
*
	if	(MODEL>=M_FM77AV)
FBASIC_MSG		FCB		$11,$07,$12,$13,$09
				FCC		/Ｆ－ＢＡＳＩＣ　ＲＯＭ　をセーブしています/,0
BOOT_MSG		FCB		$11,$07,$12,$18,$09
				FCC		/ブートＲＯＭ　をセーブしています/,0
KANJI_MSG		FCB		$11,$07,$12,$15,$09
				FCC		/第１水準漢字ＲＯＭ　をセーブしています/,0
	else
FBASIC_MSG		FCB		$11,$07,$12,$09,$09
				FCC		/Saving F-BASIC ROM .../,0
BOOTBAS_MSG		FCB		$11,$07,$12,$05,$09
				FCC		/Saving BOOT ROM(BASIC mode) .../,0
BOOTDOS_MSG		FCB		$11,$07,$12,$06,$09
				FCC		/Saving BOOT ROM(DOS mode) .../,0
BOOT1MB_MSG		FCB		$11,$07,$12,$06,$09
				FCC		/Saving BOOT ROM(1MB mode) .../,0
BOOTMMR_MSG		FCB		$11,$07,$12,$07,$09
				FCC		/Saving BOOT ROM(Hidden) .../,0
KANJI_MSG		FCB		$11,$07,$12,$0A,$09
				FCC		/Saving Kanji ROM .../,0
BOOTB_DIPSW_MSG	FCB		$11,$07,$12,$02,$09
				FCC		/Please turn ON DIP SW 1 and DIP SW 2./,0
BOOTD_DIPSW_MSG	FCB		$11,$07,$12,$07,$09
				FCC		/Please turn ON DIP SW 1 and/
				FCB		$12,$0b,$0b
				FCC		/turn OFF DIP SW 2./,0
BOOTB_DIP77_MSG	FCB		$11,$07,$12,$08,$09
				FCC		/Please push BASIC switch./,0
BOOTD_DIP77_MSG	FCB		$11,$07,$12,$08,$09
				FCC		/Please push BOOT2 switch./,0
BOOTS_DIP77_MSG	FCB		$11,$07,$12,$08,$09
				FCC		/Please push BOOT1 switch./,0
	endif
	if	(MODEL==M_FM77AV40)
KANJI2_MSG		FCB		$11,$07,$12,$15,$09
				FCC		/第２水準漢字ＲＯＭ　をセーブしています/,0
	endif
	if	(MODEL>=M_FM77AV)
INITIATE_MSG	FCB		$11,$07,$12,$15,$09
				FCC		/イニシエートＲＯＭ　をセーブしています/,0
SUBSYSA_MSG		FCB		$11,$07,$12,$12,$09
				FCC		/サブシステムＲＯＭ（Ａ）　をセーブしています/,0
SUBSYSB_MSG		FCB		$11,$07,$12,$12,$09
				FCC		/サブシステムＲＯＭ（Ｂ）　をセーブしています/,0
SUBSYSC_MSG		FCB		$11,$07,$12,$12,$09
				FCC		/サブシステムＲＯＭ（Ｃ）　をセーブしています/,0
SUBSYSCG_MSG	FCB		$11,$07,$12,$11,$09
				FCC		/サブシステムＲＯＭ（ＣＧ）　をセーブしています/,0
	else
SUBSYSC_MSG		FCB		$11,$07,$12,$02,$09
				FCC		/Saving Sub monitor ROM (200Line) .../,0
SUBSYSL4_MSG	FCB		$11,$07,$12,$02,$09
				FCC		/Saving Sub monitor ROM (400Line) .../,0
	endif
	if	(MODEL==M_FM77AV40)
DICROM_MSG		FCB		$11,$07,$12,$19,$09
				FCC		/辞書ＲＯＭ　をセーブしています/,0
EXTSUB_MSG		FCB		$11,$07,$12,$19,$09
				FCC		/拡張ＲＯＭ　をセーブしています/,0
BOOTMMR_MSG		FCB		$11,$07,$12,$16,$09
				FCC		/隠しブートＲＯＭ　をセーブしています/,0
	endif

*
* 表示メッセージ(エラー)
*
	if	(MODEL>=M_FM77AV)
NOTREADY_MSG	FCB		$11,$02,$12,$19,$10
				FCC		/ディスクがセットされていません/,0
WRPROTECT_MSG	FCB		$11,$02,$12,$1A,$10
				FCC		/ライトプロテクトされています/,0
SEEKERR_MSG		FCB		$11,$02,$12,$20,$10
				FCC		/シークエラーです/,0
READERR_MSG		FCB		$11,$02,$12,$1F,$10
				FCC		/読み込みエラーです/,0
WRITEERR_MSG	FCB		$11,$02,$12,$1F,$10
				FCC		/書き込みエラーです/,0
SPACE_MSG		FCB		$11,$04,$12,$1A,$12
				FCC		/スペース/
				FCB		$11,$07
				FCC		/キーを押してください/,0
CLEAR_MSG		FCB		$11,$07,$12,$19,$10
				FCC		/　　　　　　　　　　　　　　　/
				FCB		$12,$1A,$12
				FCC		/　　　　　　　　　　　　　　/,0
COMPLETE_MSG	FCB		$11,$07,$12,$22,$09
				FCC		/終了しました/,0

	else

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

	endif

*
* プログラム終了
*
PRGEND			EQU		*
				END
