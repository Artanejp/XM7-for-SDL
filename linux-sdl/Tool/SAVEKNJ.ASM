*
*	FM-7 EMULATOR "XM7"
*
*	Copyright (C) 1999-2012 ＰＩ．(yasushi@tanaka.net)
*	Copyright (C) 2001-2012 Ryu Takegami
*
*	[ JIS78 ROMセーバ(XM7 V3対応) ]
*
*	対象機種・生成メディアタイプに応じてアセンブル時にオプション指定すること
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
INITDIS			EQU		$FD10			* イニシエータROM ディセーブルレジスタ
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
M_KANJI			EQU		4			* FM77AV40EX/SX(漢字ROM)
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
				FCC		/SAVEKNJ /
				FCC		/ V1.35  /
				FCC		"Copyright 1999 ＰＩ．/ 2003-2012 Ryu Takegami"
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
				LBSR	FM77AV_INIT

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
MAINSAVE		LBSR	KANJI

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
				LBSR	IS_SUPPORT2DD
				BCC		INIT2
				LEAX	FM77AV_MSG,PCR
				LBSR	PRINT
				BRA		*
INIT2			RTS

*--[ ROMセーバ(JIS78準拠漢字ROM) ]--------------------------------------------

*
*	拡張サブシステムROM内JIS78準拠漢字ROM(AV40EX/SX only)
*
KANJI			CLR		INITDIS
				LDA		$6B0E
				LDB		$6B10
				CMPD	#$3431
				BNE		KANJI2

* 拡張ROMマッピング・セレクト
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
* 64回ループ
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
* イニシエータROMへ切り替え、読み出しチェック
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
* イニシエータROMへ切り替え、読み出しチェック
				CLR		INITDIS
				LDA		$6B0E
				CMPA	#$34
				BEQ		IS_SUPPORT2DD1
				CMPA	#$32
				BNE		IS_SUPPORT2DD2
* 忘れずに標準RAMに戻しておく
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

*--[ FM77AV ]-----------------------------------------------------------------

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

*--[ 漢字出力(FM77AV) ]-------------------------------------------------------

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

*--[ DOS ]--------------------------------------------------------------------

	include		diskdrv.asm

*--[ DATA ]-------------------------------------------------------------------

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

*
*	ファイルネーム
*
KANJI_FN		FCC		/KANJI   ROM/,0

*
*	表示メッセージ(起動時)
*
FM77AV_MSG		FCB		$12,$0A,$0B
				FCC		"FM77AV40EX/SX ONLY!",0

	if	(defined(FBLOADM))
TITLE_MSG		FCB		$11,$07,$12,$17,$06
				FCC		/ＸＭ７　ＲＯＭセーブユーティリティ/
				FCB		$12,$1C,$08
				FCC		/（Ｃ）　１９９９　ＰＩ．/
				FCB		$12,$14,$0a
				FCC		/（Ｃ）　２００３～２０１２　たけがみりう/,0
TITLE_MSG2		FCC		$12,$12,$10
	endif
				FCC		/ドライブ０にブランク２ＤＤディスクをセットして/,0
FORMAT_MSG		FCB		$11,$07,$12,$1B,$09
				FCC		/論理フォーマットしています/,0

*
* 表示メッセージ(ROMセーブ時)
*
KANJI_MSG		FCB		$11,$07,$12,$0F,$09
				FCC		/第１水準漢字ＲＯＭ（ＪＩＳ７８）　をセーブしています/,0

*
* 表示メッセージ(エラー)
*
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

*
* プログラム終了
*
PRGEND			EQU		*
				END
