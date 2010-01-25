;
; FM-7 EMULATOR "XM7"
;
; Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
; Copyright (C) 2001-2010 Ryu Takegami
;
; [ 6809CPU(x86アセンブラ版) ]
;

;
; NASM(Netwide Assembler) Only!
;
; ターゲット・プラットフォームに応じてオプション指定すること
;
; MS-DOS		-d __MSDOS__	-f obj
; Win32(Borland)	-d _OMF		-f obj
; Win32(Microsoft)	-d _WIN32 	-f win32
; Linux			-d _XWIN	-f elf
;
; Win32(Borland/Microsoft)の場合メモリハンドラをfastcall規約で呼び出す
; それ以外はcdecl規約
;
; 標準はXM7インタフェース、_hootを指定するとhootインタフェースとなる
; hootの場合は -d _WIN32 -d _hoot -f win32 でアセンブルすること
;

;
; 外部定義
;

%ifdef _hoot
;-----------------------------------------------------------------------------
; hoot向けここから
; http://dmpsoft.viraualave.net/
;-----------------------------------------------------------------------------
	%ifdef	_OMF
		section	.data class=DATA align=4 use32
		extern	_xm7_6809

		section	.text class=CODE align=16 use32
		global	_xm7_6809_exec
		global	_xm7_6809_rst
	%endif
	%ifdef	_WIN32
		section	.data class=DATA align=4 use32
		extern	_xm7_6809

		section	.text class=CODE align=16 use32
		global	_xm7_6809_exec
		global	_xm7_6809_rst
	%endif
	%ifdef __MSDOS__
		segment	_data public align=2 class=DATA
		extern	_xm7_6809

		segment	_cpu_x86_text public align=1 class=CODE
		global	_xm7_6809_exec
		global	_xm7_6809_rst
	%endif
	%ifdef	_XWIN
		section	.data data align=16
		extern	xm7_6809
		%define	_xm7_6809	xm7_6809
		section	.text code align=16
		global	xm7_6809_exec
		global	xm7_6809_rst
	%endif
;-----------------------------------------------------------------------------
; hoot向けここまで
; http://dmpsoft.viraualave.net/
;-----------------------------------------------------------------------------
%else

	%ifdef	_OMF
		section	.data class=DATA align=4 use32
		extern	_maincpu
		extern	_subcpu
	%ifdef	JSUB
		extern	_jsubcpu
	%endif
		extern	_fetch_op

		section	.text class=CODE align=16 use32
		global	_main_exec
		global	_sub_exec
		global	_main_line
		global	_sub_line
		global	_main_reset
		global	_sub_reset
	%ifdef	JSUB
		global	_jsub_exec
		global	_jsub_line
		global	_jsub_reset
	%endif
	%endif
	%ifdef	_WIN32
		section	.data class=DATA align=4 use32
		extern	_maincpu
		extern	_subcpu
	%ifdef	JSUB
		extern	_jsubcpu
	%endif
		extern	_fetch_op

		section	.text class=CODE align=16 use32
		global	_main_exec
		global	_sub_exec
		global	_main_line
		global	_sub_line
		global	_main_reset
		global	_sub_reset
	%ifdef	JSUB
		global	_jsub_exec
		global	_jsub_line
		global	_jsub_reset
	%endif
	%endif
	%ifdef __MSDOS__
		segment	_data public align=2 class=DATA
		extern	_maincpu
		extern	_subcpu
	%ifdef	JSUB
		extern	_jsubcpu
	%endif
		extern	_fetch_op

		segment	_cpu_x86_text public align=1 class=CODE
		global	_main_exec
		global	_sub_exec
		global	_main_line
		global	_sub_line
		global	_main_reset
		global	_sub_reset
	%ifdef	JSUB
		global	_jsub_exec
		global	_jsub_line
		global	_jsub_reset
	%endif
	%endif
	%ifdef	_XWIN
		section	.data data align=16
		extern	maincpu
		extern	subcpu
	%ifdef	JSUB
		extern	_jsubcpu
	%endif
		extern	fetch_op
		%define	_maincpu	maincpu
		%define	_subcpu		subcpu
		%define	_fetch_op	fetch_op
		%define	_jsubcpu	jsubcpu

		section	.text code align=16
		global	main_exec
		global	sub_exec
		global	main_line
		global	sub_line
		global	main_reset
		global	sub_reset
	%ifdef	JSUB
		global	_jsub_exec
		global	_jsub_line
		global	_jsub_reset
	%endif
	%endif
%endif

;
; レジスタ定義
;
	%ifdef	__MSDOS__
		%define	CCREG	byte [si]
		%define	DPREG	byte [si+1]
		%define	BREG	byte [si+2]
		%define	AREG	byte [si+3]
		%define	DREG	word [si+2]
		%define	XREG	word [si+4]
		%define	YREG	word [si+6]
		%define	UREG	word [si+8]
		%define	SREG	word [si+10]
		%define	PCREG	word [si+12]
		%define	INTR	word [si+14]
		%define	CYCLE	word [si+16]
		%define	TOTAL	word [si+18]
		%define	READMEM	far [si+20]
		%define	WRITEMEM far [si+24]
	%else
		%define	CCREG	byte [esi]
		%define	DPREG	byte [esi+1]
		%define	BREG	byte [esi+2]
		%define	AREG	byte [esi+3]
		%define	DREG	word [esi+2]
		%define	XREG	word [esi+4]
		%define	YREG	word [esi+6]
		%define	UREG	word [esi+8]
		%define	SREG	word [esi+10]
		%define	PCREG	word [esi+12]
		%define	INTR	word [esi+14]
		%define	CYCLE	word [esi+16]
		%define	TOTAL	word [esi+18]
		%define	READMEM	dword [esi+20]
		%define	WRITEMEM dword [esi+24]
	%endif

;
; アドレステーブル
;
	%ifdef	__MSDOS__
		%define	ADDR	dw
	%else
		%define	ADDR	dd
	%endif

	%macro	JMPT3	2
		ADDR	%1
		ADDR	%2
	%endmacro

;
; アラインメント
;
		%macro	ALIGN	0
	%ifndef	__MSDOS__
		align	4
	%endif
		%endmacro

;
; 読み出し(BYTE)
;
		%macro	READB	0
	%ifdef	__MSDOS__
		push	bp
		call	READMEM
		add	sp,2
	%endif
	%ifdef	_XWIN
		push	ebp
		call	READMEM
		add	esp,byte 4
	%endif
	%ifdef	_WIN32
		mov	ecx,ebp
		call	READMEM
;		push	ebp
;		call	READMEM
;		add	esp,byte 4
	%endif
	%ifdef	_OMF
		mov	eax,ebp
		call	READMEM
	%endif
		%endmacro

;
; 読み出し(WORD)
;
		%macro	READW	0
	%ifdef	__MSDOS__
		push	bp
		call	READMEM
		mov	ah,al
		xor	al,al
		mov	di,ax
		inc	bp
		push	bp
		call	READMEM
		add	sp,4
		xor	ah,ah
		or	ax,di
	%endif
	%ifdef	_XWIN
		push	ebp
		call	READMEM
		mov	ah,al
		xor	al,al
		mov	di,ax
		inc	bp
		push	ebp
		call	READMEM
		add	esp,byte 8
		xor	ah,ah
		or	ax,di
	%endif
	%ifdef	_WIN32
		mov	ecx,ebp
		call	READMEM
		mov	ebx,eax
		inc	ebp
		mov	ecx,ebp
		call	READMEM
		mov	ah,bl
;		push	ebp
;		call	READMEM
;		mov	ah,al
;		xor	al,al
;		mov	di,ax
;		inc	bp
;		push	ebp
;		call	READMEM
;		add	esp,byte 8
;		xor	ah,ah
;		or	ax,di
	%endif
	%ifdef	_OMF
		mov	eax,ebp
		call	READMEM
		mov	ebx,eax
		inc	ebp
		mov	eax,ebp
		call	READMEM
		mov	ah,bl
	%endif
		%endmacro

;
; 書き込み(BYTE)
;
		%macro	WRITEB	0
	%ifdef	__MSDOS__
		push	ax
		push	bp
		call	WRITEMEM
		add	sp,4
	%endif
	%ifdef	_XWIN
		push	eax
		push	ebp
		call	WRITEMEM
		add	esp,byte 8
	%endif
	%ifdef	_WIN32
		mov	edx,eax
		mov	ecx,ebp
		call	WRITEMEM
;		push	eax
;		push	ebp
;		call	WRITEMEM
;		add	esp,byte 8
	%endif
	%ifdef	_OMF
		mov	edx,eax
		mov	eax,ebp
		call	WRITEMEM
	%endif
		%endmacro

;
; 書き込み(WORD)
;
		%macro	WRITEW	0
	%ifdef	__MSDOS__
		mov	di,ax
		xchg	ah,al
		push	ax
		push	bp
		call	WRITEMEM
		inc	bp
		push	di
		push	bp
		call	WRITEMEM
		add	sp,8
	%endif
	%ifdef	_XWIN
		mov	edi,eax
		xchg	ah,al
		push	eax
		push	ebp
		call	WRITEMEM
		inc	bp
		push	edi
		push	ebp
		call	WRITEMEM
		add	esp,byte 16
	%endif
	%ifdef	_WIN32
		mov	ebx,eax
		mov	dl,ah
		mov	ecx,ebp
		call	WRITEMEM
		mov	ecx,ebp
		mov	edx,ebx
		inc	ecx
		call	WRITEMEM
;		mov	edi,eax
;		xchg	ah,al
;		push	eax
;		push	ebp
;		call	WRITEMEM
;		inc	bp
;		push	edi
;		push	ebp
;		call	WRITEMEM
;		add	esp,byte 16
	%endif
	%ifdef	_OMF
		mov	ebx,eax
		mov	dl,ah
		mov	eax,ebp
		call	WRITEMEM
		mov	eax,ebp
		mov	edx,ebx
		inc	eax
		call	WRITEMEM
	%endif
		%endmacro

;
; Zフラグクリア
;
		%macro	CLR_Z	0
		and	CCREG,0fbh
		%endmacro

;
; N,Zフラグクリア
;
		%macro	CLR_NZ	0
		and	CCREG,0f3h
		%endmacro

;
; N,Z,Vフラグクリア
;
		%macro	CLR_NZV	0
		and	CCREG,0f1h
		%endmacro

;
; N,Z,Cフラグクリア
;
		%macro	CLR_NZC	0
		and	CCREG,0f2h
		%endmacro

;
; N,Z,V,Cフラグクリア
;
		%macro	CLR_NZVC	0
		and	CCREG,0f0h
		%endmacro

;
; H,N,Z,V,Cフラグクリア
;
		%macro	CLR_HNZVC	0
		and	CCREG,0d0h
		%endmacro

;
; Cフラグセット
;
		%macro	SET_C	0
		or	CCREG,01h
		%endmacro

;
; Zフラグセット
;
		%macro	SET_Z	0
		or	CCREG,04h
		%endmacro

;
; N,Zフラグセット
;
		%macro	SET_NZ	0
	%ifdef	__MSDOS__
		pop	ax
	%else
		pop	eax
	%endif
		shr	al,4
		and	al,0ch
		or	CCREG,al
		%endmacro

;
; N,Z,Vフラグセット
;
		%macro	SET_NZV	0
	%ifdef	__MSDOS__
		pop	ax
	%else
		pop	eax
	%endif
		shr	ah,2
		and	ah,02h
		shr	al,4
		and	al,0ch
		or	al,ah
		or	CCREG,al
		%endmacro

;
; N,Z,Cフラグセット
;
		%macro	SET_NZC	0
	%ifdef	__MSDOS__
		pop	ax
	%else
		pop	eax
	%endif
		mov	ah,al
		and	ah,01h
		shr	al,4
		and	al,0ch
		or	al,ah
		or	CCREG,al
		%endmacro

;
; N,Z,V,Cフラグセット
;
		%macro	SET_NZVC	0
	%ifdef	__MSDOS__
		pop	ax
	%else
		pop	eax
	%endif
		shr	ah,2
		and	ah,02h
		mov	bh,al
		and	bh,01h
		shr	al,4
		and	al,0ch
		or	al,ah
		or	al,bh
		or	CCREG,al
		%endmacro

;
; H,N,Z,V,Cフラグセット
;
		%macro	SET_HNZVC	0
	%ifdef	__MSDOS__
		pop	ax
	%else
		pop	eax
	%endif
		shr	ah,2
		and	ah,02h
		mov	bh,al
		and	bh,01h
		mov	bl,al
		add	bl,bl
		and	bl,20h
		shr	al,4
		and	al,0ch
		or	al,ah
		or	al,bh
		or	al,bl
		or	CCREG,al
		%endmacro

;
; イミディエイトモード(BYTE)
;
		%macro	IMMB	0
		READB
		inc	PCREG
		%endmacro

;
; イミディエイトモード(WORD)
;
		%macro	IMMW	0
		READW
		add	PCREG,2
		%endmacro

;
; ダイレクトモード
;
		%macro	DIRECT	0
		READB
		inc	PCREG
		mov	ah,DPREG
	%ifdef __MSDOS__
		mov	bp,ax
	%else
		mov	ebp,eax
	%endif
		%endmacro

;
; エクステンドモード
;
		%macro	EXTEND	0
		READW
		add	PCREG,2
	%ifdef __MSDOS__
		mov	bp,ax
	%else
		mov	ebp,eax
	%endif
		%endmacro

;
; ロード(BYTE)
;
		%macro	LOADB	1
		mov	%1,al
		or	al,al
		pushf
		CLR_NZV
		SET_NZ
		%endmacro

;
; ロード(WORD)
;
		%macro	LOADW	1
		mov	%1,ax
		or	ax,ax
		pushf
		CLR_NZV
		SET_NZ
		%endmacro

;
; ストア(BYTE)
;
		%macro	STOREB	1
		mov	al,%1
		or	al,al
		pushf
		CLR_NZV
		SET_NZ
		mov	al,%1
		WRITEB
		%endmacro

;
; ストア(WORD)
;
		%macro	STOREW	1
		mov	ax,%1
		or	ax,ax
		pushf
		CLR_NZV
		SET_NZ
		mov	ax,%1
		WRITEW
		%endmacro

;
; クリア(BYTE)
;
		%macro	CLRB	1
		mov	%1,00h
		CLR_NZVC
		SET_Z
		%endmacro

;
; クリア(MEMORY)
;
		%macro	CLRM	0
; リード・サイクルが挟まる
; Alpha対策 (CLR $D404)
		READB
		xor	al,al
		WRITEB
		CLR_NZVC
		SET_Z
		%endmacro

;
; フラグ固定クリア(BYTE)
;
		%macro	CLCB	1
		mov	%1,0
		CLR_NZV
		SET_Z
		%endmacro

;
; 加算(BYTE)
;
		%macro	ADDB	1
		add	%1,al
		pushf
		CLR_HNZVC
		SET_HNZVC
		%endmacro

;
; 加算(WORD)
;
		%macro	ADDW	1
		add	%1,ax
		pushf
		CLR_HNZVC
		SET_HNZVC
		%endmacro

;
; キャリーつき加算(BYTE)
;
		%macro	ADCB	1
		mov	ah,CCREG
		rcr	ah,1
		adc	%1,al
		pushf
		CLR_HNZVC
		SET_HNZVC
		%endmacro

;
; 減算(BYTE)
;
		%macro	SUBB	1
		sub	%1,al
		pushf
		CLR_HNZVC
		SET_NZVC
		%endmacro

;
; キャリーつき減算(BYTE)
;
		%macro	SBCB	1
		mov	ah,CCREG
		rcr	ah,1
		sbb	%1,al
		pushf
		CLR_HNZVC
		SET_NZVC
		%endmacro

;
; 減算(WORD)
;
		%macro	SUBW	1
		sub	%1,ax
		pushf
		CLR_NZVC
		SET_NZVC
		%endmacro

;
; コンペア(BYTE)
;
		%macro	CMPB	1
		mov	bl,%1
		sub	bl,al
		pushf
		CLR_NZVC
		SET_NZVC
		%endmacro

;
; コンペア(WORD)
;
		%macro	CMPW	1
		mov	bx,%1
		sub	bx,ax
		pushf
		CLR_NZVC
		SET_NZVC
		%endmacro

;
; テスト(BYTE)
;
		%macro	TSTB	1
		mov	al,%1
		or	al,al
		pushf
		CLR_NZV
		SET_NZ
		%endmacro

;
; テスト(MEMORY)
;
		%macro	TSTM	0
		READB
		or	al,al
		pushf
		CLR_NZV
		SET_NZ
		%endmacro

;
; インクリメント(BYTE)
;
		%macro	INCB	1
		inc	%1
		pushf
		CLR_NZV
		SET_NZV
		%endmacro

;
; インクリメント(MEMORY)
;
		%macro	INCM	0
		READB
		inc	al
		pushf
		WRITEB
		CLR_NZV
		SET_NZV
		%endmacro

;
; デクリメント(BYTE)
;
		%macro	DECB	1
		dec	%1
		pushf
		CLR_NZV
		SET_NZV
		%endmacro

;
; デクリメント(MEMORY)
;
		%macro	DECM	0
		READB
		dec	al
		pushf
		WRITEB
		CLR_NZV
		SET_NZV
		%endmacro

;
; デクリメント・キャリー(BYTE)
;
		%macro	DCCB	1
		dec	%1
		pushf
		CLR_NZVC
		SET_NZV
		mov	al,CCREG
		shr	al,2
		not	al
		and	al,01h
		or	CCREG,al
		%endmacro

;
; デクリメント・キャリー(MEMORY)
;
		%macro	DCCM	0
		READB
		dec	al
		pushf
		WRITEB
		CLR_NZVC
		SET_NZV
		mov	al,CCREG
		shr	al,2
		not	al
		and	al,01h
		or	CCREG,al
		%endmacro

;
; ネガティブ(BYTE)
;
		%macro	NEGB	1
		neg	%1
		pushf
		CLR_NZVC
		SET_NZVC
		%endmacro

;
; ネガティブ(MEMORY)
;
		%macro	NEGM	0
		READB
		neg	al
		pushf
		WRITEB
		CLR_NZVC
		SET_NZVC
		%endmacro

;
; 否定(BYTE)
;
		%macro	COMB	1
		mov	al,%1
		not	al
		or	al,al
		pushf
		mov	%1,al
		CLR_NZVC
		SET_NZ
		SET_C
		%endmacro

;
; 否定(MEMORY)
;
		%macro	COMM	0
		READB
		not	al
		or	al,al
		pushf
		WRITEB
		CLR_NZVC
		SET_NZ
		SET_C
		%endmacro

;
; 論理積(BYTE)
;
		%macro	ANDB	1
		and	%1,al
		pushf
		CLR_NZV
		SET_NZ
		%endmacro

;
; 論理和(BYTE)
;
		%macro	ORB	1
		or	%1,al
		pushf
		CLR_NZV
		SET_NZ
		%endmacro

;
; 排他的論理和(BYTE)
;
		%macro	EORB	1
		xor	%1,al
		pushf
		CLR_NZV
		SET_NZ
		%endmacro

;
; ビットテスト(BYTE)
;
		%macro	BITB	1
		and	al,%1
		pushf
		CLR_NZV
		SET_NZV
		%endmacro

;
; 右論理シフト(BYTE)
;
		%macro	LSRB	1
		shr	%1,1
		pushf
		CLR_NZC
		SET_NZC
		%endmacro

;
; 右論理シフト(MEMORY)
;
		%macro	LSRM	0
		READB
		shr	al,1
		pushf
		WRITEB
		CLR_NZC
		SET_NZC
		%endmacro

;
; 左論理シフト(BYTE)
;
		%macro	LSLB	1
		shl	%1,1
		pushf
		CLR_NZVC
		SET_NZVC
		%endmacro

;
; 左論理シフト(MEMORY)
;
		%macro	LSLM	0
		READB
		shl	al,1
		pushf
		WRITEB
		CLR_NZVC
		SET_NZVC
		%endmacro

;
; 右算術シフト(BYTE)
;
		%macro	ASRB	1
		sar	%1,1
		pushf
		CLR_NZC
		SET_NZC
		%endmacro

;
; 右算術シフト(MEMORY)
;
		%macro	ASRM	0
		READB
		sar	al,1
		pushf
		WRITEB
		CLR_NZC
		SET_NZC
		%endmacro

;
; 右ローテート(BYTE)
;
		%macro	RORB	1
		mov	al,CCREG
		CLR_NZVC
		shr	al,1
		rcr	%1,1
		adc	CCREG,0
		mov	al,%1
		or	al,al
		pushf
		SET_NZ
		%endmacro

;
; 右ローテート(MEMORY)
;
		%macro	RORM	0
		READB
		mov	ah,CCREG
		CLR_NZVC
		shr	ah,1
		rcr	al,1
		adc	CCREG,0
		or	al,al
		pushf
		WRITEB
		SET_NZ
		%endmacro

;
; 左ローテート(BYTE)
;
		%macro	ROLB	1
		mov	ah,CCREG
		mov	al,%1
		shr	ah,1
		adc	al,al
		pushf
		mov	%1,al
		CLR_NZVC
		SET_NZVC
		%endmacro

;
; 左ローテート(MEMORY)
;
		%macro	ROLM	0
		READB
		mov	ah,CCREG
		shr	ah,1
		adc	al,al
		pushf
		WRITEB
		CLR_NZVC
		SET_NZVC
		%endmacro

;
; 実効アドレス代入(WORD)
;
		%macro	LEAW	1
		CLR_Z
		mov	%1,bp
		or	bp,bp
		pushf
	%ifdef	__MSDOS__
		pop	ax
	%else
		pop	eax
	%endif
		shr	al,4
		and	al,04h
		or	CCREG,al
		%endmacro

;
; ブランチ(BYTE)
;
		%macro	BRANCHB	0
		READB
		cbw
		inc	ax
		add	PCREG,ax
		%endmacro

;
; ブランチ(WORD)
;
		%macro	BRANCHW	0
		READW
		add	ax,2
		add	PCREG,ax
		%endmacro

;
; ジャンプ
;
		%macro	JUMPAB	0
		mov	PCREG,bp
		%endmacro

;
; サブルーチンコール(絶対)
;
		%macro	JUMPSR	0
		push	bp
		PSHSW	PCREG
		pop	PCREG
		%endmacro

;
; サブルーチンコール(相対)
;
		%macro	BRANCHSR	0
		push	ax
		PSHSW	PCREG
		pop	ax
		add	PCREG,ax
		%endmacro

;
; プッシュ(BYTE,S)
;
		%macro	PSHSB	1
		dec	SREG
		mov	bp,SREG
		mov	al,%1
		WRITEB
		%endmacro

;
; プッシュ(BYTE,U)
;
		%macro	PSHUB	1
		dec	UREG
		mov	bp,UREG
		mov	al,%1
		WRITEB
		%endmacro

;
; プッシュ(WORD,S)
;
		%macro	PSHSW	1
		sub	SREG,2
		mov	bp,SREG
		mov	ax,%1
		WRITEW
		%endmacro

;
; プッシュ(WORD,U)
;
		%macro	PSHUW	1
		sub	UREG,2
		mov	bp,UREG
		mov	ax,%1
		WRITEW
		%endmacro

;
; プル(BYTE,S)
;
		%macro	PULSB	1
		mov	bp,SREG
		READB
		mov	%1,al
		inc	SREG
		%endmacro

;
; プル(WORD,S)
;
		%macro	PULSW	1
		mov	bp,SREG
		READW
		mov	%1,ax
		add	SREG,2
		%endmacro

;
; プル(BYTE,U)
;
		%macro	PULUB	1
		mov	bp,UREG
		READB
		mov	%1,al
		inc	UREG
		%endmacro

;
; プル(WORD,U)
;
		%macro	PULUW	1
		mov	bp,UREG
		READW
		mov	%1,ax
		add	UREG,2
		%endmacro

;
; スタック空読み(S)
;
		%macro	DMYRDS	0
		push	ax
		mov	bp,SREG
		READB
		pop	ax
		%endmacro

;
; スタック空読み(U)
;
		%macro	DMYRDU	0
		push	ax
		mov	bp,UREG
		READB
		pop	ax
		%endmacro

;
; レジスタ取得(TFR,EXG)
;
; param.: si	REG
;	  ah	レジスタ指定(0〜15)
; result: bx	データ
; usereg: di
;
		ALIGN
getreg:
	%ifdef	__MSDOS__
		mov	di,getreg_table
		xor	bh,bh
		mov	bl,ah
		add	bl,bl
		jmp	near [cs:bx+di]
	%else
		mov	edi,getreg_table
		xor	ebx,ebx
		mov	bl,ah
		jmp	near [ebx*4+edi]
	%endif

;
; レジスタ取得テーブル
;
		ALIGN
getreg_table:
		ADDR	getreg_d
		ADDR	getreg_x
		ADDR	getreg_y
		ADDR	getreg_u
		ADDR	getreg_s
		ADDR	getreg_pc
		ADDR	getreg_dmy
		ADDR	getreg_dmy
		ADDR	getreg_a
		ADDR	getreg_b
		ADDR	getreg_cc
		ADDR	getreg_dp
		ADDR	getreg_dmy
		ADDR	getreg_dmy
		ADDR	getreg_dmy
		ADDR	getreg_dmy

;
; レジスタ取得ルーチン群
;
		ALIGN
getreg_d:
		mov	bx,DREG
		ret
		ALIGN
getreg_x:
		mov	bx,XREG
		ret
		ALIGN
getreg_y:
		mov	bx,YREG
		ret
		ALIGN
getreg_u:
		mov	bx,UREG
		ret
		ALIGN
getreg_s:
		mov	bx,SREG
		ret
		ALIGN
getreg_pc:
		mov	bx,PCREG
		ret
		ALIGN
getreg_a:
		mov	bh,0ffh
		mov	bl,AREG
		ret
		ALIGN
getreg_b:
		mov	bh,0ffh
		mov	bl,BREG
		ret
		ALIGN
getreg_cc:
		mov	bh,0ffh
		mov	bl,CCREG
		ret
		ALIGN
getreg_dp:
		mov	bh,0ffh
		mov	bl,DPREG
		ret
		ALIGN
getreg_dmy:
		mov	bx,0ffffh
		ret

;
; レジスタセット(TFR,EXG)
;
; param.: si	REG
;	  ah	レジスタ指定(0〜15)
;	  bx	データ
; usereg: cx,di
;
		ALIGN
setreg:
	%ifdef	__MSDOS__
		mov	di,setreg_table
		mov	cl,ah
		xor	ch,ch
		add	cl,cl
		add	di,cx
		jmp	near [cs:di]
	%else
		mov	edi,setreg_table
		xor	ecx,ecx
		mov	cl,ah
		jmp	near [ecx*4+edi]
	%endif

;
; レジスタセットテーブル
;
		ALIGN
setreg_table:
		ADDR	setreg_d
		ADDR	setreg_x
		ADDR	setreg_y
		ADDR	setreg_u
		ADDR	setreg_s
		ADDR	setreg_pc
		ADDR	setreg_dmy
		ADDR	setreg_dmy
		ADDR	setreg_a
		ADDR	setreg_b
		ADDR	setreg_cc
		ADDR	setreg_dp
		ADDR	setreg_dmy
		ADDR	setreg_dmy
		ADDR	setreg_dmy
		ADDR	setreg_dmy

;
; レジスタセットルーチン群
;
		ALIGN
setreg_d:
		mov	DREG,bx
		ret
		ALIGN
setreg_x:
		mov	XREG,bx
		ret
		ALIGN
setreg_y:
		mov	YREG,bx
		ret
		ALIGN
setreg_u:
		mov	UREG,bx
		ret
		ALIGN
setreg_s:
		mov	SREG,bx
; NMI許可
		or	INTR,0010h
		ret
		ALIGN
setreg_pc:
		mov	PCREG,bx
		ret
		ALIGN
setreg_a:
		mov	AREG,bl
		ret
		ALIGN
setreg_b:
		mov	BREG,bl
		ret
		ALIGN
setreg_cc:
		mov	CCREG,bl
		ret
		ALIGN
setreg_dp:
		mov	DPREG,bl
		ret
		ALIGN
setreg_dmy:
		ret

;
; サイクルテーブル１
;
		ALIGN
cycle_table1:
		db	6
		db	6
		db	6
		db	6
		db	6
		db	6
		db	6
		db	6
		db	6
		db	6
		db	6
		db	6
		db	6
		db	6
		db	3
		db	6
; $10
		db	255
		db	255
		db	2
		db	2
		db	0
		db	0
		db	5
		db	9
		db	3
		db	2
		db	3
		db	2
		db	3
		db	2
		db	8
		db	6
; $20
		db	3
		db	3
		db	3
		db	3
		db	3
		db	3
		db	3
		db	3
		db	3
		db	3
		db	3
		db	3
		db	3
		db	3
		db	3
		db	3
; $30
		db	132
		db	132
		db	132
		db	132
		db	5
		db	5
		db	5
		db	5
		db	4
		db	5
		db	3
		db	6
		db	20
		db	11
		db	1
		db	19
; $40
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
; $50
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
; $60
		db	134
		db	134
		db	134
		db	134
		db	134
		db	134
		db	134
		db	134
		db	134
		db	134
		db	134
		db	134
		db	134
		db	134
		db	131
		db	134
; $70
		db	7
		db	7
		db	7
		db	7
		db	7
		db	7
		db	7
		db	7
		db	7
		db	7
		db	7
		db	7
		db	7
		db	7
		db	4
		db	7
; $80
		db	2
		db	2
		db	2
		db	4
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	4
		db	7
		db	3
		db	3
; $90
		db	4
		db	4
		db	4
		db	6
		db	4
		db	4
		db	4
		db	4
		db	4
		db	4
		db	4
		db	4
		db	6
		db	7
		db	5
		db	5
; $A0
		db	132
		db	132
		db	132
		db	134
		db	132
		db	132
		db	132
		db	132
		db	132
		db	132
		db	132
		db	132
		db	134
		db	135
		db	133
		db	133
; $B0
		db	5
		db	5
		db	5
		db	7
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	7
		db	8
		db	6
		db	6
; $C0
		db	2
		db	2
		db	2
		db	4
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	2
		db	3
		db	0
		db	3
		db	3
; $D0
		db	4
		db	4
		db	4
		db	6
		db	4
		db	4
		db	4
		db	4
		db	4
		db	4
		db	4
		db	4
		db	5
		db	5
		db	5
		db	5
; $E0
		db	132
		db	132
		db	132
		db	134
		db	132
		db	132
		db	132
		db	132
		db	132
		db	132
		db	132
		db	132
		db	133
		db	133
		db	133
		db	133
; $F0
		db	5
		db	5
		db	5
		db	7
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	6
		db	6
		db	6
		db	6

;
; サイクルテーブル２
;
		ALIGN
cycle_table2:
; $00
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
; $10
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
; $20
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
		db	5
; $30
		db	128
		db	128
		db	128
		db	128
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	20
; $40
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
; $50
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
; $60
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
; $70
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
; $80
		db	0
		db	0
		db	0
		db	5
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	5
		db	9
		db	4
		db	0
; $90
		db	0
		db	0
		db	0
		db	7
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	7
		db	0
		db	6
		db	6
; $A0
		db	128
		db	128
		db	128
		db	135
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	135
		db	128
		db	134
		db	134
; $B0
		db	0
		db	0
		db	0
		db	8
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	8
		db	0
		db	7
		db	7
; $C0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	4
		db	0
; $D0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	6
		db	6
; $E0
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	128
		db	134
		db	134
; $F0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	0
		db	7
		db	7
		ALIGN

;
; $00
; NEG (DIR)
;
		ALIGN
neg_di:
		DIRECT
		NEGM
		ret

;
; $02
; NGC (DIR)
;
		ALIGN
ngc_di:
		test	CCREG,01h
		jz	neg_di

;
; $03
; COM (DIR)
;
		ALIGN
com_di:
		DIRECT
		COMM
		ret

;
; $04
; LSR (DIR)
;
		ALIGN
lsr_di:
		DIRECT
		LSRM
		ret

;
; $06
; ROR (DIR)
;
		ALIGN
ror_di:
		DIRECT
		RORM
		ret

;
; $07
; ASR (DIR)
;
		ALIGN
asr_di:
		DIRECT
		ASRM
		ret

;
; $08
; LSL (DIR)
;
		ALIGN
lsl_di:
		DIRECT
		LSLM
		ret

;
; $09
; ROL (DIR)
;
		ALIGN
rol_di:
		DIRECT
		ROLM
		ret

;
; $0A
; DEC (DIR)
;
		ALIGN
dec_di:
		DIRECT
		DECM
		ret

;
; $0B
; DCC (DIR)
;
		ALIGN
dcc_di:
		DIRECT
		DCCM
		ret

;
; $0C
; INC (DIR)
;
		ALIGN
inc_di:
		DIRECT
		INCM
		ret

;
; $0D
; TST (DIR)
;
		ALIGN
tst_di:
		DIRECT
		TSTM
		ret

;
; $0E
; JMP (DIR)
;
		ALIGN
jmp_di:
		DIRECT
		JUMPAB
		ret

;
; $0F
; CLR (DIR)
;
		ALIGN
clr_di:
		DIRECT
		CLRM
		ret

;
; $12
; NOP
;
		ALIGN
nop_in:
		ret

;
; $13
; SYNC
;
		ALIGN
sync_in:
		test	INTR,0020h
		jnz	.next
; 今回初めてSYNC実行
		or	INTR,0020h
		and	INTR,0ffbfh
		dec	PCREG
		ret
; SYNC実行中
		ALIGN
.next:
		test	INTR,0040h
		jnz	.exit
		dec	PCREG
		ret
		ALIGN
; 割り込みがかかった
.exit:
		and	INTR,0ff9fh
		ret

;
; $14,$15,$CD
; HALT? ($0000〜$FFFFをバイトリードし続ける)
;
		ALIGN
unknown_im:
		or	INTR,08000h
		ret

;
; $16,$10 $20
; LBRA
;
		ALIGN
lbra_re:
		BRANCHW
		ret

;
; $17,$10 $8D
; LBSR
;
		ALIGN
lbsr_re:
		IMMW
		BRANCHSR
		ret

;
; $18
; ASLCC
;
		ALIGN
aslcc_in:
		mov	al,CCREG
		test	al,04h
		jz	.next
		or	al,01h
.next:
		add	al,al
		and	al,3eh
		mov	CCREG,al
		ret

;
; $19
; DAA
;
		ALIGN
daa_in:
		mov	al,AREG
		mov	ah,al
		and	al,0fh
		and	ah,0f0h
		xor	bl,bl
		mov	bh,CCREG
; step 1
.step1
		cmp	al,0ah
		jnc	.step1ok
		test	bh,20h
		jz	.step2
.step1ok:
		or	bl,06h
; step2
.step2:
		cmp	ah,90h
		jc	.step3
		cmp	al,0ah
		jc	.step3
.step2ok:
		or	bl,60h
; step3
.step3:
		cmp	ah,0a0h
		jnc	.step3ok
		test	bh,01h
		jz	.ok
.step3ok:
		or	bl,60h
; ok
.ok:
		add	bl,AREG
		pushf
		mov	AREG,bl
		CLR_NZV
		SET_NZC
		ret

;
; $1A
; ORCC
;
		ALIGN
orcc_im:
		IMMB
		or	CCREG,al
		ret

;
; $1C
; ANDCC
;
		ALIGN
andcc_im:
		IMMB
		and	CCREG,al
		ret

;
; $1D
; SEX
;
		ALIGN
sex_in:
		mov	al,BREG
		cbw
		or	ax,ax
		pushf
		mov	DREG,ax
		CLR_NZ
		SET_NZ
		ret

;
; $1E
; EXG
;
		ALIGN
exg_in:
		READB
		inc	PCREG
; パラメータ取得
		mov	ah,al
		and	ah,0fh
		shr	al,4
; 16bit<>16bitに仮想化して扱える
		call	getreg
		mov	bp,bx
		xchg	ah,al
		call	getreg
		xchg	ah,al
		call	setreg
		xchg	ah,al
		mov	bx,bp
		call	setreg
		ret

;
; $1F
; TFR
;
		ALIGN
tfr_in:
		READB
		inc	PCREG
; 16bit<>16bitに仮想化して扱える
		mov	ah,al
		shr	ah,4
		and	al,0fh
		call	getreg
		mov	ah,al
		jmp	setreg

;
; $20
; BRA
;
		ALIGN
bra_re:
		BRANCHB
		ret

;
; $21
; BRN
;
		ALIGN
brn_re:
		inc	PCREG
		ret

;
; $10 $21
; LBRN
;
		ALIGN
lbrn_re:
		add	PCREG,2
		ret

;
; $22
; BHI
;
		ALIGN
bhi_re:
		test	CCREG,05h
		jnz	brn_re
		BRANCHB
		ret

;
; $10 $22
; LBHI
;
		ALIGN
lbhi_re:
		test	CCREG,05h
		jnz	lbrn_re
		BRANCHW
		inc	CYCLE
		ret

;
; $23
; BLS
;
		ALIGN
bls_re:
		test	CCREG,05h
		jz	brn_re
		BRANCHB
		ret

;
; $10 $23
; LBLS
;
		ALIGN
lbls_re:
		test	CCREG,05h
		jz	lbrn_re
		BRANCHW
		inc	CYCLE
		ret

;
; $24
; BCC
;
		ALIGN
bcc_re:
		test	CCREG,01h
		jnz	brn_re
		BRANCHB
		ret

;
; $10 $24
; LBCC
;
		ALIGN
lbcc_re:
		test	CCREG,01h
		jnz	lbrn_re
		BRANCHW
		inc	CYCLE
		ret

;
; $25
; BCS
;
		ALIGN
bcs_re:
		test	CCREG,01h
		jz	brn_re
		BRANCHB
		ret

;
; $10 $25
; LBCS
;
		ALIGN
lbcs_re:
		test	CCREG,01h
		jz	lbrn_re
		BRANCHW
		inc	CYCLE
		ret

;
; $26
; BNE
;
		ALIGN
bne_re:
		test	CCREG,04h
		jnz	brn_re
		BRANCHB
		ret

;
; $10 $26
; LBNE
;
		ALIGN
lbne_re:
		test	CCREG,04h
		jnz	lbrn_re
		BRANCHW
		inc	CYCLE
		ret

;
; $27
; BEQ
;
		ALIGN
beq_re:
		test	CCREG,04h
		jz	brn_re
		BRANCHB
		ret

;
; $10 $27
; LBEQ
;
		ALIGN
lbeq_re:
		test	CCREG,04h
		jz	lbrn_re
		BRANCHW
		inc	CYCLE
		ret

;
; $28
; BVC
;
		ALIGN
bvc_re:
		test	CCREG,02h
		jnz	brn_re
		BRANCHB
		ret

;
; $10 $28
; LBVC
;
		ALIGN
lbvc_re:
		test	CCREG,02h
		jnz	lbrn_re
		BRANCHW
		inc	CYCLE
		ret

;
; $29
; BVS
;
		ALIGN
bvs_re:
		test	CCREG,02h
		jz	brn_re
		BRANCHB
		ret

;
; $10 $29
; LBVS
;
		ALIGN
lbvs_re:
		test	CCREG,02h
		jz	lbrn_re
		BRANCHW
		inc	CYCLE
		ret

;
; $2A
; BPL
;
		ALIGN
bpl_re:
		test	CCREG,08h
		jnz	brn_re
		BRANCHB
		ret

;
; $10 $2A
; LBPL
;
		ALIGN
lbpl_re:
		test	CCREG,08h
		jnz	lbrn_re
		BRANCHW
		inc	CYCLE
		ret

;
; $2B
; BMI
;
		ALIGN
bmi_re:
		test	CCREG,08h
		jz	brn_re
		BRANCHB
		ret

;
; $10 $2B
; LBMI
;
		ALIGN
lbmi_re:
		test	CCREG,08h
		jz	lbrn_re
		BRANCHW
		inc	CYCLE
		ret

;
; $2C
; BGE
;
		ALIGN
bge_re:
		mov	al,CCREG
		mov	ah,al
		shl	al,2
		xor	al,ah
		test	al,08h
		jnz	brn_re
		BRANCHB
		ret

;
; $10 $2C
; LBGE
;
		ALIGN
lbge_re:
		mov	al,CCREG
		mov	ah,al
		shl	al,2
		xor	al,ah
		test	al,08h
		jnz	lbrn_re
		BRANCHW
		inc	CYCLE
		ret

;
; $2D
; BLT
;
		ALIGN
blt_re:
		mov	al,CCREG
		mov	ah,al
		shl	al,2
		xor	al,ah
		test	al,08h
		jz	brn_re
		BRANCHB
		ret

;
; $10 $2D
; LBLT
;
		ALIGN
lblt_re:
		mov	al,CCREG
		mov	ah,al
		shl	al,2
		xor	al,ah
		test	al,08h
		jz	lbrn_re
		BRANCHW
		inc	CYCLE
		ret

;
; $2E
; BGT
;
		ALIGN
bgt_re:
		mov	al,CCREG
		mov	ah,al
		shl	al,2
		xor	al,ah
		test	al,08h
		jnz	brn_re
		test	ah,04h
		jnz	brn_re
		BRANCHB
		ret

;
; $10 $2E
; LBGT
;
		ALIGN
lbgt_re:
		mov	al,CCREG
		mov	ah,al
		shl	al,2
		xor	al,ah
		test	al,08h
		jnz	lbrn_re
		test	ah,04h
		jnz	lbrn_re
		BRANCHW
		inc	CYCLE
		ret

;
; $2F
; BLE
;
		ALIGN
ble_re:
		mov	al,CCREG
		mov	ah,al
		shl	al,2
		xor	al,ah
		test	al,08h
		jnz	.jump
		test	ah,04h
		jnz	.jump
		inc	PCREG
		ret
		ALIGN
.jump:
		BRANCHB
		ret

;
; $10 $2F
; LBLE
;
		ALIGN
lble_re:
		mov	al,CCREG
		mov	ah,al
		shl	al,2
		xor	al,ah
		test	al,08h
		jnz	.jump
		test	ah,04h
		jnz	.jump
		add	PCREG,2
		ret
		ALIGN
.jump:
		BRANCHW
		inc	CYCLE
		ret

;
; $31
; LEAX (IDX)
;
		ALIGN
leax_ix:
		LEAW	XREG
		ret

;
; $31
; LEAY (IDX)
;
		ALIGN
leay_ix:
		LEAW	YREG
		ret

;
; $32
; LEAS (IDX)
;
		ALIGN
leas_ix:
		mov	SREG,bp
		ret

;
; $33
; LEAU (IDX)
;
		ALIGN
leau_ix:
		mov	UREG,bp
		ret

;
; $34
; PSHS
;
		ALIGN
pshs_in:
		IMMB
		DMYRDS
; PC
.pcreg:
		add	al,al
		jnc	.ureg
		push	ax
		PSHSW	PCREG
		pop	ax
		add	CYCLE,2
; U
.ureg:
		add	al,al
		jnc	.yreg
		push	ax
		PSHSW	UREG
		pop	ax
		add	CYCLE,2
; Y
.yreg:
		add	al,al
		jnc	.xreg
		push	ax
		PSHSW	YREG
		pop	ax
		add	CYCLE,2
; X
.xreg:
		add	al,al
		jnc	.dpreg
		push	ax
		PSHSW	XREG
		pop	ax
		add	CYCLE,2
; DP
.dpreg:
		add	al,al
		jnc	.breg
		push	ax
		PSHSB	DPREG
		pop	ax
		inc	CYCLE
; B
.breg:
		add	al,al
		jnc	.areg
		push	ax
		PSHSB	BREG
		pop	ax
		inc	CYCLE
; A
.areg:
		add	al,al
		jnc	.ccreg
		push	ax
		PSHSB	AREG
		pop	ax
		inc	CYCLE
; CC
.ccreg:
		add	al,al
		jnc	.exit
		PSHSB	CCREG
		inc	CYCLE
.exit:
		ret

;
; $35
; PULS
;
		ALIGN
puls_in:
		IMMB
; CC
.ccreg:
		shr	al,1
		jnc	.areg
		push	ax
		PULSB	CCREG
		pop	ax
		inc	CYCLE
; A
.areg:
		shr	al,1
		jnc	.breg
		push	ax
		PULSB	AREG
		pop	ax
		inc	CYCLE
; B
.breg:
		shr	al,1
		jnc	.dpreg
		push	ax
		PULSB	BREG
		pop	ax
		inc	CYCLE
; DP
.dpreg:
		shr	al,1
		jnc	.xreg
		push	ax
		PULSB	DPREG
		pop	ax
		inc	CYCLE
; X
.xreg:
		shr	al,1
		jnc	.yreg
		push	ax
		PULSW	XREG
		pop	ax
		add	CYCLE,2
; Y
.yreg:
		shr	al,1
		jnc	.ureg
		push	ax
		PULSW	YREG
		pop	ax
		add	CYCLE,2
; U
.ureg:
		shr	al,1
		jnc	.pcreg
		push	ax
		PULSW	UREG
		pop	ax
		add	CYCLE,2
; PC
.pcreg:
		shr	al,1
		jnc	.exit
		PULSW	PCREG
		add	CYCLE,2
.exit:
		DMYRDS
		ret

;
; $36
; PSHU
;
		ALIGN
pshu_in:
		IMMB
		DMYRDU
; PC
.pcreg:
		add	al,al
		jnc	.sreg
		push	ax
		PSHUW	PCREG
		pop	ax
		add	CYCLE,2
; S
.sreg:
		add	al,al
		jnc	.yreg
		push	ax
		PSHUW	SREG
		pop	ax
		add	CYCLE,2
; Y
.yreg:
		add	al,al
		jnc	.xreg
		push	ax
		PSHUW	YREG
		pop	ax
		add	CYCLE,2
; X
.xreg:
		add	al,al
		jnc	.dpreg
		push	ax
		PSHUW	XREG
		pop	ax
		add	CYCLE,2
; DP
.dpreg:
		add	al,al
		jnc	.breg
		push	ax
		PSHUB	DPREG
		pop	ax
		inc	CYCLE
; B
.breg:
		add	al,al
		jnc	.areg
		push	ax
		PSHUB	BREG
		pop	ax
		inc	CYCLE
; A
.areg:
		add	al,al
		jnc	.ccreg
		push	ax
		PSHUB	AREG
		pop	ax
		inc	CYCLE
; CC
.ccreg:
		add	al,al
		jnc	.exit
		PSHUB	CCREG
		inc	CYCLE
.exit:
		ret

;
; $37
; PULU
;
		ALIGN
pulu_in:
		IMMB
; CC
.ccreg:
		shr	al,1
		jnc	.areg
		push	ax
		PULUB	CCREG
		pop	ax
		inc	CYCLE
; A
.areg:
		shr	al,1
		jnc	.breg
		push	ax
		PULUB	AREG
		pop	ax
		inc	CYCLE
; B
.breg:
		shr	al,1
		jnc	.dpreg
		push	ax
		PULUB	BREG
		pop	ax
		inc	CYCLE
; DP
.dpreg:
		shr	al,1
		jnc	.xreg
		push	ax
		PULUB	DPREG
		pop	ax
		inc	CYCLE
; X
.xreg:
		shr	al,1
		jnc	.yreg
		push	ax
		PULUW	XREG
		pop	ax
		add	CYCLE,2
; Y
.yreg:
		shr	al,1
		jnc	.sreg
		push	ax
		PULUW	YREG
		pop	ax
		add	CYCLE,2
; S
.sreg:
		shr	al,1
		jnc	.pcreg
		push	ax
		PULUW	SREG
		pop	ax
		add	CYCLE,2
; PC
.pcreg:
		shr	al,1
		jnc	.exit
		PULUW	PCREG
		add	CYCLE,2
.exit:
		DMYRDU
		ret

;
; $39
; RTS
;
		ALIGN
rts_in:
		PULSW	PCREG
		ret

;
; $3A
; ABX
;
		ALIGN
abx_in:
	%ifdef __MSDOS__
		xor	ax,ax
	%else
		xor	eax,eax
	%endif
		mov	al,BREG
		add	XREG,ax
		ret

;
; $3B
; RTI
;
		ALIGN
rti_in:
		PULSB	CCREG
; Eフラグ判定
		test	CCREG,80h
		jnz	.nmiirq
; FIRQ
		PULSW	PCREG
		ret
; NMI,IRQ
		ALIGN
.nmiirq:
		PULSB	AREG
		PULSB	BREG
		PULSB	DPREG
		PULSW	XREG
		PULSW	YREG
		PULSW	UREG
		PULSW	PCREG
		add	CYCLE,9
		ret

;
; $3C
; CWAI
;
		ALIGN
cwai_im:
		test	INTR,0080h
		jz	.first
; CWAI実行中
		test	INTR,0100h
		jnz	.exit
		dec	PCREG
		ret
; 割り込みがかかって、RTIの後
		ALIGN
.exit:
		and	INTR,0fe7fh
		inc	PCREG
		ret
		ALIGN
; 今回初めてCWAI実行
.first:
		IMMB
		and	CCREG,al
		or	INTR,0080h
		and	INTR,0feffh
		sub	PCREG,2
		ret

;
; $3D
; MUL
;
		ALIGN
mul_in:
		mov	al,AREG
		mul	BREG
		mov	DREG,ax
		and	CCREG,0fah
		or	ax,ax
		jnz	.cy
		SET_Z
		shr	al,7
		or	CCREG,al
		ret
		ALIGN
.cy:
		shr	al,7
		or	CCREG,al
		ret

;
; $3E
; RST
;
		ALIGN
rst_in:
		call	cpu_reset
		ret

;
; $3F
; SWI
;
		ALIGN
swi_in:
		or	CCREG,80h
		PSHSW	PCREG
		PSHSW	UREG
		PSHSW	YREG
		PSHSW	XREG
		PSHSB	DPREG
		PSHSB	BREG
		PSHSB	AREG
		PSHSB	CCREG
		mov	bp,0fffah
		READW
		mov	PCREG,ax
; SWIのみ
		or	CCREG,50h
		ret

;
; $10 $3F
; SWI2
;
		ALIGN
swi2_in:
		or	CCREG,80h
		PSHSW	PCREG
		PSHSW	UREG
		PSHSW	YREG
		PSHSW	XREG
		PSHSB	DPREG
		PSHSB	BREG
		PSHSB	AREG
		PSHSB	CCREG
		mov	bp,0fff4h
		READW
		mov	PCREG,ax
		ret
;
; $11 $3F
; SWI3
;
		ALIGN
swi3_in:
		or	CCREG,80h
		PSHSW	PCREG
		PSHSW	UREG
		PSHSW	YREG
		PSHSW	XREG
		PSHSB	DPREG
		PSHSB	BREG
		PSHSB	AREG
		PSHSB	CCREG
		mov	bp,0fff2h
		READW
		mov	PCREG,ax
		ret

;
; $40
; NEGA
;
		ALIGN
nega_in:
		NEGB	AREG
		ret

;
; $42
; NGCA
;
		ALIGN
ngca_in:
		test	CCREG,01h
		jz	nega_in
;		jmp	coma_in

;
; $43
; COMA
;
		ALIGN
coma_in:
		COMB	AREG
		ret

;
; $44
; LSRA
;
		ALIGN
lsra_in:
		LSRB	AREG
		ret

;
; $46
; RORA
;
		ALIGN
rora_in:
		RORB	AREG
		ret

;
; $47
; ASRA
;
		ALIGN
asra_in:
		ASRB	AREG
		ret

;
; $48
; LSLA
;
		ALIGN
lsla_in:
		LSLB	AREG
		ret

;
; $49
; ROLA
;
		ALIGN
rola_in:
		ROLB	AREG
		ret

;
; $4A
; DECA
;
		ALIGN
deca_in:
		DECB	AREG
		ret

;
; $4B
; DCCA
;
		ALIGN
dcca_in:
		DCCB	AREG
		ret

;
; $4C
; INCA
;
		ALIGN
inca_in:
		INCB	AREG
		ret

;
; $4D
; TSTA
;
		ALIGN
tsta_in:
		TSTB	AREG
		ret

;
; $4E
; CLCA
;
		ALIGN
clca_in:
		CLCB	AREG
		ret

;
; $4F
; CLRA
;
		ALIGN
clra_in:
		CLRB	AREG
		ret

;
; $50
; NEGB
;
		ALIGN
negb_in:
		NEGB	BREG
		ret

;
; $52
; NGCB
;
		ALIGN
ngcb_in:
		test	CCREG,01h
		jz	negb_in

;
; $53
; COMB
;
		ALIGN
comb_in:
		COMB	BREG
		ret

;
; $54
; LSRB
;
		ALIGN
lsrb_in:
		LSRB	BREG
		ret

;
; $56
; RORB
;
		ALIGN
rorb_in:
		RORB	BREG
		ret

;
; $57
; ASRB
;
		ALIGN
asrb_in:
		ASRB	BREG
		ret

;
; $58
; LSLB
;
		ALIGN
lslb_in:
		LSLB	BREG
		ret

;
; $59
; ROLB
;
		ALIGN
rolb_in:
		ROLB	BREG
		ret

;
; $5A
; DECB
;
		ALIGN
decb_in:
		DECB	BREG
		ret

;
; $5B
; DCCB
;
		ALIGN
dccb_in:
		DCCB	BREG
		ret

;
; $5C
; INCB
;
		ALIGN
incb_in:
		INCB	BREG
		ret

;
; $5D
; TSTB
;
		ALIGN
tstb_in:
		TSTB	BREG
		ret

;
; $5E
; CLCB
;
		ALIGN
clcb_in:
		CLCB	BREG
		ret

;
; $5F
; CLRB
;
		ALIGN
clrb_in:
		CLRB	BREG
		ret

;
; $60
; NEG (IDX)
;
		ALIGN
neg_ix:
		NEGM
		ret

;
; $62
; NGC (IDX)
;
		ALIGN
ngc_ix:
		test	CCREG,01h
		jz	neg_ix
;		jmp	com_ix

;
; $63
; COM (IDX)
;
		ALIGN
com_ix:
		COMM
		ret

;
; $64
; LSR (IDX)
;
		ALIGN
lsr_ix:
		LSRM
		ret

;
; $66
; ROR (IDX)
;
		ALIGN
ror_ix:
		RORM
		ret

;
; $67
; ASR (IDX)
;
		ALIGN
asr_ix:
		ASRM
		ret

;
; $68
; LSL (IDX)
;
		ALIGN
lsl_ix:
		LSLM
		ret

;
; $69
; ROL (IDX)
;
		ALIGN
rol_ix:
		ROLM
		ret

;
; $6A
; DEC (IDX)
;
		ALIGN
dec_ix:
		DECM
		ret

;
; $6B
; DCC (IDX)
;
		ALIGN
dcc_ix:
		DCCM
		ret

;
; $6C
; INC (IDX)
;
		ALIGN
inc_ix:
		INCM
		ret

;
; $6D
; TST (IDX)
;
		ALIGN
tst_ix:
		TSTM
		ret

;
; $6E
; JMP (IDX)
;
		ALIGN
jmp_ix:
		JUMPAB
		ret

;
; $6F
; CLR (IDX)
;
		ALIGN
clr_ix:
		CLRM
		ret

;
; $70
; NEG (EXT)
;
		ALIGN
neg_ex:
		EXTEND
		NEGM
		ret

;
; $72
; NGC (EXT)
;
		ALIGN
ngc_ex:
		test	CCREG,01h
		jz	neg_ex
		jmp	com_ex

;
; $73
; COM (EXT)
;
		ALIGN
com_ex:
		EXTEND
		COMM
		ret

;
; $74
; LSR (EXT)
;
		ALIGN
lsr_ex:
		EXTEND
		LSRM
		ret

;
; $76
; ROR (EXT)
;
		ALIGN
ror_ex:
		EXTEND
		RORM
		ret

;
; $77
; ASR (EXT)
;
		ALIGN
asr_ex:
		EXTEND
		ASRM
		ret

;
; $78
; LSL (EXT)
;
		ALIGN
lsl_ex:
		EXTEND
		LSLM
		ret

;
; $79
; ROL (EXT)
;
		ALIGN
rol_ex:
		EXTEND
		ROLM
		ret

;
; $7A
; DEC (EXT)
;
		ALIGN
dec_ex:
		EXTEND
		DECM
		ret

;
; $7B
; DCC (EXT)
;
		ALIGN
dcc_ex:
		EXTEND
		DCCM
		ret

;
; $7C
; INC (EXT)
;
		ALIGN
inc_ex:
		EXTEND
		INCM
		ret

;
; $7D
; TST (EXT)
;
		ALIGN
tst_ex:
		EXTEND
		TSTM
		ret

;
; $7E
; JMP (EXT)
;
		ALIGN
jmp_ex:
		EXTEND
		JUMPAB
		ret

;
; $7F
; CLR (EXT)
;
		ALIGN
clr_ex:
		EXTEND
		CLRM
		ret

;
; $80
; SUBA (IMM)
;
		ALIGN
suba_im:
		IMMB
		SUBB	AREG
		ret

;
; $81
; CMPA (IMM)
;
		ALIGN
cmpa_im:
		IMMB
		CMPB	AREG
		ret

;
; $82
; SBCA (IMM)
;
		ALIGN
sbca_im:
		IMMB
		SBCB	AREG
		ret

;
; $83
; SUBD (IMM)
;
		ALIGN
subd_im:
		IMMW
		SUBW	DREG
		ret

;
; $10 $83
; CMPD (IMM)
;
		ALIGN
cmpd_im:
		IMMW
		CMPW	DREG
		ret

;
; $11 $83
; CMPU (IMM)
;
		ALIGN
cmpu_im:
		IMMW
		CMPW	UREG
		ret

;
; $84
; ANDA (IMM)
;
		ALIGN
anda_im:
		IMMB
		ANDB	AREG
		ret

;
; $85
; BITA (IMM)
;
		ALIGN
bita_im:
		IMMB
		BITB	AREG
		ret

;
; $86
; LDA (IMM)
;
		ALIGN
lda_im:
		IMMB
		LOADB	AREG
		ret

;
; $87
; FLAG8 (IMM)
;
		ALIGN
flag8_im:
		IMMB
		CLR_NZV
		or	CCREG,08h
		ret

;
; $88
; EORA (IMM)
;
		ALIGN
eora_im:
		IMMB
		EORB	AREG
		ret

;
; $89
; ADCA (IMM)
;
		ALIGN
adca_im:
		IMMB
		ADCB	AREG
		ret

;
; $8A
; ORA (IMM)
;
		ALIGN
ora_im:
		IMMB
		ORB	AREG
		ret

;
; $8B
; ADDA (IMM)
;
		ALIGN
adda_im:
		IMMB
		ADDB	AREG
		ret

;
; $8C
; CMPX (IMM)
;
		ALIGN
cmpx_im:
		IMMW
		CMPW	XREG
		ret

;
; $11 $8C
; CMPS (IMM)
;
		ALIGN
cmps_im:
		IMMW
		CMPW	SREG
		ret

;
; $10 $8C
; CMPY (IMM)
;
		ALIGN
cmpy_im:
		IMMW
		CMPW	YREG
		ret

;
; $8D
; BSR
;
		ALIGN
bsr_re:
		IMMB
		cbw
		BRANCHSR
		ret

;
; $8E
; LDX (IMM)
;
		ALIGN
ldx_im:
		IMMW
		LOADW	XREG
		ret

;
; $10 $8E
; LDY (IMM)
;
		ALIGN
ldy_im:
		IMMW
		LOADW	YREG
		ret

; $8F
; FLAG16 (IMM)
;
		ALIGN
flag16_im:
		IMMW
		CLR_NZV
		or	CCREG,08h
		ret

;
; $90
; SUBA (DIR)
;
		ALIGN
suba_di:
		DIRECT
		READB
		SUBB	AREG
		ret

;
; $91
; CMPA (DIR)
;
		ALIGN
cmpa_di:
		DIRECT
		READB
		CMPB	AREG
		ret

;
; $92
; SBCA (DIR)
;
		ALIGN
sbca_di:
		DIRECT
		READB
		SBCB	AREG
		ret

;
; $93
; SUBD (DIR)
;
		ALIGN
subd_di:
		DIRECT
		READW
		SUBW	DREG
		ret

;
; $10 $93
; CMPD (DIR)
;
		ALIGN
cmpd_di:
		DIRECT
		READW
		CMPW	DREG
		ret

;
; $11 $93
; CMPU (DIR)
;
		ALIGN
cmpu_di:
		DIRECT
		READW
		CMPW	UREG
		ret

;
; $94
; ANDA (DIR)
;
		ALIGN
anda_di:
		DIRECT
		READB
		ANDB	AREG
		ret

;
; $95
; BITA (DIR)
;
		ALIGN
bita_di:
		DIRECT
		READB
		BITB	AREG
		ret

;
; $96
; LDA (DIR)
;
		ALIGN
lda_di:
		DIRECT
		READB
		LOADB	AREG
		ret

;
; $97
; STA (DIR)
;
		ALIGN
sta_di:
		DIRECT
		STOREB	AREG
		ret

;
; $98
; EORA (DIR)
;
		ALIGN
eora_di:
		DIRECT
		READB
		EORB	AREG
		ret

;
; $99
; ADCA (DIR)
;
		ALIGN
adca_di:
		DIRECT
		READB
		ADCB	AREG
		ret

;
; $9A
; ORA (DIR)
;
		ALIGN
ora_di:
		DIRECT
		READB
		ORB	AREG
		ret

;
; $9B
; ADDA (DIR)
;
		ALIGN
adda_di:
		DIRECT
		READB
		ADDB	AREG
		ret

;
; $9C
; CMPX (DIR)
;
		ALIGN
cmpx_di:
		DIRECT
		READW
		CMPW	XREG
		ret

;
; $11 $9C
; CMPS (DIR)
;
		ALIGN
cmps_di:
		DIRECT
		READW
		CMPW	SREG
		ret

;
; $10 $9C
; CMPY (DIR)
;
		ALIGN
cmpy_di:
		DIRECT
		READW
		CMPW	YREG
		ret

;
; $9D
; JSR (DIR)
;
		ALIGN
jsr_di:
		DIRECT
		JUMPSR
		ret

;
; $9E
; LDX (DIR)
;
		ALIGN
ldx_di:
		DIRECT
		READW
		LOADW	XREG
		ret

;
; $10 $9E
; LDY (DIR)
;
		ALIGN
ldy_di:
		DIRECT
		READW
		LOADW	YREG
		ret

;
; $9F
; STX (DIR)
;
		ALIGN
stx_di:
		DIRECT
		STOREW	XREG
		ret

;
; $10 $9F
; STY (DIR)
;
		ALIGN
sty_di:
		DIRECT
		STOREW	YREG
		ret

;
; $A0
; SUBA (IDX)
;
		ALIGN
suba_ix:
		READB
		SUBB	AREG
		ret

;
; $A1
; CMPA (IDX)
;
		ALIGN
cmpa_ix:
		READB
		CMPB	AREG
		ret

;
; $A2
; SBCA (IDX)
;
		ALIGN
sbca_ix:
		READB
		SBCB	AREG
		ret

;
; $A3
; SUBD (IDX)
;
		ALIGN
subd_ix:
		READW
		SUBW	DREG
		ret

;
; $10 $A3
; CMPD (IDX)
;
		ALIGN
cmpd_ix:
		READW
		CMPW	DREG
		ret

;
; $11 $A3
; CMPU (IDX)
;
		ALIGN
cmpu_ix:
		READW
		CMPW	UREG
		ret

;
; $A4
; ANDA (IDX)
;
		ALIGN
anda_ix:
		READB
		ANDB	AREG
		ret

;
; $A5
; BITA (IDX)
;
		ALIGN
bita_ix:
		READB
		BITB	AREG
		ret

;
; $A6
; LDA (IDX)
;
		ALIGN
lda_ix:
		READB
		LOADB	AREG
		ret

;
; $A7
; STA (IDX)
;
		ALIGN
sta_ix:
		STOREB	AREG
		ret

;
; $A8
; EORA (IDX)
;
		ALIGN
eora_ix:
		READB
		EORB	AREG
		ret

;
; $A9
; ADCA (IDX)
;
		ALIGN
adca_ix:
		READB
		ADCB	AREG
		ret

;
; $AA
; ORA (IDX)
;
		ALIGN
ora_ix:
		READB
		ORB	AREG
		ret

;
; $AB
; ADDA (IDX)
;
		ALIGN
adda_ix:
		READB
		ADDB	AREG
		ret

;
; $AC
; CMPX (IDX)
;
		ALIGN
cmpx_ix:
		READW
		CMPW	XREG
		ret

;
; $11 $AC
; CMPS (IDX)
;
		ALIGN
cmps_ix:
		READW
		CMPW	SREG
		ret

;
; $10 $AC
; CMPY (IDX)
;
		ALIGN
cmpy_ix:
		READW
		CMPW	YREG
		ret

;
; $AD
; JSR (IDX)
;
		ALIGN
jsr_ix:
		JUMPSR
		ret

;
; $AE
; LDX (IDX)
;
		ALIGN
ldx_ix:
		READW
		LOADW	XREG
		ret

;
; $10 AE
; LDY (IDX)
;
		ALIGN
ldy_ix:
		READW
		LOADW	YREG
		ret

;
; $AF
; STX (IDX)
;
		ALIGN
stx_ix:
		STOREW	XREG
		ret

;
; $10 $AF
; STY (IDX)
;
		ALIGN
sty_ix:
		STOREW	YREG
		ret

;
; $B0
; SUBA (EXT)
;
		ALIGN
suba_ex:
		EXTEND
		READB
		SUBB	AREG
		ret

;
; $B1
; CMPA (EXT)
;
		ALIGN
cmpa_ex:
		EXTEND
		READB
		CMPB	AREG
		ret

;
; $B2
; SBCA (EXT)
;
		ALIGN
sbca_ex:
		EXTEND
		READB
		SBCB	AREG
		ret

;
; $B3
; SUBD (EXT)
;
		ALIGN
subd_ex:
		EXTEND
		READW
		SUBW	DREG
		ret

;
; $10 $B3
; CMPD (EXT)
;
		ALIGN
cmpd_ex:
		EXTEND
		READW
		CMPW	DREG
		ret

;
; $11 $B3
; CMPU (EXT)
;
		ALIGN
cmpu_ex:
		EXTEND
		READW
		CMPW	UREG
		ret

;
; $B4
; ANDA (EXT)
;
		ALIGN
anda_ex:
		EXTEND
		READB
		ANDB	AREG
		ret

;
; $B5
; BITA (EXT)
;
		ALIGN
bita_ex:
		EXTEND
		READB
		BITB	AREG
		ret

;
; $B6
; LDA (EXT)
;
		ALIGN
lda_ex:
		EXTEND
		READB
		LOADB	AREG
		ret

;
; $B7
; STA (EXT)
;
		ALIGN
sta_ex:
		EXTEND
		STOREB	AREG
		ret

;
; $B8
; EORA (EXT)
;
		ALIGN
eora_ex:
		EXTEND
		READB
		EORB	AREG
		ret

;
; $B9
; ADCA (EXT)
;
		ALIGN
adca_ex:
		EXTEND
		READB
		ADCB	AREG
		ret

;
; $BA
; ORA (EXT)
;
		ALIGN
ora_ex:
		EXTEND
		READB
		ORB	AREG
		ret

;
; $BB
; ADDA (EXT)
;
		ALIGN
adda_ex:
		EXTEND
		READB
		ADDB	AREG
		ret

;
; $BC
; CMPX (EXT)
;
		ALIGN
cmpx_ex:
		EXTEND
		READW
		CMPW	XREG
		ret

;
; $11 $BC
; CMPS (EXT)
;
		ALIGN
cmps_ex:
		EXTEND
		READW
		CMPW	SREG
		ret

;
; $10 $BC
; CMPY (EXT)
;
		ALIGN
cmpy_ex:
		EXTEND
		READW
		CMPW	YREG
		ret

;
; $BD
; JSR (EXT)
;
		ALIGN
jsr_ex:
		EXTEND
		JUMPSR
		ret

;
; $BE
; LDX (EXT)
;
		ALIGN
ldx_ex:
		EXTEND
		READW
		LOADW	XREG
		ret

;
; $10 BE
; LDY (EXT)
;
		ALIGN
ldy_ex:
		EXTEND
		READW
		LOADW	YREG
		ret

;
; $BF
; STX (EXT)
;
		ALIGN
stx_ex:
		EXTEND
		STOREW	XREG
		ret

;
; $10 $BF
; STY (EXT)
;
		ALIGN
sty_ex:
		EXTEND
		STOREW	YREG
		ret

;
; $C0
; SUBB (IMM)
;
		ALIGN
subb_im:
		IMMB
		SUBB	BREG
		ret

;
; $C1
; CMPB (IMM)
;
		ALIGN
cmpb_im:
		IMMB
		CMPB	BREG
		ret

;
; $C2
; SBCB (IMM)
;
		ALIGN
sbcb_im:
		IMMB
		SBCB	BREG
		ret

;
; $C3
; ADDD (IMM)
;
		ALIGN
addd_im:
		IMMW
		ADDW	DREG
		ret

;
; $C4
; ANDB (IMM)
;
		ALIGN
andb_im:
		IMMB
		ANDB	BREG
		ret

;
; $C5
; BITB (IMM)
;
		ALIGN
bitb_im:
		IMMB
		BITB	BREG
		ret

;
; $C6
; LDB (IMM)
;
		ALIGN
ldb_im:
		IMMB
		LOADB	BREG
		ret

;
; $C8
; EORB (IMM)
;
		ALIGN
eorb_im:
		IMMB
		EORB	BREG
		ret

;
; $C9
; ADCB (IMM)
;
		ALIGN
adcb_im:
		IMMB
		ADCB	BREG
		ret

;
; $CA
; ORB (IMM)
;
		ALIGN
orb_im:
		IMMB
		ORB	BREG
		ret

;
; $CB
; ADDB (IMM)
;
		ALIGN
addb_im:
		IMMB
		ADDB	BREG
		ret

;
; $CC
; LDD (IMM)
;
		ALIGN
ldd_im:
		IMMW
		LOADW	DREG
		ret

;
; $CE
; LDU (IMM)
;
		ALIGN
ldu_im:
		IMMW
		LOADW	UREG
		ret

;
; $10 $CE
; LDS (IMM)
;
		ALIGN
lds_im:
		IMMW
		LOADW	SREG
; NMI許可
		or	INTR,0010h
		ret

;
; $D0
; SUBB (DIR)
;
		ALIGN
subb_di:
		DIRECT
		READB
		SUBB	BREG
		ret

;
; $D1
; CMPB (DIR)
;
		ALIGN
cmpb_di:
		DIRECT
		READB
		CMPB	BREG
		ret

;
; $D2
; SBCB (DIR)
;
		ALIGN
sbcb_di:
		DIRECT
		READB
		SBCB	BREG
		ret

;
; $D3
; ADDD (DIR)
;
		ALIGN
addd_di:
		DIRECT
		READW
		ADDW	DREG
		ret

;
; $D4
; ANDB (DIR)
;
		ALIGN
andb_di:
		DIRECT
		READB
		ANDB	BREG
		ret

;
; $D5
; BITB (DIR)
;
		ALIGN
bitb_di:
		DIRECT
		READB
		BITB	BREG
		ret

;
; $D6
; LDB (DIR)
;
		ALIGN
ldb_di:
		DIRECT
		READB
		LOADB	BREG
		ret

;
; $D7
; STB (DIR)
;
		ALIGN
stb_di:
		DIRECT
		STOREB	BREG
		ret

;
; $D8
; EORB (DIR)
;
		ALIGN
eorb_di:
		DIRECT
		READB
		EORB	BREG
		ret

;
; $D9
; ADCB (DIR)
;
		ALIGN
adcb_di:
		DIRECT
		READB
		ADCB	BREG
		ret

;
; $DA
; ORB (DIR)
;
		ALIGN
orb_di:
		DIRECT
		READB
		ORB	BREG
		ret

;
; $DB
; ADDB (DIR)
;
		ALIGN
addb_di:
		DIRECT
		READB
		ADDB	BREG
		ret

;
; $DC
; LDD (DIR)
;
		ALIGN
ldd_di:
		DIRECT
		READW
		LOADW	DREG
		ret

;
; $DD
; STD (DIR)
;
		ALIGN
std_di:
		DIRECT
		STOREW	DREG
		ret

;
; $DE
; LDU (DIR)
;
		ALIGN
ldu_di:
		DIRECT
		READW
		LOADW	UREG
		ret

;
; $10 $DE
; LDS (DIR)
;
		ALIGN
lds_di:
		DIRECT
		READW
		LOADW	SREG
; NMI許可
		or	INTR,0010h
		ret

;
; $DF
; STU (DIR)
;
		ALIGN
stu_di:
		DIRECT
		STOREW	UREG
		ret

;
; $10 $DF
; STS (DIR)
;
		ALIGN
sts_di:
		DIRECT
		STOREW	SREG
		ret

;
; $E0
; SUBB (IDX)
;
		ALIGN
subb_ix:
		READB
		SUBB	BREG
		ret

;
; $E1
; CMPB (IDX)
;
		ALIGN
cmpb_ix:
		READB
		CMPB	BREG
		ret

;
; $E2
; SBCB (IDX)
;
		ALIGN
sbcb_ix:
		READB
		SBCB	BREG
		ret

;
; $E3
; ADDD (IDX)
;
		ALIGN
addd_ix:
		READW
		ADDW	DREG
		ret

;
; $E4
; ANDB (IDX)
;
		ALIGN
andb_ix:
		READB
		ANDB	BREG
		ret

;
; $E5
; BITB (IDX)
;
		ALIGN
bitb_ix:
		READB
		BITB	BREG
		ret

;
; $E6
; LDB (IDX)
;
		ALIGN
ldb_ix:
		READB
		LOADB	BREG
		ret

;
; $E7
; STB (IDX)
;
		ALIGN
stb_ix:
		STOREB	BREG
		ret

;
; $E8
; EORB (IDX)
;
		ALIGN
eorb_ix:
		READB
		EORB	BREG
		ret

;
; $E9
; ADCB (IDX)
;
		ALIGN
adcb_ix:
		READB
		ADCB	BREG
		ret

;
; $EA
; ORB (IDX)
;
		ALIGN
orb_ix:
		READB
		ORB	BREG
		ret

;
; $EB
; ADDB (IDX)
;
		ALIGN
addb_ix:
		READB
		ADDB	BREG
		ret

;
; $EC
; LDD (IDX)
;
		ALIGN
ldd_ix:
		READW
		LOADW	DREG
		ret

;
; $ED
; STD (IDX)
;
		ALIGN
std_ix:
		STOREW	DREG
		ret

;
; $EE
; LDU (IDX)
;
		ALIGN
ldu_ix:
		READW
		LOADW	UREG
		ret

;
; $10 $EE
; LDS (IDX)
;
		ALIGN
lds_ix:
		READW
		LOADW	SREG
; NMI許可
		or	INTR,0010h
		ret

;
; $EF
; STU (IDX)
;
		ALIGN
stu_ix:
		STOREW	UREG
		ret

;
; $10 $EF
; STS (IDX)
;
		ALIGN
sts_ix:
		STOREW	SREG
		ret

;
; $F0
; SUBB (EXT)
;
		ALIGN
subb_ex:
		EXTEND
		READB
		SUBB	BREG
		ret

;
; $F1
; CMPB (EXT)
;
		ALIGN
cmpb_ex:
		EXTEND
		READB
		CMPB	BREG
		ret

;
; $F2
; SBCB (EXT)
;
		ALIGN
sbcb_ex:
		EXTEND
		READB
		SBCB	BREG
		ret

;
; $F3
; ADDD (EXT)
;
		ALIGN
addd_ex:
		EXTEND
		READW
		ADDW	DREG
		ret

;
; $F4
; ANDB (EXT)
;
		ALIGN
andb_ex:
		EXTEND
		READB
		ANDB	BREG
		ret

;
; $F5
; BITB (EXT)
;
		ALIGN
bitb_ex:
		EXTEND
		READB
		BITB	BREG
		ret

;
; $F6
; LDB (EXT)
;
		ALIGN
ldb_ex:
		EXTEND
		READB
		LOADB	BREG
		ret

;
; $F7
; STB (EXT)
;
		ALIGN
stb_ex:
		EXTEND
		STOREB	BREG
		ret

;
; $F8
; EORB (EXT)
;
		ALIGN
eorb_ex:
		EXTEND
		READB
		EORB	BREG
		ret

;
; $F9
; ADCB (EXT)
;
		ALIGN
adcb_ex:
		EXTEND
		READB
		ADCB	BREG
		ret

;
; $FA
; ORB (EXT)
;
		ALIGN
orb_ex:
		EXTEND
		READB
		ORB	BREG
		ret

;
; $FB
; ADDB (EXT)
;
		ALIGN
addb_ex:
		EXTEND
		READB
		ADDB	BREG
		ret

;
; $FC
; LDD (EXT)
;
		ALIGN
ldd_ex:
		EXTEND
		READW
		LOADW	DREG
		ret

;
; $FD
; STD (EXT)
;
		ALIGN
std_ex:
		EXTEND
		STOREW	DREG
		ret

;
; $FE
; LDU (EXT)
;
		ALIGN
ldu_ex:
		EXTEND
		READW
		LOADW	UREG
		ret

;
; $10 FE
; LDS (EXT)
;
		ALIGN
lds_ex:
		EXTEND
		READW
		LOADW	SREG
; NMI許可
		or	INTR,0010h
		ret

;
; $FF
; STU (EXT)
;
		ALIGN
stu_ex:
		EXTEND
		STOREW	UREG
		ret

;
; $10 $FF
; STS (EXT)
;
		ALIGN
sts_ex:
		EXTEND
		STOREW	SREG
		ret

;
; ジャンプテーブル１
;
		ALIGN
jump_table1:
; $00
		ADDR	neg_di
		ADDR	neg_di
		ADDR	ngc_di
		ADDR	com_di
		ADDR	lsr_di
		ADDR	lsr_di
		ADDR	ror_di
		ADDR	asr_di
		ADDR	lsl_di
		ADDR	rol_di
		ADDR	dec_di
		ADDR	dcc_di
		ADDR	inc_di
		ADDR	tst_di
		ADDR	jmp_di
		ADDR	clr_di
; $10
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	nop_in
		ADDR	sync_in
		ADDR	unknown_im
		ADDR	unknown_im
		ADDR	lbra_re
		ADDR	lbsr_re
		ADDR	aslcc_in
		ADDR	daa_in
		ADDR	orcc_im
		ADDR	nop_in
		ADDR	andcc_im
		ADDR	sex_in
		ADDR	exg_in
		ADDR	tfr_in
; $20
		ADDR	bra_re
		ADDR	brn_re
		ADDR	bhi_re
		ADDR	bls_re
		ADDR	bcc_re
		ADDR	bcs_re
		ADDR	bne_re
		ADDR	beq_re
		ADDR	bvc_re
		ADDR	bvs_re
		ADDR	bpl_re
		ADDR	bmi_re
		ADDR	bge_re
		ADDR	blt_re
		ADDR	bgt_re
		ADDR	ble_re
; $30
		ADDR	leax_ix
		ADDR	leay_ix
		ADDR	leas_ix
		ADDR	leau_ix
		ADDR	pshs_in
		ADDR	puls_in
		ADDR	pshu_in
		ADDR	pulu_in
		ADDR	andcc_im
		ADDR	rts_in
		ADDR	abx_in
		ADDR	rti_in
		ADDR	cwai_im
		ADDR	mul_in
		ADDR	rst_in
		ADDR	swi_in
; $40
		ADDR	nega_in
		ADDR	nega_in
		ADDR	ngca_in
		ADDR	coma_in
		ADDR	lsra_in
		ADDR	lsra_in
		ADDR	rora_in
		ADDR	asra_in
		ADDR	lsla_in
		ADDR	rola_in
		ADDR	deca_in
		ADDR	dcca_in
		ADDR	inca_in
		ADDR	tsta_in
		ADDR	clca_in
		ADDR	clra_in
; $50
		ADDR	negb_in
		ADDR	negb_in
		ADDR	ngcb_in
		ADDR	comb_in
		ADDR	lsrb_in
		ADDR	lsrb_in
		ADDR	rorb_in
		ADDR	asrb_in
		ADDR	lslb_in
		ADDR	rolb_in
		ADDR	decb_in
		ADDR	dccb_in
		ADDR	incb_in
		ADDR	tstb_in
		ADDR	clcb_in
		ADDR	clrb_in
; $60
		ADDR	neg_ix
		ADDR	neg_ix
		ADDR	ngc_ix
		ADDR	com_ix
		ADDR	lsr_ix
		ADDR	lsr_ix
		ADDR	ror_ix
		ADDR	asr_ix
		ADDR	lsl_ix
		ADDR	rol_ix
		ADDR	dec_ix
		ADDR	dcc_ix
		ADDR	inc_ix
		ADDR	tst_ix
		ADDR	jmp_ix
		ADDR	clr_ix
; $70
		ADDR	neg_ex
		ADDR	neg_ex
		ADDR	ngc_ex
		ADDR	com_ex
		ADDR	lsr_ex
		ADDR	lsr_ex
		ADDR	ror_ex
		ADDR	asr_ex
		ADDR	lsl_ex
		ADDR	rol_ex
		ADDR	dec_ex
		ADDR	dcc_ex
		ADDR	inc_ex
		ADDR	tst_ex
		ADDR	jmp_ex
		ADDR	clr_ex
; $80
		ADDR	suba_im
		ADDR	cmpa_im
		ADDR	sbca_im
		ADDR	subd_im
		ADDR	anda_im
		ADDR	bita_im
		ADDR	lda_im
		ADDR	flag8_im
		ADDR	eora_im
		ADDR	adca_im
		ADDR	ora_im
		ADDR	adda_im
		ADDR	cmpx_im
		ADDR	bsr_re
		ADDR	ldx_im
		ADDR	flag16_im
; $90
		ADDR	suba_di
		ADDR	cmpa_di
		ADDR	sbca_di
		ADDR	subd_di
		ADDR	anda_di
		ADDR	bita_di
		ADDR	lda_di
		ADDR	sta_di
		ADDR	eora_di
		ADDR	adca_di
		ADDR	ora_di
		ADDR	adda_di
		ADDR	cmpx_di
		ADDR	jsr_di
		ADDR	ldx_di
		ADDR	stx_di
; $A0
		ADDR	suba_ix
		ADDR	cmpa_ix
		ADDR	sbca_ix
		ADDR	subd_ix
		ADDR	anda_ix
		ADDR	bita_ix
		ADDR	lda_ix
		ADDR	sta_ix
		ADDR	eora_ix
		ADDR	adca_ix
		ADDR	ora_ix
		ADDR	adda_ix
		ADDR	cmpx_ix
		ADDR	jsr_ix
		ADDR	ldx_ix
		ADDR	stx_ix
; $B0
		ADDR	suba_ex
		ADDR	cmpa_ex
		ADDR	sbca_ex
		ADDR	subd_ex
		ADDR	anda_ex
		ADDR	bita_ex
		ADDR	lda_ex
		ADDR	sta_ex
		ADDR	eora_ex
		ADDR	adca_ex
		ADDR	ora_ex
		ADDR	adda_ex
		ADDR	cmpx_ex
		ADDR	jsr_ex
		ADDR	ldx_ex
		ADDR	stx_ex
; $C0
		ADDR	subb_im
		ADDR	cmpb_im
		ADDR	sbcb_im
		ADDR	addd_im
		ADDR	andb_im
		ADDR	bitb_im
		ADDR	ldb_im
		ADDR	flag8_im
		ADDR	eorb_im
		ADDR	adcb_im
		ADDR	orb_im
		ADDR	addb_im
		ADDR	ldd_im
		ADDR	unknown_im
		ADDR	ldu_im
		ADDR	flag16_im
; $D0
		ADDR	subb_di
		ADDR	cmpb_di
		ADDR	sbcb_di
		ADDR	addd_di
		ADDR	andb_di
		ADDR	bitb_di
		ADDR	ldb_di
		ADDR	stb_di
		ADDR	eorb_di
		ADDR	adcb_di
		ADDR	orb_di
		ADDR	addb_di
		ADDR	ldd_di
		ADDR	std_di
		ADDR	ldu_di
		ADDR	stu_di
; $E0
		ADDR	subb_ix
		ADDR	cmpb_ix
		ADDR	sbcb_ix
		ADDR	addd_ix
		ADDR	andb_ix
		ADDR	bitb_ix
		ADDR	ldb_ix
		ADDR	stb_ix
		ADDR	eorb_ix
		ADDR	adcb_ix
		ADDR	orb_ix
		ADDR	addb_ix
		ADDR	ldd_ix
		ADDR	std_ix
		ADDR	ldu_ix
		ADDR	stu_ix
; $F0
		ADDR	subb_ex
		ADDR	cmpb_ex
		ADDR	sbcb_ex
		ADDR	addd_ex
		ADDR	andb_ex
		ADDR	bitb_ex
		ADDR	ldb_ex
		ADDR	stb_ex
		ADDR	eorb_ex
		ADDR	adcb_ex
		ADDR	orb_ex
		ADDR	addb_ex
		ADDR	ldd_ex
		ADDR	std_ex
		ADDR	ldu_ex
		ADDR	stu_ex

;
; ジャンプテーブル２
;
		ALIGN
jump_table2:
; $00
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
; $10
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
; $20
		ADDR	lbra_re
		ADDR	lbrn_re
		ADDR	lbhi_re
		ADDR	lbls_re
		ADDR	lbcc_re
		ADDR	lbcs_re
		ADDR	lbne_re
		ADDR	lbeq_re
		ADDR	lbvc_re
		ADDR	lbvs_re
		ADDR	lbpl_re
		ADDR	lbmi_re
		ADDR	lbge_re
		ADDR	lblt_re
		ADDR	lbgt_re
		ADDR	lble_re
; $30
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	swi2_in
; $40
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
; $50
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
; $60
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
; $70
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
; $80
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cmpd_im
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cmpy_im
		ADDR	lbsr_re
		ADDR	ldy_im
		ADDR	cpu_error
; $90
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cmpd_di
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cmpy_di
		ADDR	cpu_error
		ADDR	ldy_di
		ADDR	sty_di
; $A0
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cmpd_ix
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cmpy_ix
		ADDR	cpu_error
		ADDR	ldy_ix
		ADDR	sty_ix
; $B0
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cmpd_ex
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cmpy_ex
		ADDR	cpu_error
		ADDR	ldy_ex
		ADDR	sty_ex
; $C0
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	lds_im
		ADDR	cpu_error
; $D0
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	lds_di
		ADDR	sts_di
; $E0
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	lds_ix
		ADDR	sts_ix
; $F0
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	cpu_error
		ADDR	lds_ex
		ADDR	sts_ex

;
; ジャンプテーブル３
;
		ALIGN
jump_table3:
		JMPT3	03fh, swi3_in
		JMPT3	083h, cmpu_im
		JMPT3	08ch, cmps_im
		JMPT3	093h, cmpu_di
		JMPT3	09ch, cmps_di
		JMPT3	0a3h, cmpu_ix
		JMPT3	0ach, cmps_ix
		JMPT3	0b3h, cmpu_ex
		JMPT3	0bch, cmps_ex
		JMPT3	0, 0


;
; インデックスモードマクロ
;
		%macro	INORM	1
		mov	bp,%1
		%endmacro
		%macro	INDNORM	1
		mov	bp,%1
		READW
		mov	bp,ax
		%endmacro
		%macro	IINC1	1
		mov	bp,%1
		inc	%1
		add	CYCLE,2
		%endmacro
		%macro	IINC2	1
		mov	bp,%1
		add	%1,2
		add	CYCLE,3
		%endmacro
		%macro	INDINC1	1
		mov	bp,%1
		READW
		mov	bp,ax
		inc	%1
		add	CYCLE,5
		%endmacro
		%macro	INDINC2	1
		mov	bp,%1
		READW
		mov	bp,ax
		add	%1,2
		add	CYCLE,6
		%endmacro
		%macro	IDEC1	1
		dec	%1
		mov	bp,%1
		add	CYCLE,2
		%endmacro
		%macro	IDEC2	1
		sub	%1,2
		mov	bp,%1
		add	CYCLE,3
		%endmacro
		%macro	INDDEC1	1
		dec	%1
		mov	bp,%1
		READW
		mov	bp,ax
		add	CYCLE,5
		%endmacro
		%macro	INDDEC2	1
		sub	%1,2
		mov	bp,%1
		READW
		mov	bp,ax
		add	CYCLE,6
		%endmacro
		%macro	IAREG	1
		mov	al,AREG
		cbw
		mov	bp,%1
		add	bp,ax
		inc	CYCLE
		%endmacro
		%macro	INDAREG	1
		mov	al,AREG
		cbw
		mov	bp,%1
		add	bp,ax
		READW
		mov	bp,ax
		add	CYCLE,4
		%endmacro
		%macro	IBREG	1
		mov	al,BREG
		cbw
		mov	bp,%1
		add	bp,ax
		inc	CYCLE
		%endmacro
		%macro	INDBREG	1
		mov	al,BREG
		cbw
		mov	bp,%1
		add	bp,ax
		READW
		mov	bp,ax
		add	CYCLE,4
		%endmacro
		%macro	IDREG	1
		mov	bp,%1
		add	bp,DREG
		add	CYCLE,4
		%endmacro
		%macro	INDDREG	1
		mov	bp,%1
		add	bp,DREG
		READW
		mov	bp,ax
		add	CYCLE,7
		%endmacro
		%macro	I8OFF	1
		IMMB
		cbw
		mov	bp,%1
		add	bp,ax
		inc	CYCLE
		%endmacro
		%macro	IND8OFF	1
		IMMB
		cbw
		mov	bp,%1
		add	bp,ax
		READW
		mov	bp,ax
		add	CYCLE,4
		%endmacro
		%macro	I16OFF	1
		IMMW
		mov	bp,%1
		add	bp,ax
		add	CYCLE,4
		%endmacro
		%macro	IND16OFF	1
		IMMW
		mov	bp,%1
		add	bp,ax
		READW
		mov	bp,ax
		add	CYCLE,7
		%endmacro

;
; ポストバイト $80
; ,X+
;
		ALIGN
xinc:
		IINC1	XREG
		ret

;
; ポストバイト $81
; ,X++
;
		ALIGN
xincinc:
		IINC2	XREG
		ret

;
; ポストバイト $82
; ,-X
;
		ALIGN
xdec:
		IDEC1	XREG
		ret

;
; ポストバイト $83
; ,--X
;
		ALIGN
xdecdec:
		IDEC2	XREG
		ret

;
; ポストバイト $84
; ,X
;
		ALIGN
xnormal:
		INORM	XREG
		ret

;
; ポストバイト $85
; B,X
;
		ALIGN
xbreg:
		IBREG	XREG
		ret

;
; ポストバイト $86,$87
; A,X
;
		ALIGN
xareg:
		IAREG	XREG
		ret

;
; ポストバイト $88
; 8bit,X
;
		ALIGN
x8off:
		I8OFF	XREG
		ret

;
; ポストバイト $89
; 16bit,X
;
		ALIGN
x16off:
		I16OFF	XREG
		ret

;
; ポストバイト $8A,$AA,$CA,$EA
; (PC+1)|$FF
;
		ALIGN
pcp1orFF:
		mov	bp,PCREG
		inc	bp
		or	bp,0ffh
		ret

;
; ポストバイト $8B
; D,X
;
		ALIGN
xdreg:
		IDREG	XREG
		ret

;
; ポストバイト $8C,$AC,$CC,$EC
; 8bit,PCR
;
		ALIGN
pcr8:
		I8OFF	PCREG
		ret

;
; ポストバイト $8D,$AD,$CD,$ED
; 16bit,PCR
;
		ALIGN
pcr16:
		I16OFF	PCREG
		inc	CYCLE
		ret

;
; ポストバイト $8E,$AE,$CE,$EE
; $FFFF
;
		ALIGN
eaFFFF:
		mov	bp,0ffffh
		ret

;
; ポストバイト $8F,$AF,$CF,$EF
; addr
;
		ALIGN
iextend:
		EXTEND
		add	CYCLE,2
		ret

;
; ポストバイト $90
; [,X+]
;
		ALIGN
ixinc:
		INDINC1	XREG
		ret

;
; ポストバイト $91
; [,X++]
;
		ALIGN
ixincinc:
		INDINC2	XREG
		ret

;
; ポストバイト $92
; [,-X]
;
		ALIGN
ixdec:
		INDDEC1	XREG
		ret

;
; ポストバイト $93
; [,--X]
;
		ALIGN
ixdecdec:
		INDDEC2	XREG
		ret

;
; ポストバイト $94
; [,X]
;
		ALIGN
ixnormal:
		INDNORM	XREG
		ret

;
; ポストバイト $95
; [B,X]
;
		ALIGN
ixbreg:
		INDBREG	XREG
		ret

;
; ポストバイト $96,$97
; [A,X]
;
		ALIGN
ixareg:
		INDAREG	XREG
		ret

;
; ポストバイト $98
; [8bit,X]
;
		ALIGN
ix8off:
		IND8OFF	XREG
		ret

;
; ポストバイト $99
; [16bit,X]
;
		ALIGN
ix16off:
		IND16OFF XREG
		ret

;
; ポストバイト $9A,$BA,$DA,$FA
; [(PC+1)|$FF]
;
		ALIGN
ipcp1orFF:
		mov	bp,PCREG
		inc	bp
		or	bp,0ffh
		READW
		mov	bp,ax
		add	CYCLE,3
		ret

;
; ポストバイト $9B
; [D,X]
;
		ALIGN
ixdreg:
		INDDREG	XREG
		ret

;
; ポストバイト $9C,$BC,$DC,$FC
; [8bit,PCR]
;
		ALIGN
ipcr8:
		IND8OFF PCREG
		ret

;
; ポストバイト $9D,$BD,$DD,$FD
; [16bit,PCR]
;
		ALIGN
ipcr16:
		IND16OFF PCREG
		inc	CYCLE
		ret

;
; ポストバイト $9E,$BE,$DE,$FE
; [$FFFF]
;
		ALIGN
iFFFF:
		mov	bp,0ffffh
		READW
		add	CYCLE,3
		ret

;
; ポストバイト $9F,$BF,$DF,$FF
; [addr]
;
		ALIGN
indirect:
		EXTEND
		READW
		mov	bp,ax
		add	CYCLE,5
		ret

;
; ポストバイト $A0
; ,Y+
;
		ALIGN
yinc:
		IINC1	YREG
		ret

;
; ポストバイト $A1
; ,Y++
;
		ALIGN
yincinc:
		IINC2	YREG
		ret

;
; ポストバイト $A2
; ,-Y
;
		ALIGN
ydec:
		IDEC1	YREG
		ret

;
; ポストバイト $A3
; ,--Y
;
		ALIGN
ydecdec:
		IDEC2	YREG
		ret

;
; ポストバイト $A4
; ,Y
;
		ALIGN
ynormal:
		INORM	YREG
		ret

;
; ポストバイト $A5
; B,Y
;
		ALIGN
ybreg:
		IBREG	YREG
		ret

;
; ポストバイト $A6,$A7
; A,Y
;
		ALIGN
yareg:
		IAREG	YREG
		ret

;
; ポストバイト $A8
; 8bit,Y
;
		ALIGN
y8off:
		I8OFF	YREG
		ret

;
; ポストバイト $A9
; 16bit,S
;
		ALIGN
y16off:
		I16OFF	YREG
		ret

;
; ポストバイト $AB
; D,Y
;
		ALIGN
ydreg:
		IDREG	YREG
		ret

;
; ポストバイト $B0
; [,Y+]
;
		ALIGN
iyinc:
		INDINC1	YREG
		ret

;
; ポストバイト $B1
; [,Y++]
;
		ALIGN
iyincinc:
		INDINC2	YREG
		ret

;
; ポストバイト $B2
; [,-Y]
;
		ALIGN
iydec:
		INDDEC1	YREG
		ret

;
; ポストバイト $B3
; [,--Y]
;
		ALIGN
iydecdec:
		INDDEC2	YREG
		ret

;
; ポストバイト $B4
; [,Y]
;
		ALIGN
iynormal:
		INDNORM	YREG
		ret

;
; ポストバイト $B5
; [B,Y]
;
		ALIGN
iybreg:
		INDBREG	YREG
		ret

;
; ポストバイト $B6,$B7
; [A,Y]
;
		ALIGN
iyareg:
		INDAREG	YREG
		ret

;
; ポストバイト $B8
; [8bit,Y]
;
		ALIGN
iy8off:
		IND8OFF	YREG
		ret

;
; ポストバイト $B9
; [16bit,Y]
;
		ALIGN
iy16off:
		IND16OFF YREG
		ret

;
; ポストバイト $BB
; [D,Y]
;
		ALIGN
iydreg:
		INDDREG	YREG
		ret

;
; ポストバイト $C0
; ,U+
;
		ALIGN
uinc:
		IINC1	UREG
		ret

;
; ポストバイト $C1
; ,U++
;
		ALIGN
uincinc:
		IINC2	UREG
		ret

;
; ポストバイト $C2
; ,-U
;
		ALIGN
udec:
		IDEC1	UREG
		ret

;
; ポストバイト $C3
; ,--U
;
		ALIGN
udecdec:
		IDEC2	UREG
		ret

;
; ポストバイト $C4
; ,U
;
		ALIGN
unormal:
		INORM	UREG
		ret

;
; ポストバイト $C5
; B,U
;
		ALIGN
ubreg:
		IBREG	UREG
		ret

;
; ポストバイト $C6,$C7
; A,U
;
		ALIGN
uareg:
		IAREG	UREG
		ret

;
; ポストバイト $C8
; 8bit,U
;
		ALIGN
u8off:
		I8OFF	UREG
		ret

;
; ポストバイト $C9
; 16bit,U
;
		ALIGN
u16off:
		I16OFF	UREG
		ret

;
; ポストバイト $CB
; D,U
;
		ALIGN
udreg:
		IDREG	UREG
		ret

;
; ポストバイト $D0
; [,U+]
;
		ALIGN
iuinc:
		INDINC1	UREG
		ret

;
; ポストバイト $D1
; [,U++]
;
		ALIGN
iuincinc:
		INDINC2	UREG
		ret

;
; ポストバイト $D2
; [,-U]
;
		ALIGN
iudec:
		INDDEC1	UREG
		ret

;
; ポストバイト $D3
; [,--U]
;
		ALIGN
iudecdec:
		INDDEC2	UREG
		ret

;
; ポストバイト $D4
; [,U]
;
		ALIGN
iunormal:
		INDNORM	UREG
		ret

;
; ポストバイト $D5
; [B,U]
;
		ALIGN
iubreg:
		INDBREG	UREG
		ret

;
; ポストバイト $D6,$D7
; [A,U]
;
		ALIGN
iuareg:
		INDAREG	UREG
		ret

;
; ポストバイト $D8
; [8bit,U]
;
		ALIGN
iu8off:
		IND8OFF	UREG
		ret

;
; ポストバイト $D9
; [16bit,U]
;
		ALIGN
iu16off:
		IND16OFF UREG
		ret

;
; ポストバイト $DB
; [D,U]
;
		ALIGN
iudreg:
		INDDREG	UREG
		ret

;
; ポストバイト $E0
; ,S+
;
		ALIGN
sinc:
		IINC1	SREG
		ret

;
; ポストバイト $E1
; ,S++
;
		ALIGN
sincinc:
		IINC2	SREG
		ret

;
; ポストバイト $E2
; ,-S
;
		ALIGN
sdec:
		IDEC1	SREG
		ret

;
; ポストバイト $E3
; ,--S
;
		ALIGN
sdecdec:
		IDEC2	SREG
		ret

;
; ポストバイト $E4
; ,S
;
		ALIGN
snormal:
		INORM	SREG
		ret

;
; ポストバイト $E5
; B,S
;
		ALIGN
sbreg:
		IBREG	SREG
		ret

;
; ポストバイト $E6,$E7
; A,S
;
		ALIGN
sareg:
		IAREG	SREG
		ret

;
; ポストバイト $E8
; 8bit,S
;
		ALIGN
s8off:
		I8OFF	SREG
		ret

;
; ポストバイト $E9
; 16bit,S
;
		ALIGN
s16off:
		I16OFF	SREG
		ret

;
; ポストバイト $EB
; D,S
;
		ALIGN
sdreg:
		IDREG	SREG
		ret

;
; ポストバイト $F0
; [,S+]
;
		ALIGN
isinc:
		INDINC1	SREG
		ret

;
; ポストバイト $F1
; [,S++]
;
		ALIGN
isincinc:
		INDINC2	SREG
		ret

;
; ポストバイト $F2
; [,-S]
;
		ALIGN
isdec:
		INDDEC1	SREG
		ret

;
; ポストバイト $F3
; [,--S]
;
		ALIGN
isdecdec:
		INDDEC2	SREG
		ret

;
; ポストバイト $F4
; [,S]
;
		ALIGN
isnormal:
		INDNORM	SREG
		ret

;
; ポストバイト $F5
; [B,U]
;
		ALIGN
isbreg:
		INDBREG	SREG
		ret

;
; ポストバイト $F6,$F7
; [A,S]
;
		ALIGN
isareg:
		INDAREG	SREG
		ret

;
; ポストバイト $F8
; [8bit,S]
;
		ALIGN
is8off:
		IND8OFF	SREG
		ret

;
; ポストバイト $F9
; [16bit,S]
;
		ALIGN
is16off:
		IND16OFF SREG
		ret

;
; ポストバイト $FB
; [D,S]
;
		ALIGN
isdreg:
		INDDREG	SREG
		ret

;
; ジャンプテーブル(インデックス)
;
		ALIGN
jump_table_idx:
; $80
		ADDR	xinc
		ADDR	xincinc
		ADDR	xdec
		ADDR	xdecdec
		ADDR	xnormal
		ADDR	xbreg
		ADDR	xareg
		ADDR	xareg
		ADDR	x8off
		ADDR	x16off
		ADDR	pcp1orFF
		ADDR	xdreg
		ADDR	pcr8
		ADDR	pcr16
		ADDR	eaFFFF
		ADDR	iextend
; $90
		ADDR	ixinc
		ADDR	ixincinc
		ADDR	ixdec
		ADDR	ixdecdec
		ADDR	ixnormal
		ADDR	ixbreg
		ADDR	ixareg
		ADDR	ixareg
		ADDR	ix8off
		ADDR	ix16off
		ADDR	ipcp1orFF
		ADDR	ixdreg
		ADDR	ipcr8
		ADDR	ipcr16
		ADDR	iFFFF
		ADDR	indirect
; $A0
		ADDR	yinc
		ADDR	yincinc
		ADDR	ydec
		ADDR	ydecdec
		ADDR	ynormal
		ADDR	ybreg
		ADDR	yareg
		ADDR	yareg
		ADDR	y8off
		ADDR	y16off
		ADDR	pcp1orFF
		ADDR	ydreg
		ADDR	pcr8
		ADDR	pcr16
		ADDR	eaFFFF
		ADDR	iextend
; $B0
		ADDR	iyinc
		ADDR	iyincinc
		ADDR	iydec
		ADDR	iydecdec
		ADDR	iynormal
		ADDR	iybreg
		ADDR	iyareg
		ADDR	iyareg
		ADDR	iy8off
		ADDR	iy16off
		ADDR	ipcp1orFF
		ADDR	iydreg
		ADDR	ipcr8
		ADDR	ipcr16
		ADDR	iFFFF
		ADDR	indirect
; $C0
		ADDR	uinc
		ADDR	uincinc
		ADDR	udec
		ADDR	udecdec
		ADDR	unormal
		ADDR	ubreg
		ADDR	uareg
		ADDR	uareg
		ADDR	u8off
		ADDR	u16off
		ADDR	pcp1orFF
		ADDR	udreg
		ADDR	pcr8
		ADDR	pcr16
		ADDR	eaFFFF
		ADDR	iextend
; $D0
		ADDR	iuinc
		ADDR	iuincinc
		ADDR	iudec
		ADDR	iudecdec
		ADDR	iunormal
		ADDR	iubreg
		ADDR	iuareg
		ADDR	iuareg
		ADDR	iu8off
		ADDR	iu16off
		ADDR	ipcp1orFF
		ADDR	iudreg
		ADDR	ipcr8
		ADDR	ipcr16
		ADDR	iFFFF
		ADDR	indirect
; $E0
		ADDR	sinc
		ADDR	sincinc
		ADDR	sdec
		ADDR	sdecdec
		ADDR	snormal
		ADDR	sbreg
		ADDR	sareg
		ADDR	sareg
		ADDR	s8off
		ADDR	s16off
		ADDR	pcp1orFF
		ADDR	sdreg
		ADDR	pcr8
		ADDR	pcr16
		ADDR	eaFFFF
		ADDR	iextend
; $F0
		ADDR	isinc
		ADDR	isincinc
		ADDR	isdec
		ADDR	isdecdec
		ADDR	isnormal
		ADDR	isbreg
		ADDR	isareg
		ADDR	isareg
		ADDR	is8off
		ADDR	is16off
		ADDR	ipcp1orFF
		ADDR	isdreg
		ADDR	ipcr8
		ADDR	ipcr16
		ADDR	iFFFF
		ADDR	indirect

;
; インデックスモード
;
; param.: bp	PCREG
; result: bp	EA
;
		ALIGN
cpu_index:
; ポストバイト取得
		READB
		inc	PCREG
		inc	bp
; 判定
		or	al,al
		js	.jump
; 5bit オフセット
		mov	bp,XREG
		cmp	al,20h
		jc	.bitok
		mov	bp,YREG
		cmp	al,40h
		jc	.bitok
		mov	bp,UREG
		cmp	al,60h
		jc	.bitok
		mov	bp,SREG
.bitok:
		and	al,1fh
		cmp	al,10h
		jnc	.bitminus
; 5bit オフセット(0〜15)
		and	ax,000fh
		add	bp,ax
		inc	CYCLE
		ret
		ALIGN
; 5bit オフセット(-0〜-15)
.bitminus:
		or	ax,0fff0h
		add	bp,ax
		inc	CYCLE
		ret
		ALIGN
; ジャンプ
.jump:
	%ifdef	__MSDOS__
		mov	di,jump_table_idx
		mov	bl,al
		and	bl,7fh
		xor	bh,bh
		add	bx,bx
		jmp	near [cs:bx+di]
	%else
		mov	edi,jump_table_idx
		xor	ebx,ebx
		mov	bl,al
		and	bl,7fh
		jmp	near [ebx*4+edi]
	%endif

;
; 未定義命令、もしくは未サポート
;
		ALIGN
cpu_error:
		xor	al,al
		ret

;
; CPU実行(共通)
;
		ALIGN
cpu_exec:
		mov	cx,INTR
		mov	dl,CCREG
; 謎命令($14,$15,$CD)チェック
		test	cx,8000h
		jz	.nmi_chk
; 謎命令($14,$15,$CD)実行中
		mov	bp,PCREG
		READB
		inc	PCREG
		mov	CYCLE,2
		ret
; NMIチェック
.nmi_chk:
		test	cx,0007h
		jz	.ok
		test	cx,0001h
		jz	.firq_chk
; ~NMI
		or	INTR,0040h
		test	cx,0010h
		jz	.firq_chk
		call	cpu_nmi
		call	cpu_execline
		mov	cx,19
		jmp	.int_cycle
; FIRQチェック
		ALIGN
.firq_chk:
		test	cx,0002h
		jz	.irq_chk
; ~FIRQ
		or	INTR,0040h
		test	dl,040h
		jnz	.irq_chk
		call	cpu_firq
		call	cpu_execline
		mov	cx,10
		jmp	.int_cycle
; IRQチェック
		ALIGN
.irq_chk:
		test	cx,0004h
		jz	.ok
; ~IRQ
		or	INTR,0040h
		test	dl,10h
		jnz	.ok
		call	cpu_irq
		call	cpu_execline
		mov	cx,19
		jmp	.int_cycle
; 割り込み時スタック退避サイクルの加算
		ALIGN
.int_cycle:
		test	INTR,0080h
		jnz	.exit
		add	CYCLE,cx
.exit:
		ret

; 実行
		ALIGN
.ok:
; ステップ実行に続ける

;
; CPUステップ実行
;
cpu_execline:
; 命令フェッチ
		mov	bp,PCREG
		READB
	%ifndef _hoot
		mov	[_fetch_op], al
	%endif
		inc	PCREG
		inc	bp
; サイクルテーブル１を参照
.first:
	%ifdef	__MSDOS__
		mov	di,cycle_table1
		mov	bl,al
		xor	bh,bh
		mov	dl,[cs:bx+di]
	%else
		mov	edi,cycle_table1
		xor	ebx,ebx
		mov	bl,al
		mov	dl,[ebx+edi]
	%endif
		cmp	dl,0ffh
		jz	.second
		mov	CYCLE,dx
		and	CYCLE,007fh
		or	dl,dl
		jns	.normal
; インデックスモード
	%ifdef	__MSDOS__
		push	bx
		call	cpu_index
		pop	bx
	%else
		push	ebx
		call	cpu_index
		pop	ebx
	%endif
; １バイト命令呼び出し si=REG bp=PCREG
.normal:
	%ifdef	__MSDOS__
		mov	di,jump_table1
		shl	bx,1
		jmp	near [cs:bx+di]
	%else
		mov	edi,jump_table1
		jmp	near [ebx*4+edi]
	%endif
		ALIGN
; $10 2ndページ
.second:
		cmp	al,10h
		jnz	.third
		READB
		inc	PCREG
		inc	bp
; サイクルテーブル２を参照
	%ifdef	__MSDOS__
		mov	di,cycle_table2
		mov	bl,al
		xor	bh,bh
		mov	dl,[cs:bx+di]
	%else
		mov	edi,cycle_table2
		xor	ebx,ebx
		mov	bl,al
		mov	dl,[ebx+edi]
	%endif
		mov	CYCLE,dx
		and	CYCLE,007fh
		or	dl,dl
		jz	.first
		jns	.sec_norm
; インデックスモード
	%ifdef	__MSDOS__
		push	bx
		call	cpu_index
		pop	bx
	%else
		push	ebx
		call	cpu_index
		pop	ebx
	%endif
; １バイト命令呼び出し si=REG bp=PCREG
.sec_norm:
	%ifdef	__MSDOS__
		mov	di,jump_table2
		shl	bx,1
		jmp	near [cs:bx+di]
	%else
		mov	edi,jump_table2
		jmp	near [ebx*4+edi]
	%endif
		ALIGN
; $11 3rdページ
.third:
		cmp	al,11h
		jnz	.error
		READB
		inc	PCREG
		inc	bp
; サイクルテーブル２を参照
	%ifdef	__MSDOS__
		mov	di,cycle_table2
		mov	bl,al
		xor	bh,bh
		mov	dl,[cs:bx+di]
	%else
		mov	edi,cycle_table2
		xor	ebx,ebx
		mov	bl,al
		mov	dl,[ebx+edi]
	%endif
		mov	CYCLE,dx
		and	CYCLE,007fh
		or	dl,dl
		jns	.third_ok
		jnz	.third_idx
		jmp	.first
; インデックスモード
.third_idx:
		push	ax
		call	cpu_index
		pop	ax
; テーブル検索
.third_ok:
	%ifdef	__MSDOS__
		mov	bx,jump_table3
.third_loop:
		cmp	byte [cs:bx],00h
		jz	.error
		cmp	al,[cs:bx]
		jz	.third_jump
		add	bx,4
		jmp	.third_loop
; サイクル取り出し
.third_jump:
		add	bx,2
		jmp	near [cs:bx]
	%else
		mov	ebx,jump_table3
.third_loop:
		cmp	byte [ebx],00h
		jz	.error
		cmp	al,[ebx]
		jz	.third_jump
		add	ebx,8
		jmp	.third_loop
; サイクル取り出し
.third_jump:
		jmp	near [ebx+4]
	%endif
		ALIGN
.error:
		call	cpu_error
		ret

;
; CPUリセット
;
		ALIGN
cpu_reset:
		mov	CCREG,50h
	%ifdef __MSDOS__
		xor	eax,eax
	%else
		xor	ax,ax
	%endif
		mov	DPREG,al
		mov	DREG,ax
		mov	XREG,ax
		mov	YREG,ax
		mov	UREG,ax
		mov	SREG,ax
		mov	INTR,ax
		mov	CYCLE,ax
		mov	TOTAL,ax
		mov	bp,0fffeh
		READW
		mov	PCREG,ax
		ret

;
; NMI
;
; param.: si	REG
;	  cx	INTR
;
		ALIGN
cpu_nmi:
; CWAI対策
		or	INTR,0100h
; Eフラグ設定
		or	CCREG,80h
; レジスタ退避
		PSHSW	PCREG
		PSHSW	UREG
		PSHSW	YREG
		PSHSW	XREG
		PSHSB	DPREG
		PSHSB	BREG
		PSHSB	AREG
		PSHSB	CCREG
; NMIをかける
		or	CCREG,50h
		mov	bp,0fffch
		READW
		mov	PCREG,ax
; フラグ処理
		and	INTR,0fffeh
		ret

;
; FIRQ
;
; param.: si	REG
;
		ALIGN
cpu_firq:
; CWAIチェック
		test	INTR,0080h
		jz	.normal
		jmp	.cwai
; Eフラグクリア
		ALIGN
.normal:
		and	CCREG,7fh
; レジスタ退避
		PSHSW	PCREG
		PSHSB	CCREG
; FIRQをかける
		or	CCREG,50h
		mov	bp,0fff6h
		READW
		mov	PCREG,ax
; フラグ処理
;		and	INTR,0fffdh
		ret
; CWAIの場合は、Eフラグ=1で行くので注意
		ALIGN
.cwai:
		or	INTR,0100h
; Eフラグ設定
		or	CCREG,80h
; レジスタ退避
		PSHSW	PCREG
		PSHSW	UREG
		PSHSW	YREG
		PSHSW	XREG
		PSHSB	DPREG
		PSHSB	BREG
		PSHSB	AREG
		PSHSB	CCREG
; FIRQをかける
		or	CCREG,50h
		mov	bp,0fff6h
		READW
		mov	PCREG,ax
; フラグ処理
;		and	INTR,0fffdh
		ret

;
; IRQ
;
; param.: si	REG
;
		ALIGN
cpu_irq:
; CWAI対策
		or	INTR,0100h
; Eフラグ設定
		or	CCREG,80h
; レジスタ退避
		PSHSW	PCREG
		PSHSW	UREG
		PSHSW	YREG
		PSHSW	XREG
		PSHSB	DPREG
		PSHSB	BREG
		PSHSB	AREG
		PSHSB	CCREG
; IRQをかける
		or	CCREG,10h
		mov	bp,0fff8h
		READW
		mov	PCREG,ax
; フラグ処理
;		and	INTR,0fffbh
		ret

;-----------------------------------------------------------------------------
; hoot向けここから
; http://dmpsoft.viraualave.net/
;-----------------------------------------------------------------------------
%ifdef _hoot
;
; CPU実行
;
		ALIGN
%ifdef	_XWIN
xm7_6809_exec:
%else
_xm7_6809_exec:
%endif
	%ifdef	__MSDOS__
		push	si
		push	di
		push	bp
		mov	bp,sp
		mov	si,_xm7_6809
		mov	ax,[bp+10]
		neg	ax
.loop:
		mov	TOTAL,ax
		call	cpu_exec
		mov	ax,TOTAL
		add	ax,CYCLE
		js	.loop
		pop	bp
		pop	di
		pop	si
		retf
	%else
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov	ebp,esp
		mov	esi,_xm7_6809
		mov	eax,[ebp+20]
		cmp	eax,10000h
		jnc	.exit
		neg	ax
		xor	ebp,ebp
.loop:
		mov	TOTAL,ax
		call	cpu_exec
		mov	ax,TOTAL
		add	ax,CYCLE
		js	.loop
.exit:
		pop	ebp
		pop	edi
		pop	esi
		pop	ebx
		ret
	%endif

;
; CPUリセット
;
		ALIGN
%ifdef	_XWIN
xm7_6809_rst:
%else
_xm7_6809_rst:
%endif
	%ifdef	__MSDOS__
		push	si
		push	di
		push	bp
		mov	si,_xm7_6809
		call	cpu_reset
		pop	bp
		pop	di
		pop	si
		retf
	%else
		push	ebx
		push	esi
		push	edi
		push	ebp
		xor	ebp,ebp
		mov	esi,_xm7_6809
		call	cpu_reset
		pop	ebp
		pop	edi
		pop	esi
		pop	ebx
		ret
	%endif
;-----------------------------------------------------------------------------
; hoot向けここまで
; http://dmpsoft.viraualave.net/
;-----------------------------------------------------------------------------
%else

;
; メインCPUステップ実行
;
		ALIGN
%ifdef	_XWIN
main_line:
%else
_main_line:
%endif
	%ifdef	__MSDOS__
		push	si
		push	di
		push	bp
		mov	si,_maincpu
		call	cpu_execline
		pop	bp
		pop	di
		pop	si
		retf
	%else
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov	esi,_maincpu
		call	cpu_execline
		pop	ebp
		pop	edi
		pop	esi
		pop	ebx
		ret
	%endif

;
; サブCPUステップ実行
;
		ALIGN
%ifdef	_XWIN
sub_line:
%else
_sub_line:
%endif
	%ifdef	__MSDOS__
		push	si
		push	di
		push	bp
		mov	si,_subcpu
		call	cpu_execline
		pop	bp
		pop	di
		pop	si
		retf
	%else
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov	esi,_subcpu
		call	cpu_execline
		pop	ebp
		pop	edi
		pop	esi
		pop	ebx
		ret
	%endif

;
; 日本語サブCPUステップ実行
;
	%ifdef	JSUB
		ALIGN
%ifdef	_XWIN
jsub_line:
%else
_jsub_line:
%endif
	%ifdef	__MSDOS__
		push	si
		push	di
		push	bp
		mov	si,_jsubcpu
		call	cpu_execline
		pop	bp
		pop	di
		pop	si
		retf
	%else
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov	esi,_jsubcpu
		call	cpu_execline
		pop	ebp
		pop	edi
		pop	esi
		pop	ebx
		ret
	%endif
	%endif

;
; メインCPU実行
;
		ALIGN
%ifdef	_XWIN
main_exec:
%else
_main_exec:
%endif
	%ifdef	__MSDOS__
		push	si
		push	di
		push	bp
		mov	si,_maincpu
		call	cpu_exec
		mov	ax,CYCLE
		add	TOTAL,ax
		pop	bp
		pop	di
		pop	si
		retf
	%else
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov	esi,_maincpu
		call	cpu_exec
		mov	ax,CYCLE
		add	TOTAL,ax
		pop	ebp
		pop	edi
		pop	esi
		pop	ebx
		ret
	%endif

;
; サブCPU実行
;
		ALIGN
%ifdef	_XWIN
sub_exec:
%else
_sub_exec:
%endif
	%ifdef	__MSDOS__
		push	si
		push	di
		push	bp
		mov	si,_subcpu
		call	cpu_exec
		mov	ax,CYCLE
		add	TOTAL,ax
		pop	bp
		pop	di
		pop	si
		retf
	%else
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov	esi,_subcpu
		call	cpu_exec
		mov	ax,CYCLE
		add	TOTAL,ax
		pop	ebp
		pop	edi
		pop	esi
		pop	ebx
		ret
	%endif

;
; 日本語サブCPU実行
;
	%ifdef	JSUB
		ALIGN
%ifdef	_XWIN
jsub_exec:
%else
_jsub_exec:
%endif
	%ifdef	__MSDOS__
		push	si
		push	di
		push	bp
		mov	si,_jsubcpu
		call	cpu_exec
		mov	ax,CYCLE
		add	TOTAL,ax
		pop	bp
		pop	di
		pop	si
		retf
	%else
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov	esi,_jsubcpu
		call	cpu_exec
		mov	ax,CYCLE
		add	TOTAL,ax
		pop	ebp
		pop	edi
		pop	esi
		pop	ebx
		ret
	%endif
	%endif

;
; メインCPUリセット
;
		ALIGN
%ifdef	_XWIN
main_reset:
%else
_main_reset:
%endif
	%ifdef	__MSDOS__
		push	si
		push	di
		push	bp
		mov	si,_maincpu
		call	cpu_reset
		pop	bp
		pop	di
		pop	si
		retf
	%else
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov	esi,_maincpu
		call	cpu_reset
		pop	ebp
		pop	edi
		pop	esi
		pop	ebx
		ret
	%endif

;
; サブCPUリセット
;
		ALIGN
%ifdef	_XWIN
sub_reset:
%else
_sub_reset:
%endif
	%ifdef	__MSDOS__
		push	si
		push	di
		push	bp
		mov	si,_subcpu
		call	cpu_reset
		pop	bp
		pop	di
		pop	si
		retf
	%else
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov	esi,_subcpu
		call	cpu_reset
		pop	ebp
		pop	edi
		pop	esi
		pop	ebx
		ret
	%endif

;
; 日本語サブCPUリセット
;
	%ifdef	JSUB
		ALIGN
%ifdef	_XWIN
jsub_reset:
%else
_jsub_reset:
%endif
	%ifdef	__MSDOS__
		push	si
		push	di
		push	bp
		mov	si,_jsubcpu
		call	cpu_reset
		pop	bp
		pop	di
		pop	si
		retf
	%else
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov	esi,_jsubcpu
		call	cpu_reset
		pop	ebp
		pop	edi
		pop	esi
		pop	ebx
		ret
	%endif
	%endif

%endif

;
; プログラム終了
;
		end
