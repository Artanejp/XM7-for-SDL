*
*	FM-7 EMULATOR "XM7"
*
*	Copyright (C) 1999-2009 ＰＩ．(yasushi@tanaka.net)
*	Copyright (C) 2001-2009 Ryu Takegami
*
*	[ 日本語サブシステム ROMセーバ(XM7 V1.1対応) ]
*
*	対象機種・生成メディアタイプに応じてアセンブル時にオプション指定すること
*		-dDISKTYPE=0	360KB(2D)  通常フォーマット
*		-dDISKTYPE=1	720KB(2DD) 1トラック抜きフォーマット
*		-dDISKTYPE=2	720KB(2DD) 通常フォーマット
*

				ORG		$1000

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
BANKSEL			EQU		$FD0F			* ROM/裏RAMセレクトレジスタ
INITDIS			EQU		$FD10			* イニシエートROM ディセーブルレジスタ
FDCSTAT			EQU		$FD18			* FDC ステータスレジスタ
FDCCMD			EQU		FDCSTAT			* FDC コマンドレジスタ
FDCTRACK		EQU		$FD19			* FDC トラックレジスタ
FDCSECTOR		EQU		$FD1A			* FDC セクタレジスタ
FDCDATA			EQU		$FD1B			* FDC データレジスタ
FDDSIDE			EQU		$FD1C			* FDD サイドレジスタ
FDDDRIVE		EQU		$FD1D			* FDD ドライブレジスタ
FDDMODE			EQU		$FD1E			* FDD モードレジスタ
FDDDRQ			EQU		$FD1F			* FDC DRQ/IRQレジスタ
JSYNCR			EQU		$FD28			* 日本語サブ 同期レジスタ(R)
JRCBR			EQU		$FD29			* 日本語サブ RCBレジスタ(R)
JSYNCW			EQU		$FD2A			* 日本語サブ 同期レジスタ(W)
JRCBW			EQU		$FD2B			* 日本語サブ RCBレジスタ(W)
MMR				EQU		$FD80			* MMR メモリマッピングレジスタ
MSGR			EQU		$FD90			* MMR セグメントレジスタ
MMSR			EQU		$FD93			* MMR モードセレクトレジスタ

*
*	定数定義 (サブI/O)
*
SUBMODEL4		EQU		$D405			* サブモードレジスタ (L4)
CRTDSP			EQU		$D408			* CRT ON/OFFレジスタ
VACC			EQU		$D409			* VRAMアクセスフラグ

*
*	定数定義 (日本語サブシステム)
*
JSRCBAREA		EQU		$9F00			* メインとのRCB通信エリア
JSDICBNK		EQU		$9FFF			* 辞書バンクレジスタ

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
				FCC		/SAVEJDIC V1.02  /
				FCC		"Copyright 1999 ＰＩ．/ 2006 Ryu Takegami"
				FCB		$0D,$0A,$00

*
*	メインプログラム
*
				ORG		START+$80
MAIN			ORCC	#$50
				LEAS	STACK,PCR

* ハードウェア初期化
				LBSR	INIT
				LBSR	JRESET
				LBSR	FM7_INIT

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

* 日本語サブ側セーブ処理
MAINSAVE		LBSR	JSUBMON
				LBSR	DICROM

* 完了(メッセージ表示・ループ)
				LBSR	JRESET
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
INIT1			CLR		,X+
				DECA
				BNE		INIT1
* 画面初期化
				LEAX	SUBINIT_CMD,PCR
				LDB		#$09
				LBSR	SUB_CMD
				LEAX	SUBCONS_CMD,PCR
				LDB		#$02
				LBSR	SUB_CMD
* 日本語通信カードチェック
				LBSR	JSUBCHK
				BCC		INIT2
				LEAX	NOTFOUND_MSG,PCR
				LBSR	PUT_STRING
				BRA		*
INIT2			RTS

*--[ ROMセーバ(日本語サブシステム) ]------------------------------------------

*
*	日本語サブシステムROM
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
*	辞書ROM
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

* 64回ループ
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

*--[ 日本語通信カード制御部 v1.01  coded by RyuTak 2006/07/01 ]---------------

*
*	日本語通信カード認識チェック
*
*	out
*		Cflag=1	カードが存在しない(たぶん)
*
JSUBCHK			LDA		JSYNCR
				BMI		JCHKNOCARD
				ANDCC	#$FE
				RTS
JCHKNOCARD		ORCC	#$01
				RTS

*
*	日本語サブシステム停止
*
JSUBHALT		PSHS	A
JSUBHALT1		LDA		JSYNCR
				BMI		JSUBHALT1
				CLR		JSYNCW
				PULS	A,PC

*
*	日本語サブシステム始動
*
JSUBRUN			PSHS	A
				LDA		#$80
				STA		JSYNCW
				PULS	A,PC

*
*	日本語サブシステムリセット
*
JRESET			PSHS	A
				LBSR	JSUBHALT
				LDA		#$0C
				STA		JRCBW
				LBSR	JSUBRUN
				PULS	A,PC

*
*	日本語サブシステムへのコマンド送信
*
JSENDBLK		CLR		JSYNCW
JSENDBLK1		LDA		,X+
				STA		JRCBW
				DECB
				BNE		JSENDBLK1
				LBSR	JSUBRUN
				RTS

*
*	日本語サブシステムからのデータ転送(yamamoto ver.)
*
*	in
*		A = 日本語サブ側辞書ROMバンクナンバ
*		X = 日本語サブ側転送元アドレス
*		Y = メインCPU 側転送先アドレス
*		U = 転送サイズ(手抜きなので128バイト単位で)
*
JTFR			PSHS	D,X,Y,U,CC,DP
				PSHS	A
* YAMAMOTOコマンド使用のため一旦ダミーコマンドを送信
				PSHS	X
				LDB		#JDUMMYCMDEND-JDUMMYCMD
				LEAX	JDUMMYCMD,PCR
				LBSR	JSENDBLK
* 転送プログラムを送信
				LDB		#JSENDPRGEND-JSENDPRG
				LEAX	JSENDPRG,PCR
				LBSR	JSENDBLK
				PULS	X
* 転送要求する辞書ROMバンクナンバ・アドレスを送信
JTFRLOOP1		LBSR	JSUBHALT
				LDA		,S
				STA		JRCBW
				TFR		X,D
				STA		JRCBW
				STB		JRCBW
				LBSR	JSUBRUN
				LBSR	JSUBHALT
* 128バイト単位で転送を行う
				LDB		#128
JTFRLOOP3		LDA		JRCBR
				STA		,Y+
				DECB
				BNE		JTFRLOOP3
				LEAX	128,X
				LEAU	-128,U
				CMPU	#$0000
				BNE		JTFRLOOP1
* 転送ストップ命令を送信
				LEAS	1,S
				LBSR	JSUBHALT
				LDA		#$FF
				STA		JRCBW
				LBSR	JSUBRUN
				LBSR	JSUBHALT
				PULS	D,X,Y,U,CC,DP,PC

*
*	起動時に送信するダミーYAMAMOTOコマンド
*	  (これをやらないと以後のYAMAMOTOコマンドが正常動作しない)
*
JDUMMYCMD		FCB		$80
				FCC		/YAMAMOTO/
				FCB		$00
JDUMMYCMDEND	EQU		*

*
*	データ転送用プログラム
*	 (日本語サブシステム側 $9E00-に転送される)
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
* メイン側からのデータ送信を待つ
JSENDPRG1		CLR		$9FFF
				LDX		#10
JSENDPRG2		MUL
				LEAX	-1,X
				BNE		JSENDPRG2
* 受信データが$FFなら転送ストップ命令
* 受信データが$FF以外ならそのバンク(下位5bit)のデータを128バイト転送
* (辞書ROM以外の領域ではバンク情報は関係なし)
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
* YAMAMOTOコマンド処理アドレスを調整して辞書アクセスルーチンに飛ぶ
JSENDPRG4		LDX		#JSRCBAREA+9
				STX		$800C
				JMP		[$FFFE]
JSENDPRGEND		EQU		*

*--[ FM-7 ]-------------------------------------------------------------------

*
*	FM-7初期化 (77チェック付き)
*
FM7_INIT		TST		BANKSEL
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
CLS				LEAX	CLS_CMD,PCR
				LDB		#CLS_CMDe-CLS_CMD
				LBRA	SUB_CMD

*
*	サブCPU 画面クリアプログラム
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

*
*	サブCPU コマンド (FM-7)
*
SUBINIT_CMD		FCB		$01,$00,40,25,0,25,0,1,0
SUBCONS_CMD		FCB		$0C,$06
INKEY_CMD		FCB		$29,$03
INKEY_CHK_CMD	FCB		$29,$00

*
*	ファイルネーム
*
JSUBMON_FN		FCC		/JSUBMON ROM/,0
DICROM_FN		FCC		/DICROM  ROM/,0

*
*	表示メッセージ(起動時)
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
* 表示メッセージ(ROMセーブ時)
*
JSUBMON_MSG		FCB		$11,$07,$12,$03,$09
				FCC		/Saving Japanese Sub Monitor ROM .../,0
DICROM_MSG		FCB		$11,$07,$12,$07,$09
				FCC		/Saving Dictionary ROM .../,0

*
* 表示メッセージ(エラー)
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
* プログラム終了
*
PRGEND			EQU		*
				END
