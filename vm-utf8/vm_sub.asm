;
; FM-7 EMULATOR "XM7"
;
; Copyright (C) 1999-2009 ＰＩ．(yasushi@tanaka.net)
; Copyright (C) 2001-2009 Ryu Takegami
;
; [ 仮想マシン アセンブラサブ ]
;

;
; 外部定義
;
	%if	XM7_VER >= 3
	  %ifdef	_OMF
		section	.text class=CODE align=16 use32
		global	_memcpy400l
	  %endif
	  %ifdef	_WIN32
		section	.text class=CODE align=16 use32
		global	_memcpy400l
	  %endif
	  %ifdef	_XWIN
		section	.text code align=16
		global	memcpy400l
	  %endif
	  %ifdef	_TOWNS
		section	.text code align=16 use32
		global	memcpy400l
	  %endif
	%endif
	
	%if	XM7_VER >= 3
;
; 400ラインモード スクロール処理用メモリ転送
;
; void memcpy400l(void *dest, void *src, int count)
; dest		ebp+8
; src		ebp+12
; count		ebp+16
;
		align	4
%ifdef _XWIN
memcpy400l:
%else
  %ifdef _TOWNS
memcpy400l:
  %else
_memcpy400l:
  %endif
%endif
		push	ebp
		mov	ebp,esp
		push	edi
		mov	edi,[ebp+8]
		mov	edx,[ebp+12]
		mov	ecx,[ebp+16]
		shr	ecx,1

		align	4
.loop:
		jz	.exit
		add	edx,2
		add	edi,2
		mov	al,[edx-2]
		mov	[edi-2],al
		dec	ecx
		jmp	.loop
.exit:
		pop	edi
		pop	ebp
		ret

	%endif

;
; プログラム終了
;
		end
