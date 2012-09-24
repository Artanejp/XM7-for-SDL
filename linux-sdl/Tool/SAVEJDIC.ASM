*
*	FM-7 EMULATOR "XM7"
*
*	Copyright (C) 1999-2012 ＰＩ．(yasushi@tanaka.net)
*	Copyright (C) 2001-2012 Ryu Takegami
*
*	[ 日本語サブシステム ROMセーバ(XM7 V1.1対応) ]
*
*	対象機種・生成メディアタイプに応じてアセンブル時にオプション指定すること
*		-dDISKTYPE=0	360KB(2D)  通常フォーマット
*		-dDISKTYPE=1	720KB(2DD) 1トラック抜きフォーマット
*		-dDISKTYPE=2	720KB(2DD) 通常フォーマット
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
*	実機CMT I/F経由強制動作用 転送プログラム
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
				FCC		/SAVEJDIC V1.03  /
				FCC		"Copyright 1999 ＰＩ．/ 2012 Ryu Takegami"
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
* ブートROMの裏RAMが使用できるか確認 (使用できるならFM-77)
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

	include	diskdrv.asm

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
				FCC		"(C) 1999 PI. / 2003-2012 Ryu Takegami",0
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
