/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2013 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2013 Ryu Takegami
 *	Copyright (C) 2010-2013 Toma
 *
 *	[ バブルメモリ コントローラ (32KB専用版) ]
 */  
	
#if XM7_VER == 1 && defined(BUBBLE)
	
#include <string.h>
#include <stdlib.h>
#include "xm7.h"
#include "device.h"
#include "bubble.h"
#include "fdc.h"
#include "opn.h"
	
/*
 *	グローバル ワーク
 */ 
	
/* レジスタ */ 
	BYTE bmc_datareg;	/* データレジスタ */
BYTE bmc_command;	/* コマンドレジスタ */
BYTE bmc_status;	/* ステータスレジスタ */
BYTE bmc_errorreg;	/* エラーステータスレジスタ */
WORD bmc_pagereg;	/* ページレジスタ */
WORD bmc_countreg;	/* ページカウントレジスタ */
 
/* 外部ワーク */ 
	WORD bmc_totalcnt;	/* トータルカウンタ */
WORD bmc_nowcnt;	/* カレントカウンタ */
BYTE bmc_unit;	/* ユニット */
BYTE bmc_ready[BMC_UNITS_32];	/* レディ状態 */
BOOL bmc_teject[BMC_UNITS_32];	/* 一時イジェクト */
BOOL bmc_writep[BMC_UNITS_32];	/* 書き込み禁止状態 */
 char bmc_fname[BMC_UNITS_32][256 + 1];	/* ファイル名 */
char bmc_name[BMC_UNITS_32][BMC_MEDIAS][17];	/* イメージ名 */
BOOL bmc_fwritep[BMC_UNITS_32];	/* ライトプロテクト状態 */
BYTE bmc_header[BMC_UNITS_32][0x20];	/* B77ファイルヘッダ */
BYTE bmc_medias[BMC_UNITS_32];	/* メディア枚数 */
BYTE bmc_media[BMC_UNITS_32];	/* メディアセレクト状態 */
BYTE bmc_access[BMC_UNITS_32];	/* アクセスLED */
 BOOL bmc_enable;	/* 有効・無効フラグ */
BOOL bmc_use;	/* 使用フラグ */
 
/*
 *	スタティック ワーク
 */ 
static BYTE bmc_buffer[BMC_PSIZE_32];	/* 32byteデータバッファ */
static BYTE *bmc_dataptr;			/* データポインタ */
static DWORD bmc_offset;				/* オフセット */
static DWORD bmc_foffset[BMC_UNITS_32][BMC_MEDIAS];
static DWORD bmc_fsize[BMC_UNITS_32];

#ifdef FDDSND
static BOOL bmc_wait;						/* ウェイトモード実行フラグ */

#endif	/*  */
	
/*
 *	バブルメモリ コントローラ
 *	初期化
 */ 
	BOOL FASTCALL bmc_init(void) 
{
	
		/* ファイル関係をリセット */ 
		bmc_totalcnt = 0;
	bmc_nowcnt = 0;
	 memset(bmc_ready, BMC_TYPE_NOTREADY, sizeof(bmc_ready));
	memset(bmc_teject, FALSE, sizeof(bmc_teject));
	memset(bmc_writep, FALSE, sizeof(bmc_writep));
	memset(bmc_fname, 0, sizeof(bmc_fname));
	memset(bmc_fwritep, FALSE, sizeof(bmc_fwritep));
	memset(bmc_medias, 0, sizeof(bmc_medias));
	 
		/* ファイルオフセットを全てクリア */ 
		memset(bmc_foffset, 0, sizeof(bmc_foffset));
	 
		/* デフォルトは無効 */ 
		bmc_enable = FALSE;
	 
		/* ウェイト挿入モードフラグ初期化 */ 
#ifdef FDDSND
		bmc_wait = FALSE;
	
#endif	/*  */
		return TRUE;
}

 
/*
 *	バブルメモリ コントローラ
 *	クリーンアップ
 */ 
	void FASTCALL
bmc_cleanup(void) 
{
	
		/* ファイル関係をリセット */ 
		memset(bmc_ready, 0, sizeof(bmc_ready));
}

 
/*
 *	バブルメモリ コントローラ
 *	リセット
 */ 
	void FASTCALL
bmc_reset(void) 
{
	
		/* 物理レジスタをリセット */ 
		bmc_datareg = 0;
	bmc_command = 0xff;
	bmc_status = 0;
	bmc_errorreg = 0;
	bmc_pagereg = 0;
	bmc_countreg = 0;
	 memset(bmc_access, 0, sizeof(bmc_access));
	bmc_unit = 0;
	bmc_dataptr = 0;
	bmc_offset = 0;
	 bmc_use = FALSE;
}

 
/*-[ ファイル管理 ]---------------------------------------------------------*/ 
	
/*
 *	ページ書き込み終了
 */ 
	static BOOL FASTCALL
bmc_write_page(void) 
{
	DWORD offset;
	int handle;
	 
		/* assert */ 
		ASSERT(bmc_ready[bmc_unit] != BMC_TYPE_NOTREADY);
	ASSERT(bmc_dataptr);
	ASSERT(bmc_totalcnt > 0);
	 
		/* オフセット算出 */ 
		offset = (DWORD) ((bmc_pagereg & BMC_MAXADDR_32) * BMC_PSIZE_32);
	if (bmc_ready[bmc_unit] == BMC_TYPE_B77) {
		if (bmc_fsize[bmc_unit] < offset + BMC_PSIZE_32 + 0x0020) {
			return FALSE;
		}
		offset += *(DWORD *) (&bmc_header[bmc_unit][0x0014]);
	}
	 
		/* 書き込み */ 
		handle = file_open(bmc_fname[bmc_unit], OPEN_RW);
	if (handle == -1) {
		return FALSE;
	}
	if (!file_seek(handle, offset)) {
		file_close(handle);
		return FALSE;
	}
	if (!file_write(handle, bmc_dataptr, bmc_totalcnt)) {
		file_close(handle);
		return FALSE;
	}
	file_close(handle);
	 return TRUE;
}

 
/*
 *	ページ読み込み
 */ 
	static BOOL FASTCALL
bmc_readbuf(void) 
{
	int handle;
	DWORD offset;
	 
		/* ページアドレスチェック */ 
		if (bmc_unit >= BMC_UNITS_32) {
		memset(bmc_buffer, 0, BMC_PSIZE_32);
		return FALSE;
	}
	 
		/* レディチェック */ 
		if (bmc_ready[bmc_unit] == BMC_TYPE_NOTREADY) {
		return FALSE;
	}
	 
		/* オフセット算出 */ 
		offset = (DWORD) (bmc_pagereg & BMC_MAXADDR_32);
	offset *= BMC_PSIZE_32;
	if (bmc_ready[bmc_unit] == BMC_TYPE_B77) {
		if (bmc_fsize[bmc_unit] < offset + BMC_PSIZE_32 + 0x0020) {
			return FALSE;
		}
		offset += *(DWORD *) (&bmc_header[bmc_unit][0x0014]);
	}
	bmc_offset = offset;
	 
		/* 読み込み */ 
		memset(bmc_buffer, 0, BMC_PSIZE_32);
	handle = file_open(bmc_fname[bmc_unit], OPEN_R);
	if (handle == -1) {
		return FALSE;
	}
	if (!file_seek(handle, offset)) {
		file_close(handle);
		return FALSE;
	}
	file_read(handle, bmc_buffer, BMC_PSIZE_32);
	file_close(handle);
	return TRUE;
}

 
/*
 *	B77ファイル ヘッダ読み込み
 */ 
	static BOOL FASTCALL
bmc_readhead(int unit, int index) 
{
	DWORD offset;
	DWORD temp;
	int handle;
	 
		/* assert */ 
		ASSERT((unit >= 0) && (unit < BMC_UNITS_32));
	ASSERT((index >= 0) && (index < BMC_MEDIAS));
	ASSERT(bmc_ready[unit] == BMC_TYPE_B77);
	 
		/* オフセット決定 */ 
		offset = bmc_foffset[unit][index];
	 
		/* シーク、読み込み */ 
		handle = file_open(bmc_fname[unit], OPEN_R);
	if (handle == -1) {
		return FALSE;
	}
	if (!file_seek(handle, offset)) {
		file_close(handle);
		return FALSE;
	}
	if (!file_read(handle, bmc_header[unit], 0x20)) {
		file_close(handle);
		return FALSE;
	}
	file_close(handle);
	 
		/* カセットサイズ */ 
		temp = 0;
	temp |= bmc_header[unit][0x001c + 3];
	temp *= 256;
	temp |= bmc_header[unit][0x001c + 2];
	temp *= 256;
	temp |= bmc_header[unit][0x001c + 1];
	temp *= 256;
	temp |= bmc_header[unit][0x001c + 0];
	bmc_fsize[unit] = temp;
	 
		/* タイプチェック */ 
		if (bmc_header[unit][0x001b] != 0x80) {
		
			/* 32KBでない */ 
			return FALSE;
	}
	 
		/* ライトプロテクト設定 */ 
		if (bmc_fwritep[unit]) {
		bmc_writep[unit] = TRUE;
	}
	
	else {
		if (bmc_header[unit][0x001a] & 0x10) {
			bmc_writep[unit] = TRUE;
		}
		
		else {
			bmc_writep[unit] = FALSE;
		}
	}
	 
		/* オフセット */ 
		*(DWORD *) (&bmc_header[unit][0x0014]) = offset + 0x0020;
	 return TRUE;
}

 
/*
 *	現在のメディアのライトプロテクトを切り替える
 */ 
	BOOL FASTCALL bmc_setwritep(int unit, BOOL writep) 
{
	BYTE header[0x2b0];
	DWORD offset;
	int handle;
	 
		/* assert */ 
		ASSERT((unit >= 0) && (unit < 2));
	ASSERT((writep == TRUE) || (writep == FALSE));
	 
		/* レディでなければならない */ 
		if (bmc_ready[unit] == BMC_TYPE_NOTREADY) {
		return FALSE;
	}
	 
		/* ファイルが書き込み不可ならダメ */ 
		if (bmc_fwritep[unit]) {
		return FALSE;
	}
	if (bmc_ready[unit] == BMC_TYPE_B77) {
		offset = bmc_foffset[unit][bmc_media[unit]];
		handle = file_open(bmc_fname[unit], OPEN_RW);
		if (handle == -1) {
			return FALSE;
		}
		if (!file_seek(handle, offset)) {
			file_close(handle);
			return FALSE;
		}
		if (!file_read(handle, header, 0x20)) {
			file_close(handle);
			return FALSE;
		}
		if (writep) {
			header[0x001a] |= 0x10;
		}
		
		else {
			header[0x001a] &= ~0x10;
		}
		if (!file_seek(handle, offset)) {
			file_close(handle);
			return FALSE;
		}
		if (!file_write(handle, header, 0x20)) {
			file_close(handle);
			return FALSE;
		}
		 file_close(handle);
	}
	 
		/* 成功 */ 
		bmc_writep[unit] = writep;
	 return TRUE;
}

 
/*
 *	メディア番号を設定
 */ 
	BOOL FASTCALL bmc_setmedia(int unit, int index) 
{
	
		/* assert */ 
		ASSERT((unit >= 0) && (unit <= 1));
	ASSERT((index >= 0) && (index < BMC_MEDIAS));
	 
		/* レディ状態か */ 
		if (bmc_ready[unit] == BMC_TYPE_NOTREADY) {
		return FALSE;
	}
	 
		/* 32KBファイルの場合、index = 0か */ 
		if ((bmc_ready[unit] == BMC_TYPE_32) && (index != 0)) {
		return FALSE;
	}
	 
		/* index > 0 なら、bmc_foffsetを調べて>0が必要 */ 
		if (index > 0) {
		if (bmc_foffset[unit][index] == 0) {
			return FALSE;
		}
	}
	 
		/* B77ファイルの場合、ヘッダ読み込み */ 
		if (bmc_ready[unit] == BMC_TYPE_B77) {
		
			/* ライトプロテクトは内部で設定 */ 
			if (!bmc_readhead(unit, index)) {
			return FALSE;
		}
	}
	
	else {
		
			/* 32KBファイルなら、ファイル属性に従う */ 
			bmc_writep[unit] = bmc_fwritep[unit];
	}
	 
		/* メディアが交換された場合、一時イジェクトを強制解除 */ 
		if (bmc_media[unit] != (BYTE) index) {
		bmc_media[unit] = (BYTE) index;
		bmc_teject[unit] = FALSE;
	}
	 return TRUE;
}

 
/*
 *	B77ファイル解析、メディア数および名称取得
 */ 
	static int FASTCALL
bmc_chkb77(int unit) 
{
	int i;
	int handle;
	int count;
	DWORD offset;
	DWORD len;
	BYTE buf[0x20];
	 
		/* 初期化 */ 
		for (i = 0; i < BMC_MEDIAS; i++) {
		bmc_foffset[unit][i] = 0;
		bmc_name[unit][i][0] = '\0';
	}
	count = 0;
	offset = 0;
	 
		/* ファイルオープン */ 
		handle = file_open(bmc_fname[unit], OPEN_R);
	if (handle == -1) {
		return count;
	}
	 
		/* メディアループ */ 
		while (count < BMC_MEDIAS) {
		
			/* シーク */ 
			if (!file_seek(handle, offset)) {
			file_close(handle);
			return count;
		}
		 
			/* 読み込み */ 
			if (!file_read(handle, buf, 0x0020)) {
			file_close(handle);
			return count;
		}
		 
			/* タイプチェック。32KBのみ対応 */ 
			if (buf[0x001b] != 0x80) {
			file_close(handle);
			return count;
		}
		 
			/* ok,ファイル名、オフセット格納 */ 
			buf[17] = '\0';
		memcpy(bmc_name[unit][count], buf, 17);
		bmc_foffset[unit][count] = offset;
		 
			/* next処理 */ 
			len = 0;
		len |= buf[0x1f];
		len *= 256;
		len |= buf[0x1e];
		len *= 256;
		len |= buf[0x1d];
		len *= 256;
		len |= buf[0x1c];
		offset += len;
		count++;
	}
	 
		/* 最大メディア枚数に達した */ 
		file_close(handle);
	return count;
}

  
/*
 *	ファイルを設定
 */ 
	int FASTCALL
bmc_setfile(int unit, char *fname) 
{
	BOOL writep;
	int handle;
	DWORD fsize;
	int count;
	 ASSERT((unit >= 0) && (unit < 2));
	 
		/* ノットレディにする場合 */ 
		if (fname == NULL) {
		bmc_ready[unit] = BMC_TYPE_NOTREADY;
		bmc_fname[unit][0] = '\0';
		return 1;
	}
	 
		/* ファイルをオープンし、ファイルサイズを調べる */ 
		if (strlen(fname) >= sizeof(bmc_fname[unit])) {
		bmc_ready[unit] = BMC_TYPE_NOTREADY;
		bmc_fname[unit][0] = '\0';
		return 1;
	}
	writep = FALSE;
	handle = file_open(fname, OPEN_RW);
	if (handle == -1) {
		handle = file_open(fname, OPEN_R);
		if (handle == -1) {
			bmc_ready[unit] = BMC_TYPE_NOTREADY;
			return 0;
		}
		writep = TRUE;
	}
	strcpy(bmc_fname[unit], fname);
	fsize = file_getsize(handle);
	file_close(handle);
	 
		/*
		 * 32KBファイル
		 */ 
		if (fsize == 32768) {
		
			/* タイプ、書き込み属性設定 */ 
			bmc_ready[unit] = BMC_TYPE_32;
		bmc_fwritep[unit] = writep;
		 
			/* メディア設定 */ 
			if (!bmc_setmedia(unit, 0)) {
			bmc_ready[unit] = BMC_TYPE_NOTREADY;
			bmc_fname[unit][0] = '\0';
			return 0;
		}
		 
			/* 成功。一時イジェクト解除 */ 
			bmc_teject[unit] = FALSE;
		bmc_medias[unit] = 1;
		return 1;
	}
	 
		/*
		 * B77ファイル
		 */ 
		bmc_ready[unit] = BMC_TYPE_B77;
	bmc_fwritep[unit] = writep;
	 
		/* ファイル検査 */ 
		count = bmc_chkb77(unit);
	if (count != 0) {
		
			/* メディア設定 */ 
			if (bmc_setmedia(unit, 0)) {
			
				/* 成功。一時イジェクト解除 */ 
				bmc_teject[unit] = FALSE;
			bmc_medias[unit] = (BYTE) count;
			return count;
		}
	}
	  bmc_ready[unit] = BMC_TYPE_NOTREADY;
	bmc_fname[unit][0] = '\0';
	return 0;
}

   
/*-[ BMCコマンド ]----------------------------------------------------------*/ 
	
/*
 *	ステータス作成
 */ 
	static void FASTCALL
bmc_make_stat(void) 
{
	
		/* 有効なユニット */ 
		if (bmc_ready[bmc_unit] == BMC_TYPE_NOTREADY) {
		bmc_status |= (BYTE) BMC_ST_NOTREADY;
		bmc_status |= (BYTE) (BMC_ST_ERROR | BMC_ST_CME);
		bmc_errorreg |= (BYTE) BMC_ES_EJECT;
	}
	
	else {
		bmc_status &= (BYTE) (~BMC_ST_NOTREADY);
	}
	 
		/* 一時イジェクト */ 
		if (bmc_teject[bmc_unit]) {
		bmc_status |= (BYTE) BMC_ST_NOTREADY;
		bmc_status |= (BYTE) BMC_ST_ERROR;
		bmc_errorreg |= (BYTE) BMC_ES_EJECT;
	}
	 
		/* ライトプロテクト */ 
		if (bmc_writep[bmc_unit]) {
		bmc_status |= (BYTE) BMC_ST_WRITEP;
	}
	
	else {
		bmc_status &= (BYTE) (~BMC_ST_WRITEP);
	}
}

 
/*
 *	バブルメモリ コントローラ初期化
 */ 
	static void FASTCALL
bmc_initialize(void) 
{
	
		/* 初期化 */ 
		bmc_datareg = (BYTE) 0;
	bmc_status = (BYTE) 0;
	bmc_errorreg = (BYTE) 0;
	bmc_pagereg = 0;
	bmc_countreg = 0;
	bmc_totalcnt = 0;
	bmc_nowcnt = 0;
	 
		/* ステータスを設定する */ 
		bmc_status = (BYTE) BMC_ST_CME;
	 
		/* アクセス(READY) */ 
		bmc_access[bmc_unit] = (BYTE) BMC_ACCESS_READY;
	 bmc_unit = 0;
	bmc_dataptr = NULL;
}  

/*
 *	READ/WRITE サブ
 */ 
	static BOOL FASTCALL
bmc_rw_sub(void) 
{
	bmc_status = (BYTE) 0;
	bmc_errorreg = (BYTE) 0;
	 
		/* ページアドレスチェック */ 
		if ((bmc_unit >= BMC_UNITS_32) || 
				(bmc_pagereg >= (BMC_MAXADDR_32 + 1) * 2)) {
		bmc_status |= (BYTE) (BMC_ST_ERROR | BMC_ST_CME);
		bmc_errorreg |= (BYTE) BMC_ES_PAGEOVER;
		return FALSE;
	}
	 
		/* NOT READYチェック */ 
		if ((bmc_ready[bmc_unit] == BMC_TYPE_NOTREADY) || bmc_teject[bmc_unit]) {
		bmc_make_stat();
		 
			/* ステータスを設定する */ 
			bmc_status &= (BYTE) (~BMC_ST_BUSY);
		bmc_status |= (BYTE) BMC_ST_CME;
		return FALSE;
	}
	 return TRUE;
}

 
/*
 *	ページ読み込み開始
 */ 
	static void FASTCALL
bmc_read_data(void) 
{
	
		/* 基本チェック */ 
		if (!bmc_rw_sub()) {
		return;
	}
	 
		/* カウンタ設定 */ 
		bmc_totalcnt = BMC_PSIZE_32;
	bmc_nowcnt = 0;
	 
		/* ステータス設定 */ 
		bmc_status |= (BYTE) BMC_ST_BUSY;
	bmc_status &= (BYTE) (~BMC_ST_CME);
	 
		/* データポインタ設定 */ 
		bmc_dataptr = bmc_buffer;
	bmc_offset = 0;
	 
		/* アクセス(READ) */ 
		bmc_access[bmc_unit] = (BYTE) BMC_ACCESS_READ;
	 
		/* データバッファ読み込み */ 
		if (!bmc_readbuf()) {
		bmc_status |= (BYTE) (BMC_ST_ERROR | BMC_ST_CME);
		bmc_errorreg |= (BYTE) BMC_ES_NOMAKER;
		return;
	}
	 
		/* 最初のデータを設定 */ 
		bmc_datareg = bmc_dataptr[0];
	 
#ifdef FDDSND
		if (!bmc_wait) {
		bmc_status |= (BYTE) BMC_ST_RDA;
	}
	
#else	/*  */
		bmc_status |= (BYTE) BMC_ST_RDA;
	
#endif	/*  */
}

 
/*
 *	ページ書み込み開始
 */ 
	static void FASTCALL
bmc_write_data(void) 
{
	
		/* 基本チェック */ 
		if (!bmc_rw_sub()) {
		return;
	}
	 
		/* カウンタ設定 */ 
		bmc_totalcnt = BMC_PSIZE_32;
	bmc_nowcnt = 0;
	 
		/* ステータス設定 */ 
		bmc_status |= (BYTE) BMC_ST_BUSY;
	bmc_status &= (BYTE) (~BMC_ST_CME);
	 
		/* WRITE PROTECTチェック */ 
		if (bmc_writep[bmc_unit] != 0) {
		bmc_make_stat();
		bmc_status &= (BYTE) (~BMC_ST_BUSY);
		bmc_status |= (BYTE) BMC_ST_CME;
		return;
	}
	 
		/* データポインタ設定 */ 
		bmc_dataptr = bmc_buffer;
	bmc_offset = 0;
	 
		/* アクセス(WRITE) */ 
		bmc_access[bmc_unit] = (BYTE) BMC_ACCESS_WRITE;
	 
#ifdef FDDSND
		if (!bmc_wait) {
		bmc_status |= (BYTE) BMC_ST_TDRA;
	}
	
#else	/*  */
		bmc_status |= (BYTE) BMC_ST_TDRA;
	
#endif	/*  */
}

 
/*
 *	コマンド処理
 */ 
	static void FASTCALL
bmc_process_cmd(void) 
{
	
		/* データ転送を実行していれば、即時止める */ 
		bmc_dataptr = NULL;
	 
#ifdef FDDSND
		/* ウェイトフラグ設定 */ 
		bmc_wait = fdc_waitmode;
	
#endif	/*  */
		
		/* 分岐 */ 
		switch (bmc_command) {
			
				/* bubble read */ 
		case 0x01:
			bmc_read_data();
			break;
			
				/* bubble write */ 
		case 0x02:
			bmc_write_data();
			break;
			
				/* initialize */ 
		case 0x04:
		case 0x0f:
			bmc_initialize();
			break;
			
				/* それ以外 */ 
		default:
			
				/* ステータスを設定する */ 
				bmc_status = (BYTE) (BMC_ST_ERROR | BMC_ST_CME);
			bmc_errorreg = (BYTE) BMC_ES_UNDEF;
			ASSERT(FALSE);
			break;
	}
}

 
/*
 *	バブルメモリ コントローラ
 *	１バイト読み出し
 */ 
	BOOL FASTCALL bmc_readb(WORD addr, BYTE * dat) 
{
	
		/* 有効・無効チェック */ 
		if (!bmc_enable) {
		return FALSE;
	}
	 
		/* FM-8モード時限定 */ 
		if (fm_subtype != FMSUB_FM8) {
		return FALSE;
	}
	
		/* アドレスチェック */ 
		if ((addr & 0xfff8) != 0xfd10) {
		return FALSE;
	}
	 bmc_use = TRUE;
	 switch (addr) {
			
				/* データレジスタ(BDATA) */ 
		case 0xfd10:
			*dat = bmc_datareg;
			
				/* カウンタ処理 */ 
				if (bmc_dataptr) {
				bmc_nowcnt++;
				if (bmc_nowcnt == bmc_totalcnt) {
					bmc_status &= (BYTE) (~BMC_ST_BUSY);
					bmc_status &= (BYTE) (~BMC_ST_RDA);
					bmc_status |= (BYTE) BMC_ST_CME;
					 bmc_countreg--;
					if ((bmc_countreg > 0) 
							 &&((bmc_pagereg & BMC_MAXADDR_32) < BMC_MAXADDR_32)) {
						
							/* マルチページ処理 */ 
							bmc_status |= (BYTE) BMC_ST_BUSY;
						bmc_status &= (BYTE) (~BMC_ST_CME);
						bmc_pagereg++;
						bmc_read_data();
						bmc_nowcnt = 0;
						return TRUE;
					}
					
						/* シングルページ処理 or マルチページ終了処理 */ 
						bmc_dataptr = NULL;
					 
						/* アクセス(READY) */ 
						bmc_access[bmc_unit] = (BYTE) BMC_ACCESS_READY;
				}
				
				else {
					bmc_datareg = bmc_dataptr[bmc_nowcnt];
					
#ifdef FDDSND
						if (bmc_wait) {
						bmc_status &= (BYTE) (~BMC_ST_RDA);
					}
					
#endif	/*  */
				}
			}
			return TRUE;
			 
				/* ステータスレジスタ(BSTAT) */ 
		case 0xfd12:
			bmc_make_stat();
			*dat = bmc_status;
			
				/* BUSY処理 */ 
				if ((bmc_status & BMC_ST_BUSY) && (bmc_dataptr == NULL)) {
				
					/* BUSYフラグを落とす */ 
					bmc_status &= (BYTE) (~BMC_ST_BUSY);
				bmc_access[bmc_unit] = (BYTE) BMC_ACCESS_READY;
			}
			
				/* CAN READ処理 */ 
				if ((bmc_command == 0x01) && 
						(bmc_dataptr) && !(bmc_status & BMC_ST_RDA)) {
				
					/* CAN READフラグを立てる */ 
					bmc_status |= (BYTE) BMC_ST_RDA;
			}
			
				/* CAN WRITE処理 */ 
				if ((bmc_command == 0x02) && 
						(bmc_dataptr) && !(bmc_status & BMC_ST_TDRA)) {
				
					/* CAN WRITEフラグを立てる */ 
					bmc_status |= (BYTE) BMC_ST_TDRA;
			}
			return TRUE;
			 
				/* エラーステータスレジスタ(BERRST) */ 
		case 0xfd13:
			*dat = bmc_errorreg;
			return TRUE;
	}
	 return FALSE;
}

 
/*
 *	バブルメモリ コントローラ
 *	１バイト書き込み
 */ 
	BOOL FASTCALL bmc_writeb(WORD addr, BYTE dat) 
{
	
		/* 有効・無効チェック */ 
		if (!bmc_enable) {
		return FALSE;
	}
	 
		/* 32KBはFM-8モード時限定 */ 
		if (fm_subtype != FMSUB_FM8) {
		return FALSE;
	}
	
		/* アドレスチェック */ 
		if ((addr & 0xfff8) != 0xfd10) {
		return FALSE;
	}
	 bmc_use = TRUE;
	 switch (addr) {
			
				/* データレジスタ(BDATA) */ 
		case 0xfd10:
			bmc_datareg = dat;
			
				/* カウンタ処理 */ 
				if (bmc_dataptr) {
				bmc_dataptr[bmc_nowcnt] = bmc_datareg;
				bmc_nowcnt++;
				if (bmc_nowcnt == bmc_totalcnt) {
					bmc_status &= (BYTE) (~BMC_ST_BUSY);
					bmc_status &= (BYTE) (~BMC_ST_TDRA);
					bmc_status |= (BYTE) BMC_ST_CME;
					 
						/* ライトページ処理 */ 
						if (!bmc_write_page()) {
						bmc_status |= (BYTE) (BMC_ST_ERROR | BMC_ST_CME);
						bmc_errorreg |= (BYTE) BMC_ES_NOMAKER;
					}
					 bmc_countreg--;
					if ((bmc_countreg > 0) 
							 &&((bmc_pagereg & BMC_MAXADDR_32) < BMC_MAXADDR_32)) {
						
							/* マルチページ処理 */ 
							bmc_status |= (BYTE) BMC_ST_BUSY;
						
#ifdef FDDSND
							if (!bmc_wait) {
							bmc_status |= (BYTE) BMC_ST_TDRA;
						}
						
#else	/*  */
							bmc_status |= (BYTE) BMC_ST_TDRA;
						
#endif	/*  */
							bmc_status &= (BYTE) (~BMC_ST_CME);
						bmc_pagereg++;
						bmc_nowcnt = 0;
						return TRUE;
					}
					
						/* シングルページ処理 or マルチページ終了処理 */ 
						bmc_dataptr = NULL;
					 
						/* アクセス(READY) */ 
						bmc_access[bmc_unit] = (BYTE) BMC_ACCESS_READY;
				}
				
				else {
					
#ifdef FDDSND
						if (bmc_wait) {
						bmc_status &= (BYTE) (~BMC_ST_TDRA);
					}
					
#endif	/*  */
				}
			}
			return TRUE;
			 
				/* コマンドレジスタ(BCMD) */ 
		case 0xfd11:
			bmc_command = (BYTE) (dat & 0x0f);
			bmc_process_cmd();
			return TRUE;
			 
				/* ページアドレスレジスタH(BPGADH) */ 
		case 0xfd14:
			bmc_pagereg &= 0x00ff;
			bmc_pagereg |= (WORD) (dat << 8);
			 
				/* ページからユニットを算出 */ 
				bmc_unit = (BYTE) (bmc_pagereg >> 10);
			return TRUE;
			 
				/* ページアドレスレジスタL(BPGADL) */ 
		case 0xfd15:
			bmc_pagereg &= 0xff00;
			bmc_pagereg |= dat;
			return TRUE;
			 
				/* ページカウントレジスタH(BPGCTH) */ 
		case 0xfd16:
			bmc_countreg &= 0x00ff;
			bmc_countreg |= (WORD) (dat << 8);
			return TRUE;
			 
				/* ページカウントレジスタL(BPGCTL) */ 
		case 0xfd17:
			bmc_countreg &= 0xff00;
			bmc_countreg |= dat;
			return TRUE;
	}
	 return FALSE;
}

 
/*
 *	バブルメモリ コントローラ
 *	セーブ
 */ 
	BOOL FASTCALL bmc_save(int fileh) 
{
	int i, bmc_units;
	DWORD size;
	 for (i = 0; i < 2; i++) {
		bmc_units = BMC_UNITS_32;
		size = (DWORD) BMC_PSIZE_32;
		 
			/* ファイル関係を先に持ってくる */ 
			for (i = 0; i < bmc_units; i++) {
			if (!file_byte_write(fileh, bmc_ready[i])) {
				return FALSE;
			}
		}
		for (i = 0; i < bmc_units; i++) {
			if (!file_write(fileh, (BYTE *) bmc_fname[i], 256 + 1)) {
				return FALSE;
			}
		}
		 for (i = 0; i < bmc_units; i++) {
			if (!file_byte_write(fileh, bmc_media[i])) {
				return FALSE;
			}
		}
		for (i = 0; i < bmc_units; i++) {
			if (!file_bool_write(fileh, bmc_teject[i])) {
				return FALSE;
			}
		}
		 
			/* ファイルステータス */ 
			if (!file_write(fileh, bmc_buffer, size)) {
			return FALSE;
		}
		 
			/* bmc_dataptrは環境に依存するデータポインタ */ 
			if (!bmc_dataptr) {
			if (!file_word_write(fileh, (WORD) size)) {
				return FALSE;
			}
		}
		
		else {
			if (!file_word_write(fileh, (WORD) (bmc_dataptr - &bmc_buffer[0]))) {
				return FALSE;
			}
		}
		 if (!file_dword_write(fileh, bmc_offset)) {
			return FALSE;
		}
		 
			/* I/O */ 
			if (!file_byte_write(fileh, bmc_datareg)) {
			return FALSE;
		}
		if (!file_byte_write(fileh, bmc_command)) {
			return FALSE;
		}
		if (!file_byte_write(fileh, bmc_status)) {
			return FALSE;
		}
		if (!file_byte_write(fileh, bmc_errorreg)) {
			return FALSE;
		}
		if (!file_word_write(fileh, bmc_pagereg)) {
			return FALSE;
		}
		if (!file_word_write(fileh, bmc_countreg)) {
			return FALSE;
		}
		 
			/* その他 */ 
			if (!file_word_write(fileh, bmc_totalcnt)) {
			return FALSE;
		}
		if (!file_word_write(fileh, bmc_nowcnt)) {
			return FALSE;
		}
		for (i = 0; i < bmc_units; i++) {
			if (!file_byte_write(fileh, bmc_access[i])) {
				return FALSE;
			}
		}
		 
			/* 有効・無効チェック */ 
			if (!file_bool_write(fileh, bmc_enable)) {
			return FALSE;
		}
		if (!file_bool_write(fileh, bmc_use)) {
			return FALSE;
		}
	}
	 
#ifdef FDDSND
		if (!file_bool_write(fileh, bmc_wait)) {
		return FALSE;
	}
	
#else	/*  */
		if (!file_bool_write(fileh, FALSE)) {
		return FALSE;
	}
	
#endif	/*  */
		return TRUE;
}

 
/*
 *	バブルメモリ コントローラ
 *	ロード
 */ 
	BOOL FASTCALL bmc_load(int fileh, int ver) 
{
	int i, bmc_units;
	BYTE ready[BMC_UNITS_32];
	char fname[BMC_UNITS_32][256 + 1];
	BYTE media[BMC_UNITS_32];
	WORD offset;
	DWORD size;
	
#ifndef FDDSND
		BOOL tmp;
	
#endif	/*  */
		
		/* バージョンチェック */ 
		if (ver < 200) {
		return FALSE;
	}
	 if (ver < 307) {
		bmc_init();
		bmc_reset();
		return TRUE;
	}
	 bmc_units = BMC_UNITS_32;
	size = (DWORD) BMC_PSIZE_32;
	 
		/* ファイル関係を先に持ってくる */ 
		for (i = 0; i < bmc_units; i++) {
		if (!file_byte_read(fileh, &ready[i])) {
			return FALSE;
		}
	}
	for (i = 0; i < bmc_units; i++) {
		if (!file_read(fileh, (BYTE *) fname[i], 256 + 1)) {
			return FALSE;
		}
	}
	if (ver >= 308) {
		for (i = 0; i < bmc_units; i++) {
			if (!file_byte_read(fileh, &media[i])) {
				return FALSE;
			}
		}
	}
	
	else {
		for (i = 0; i < bmc_units; i++) {
			media[i] = 0;
		}
	}
	 
		/* 再マウントを試みる */ 
		for (i = 0; i < bmc_units; i++) {
		bmc_setfile(i, NULL);
		if (ready[i] != BMC_TYPE_NOTREADY) {
			bmc_setfile(i, fname[i]);
			if (bmc_ready[i] != BMC_TYPE_NOTREADY) {
				if (bmc_medias[i] >= (media[i] + 1)) {
					bmc_setmedia(i, media[i]);
				}
			}
		}
	}
	 for (i = 0; i < bmc_units; i++) {
		if (!file_bool_read(fileh, &bmc_teject[i])) {
			return FALSE;
		}
	}
	 
		/* ファイルステータス */ 
		if (!file_read(fileh, bmc_buffer, size)) {
		return FALSE;
	}
	 
		/* bmc_dataptrは環境に依存するデータポインタ */ 
		if (!file_word_read(fileh, &offset)) {
		return FALSE;
	}
	if (offset >= (WORD) size) {
		bmc_dataptr = NULL;
	}
	
	else {
		bmc_dataptr = &bmc_buffer[offset];
	}
	 if (!file_dword_read(fileh, &bmc_offset)) {
		return FALSE;
	}
	 
		/* I/O */ 
		if (!file_byte_read(fileh, &bmc_datareg)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &bmc_command)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &bmc_status)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &bmc_errorreg)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &bmc_pagereg)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &bmc_countreg)) {
		return FALSE;
	}
	 
		/* その他 */ 
		if (!file_word_read(fileh, &bmc_totalcnt)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &bmc_nowcnt)) {
		return FALSE;
	}
	for (i = 0; i < bmc_units; i++) {
		if (!file_byte_read(fileh, &bmc_access[i])) {
			return FALSE;
		}
	}
	 
		/* ページからユニットを算出 */ 
		bmc_unit = (BYTE) (bmc_pagereg >> 10);
	 
		/* 有効・無効チェック */ 
		if (!file_bool_read(fileh, &bmc_enable)) {
		return FALSE;
	}
	 if (!file_bool_read(fileh, &bmc_use)) {
		return FALSE;
	}
	 
#ifdef FDDSND
		if (!file_bool_read(fileh, &bmc_wait)) {
		return FALSE;
	}
	
#else	/*  */
		if (!file_bool_read(fileh, &tmp)) {
		return FALSE;
	}
	
#endif	/*  */
		return TRUE;
}

 
#endif	/* XM7_VER == 1 && defined(BUBBLE) */
