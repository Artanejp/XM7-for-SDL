*
*	FM-7 EMULATOR "XM7"
*
*	Copyright (C) 1999-2012 ＰＩ．(yasushi@tanaka.net)
*	Copyright (C) 2001-2012 Ryu Takegami
*
*	[ ROMセーバ RS-232Cドライバ ]
*

*--[ RS-232Cドライバ ]--------------------------------------------------------

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
