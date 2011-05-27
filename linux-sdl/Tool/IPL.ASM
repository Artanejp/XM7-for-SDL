*
*	FM-7 EMULATOR "XM7"
*
*	Copyright (C) 1999-2009 ＰＩ．(yasushi@tanaka.net)
*	Copyright (C) 2001-2009 Ryu Takegami
*
*	[ ユーティリティIPL ]
*

				ORG		$0100

*
*	定数定義
*
PRGENT			EQU		$1000		* ロードするプログラムの先頭アドレス
BEEP			EQU		$FD03		* BEEP制御レジスタ
BANKSEL			EQU		$FD0F		* ROM/裏RAMセレクトレジスタ
FDDMODE			EQU		$FD1E		* FDD モードレジスタ
DREAD			EQU		$FE08		* ディスクリードエントリ
RCBDBA			EQU		2			* RCB データバッファアドレス
RCBTRK			EQU		4			* RCB トラックナンバ
RCBSCT			EQU		5			* RCB セクタナンバ
RCBSID			EQU		6			* RCB サイドナンバ
RCBUNT			EQU		7			* RCB ユニットナンバ

*
* プログラムスタート
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
* メイン
*
MAIN			ORCC	#$50
* 初期化
				LDS		#$0F00
				LDA		#$FD
				TFR		A,DP
				STA		<BANKSEL
				CLR		<FDDMODE
* 本体読み込み
				LDB		#$08
LOOP			LEAX	<RCB,PCR
				PSHS	B
				JSR		DREAD
				PULS	B
				TSTA
				BNE		ERROR
* 次のセクタへ
				INC		RCBDBA,X
				INC		RCBDBA,X
				INC		RCBSCT,X
				DECB
				BNE		LOOP

*
* ジャンプ
*
				JMP		PRGENT

*
* 読み込みエラー
*
ERROR			LDA		#$81
				STA		<BEEP
				BRA		*

*
* RCB
*
RCB				FCB		$0A,$00		* DREAD
				FDB		PRGENT		* バッファアドレス
				FCB		$00			* トラック
				FCB		$02			* セクタ
				FCB		$00			* サイド
				FCB		$00			* ドライブ

*
* あまりデータ
*
OMAKE			ORG		START+$80
				FCC		/このプログラムはXM7のために/
				FCC		/今年(1999年)に作成したものです。/
				FCC		/特徴として、MS-DOS下位互換の/
				FCC		/簡単なファイルシステムを持っており/
				FCC		/Win9xで互換性のためサポートされている、360KB/
				FCC		/メディアの作成及びファイル書き込み/
				FCC		/ができます。なお/
				FCC		/開発にはMS-DOS上でクロスアセンブラ/
				FCC		/AS63.EXEを使用しました。使いやすい/
				FCC		/アセンブラをFSWで提供頂いた作者/
				FCC		/の方に感謝します。/
				FCC		/1999.7.31 written by ＰＩ．/
				FCC		/(ytanaka@ipc-tokai.or.jp)/

*
* プログラム終了
*
				END
