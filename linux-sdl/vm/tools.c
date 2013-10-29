/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2013 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2013 Ryu Takegami
 *
 *      [ 補助ツール ]
 */


#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "xm7.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "display.h"
#include "device.h"
#include "tools.h"


/*
 *      デジタルパレットテーブル
 *      (RGBQUAD準拠)
 */
static BYTE     bmp_palet_table[] = {
    0x00, 0x00, 0x00, 0x00,
    0xff, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0xff, 0x00, 0xff, 0x00,
    0x00, 0xff, 0x00, 0x00,
    0xff, 0xff, 0x00, 0x00,
    0x00, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00,
};

#if (XM7_VER == 1) && defined(L4CARD)
static BYTE     bmp_palet_table_16[] = {
    0x00, 0x00, 0x00, 0x00,
    0xbb, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xbb, 0x00,
    0xbb, 0x00, 0xbb, 0x00,
    0x00, 0xbb, 0x00, 0x00,
    0xbb, 0xbb, 0x00, 0x00,
    0x00, 0xbb, 0xbb, 0x00,
    0xbb, 0xbb, 0xbb, 0x00,
    0x44, 0x44, 0x44, 0x00,
    0xff, 0x44, 0x44, 0x00,
    0x44, 0x44, 0xff, 0x00,
    0xff, 0x44, 0xff, 0x00,
    0x44, 0xff, 0x44, 0x00,
    0xff, 0xff, 0x44, 0x00,
    0x44, 0xff, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00,
};
#endif

/*
 *      画像縮小用
 */
#define GAMMA200L	1.483239697419133	/* 画像縮小時のγ補正値
						 * (200LINE) */
#define GAMMA400L	1.217883285630907	/* 画像縮小時のγ補正値
						 * (400LINE) */
static BYTE     color_bit_mask[3] = { 1, 4, 2 };

static DWORD    color_add_data[5];


/*
 *      ブランクディスク作成 サブ
 */
static BOOL     FASTCALL
make_d77_sub(SDL_RWops *fileh, DWORD dat)
{
    BYTE            buf[4];

    buf[0] = (BYTE) (dat & 0xff);
    buf[1] = (BYTE) ((dat >> 8) & 0xff);
    buf[2] = (BYTE) ((dat >> 16) & 0xff);
    buf[3] = (BYTE) ((dat >> 24) & 0xff);

    return file_write(fileh, buf, 4);
}

/*
 *      ブランクディスク作成
 */
BOOL            FASTCALL
make_new_d77(char *fname, char *name, BOOL mode2dd)
{
    SDL_RWops       *fileh;
    BYTE            header[0x2b0];
    DWORD           offset;
    int             i;
    int             j;

    /*
     * assert
     */
    ASSERT(fname);
     /* unused */
#if XM7_VER <= 2
       UNUSED(mode2dd);
#endif
    /*
     * ファイルオープン
     */
    fileh = file_open(fname, OPEN_W);
    if (fileh == NULL) {
	return FALSE;
    }

    /*
     * ヘッダ作成
     */
    memset(header, 0, sizeof(header));
    if (name != NULL) {
	for (i = 0; i < 16; i++) {
	    if (name[i] == '\0') {
		break;
	    }
	    header[i] = name[i];
	}
    } else {
	strcpy((char *) header, "Default");
    }

    /*
     * 密度(2D,2DD,2HD)
     */
#if XM7_VER >= 3
    if (mode2dd) {
	header[0x1b] = 0x10;
    }
#endif

    /*
     * ヘッダ書き込み
     */
    if (!file_write(fileh, header, 0x1c)) {
	file_close(fileh);
	return FALSE;
    }

    /*
     * ファイルトータルサイズ
     */
#if XM7_VER >= 3
    if (mode2dd) {
	offset = 0x0e1ab0;
    } else {
	offset = 0x073ab0;
    }
#else
    offset = 0x073ab0;
#endif
    if (!make_d77_sub(fileh, offset)) {
	file_close(fileh);
	return FALSE;
    }

    /*
     * トラックオフセット
     */
    offset = 0x2b0;
    for (i = 0; i < 84; i++) {
	if (!make_d77_sub(fileh, offset)) {
	    file_close(fileh);
	    return FALSE;
	}
	offset += 0x1600;
    }

#if XM7_VER >= 3

    if (mode2dd) {

	for (i = 0; i < 80; i++) {
	    if (!make_d77_sub(fileh, offset)) {
		file_close(fileh);
		return FALSE;
	    }
	    offset += 0x1600;

	}
    } else {
	/*
	 * ヘッダ書き込み
	 */
	if (!file_write(fileh, &header[0x170], 0x2b0 - 0x170)) {
	    file_close(fileh);
	    return FALSE;
	}

    }
#else
    /*
     * ヘッダ書き込み
     */
    if (!file_write(fileh, &header[0x170], 0x2b0 - 0x170)) {
	file_close(fileh);
	return FALSE;
    }
#endif

    /*
     * ヌルデータ書き込み
     */
    memset(header, 0, sizeof(header));
    for (i = 0; i < 84; i++) {

	for (j = 0; j < 11; j++) {
	    if (!file_write(fileh, header, 0x200)) {
		file_close(fileh);
		return FALSE;
	    }
	}
    }

#if XM7_VER >= 3

    if (mode2dd) {
	for (i = 0; i < 80; i++) {
	    for (j = 0; j < 11; j++) {
		if (!file_write(fileh, header, 0x200)) {
		    file_close(fileh);
		    return FALSE;
		}
	    }
	}
    }
#endif
    /*
     * ok
     */
    file_close(fileh);
    return TRUE;
}

/*
 *      ユーザディスク作成
 */
BOOL            FASTCALL
make_new_userdisk(char *fname, char *name, BOOL mode2dd)
{
    static const BYTE dummyipl[9] = {
	0x1a, 0x50,		/* ORCC #$50 */
	0x86, 0x41,		/* LDA #$41 */
	0xb7, 0xfd, 0x03,	/* STA $FD03 */
	0x20, 0xfe		/* BRA * */
    };

    SDL_RWops       *fileh;
    BYTE            header[0x2b0];
    DWORD           offset;
    int             i;
    int             j;
    int             k;

    /*
     * assert
     */
    ASSERT(fname);
       /* unused */
#if XM7_VER <= 2
       UNUSED(mode2dd);
#endif


    /*
     * ファイルオープン
     */
    fileh = file_open(fname, OPEN_W);
    if (fileh == NULL) {
	return FALSE;
    }

    /*
     * ヘッダ作成
     */
    memset(header, 0, sizeof(header));
    if (name != NULL) {
	for (i = 0; i < 16; i++) {
	    if (name[i] == '\0') {
		break;
	    }
	    header[i] = name[i];
	}
    } else {
	strcpy((char *) header, "Default");
    }

    /*
     * 密度(2D,2DD,2HD)
     */
#if XM7_VER >= 3
    if (mode2dd) {
	header[0x1b] = 0x10;
    }
#endif

    /*
     * ヘッダ書き込み
     */
    if (!file_write(fileh, header, 0x1c)) {
	file_close(fileh);
	return FALSE;
    }

    /*
     * ファイルトータルサイズ
     */
#if XM7_VER >= 3
    if (mode2dd) {
	offset = 0x0aa2b0;
    } else {
	offset = 0x0552b0;
    }
#else
    offset = 0x0552b0;
#endif
    if (!make_d77_sub(fileh, offset)) {
	file_close(fileh);
	return FALSE;
    }

    /*
     * トラックオフセット
     */
    offset = 0x2b0;
#if XM7_VER >= 3
    if (mode2dd) {
	k = 160;
    } else {
	k = 80;
    }
#else
    k = 80;
#endif
    for (i = 0; i < 164; i++) {
	if (i >= k) {
	    offset = 0x0000;
	}
	if (!make_d77_sub(fileh, offset)) {
	    file_close(fileh);
	    return FALSE;
	}
	if (offset) {
	    offset += 0x1100;
	}
    }

    /*
     * ヌルデータ書き込み
     */
    memset(header, 0, sizeof(header));
    for (i = 0; i < k; i++) {
	for (j = 1; j <= 16; j++) {
	    memset(header, 0, 0x10);
	    header[0] = (BYTE) (i >> 1);
	    header[1] = (BYTE) (i & 1);
	    header[2] = (BYTE) j;
	    header[3] = 0x01;
	    header[4] = 0x10;
	    header[14] = 0x00;
	    header[15] = 0x01;
	    if (!file_write(fileh, header, 0x10)) {
		file_close(fileh);
		return FALSE;
	    }

	    if ((i == 0) && (j == 1)) {
		/*
		 * ダミーIPLセクタ作成
		 */
		memset(header, 0x00, 0x100);
		memcpy(header, dummyipl, sizeof(dummyipl));
	    } else if ((i == 0) && (j == 3)) {
		/*
		 * IDセクタ作成
		 */
		memset(header, 0x00, 0x100);
#if XM7_VER >= 3
		if (mode2dd) {
		    header[0] = 0x45;
		} else {
		    header[0] = 0x53;
		}
#else
		header[0] = 0x53;
#endif
		header[1] = 0x20;
		header[2] = 0x20;
	    } else if ((i == 2) || (i == 3)) {
		/*
		 * FAT/ディレクトリ作成
		 */
		memset(header, 0xff, 0x100);
		if ((i == 2) && (j == 1)) {
		    header[0] = 0x00;
		}
	    } else {
		/*
		 * 通常セクタ作成
		 */
		memset(header, 0xe5, 0x100);
	    }
	    if (!file_write(fileh, header, 0x100)) {
		file_close(fileh);
		return FALSE;
	    }
	}
    }

    /*
     * ok
     */
    file_close(fileh);
    return TRUE;
}

/*
 *      ブランクテープ作成
 */
BOOL            FASTCALL
make_new_t77(char *fname)
{
    SDL_RWops   *fileh;

    ASSERT(fname);

    /*
     * ファイルオープン
     */
    fileh = file_open(fname, OPEN_W);
    if (fileh == NULL) {
	return FALSE;
    }

    /*
     * ヘッダ書き込み
     */
    if (!file_write(fileh, (BYTE *) "XM7 TAPE IMAGE 0", 16)) {
	file_close(fileh);
	return FALSE;
    }

    /*
     * 成功
     */
    file_close(fileh);
    return TRUE;
}

#if XM7_VER == 1 && defined(BUBBLE)
/*
 *	ブランクバブルカセット作成
 */
BOOL FASTCALL make_new_bubble(char *fname, char *name)
{
	static const char volumelabel[] = "VOL00000";

	SDL_RWops *fileh;
	BYTE buffer[0x40];
	BYTE header[0x20];
	DWORD i;
	/* assert */
	ASSERT(fname);

	/* ファイルオープン */
	fileh = file_open(fname, OPEN_W);
	if (fileh == -1) {
		return FALSE;
	}
   
	/* ヘッダ作成 */
	if (name != NULL) {
		memset(header, 0, sizeof(header));
		for (i=0; i<16; i++) {
			if (name[i] == '\0') {
				break;
			}
			header[i] = name[i];
		}

		/* サイズ(32KB固定) */
		header[0x1b] = 0x80;

		/* ヘッダ書き込み */
		if (!file_write(fileh, header, 0x1c)) {
			file_close(fileh);
			return FALSE;
		}

		/* ファイルトータルサイズ */
		if (!make_d77_sub(fileh, 0x008020)) {
			file_close(fileh);
			return FALSE;
		}
	}


	/* ヌルデータ書き込み */
	for (i=0; i<=0x03ff; i++) {
		memset(buffer, 0, sizeof(buffer));
		if (i == 0) {
			/* IDセクタ作成 */
			memcpy((char *)buffer, volumelabel, strlen(volumelabel));
			buffer[8] = 0x08;
			buffer[9] = 0x00;
		        buffer[10] = 0x00;
			buffer[11] = 0x01;
		}
		if (!file_write(fileh, buffer, 32)) {
			file_close(fileh);
			return FALSE;
		}
	}

	/* ok */
	file_close(fileh);
	return TRUE;
}
#endif	/* XM7_VER == 1 && defined(BUBBLE) */



/*
 *      VFD→D77変換
 */
BOOL FASTCALL conv_vfd_to_d77(char *src, char *dst, char *name)
{
    SDL_RWops       *files;
    SDL_RWops       *filed;
    BYTE            vfd_h[480];
    BYTE            d77_h[0x2b0];
    BYTE           *buffer;
    int             trk;
    int             sec;
    int             secs;
    int             len;
    int             trklen;
    DWORD           offset;
    DWORD           srclen;
    DWORD           wrlen;
    BYTE           *header;
    BYTE           *ptr;

    /*
     * assert
     */
    ASSERT(src);
    ASSERT(dst);
    ASSERT(name);

    /*
     * ワークメモリ確保
     */
    buffer = (BYTE *) malloc(8192);
    if (buffer == NULL) {
	return FALSE;
    }

    /*
     * VFDファイルオープン
     */
    files = file_open(src, OPEN_R);
    if (files == NULL) {
	free(buffer);
	return FALSE;
    }

    /*
     * ここで、ファイルサイズを取得しておく
     */
    srclen = file_getsize(files);

    /*
     * VFDヘッダ読み込み
     */
    if (!file_read(files, vfd_h, sizeof(vfd_h))) {
	free(buffer);
	file_close(files);
	return FALSE;
    }

    /*
     * D77ファイル作成
     */
    filed = file_open(dst, OPEN_W);
    if (filed == NULL) {
	free(buffer);
	file_close(files);
	return FALSE;
    }

    /*
     * ヘッダ作成
     */
    memset(d77_h, 0, sizeof(d77_h));
    if (strlen(name) <= 16) {
	strcpy((char *) d77_h, name);
    } else {
	memcpy(d77_h, name, 16);
    }

    /*
     * 一旦、ヘッダを書く
     */
    if (!file_write(filed, d77_h, sizeof(d77_h))) {
	free(buffer);
	file_close(files);
	file_close(filed);
	return FALSE;
    }

    /*
     * 書き込みポインタを初期化
     */
    wrlen = sizeof(d77_h);

    /*
     * トラックループ
     */
    header = vfd_h;
    for (trk = 0; trk < 80; trk++) {
	/*
	 * ヘッダ取得
	 */
	offset = header[3];
	offset *= 256;
	offset |= header[2];
	offset *= 256;
	offset |= header[1];
	offset *= 256;
	offset |= header[0];
	header += 4;
	len = *header++;
	len &= 7;
	if (len >= 4) {
	    len = 3;
	}
	secs = *header++;

	/*
	 * secs=0への対応
	 */
	if (secs == 0) {
	    continue;
	} else {
	    /*
	     * 書き込みポインタを記入
	     */
	    d77_h[trk * 4 + 0x20 + 3] = (BYTE) (wrlen >> 24);
	    d77_h[trk * 4 + 0x20 + 2] = (BYTE) ((wrlen >> 16) & 255);
	    d77_h[trk * 4 + 0x20 + 1] = (BYTE) ((wrlen >> 8) & 255);
	    d77_h[trk * 4 + 0x20 + 0] = (BYTE) (wrlen & 255);
	}

	/*
	 * トラック長を計算
	 */
	switch (len) {
	case 0:
	    trklen = secs * (128 + 16);
	    break;
	case 1:
	    trklen = secs * (256 + 16);
	    break;
	case 2:
	    trklen = secs * (512 + 16);
	    break;
	case 3:
	    trklen = secs * (1024 + 16);
	    break;
	}

	/*
	 * ヘッダ検査
	 */
	if ((offset > srclen) | (trklen > 8192)) {
	    free(buffer);
	    file_close(files);
	    file_close(filed);
	    return FALSE;
	}

	/*
	 * シーク
	 */
	if (!file_seek(files, offset)) {
	    free(buffer);
	    file_close(files);
	    file_close(filed);
	    return FALSE;
	}

	/*
	 * セクタループ
	 */
	ptr = buffer;
	for (sec = 1; sec <= secs; sec++) {
	    memset(ptr, 0, 0x10);
	    /*
	     * C,H,R,N
	     */
	    ptr[0] = (BYTE) (trk >> 1);
	    ptr[1] = (BYTE) (trk & 1);
	    ptr[2] = (BYTE) sec;
	    ptr[3] = (BYTE) len;
	    /*
	     * セクタ数
	     */
	    ptr[4] = (BYTE) (secs);
	    /*
	     * セクタ長＆データ読み込み
	     */
	    switch (len) {
	    case 0:
		ptr[0x0e] = 0x80;
		ptr += 0x10;
		file_read(files, ptr, 128);
		ptr += 128;
		break;
	    case 1:
		ptr[0x0f] = 0x01;
		ptr += 0x10;
		file_read(files, ptr, 256);
		ptr += 256;
		break;
	    case 2:
		ptr[0x0f] = 0x02;
		ptr += 0x10;
		file_read(files, ptr, 512);
		ptr += 512;
		break;
	    case 3:
		ptr[0x0f] = 0x04;
		ptr += 0x10;
		file_read(files, ptr, 1024);
		ptr += 1024;
		break;
	    }
	}

	/*
	 * 一括書き込み
	 */
	if (!file_write(filed, buffer, trklen)) {
	    free(buffer);
	    file_close(files);
	    file_close(filed);
	    return FALSE;
	}

	/*
	 * 書き込みポインタを進める
	 */
	wrlen += trklen;
    }

    /*
     * ファイルサイズ設定
     */
    d77_h[0x1f] = (BYTE) ((wrlen >> 24) & 0xff);
    d77_h[0x1e] = (BYTE) ((wrlen >> 16) & 0xff);
    d77_h[0x1d] = (BYTE) ((wrlen >> 8) & 0xff);
    d77_h[0x1c] = (BYTE) (wrlen & 0xff);

    /*
     * 再度、ヘッダを書き込んで
     */
    if (!file_seek(filed, 0)) {
	free(buffer);
	file_close(files);
	file_close(filed);
	return FALSE;
    }
    if (!file_write(filed, d77_h, sizeof(d77_h))) {
	free(buffer);
	file_close(files);
	file_close(filed);
	return FALSE;
    }

    /*
     * すべて終了
     */
    free(buffer);
    file_close(files);
    file_close(filed);
    return TRUE;
}

/*
 *      2D/2DD→D77変換
 */
BOOL            FASTCALL
conv_2d_to_d77(char *src, char *dst, char *name)
{
    SDL_RWops      *files;
    SDL_RWops      *filed;
    BYTE            d77_h[0x2b0];
    BYTE           *buffer;
    BYTE           *ptr;
    DWORD           offset;
    int             trk;
    int             sec;
    int             size;
    int             max_track;

    /*
     * assert
     */
    ASSERT(src);
    ASSERT(dst);
    ASSERT(name);

    /*
     * ワークメモリ確保
     */
    buffer = (BYTE *) malloc(0x1100);
    if (buffer == NULL) {
	return FALSE;
    }

    /*
     * 2Dファイルオープン、ファイルサイズチェック
     */
    files = file_open(src, OPEN_R);
    if (files == NULL) {
	free(buffer);
	return FALSE;
    }
    size = file_getsize(files);
#if XM7_VER >= 3
    if ((size != 327680) && (size != 655360)) {
#else
    if (size != 327680) {
	file_close(files);
#endif
	free(buffer);
	return FALSE;
    }

    /*
     * D77ファイル作成
     */
    filed = file_open(dst, OPEN_W);
    if (filed == NULL) {
	free(buffer);
	file_close(files);
	return FALSE;
    }

    /*
     * ヘッダ作成
     */
    memset(d77_h, 0, sizeof(d77_h));
    if (strlen(name) <= 16) {
	strcpy((char *) d77_h, name);
    } else {
	memcpy(d77_h, name, 16);
    }

    /*
     * ファイルサイズ
     */
#if XM7_VER >= 3
    if (size == 655360) {
	d77_h[0x1b] = 0x10;
	d77_h[0x1c] = 0xb0;
	d77_h[0x1d] = 0xa2;
	d77_h[0x1e] = 0x0a;
	max_track = 160;
    } else {
	d77_h[0x1b] = 0x00;
	d77_h[0x1c] = 0xb0;
	d77_h[0x1d] = 0x52;
	d77_h[0x1e] = 0x05;
	max_track = 80;
    }
#else
    d77_h[0x1b] = 0x00;
    d77_h[0x1c] = 0xb0;
    d77_h[0x1d] = 0x52;
    d77_h[0x1e] = 0x05;
    max_track = 80;
#endif

    /*
     * トラックオフセット
     */
    offset = 0x2b0;
    for (trk = 0; trk < max_track; trk++) {
	d77_h[0x20 + trk * 4 + 0] = (BYTE) (offset & 0xff);
	d77_h[0x20 + trk * 4 + 1] = (BYTE) ((offset >> 8) & 0xff);
	d77_h[0x20 + trk * 4 + 2] = (BYTE) ((offset >> 16) & 0xff);
	d77_h[0x20 + trk * 4 + 3] = (BYTE) ((offset >> 24) & 0xff);
	offset += 0x1100;
    }

    /*
     * ヘッダ書き込み
     */
    if (!file_write(filed, d77_h, sizeof(d77_h))) {
	free(buffer);
	file_close(files);
	file_close(filed);
	return FALSE;
    }

    /*
     * トラックループ
     */
    for (trk = 0; trk < max_track; trk++) {
	ptr = buffer;
	/*
	 * セクタループ
	 */
	for (sec = 1; sec <= 16; sec++) {
	    memset(ptr, 0, 0x10);
	    /*
	     * C,H,R,N
	     */
	    ptr[0] = (BYTE) (trk >> 1);
	    ptr[1] = (BYTE) (trk & 1);
	    ptr[2] = (BYTE) sec;
	    ptr[3] = 1;

	    /*
	     * セクタ数、レングス
	     */
	    ptr[4] = 16;
	    ptr[0x0f] = 0x01;
	    ptr += 0x10;

	    /*
	     * データ読み込み
	     */
	    file_read(files, ptr, 256);
	    ptr += 256;
	}

	/*
	 * 一括書き込み
	 */
	if (!file_write(filed, buffer, 0x1100)) {
	    free(buffer);
	    file_close(files);
	    file_close(filed);
	    return FALSE;
	}
    }

    /*
     * すべて終了
     */
    free(buffer);
    file_close(files);
    file_close(filed);
    return TRUE;
}

/*
 *      VTP→T77変換
 *      １バイト出力
 */
static BOOL     FASTCALL
vtp_conv_write(SDL_RWops *handle, BYTE dat)
{
    int             i;
    BYTE            buf[44];

    /*
     * スタートビット設定
     */
    buf[0] = 0x00;
    buf[1] = 0x34;
    buf[2] = 0x80;
    buf[3] = 0x1a;
    buf[4] = 0x00;
    buf[5] = 0x1a;

    /*
     * ストップビット設定
     */
    buf[38] = 0x80;
    buf[39] = 0x2f;
    buf[40] = 0x00;
    buf[41] = 0x37;
    buf[42] = 0x80;
    buf[43] = 0x2f;

    /*
     * 8ビット処理
     */
    for (i = 0; i < 8; i++) {
	if (dat & 0x01) {
	    buf[i * 4 + 6 + 0] = 0x80;
	    buf[i * 4 + 6 + 1] = 0x30;
	    buf[i * 4 + 6 + 2] = 0x00;
	    buf[i * 4 + 6 + 3] = 0x30;
	} else {
	    buf[i * 4 + 6 + 0] = 0x80;
	    buf[i * 4 + 6 + 1] = 0x18;
	    buf[i * 4 + 6 + 2] = 0x00;
	    buf[i * 4 + 6 + 3] = 0x1a;
	}
	dat >>= 1;
    }

    /*
     * 44バイトに拡大して書き込む
     */
    if (!file_write(handle, buf, 44)) {
	return FALSE;
    }

    return TRUE;
}

/*
 *      VTP→T77変換
 */
BOOL            FASTCALL
conv_vtp_to_t77(char *src, char *dst)
{
    SDL_RWops      *files;
    SDL_RWops      *filed;
    int             i;
    BYTE            buf[44];
    BYTE            hdr[4];
    char           *header = "XM7 TAPE IMAGE 0";
    BYTE            dat;
    int             count;

    /*
     * assert
     */
    ASSERT(src);
    ASSERT(dst);

    /*
     * VTPファイルオープン
     */
    files = file_open(src, OPEN_R);
    if (files == NULL) {
	return FALSE;
    }

    /*
     * T77ファイル作成
     */
    filed = file_open(dst, OPEN_W);
    if (filed == NULL) {
	file_close(files);
	return FALSE;
    }

    /*
     * ヘッダ書き込み
     */
    if (!file_write(filed, (BYTE *) header, 16)) {
	file_close(filed);
	file_close(files);
	return FALSE;
    }

    /*
     * T77データ作成
     */
    while (TRUE) {
	/*
	 * カウンタ初期化
	 */
	count = 0;

	/*
	 * ゴミデータスキップ
	 */
	/*
	 * 32個以上の連続した 0xFF
	 * を発見できるまで繰り返す
	 */
	do {
	    /*
	     * 途中でファイルが終わった場合は正常終了
	     */
	    if (!file_read(files, &dat, 1)) {
		file_close(filed);
		file_close(files);
		return TRUE;
	    }

	    /*
	     * 0xFFを発見できたらカウンタ増加、それ以外ならカウンタクリア
	     */
	    if (dat == 0xff) {
		count++;
	    } else {
		count = 0;
	    }
	}
	while (count < 32);

	/*
	 * 1ファイル分のデータを作成する
	 */
	do {
	    /*
	     * ヘッダ検索
	     */
	    /*
	     * 途中でファイルが終わった場合は正常終了
	     */
	    do {
		do {
		    if (!file_read(files, &hdr[0], 1)) {
			file_close(filed);
			file_close(files);
			return TRUE;
		    }
		    count++;
		}
		while (hdr[0] != 0x01);

		if (!file_read(files, &hdr[1], 1)) {
		    file_close(filed);
		    file_close(files);
		    return TRUE;
		}
	    }
	    while ((hdr[0] != 0x01) || (hdr[1] != 0x3c));

	    /*
	     * 残りのヘッダ部を読み込む
	     */
	    for (i = 2; i < 4; i++) {
		if (!file_read(files, &hdr[i], 1)) {
		    file_close(filed);
		    file_close(files);
		    return FALSE;
		}
	    }

	    /*
	     * ファイル情報ブロックだった場合はGapの前にマーカを設定
	     */
	    if (hdr[2] == 0x00) {
		buf[0] = 0;
		buf[1] = 0;
		buf[2] = 0x7f;
		buf[3] = 0xff;
		if (!file_write(filed, buf, 4)) {
		    file_close(filed);
		    file_close(files);
		    return FALSE;
		}
	    }

	    /*
	     * Gap書き込み
	     */
	    /*
	     * データは全て 0xFF に統一する
	     */
	    for (i = 0; i < count; i++) {
		if (!vtp_conv_write(filed, 0xFF)) {
		    file_close(filed);
		    file_close(files);
		    return FALSE;
		}
	    }

	    /*
	     * ヘッダ書き込み
	     */
	    for (i = 0; i < 4; i++) {
		if (!vtp_conv_write(filed, hdr[i])) {
		    file_close(filed);
		    file_close(files);
		    return FALSE;
		}
	    }

	    /*
	     * データ・チェックサム書き込み
	     */
	    for (i = 0; i <= hdr[3]; i++) {
		if (!file_read(files, &dat, 1)) {
		    file_close(filed);
		    file_close(files);
		    return FALSE;
		}
		if (!vtp_conv_write(filed, dat)) {
		    file_close(filed);
		    file_close(files);
		    return FALSE;
		}
	    }

	    /*
	     * カウンタ初期化
	     */
	    count = 0;
	}
	while (hdr[2] != 0xFF);
    }
}

#if XM7_VER == 1 && defined(BUBBLE)
/*
 *	BBL→B77変換
 */
BOOL FASTCALL conv_bbl_to_b77(char *src, char *dst, char *name)
{
	int files;
	int filed;
	BYTE buffer[0x40];
	BYTE b77_h[0x20];
	int size;
	int page;

	/* assert */
	ASSERT(src);
	ASSERT(dst);
	ASSERT(name);

	/* BBLファイルオープン、ファイルサイズチェック */
	files = file_open(src, OPEN_R);
	if (files == -1) {
		return FALSE;
        }
	size = file_getsize(files);
	if (size != 32768) {
		file_close(files);
		return FALSE;
	}

	/* B77ファイル作成 */
	filed = file_open(dst, OPEN_W);
	if (filed == -1) {
		file_close(files);
		return FALSE;
	}

	/* ヘッダ作成 */
	memset(b77_h, 0, sizeof(b77_h));
	if (strlen(name) <= 16) {
		strcpy((char*)b77_h, name);
	}
	else {
		memcpy(b77_h, name, 16);
	}

	/* ファイルサイズ */
	b77_h[0x1b] = 0x80;
	b77_h[0x1c] = 0x20;
	b77_h[0x1d] = 0x80;
	b77_h[0x1e] = 0x00;

	/* ヘッダ書き込み */
	if (!file_write(filed, b77_h, sizeof(b77_h))) {
		file_close(files);
		file_close(filed);
		return FALSE;
	}

	/* ページループ */
	for (page=0; page<0x0400; page++) {

		/* データ読み込み */
		file_read(files, buffer, 0x0020);

		/* データ書き込み */
		if (!file_write(filed, buffer, 0x0020)) {
			free(buffer);
			file_close(files);
			file_close(filed);
			return FALSE;
		}
	}

	/* すべて終了 */
	file_close(files);
	file_close(filed);

	return TRUE;
}
#endif


/*
 *      BMPヘッダ書き込み
 */
static BOOL     FASTCALL
bmp_header_sub(SDL_RWops *fileh)
{
    BYTE            filehdr[14];
    BYTE            infohdr[40];

    ASSERT(fileh >= 0);

    /*
     * 構造体クリア
     */
    memset(filehdr, 0, sizeof(filehdr));
    memset(infohdr, 0, sizeof(infohdr));

    /*
     * BITMAPFILEHEADER
     */
    filehdr[0] = 'B';
    filehdr[1] = 'M';

#if XM7_VER >= 3
    switch (screen_mode) {
    case SCR_262144:		/* 262144色 ファイルサイズ
				 * 14+40+768000 */
	filehdr[2] = 0x36;
	filehdr[3] = 0xb8;
	filehdr[4] = 0x0b;
	break;
    case SCR_4096:		/* 4096色 ファイルサイズ
				 * 14+40+512000 */
	filehdr[2] = 0x36;
	filehdr[3] = 0xd0;
	filehdr[4] = 0x07;
	break;
    default:			/* 640x200/400 8色 ファイルサイズ
				 * 14+40+16*4+128000 */
	filehdr[2] = 0x76;
	filehdr[3] = 0xf4;
	filehdr[4] = 0x01;
    }
#elif XM7_VER >= 2
    if (mode320) {
	/*
	 * 4096色 ファイルサイズ 14+40+512000
	 */
	filehdr[2] = 0x36;
	filehdr[3] = 0xd0;
	filehdr[4] = 0x07;
    } else {
	/*
	 * 640x200 8色 ファイルサイズ 14+40+16*4+128000
	 */
	filehdr[2] = 0x76;
	filehdr[3] = 0xf4;
	filehdr[4] = 0x01;
    }
#else
    /*
     * 640x200 8色 ファイルサイズ 14+40+16*4+128000
     */
    filehdr[2] = 0x76;
    filehdr[3] = 0xf4;
    filehdr[4] = 0x01;
#endif

    /*
     * ビットマップへのオフセット
     */
#if XM7_VER >= 2
#if XM7_VER >= 3
    if (screen_mode & SCR_ANALOG) {
#else
    if (mode320) {
#endif
	/*
	 * 4096色 or 262144色
	 */
	filehdr[10] = 14 + 40;
    } else {
	/*
	 * 8色
	 */
	filehdr[10] = 14 + 40 + (16 * 4);
    }
#else
    /*
     * 8色
     */
    filehdr[10] = 14 + 40 + (16 * 4);
#endif

    /*
     * BITMAPFILEHEADER 書き込み
     */
    if (!file_write(fileh, filehdr, sizeof(filehdr))) {
	return FALSE;
    }

    /*
     * BITMAPINFOHEADER
     */
    infohdr[0] = 40;
    infohdr[4] = 0x80;
    infohdr[5] = 0x02;
    infohdr[8] = 0x90;
    infohdr[9] = 0x01;
    infohdr[12] = 0x01;
    /*
     * BiBitCount
     */
#if XM7_VER >= 3
    switch (screen_mode) {
    case SCR_262144:		/* 262144色 */
	infohdr[14] = 24;
	break;
    case SCR_4096:		/* 4096色 */
	infohdr[14] = 16;
	break;
    default:			/* 640x200/400 8色 */
	infohdr[14] = 4;
	break;
    }
#elif XM7_VER >= 2
    if (mode320) {
	infohdr[14] = 16;
    } else {
	infohdr[14] = 4;
    }
#else
    infohdr[14] = 4;
#endif

    /*
     * BITMAPFILEHEADER 書き込み
     */
    if (!file_write(fileh, infohdr, sizeof(infohdr))) {
	return FALSE;
    }

    return TRUE;
}

/*
 *      8色モード画像縮小用 カラー混合テーブル初期化
 */
void            FASTCALL
mix_color_init(double gamma)
{
    double          maxbrg;
    double          brg;
    int             i;

    maxbrg = pow(255., 1 / gamma);
    for (i = 0; i <= 4; i++) {
	brg = pow((double) (i << 6), 1 / gamma);
	color_add_data[i] = ((DWORD) ((brg / maxbrg) * 31.) << 15);
    }
}

/*
 *      8色モード画像縮小用 カラー混合処理
 */
WORD            FASTCALL
mix_color(BYTE * palet_table, BYTE palet_count)
{
    DWORD           col;
    BYTE            colcount;
    int             i;
    int             j;

    col = 0;
    for (i = 0; i < 3; i++) {
	colcount = 0;
	for (j = 0; j < palet_count; j++) {
	    if (palet_table[j] & color_bit_mask[i]) {
		colcount += (BYTE) (4 / palet_count);
	    }
	}
	col = (col | color_add_data[colcount]) >> 5;
    }

    return (WORD) (col & 0x7fff);
}

/*
 *      16色モード画像縮小用 カラー混合処理
 */
#if XM7_VER == 1
WORD            FASTCALL
mix_color_16(double gamma, BYTE * palet_table, BYTE palet_count)
{
    DWORD           col;
    DWORD           colcount;
    DWORD           brg;
    double          maxbrg;
    int             i;
    int             j;

    maxbrg = pow(255., 1 / gamma);
    col = 0;

    for (i = 0; i < 3; i++) {
	colcount = 0;
	for (j = 0; j < palet_count; j++) {
	    if (palet_table[j] & 8) {
		colcount += 0x44;
	    }
	    if (palet_table[j] & color_bit_mask[i]) {
		colcount += 0xbb;
	    }
	}
	colcount /= palet_count;
	brg =
	    (int) ((pow((double) colcount, 1 / gamma) / maxbrg) *
		   31.) << 15;
	col = (col | brg) >> 5;
    }

    return (WORD) (col & 0x7fff);
}
#endif

/*
 *      BMPヘッダ書き込み (縮小画像用)
 */
static BOOL     FASTCALL
bmp_header_sub2(SDL_RWops *fileh)
{
    BYTE            filehdr[14];
    BYTE            infohdr[40];

    ASSERT(fileh >= 0);

    /*
     * 構造体クリア
     */
    memset(filehdr, 0, sizeof(filehdr));
    memset(infohdr, 0, sizeof(infohdr));

    /*
     * BITMAPFILEHEADER
     */
    filehdr[0] = 'B';
    filehdr[1] = 'M';

    /*
     * BiBitCount
     */
#if XM7_VER >= 3
    if (screen_mode == SCR_262144) {
	/*
	 * ファイルサイズ 14+40+192000
	 */
	/*
	 * 262144色
	 */
	filehdr[2] = 0x36;
	filehdr[3] = 0xee;
	filehdr[4] = 0x02;
    } else {
	/*
	 * ファイルサイズ 14+40+128000
	 */
	/*
	 * 8色/4096色
	 */
	filehdr[2] = 0x36;
	filehdr[3] = 0xf4;
	filehdr[4] = 0x01;
    }
#else
    /*
     * ファイルサイズ 14+40+128000
     */
    /*
     * 8色/4096色
     */
    filehdr[2] = 0x36;
    filehdr[3] = 0xf4;
    filehdr[4] = 0x01;
#endif

    /*
     * ビットマップへのオフセット
     */
    filehdr[10] = 14 + 40;

    /*
     * BITMAPFILEHEADER 書き込み
     */
    if (!file_write(fileh, filehdr, sizeof(filehdr))) {
	return FALSE;
    }

    /*
     * BITMAPINFOHEADER
     */
    infohdr[0] = 40;
    infohdr[4] = 0x40;
    infohdr[5] = 0x01;
    infohdr[8] = 0xC8;
    infohdr[9] = 0x00;
    infohdr[12] = 0x01;

    /*
     * BiBitCount
     */
#if XM7_VER >= 3
    if (screen_mode == SCR_262144) {
	/*
	 * 262144色
	 */
	infohdr[14] = 24;
    } else {
	/*
	 * 8色/4096色
	 */
	infohdr[14] = 16;
    }
#else
    /*
     * 8色/4096色
     */
    infohdr[14] = 16;
#endif

    /*
     * BITMAPFILEHEADER 書き込み
     */
    if (!file_write(fileh, infohdr, sizeof(infohdr))) {
	return FALSE;
    }

    return TRUE;
}

/*
 *      BMPパレット書き込み(8色/16色のみ)
 */
static BOOL     FASTCALL
bmp_palette_sub(SDL_RWops *fileh)
{
    int             i;
    BYTE           *p;
    int             vpage;

    ASSERT(fileh >= 0);

    /*
     * 表示ページを考慮
     */
    vpage = (~(multi_page >> 4)) & 0x07;

#if XM7_VER == 1 && defined(L4CARD)
    if (enable_400line) {
	if (crt_flag) {
	    /*
	     * 固定パレット16色
	     */
	    if (!file_write(fileh, bmp_palet_table_16, 4 * 16)) {
		return FALSE;
	    }
	} else {
	    /*
	     * 黒から16色
	     */
	    p = bmp_palet_table_16;
	    for (i = 0; i < 16; i++) {
		if (!file_write(fileh, p, 4)) {
		    return FALSE;
		}
	    }
	}

	return TRUE;
    }
#endif

    /*
     * パレットより8色
     */
    for (i = 0; i < 8; i++) {
	if (crt_flag) {
	    p = &bmp_palet_table[(ttl_palet[i & vpage] & 0x07) * 4];
	} else {
	    p = bmp_palet_table;
	}
	if (!file_write(fileh, p, 4)) {
	    return FALSE;
	}
    }

    /*
     * 黒から8色
     */
    p = bmp_palet_table;
    for (i = 0; i < 8; i++) {
	if (!file_write(fileh, p, 4)) {
	    return FALSE;
	}
    }

    return TRUE;
}

/*
 *  BMPデータ書き込み(320モード)
 */
#if XM7_VER >= 2
static BOOL     FASTCALL
bmp_320_sub(SDL_RWops *fileh, BOOL fullscan)
{
    int             x,
                    y;
    int             offset;
    int             i;
    BYTE            buffer[2][1280];
    int             dat;
    BYTE            bit;
    WORD            color;
    int             mask;
    BYTE           *vramptr;
#if XM7_VER >= 3
    WORD            dx1,
                    dx2;
    BOOL            winy;
#endif

    ASSERT(fileh >= 0);

    /*
     * 初期オフセット設定
     */
    offset = 40 * 199;

#if XM7_VER >= 3
    /*
     * ウィンドウ領域のクリッピングを行う
     */
    window_clip(1);
    dx1 = (WORD) (window_dx1 >> 3);
    dx2 = (WORD) (window_dx2 >> 3);
#endif

    /*
     * マスク取得
     */
    mask = 0;
    if (!(multi_page & 0x10)) {
	mask |= 0x000f;
    }
    if (!(multi_page & 0x20)) {
	mask |= 0x00f0;
    }
    if (!(multi_page & 0x40)) {
	mask |= 0x0f00;
    }

    /*
     * 0で書き込み
     */
    memset(buffer[0], 0, sizeof(buffer[0]));

    /*
     * yループ
     */
    for (y = 0; y < 200; y++) {
#if XM7_VER >= 3
	winy = (((199 - y) >= window_dy1) && ((199 - y) <= window_dy2));

	/*
	 * xループ
	 */
	for (x = 0; x < 40; x++) {
	    bit = 0x80;
	    if (winy && (x >= dx1) && (x < dx2)) {
		/*
		 * ウィンドウ内(裏ブロック)
		 */
		vramptr = vram_bdptr;
	    } else {
		/*
		 * ウィンドウ外(表ブロック)
		 */
		vramptr = vram_dptr;
	    }
#else
	/*
	 * xループ
	 */
	for (x = 0; x < 40; x++) {
	    bit = 0x80;
	    vramptr = vram_dptr;
#endif

	    /*
	     * ビットループ
	     */
	    for (i = 0; i < 8; i++) {
		dat = 0;

#if XM7_VER >= 3
		/*
		 * G評価
		 */
		if (vramptr[offset + 0x10000] & bit) {
		    dat |= 0x800;
		}
		if (vramptr[offset + 0x12000] & bit) {
		    dat |= 0x400;
		}
		if (vramptr[offset + 0x14000] & bit) {
		    dat |= 0x200;
		}
		if (vramptr[offset + 0x16000] & bit) {
		    dat |= 0x100;
		}

		/*
		 * R評価
		 */
		if (vramptr[offset + 0x08000] & bit) {
		    dat |= 0x080;
		}
		if (vramptr[offset + 0x0a000] & bit) {
		    dat |= 0x040;
		}
		if (vramptr[offset + 0x0c000] & bit) {
		    dat |= 0x020;
		}
		if (vramptr[offset + 0x0e000] & bit) {
		    dat |= 0x010;
		}

		/*
		 * B評価
		 */
		if (vramptr[offset + 0x00000] & bit) {
		    dat |= 0x008;
		}
		if (vramptr[offset + 0x02000] & bit) {
		    dat |= 0x004;
		}
		if (vramptr[offset + 0x04000] & bit) {
		    dat |= 0x002;
		}
		if (vramptr[offset + 0x06000] & bit) {
		    dat |= 0x001;
		}
#else
		/*
		 * G評価
		 */
		if (vramptr[offset + 0x08000] & bit) {
		    dat |= 0x800;
		}
		if (vramptr[offset + 0x0a000] & bit) {
		    dat |= 0x400;
		}
		if (vramptr[offset + 0x14000] & bit) {
		    dat |= 0x200;
		}
		if (vramptr[offset + 0x16000] & bit) {
		    dat |= 0x100;
		}

		/*
		 * R評価
		 */
		if (vramptr[offset + 0x04000] & bit) {
		    dat |= 0x080;
		}
		if (vramptr[offset + 0x06000] & bit) {
		    dat |= 0x040;
		}
		if (vramptr[offset + 0x10000] & bit) {
		    dat |= 0x020;
		}
		if (vramptr[offset + 0x12000] & bit) {
		    dat |= 0x010;
		}

		/*
		 * B評価
		 */
		if (vramptr[offset + 0x00000] & bit) {
		    dat |= 0x008;
		}
		if (vramptr[offset + 0x02000] & bit) {
		    dat |= 0x004;
		}
		if (vramptr[offset + 0x0c000] & bit) {
		    dat |= 0x002;
		}
		if (vramptr[offset + 0x0e000] & bit) {
		    dat |= 0x001;
		}
#endif

		/*
		 * アナログパレットよりデータ取得
		 */
		dat &= mask;
		color = apalet_r[dat];
		color <<= 1;
		if (apalet_r[dat] > 0) {
		    color |= 1;
		}
		color <<= 4;

		color |= apalet_g[dat];
		color <<= 1;
		if (apalet_g[dat] > 0) {
		    color |= 1;
		}
		color <<= 4;

		color |= apalet_b[dat];
		color <<= 1;
		if (apalet_b[dat] > 0) {
		    color |= 1;
		}

		/*
		 * CRTフラグ
		 */
		if (!crt_flag) {
		    color = 0;
		}

		/*
		 * ２回続けて同じものを書き込む
		 */
		buffer[1][x * 32 + i * 4 + 0] = (BYTE) (color & 255);
		buffer[1][x * 32 + i * 4 + 1] = (BYTE) (color >> 8);
		buffer[1][x * 32 + i * 4 + 2] = (BYTE) (color & 255);
		buffer[1][x * 32 + i * 4 + 3] = (BYTE) (color >> 8);

		/*
		 * 次のビットへ
		 */
		bit >>= 1;
	    }
	    offset++;
	}

	/*
	 * フルスキャン補間
	 */
	if (fullscan) {
	    memcpy(buffer[0], buffer[1], sizeof(buffer[1]));
	}

	/*
	 * 書き込み
	 */
	if (!file_write(fileh, (BYTE *) buffer, sizeof(buffer))) {
	    return FALSE;
	}

	/*
	 * 次のyへ(戻る)
	 */
	offset -= (40 * 2);
    }

    return TRUE;
}
#endif

#if XM7_VER >= 3
/*
 *  BMPデータ書き込み(26万色モード)
 */
static BOOL     FASTCALL
bmp_256k_sub(SDL_RWops *fileh, BOOL fullscan)
{
    int             x,
                    y;
    int             offset;
    int             i;
    BYTE            buffer[2][1920];
    BYTE            bit;
    BYTE            r,
                    g,
                    b;

    ASSERT(fileh >= 0);

    /*
     * 初期オフセット設定
     */
    offset = 40 * 199;

    /*
     * ０で書き込み
     */
    memset(buffer[0], 0, sizeof(buffer[0]));

    /*
     * yループ
     */
    for (y = 0; y < 200; y++) {

	/*
	 * xループ
	 */
	for (x = 0; x < 40; x++) {
	    bit = 0x80;
	    /*
	     * ビットループ
	     */
	    for (i = 0; i < 8; i++) {
		r = g = b = 0;

		if (!(multi_page & 0x40)) {
		    /*
		     * G評価
		     */
		    if (vram_c[offset + 0x10000] & bit) {
			g |= 0x20;
		    }
		    if (vram_c[offset + 0x12000] & bit) {
			g |= 0x10;
		    }
		    if (vram_c[offset + 0x14000] & bit) {
			g |= 0x08;
		    }
		    if (vram_c[offset + 0x16000] & bit) {
			g |= 0x04;
		    }
		    if (vram_c[offset + 0x28000] & bit) {
			g |= 0x02;
		    }
		    if (vram_c[offset + 0x2a000] & bit) {
			g |= 0x01;
		    }
		}

		if (!(multi_page & 0x20)) {
		    /*
		     * R評価
		     */
		    if (vram_c[offset + 0x08000] & bit) {
			r |= 0x20;
		    }
		    if (vram_c[offset + 0x0a000] & bit) {
			r |= 0x10;
		    }
		    if (vram_c[offset + 0x0c000] & bit) {
			r |= 0x08;
		    }
		    if (vram_c[offset + 0x0e000] & bit) {
			r |= 0x04;
		    }
		    if (vram_c[offset + 0x20000] & bit) {
			r |= 0x02;
		    }
		    if (vram_c[offset + 0x22000] & bit) {
			r |= 0x01;
		    }
		}

		if (!(multi_page & 0x10)) {
		    /*
		     * B評価
		     */
		    if (vram_c[offset + 0x00000] & bit) {
			b |= 0x20;
		    }
		    if (vram_c[offset + 0x02000] & bit) {
			b |= 0x10;
		    }
		    if (vram_c[offset + 0x04000] & bit) {
			b |= 0x08;
		    }
		    if (vram_c[offset + 0x06000] & bit) {
			b |= 0x04;
		    }
		    if (vram_c[offset + 0x18000] & bit) {
			b |= 0x02;
		    }
		    if (vram_c[offset + 0x1a000] & bit) {
			b |= 0x01;
		    }
		}

		/*
		 * CRTフラグ
		 */
		if (!crt_flag) {
		    r = g = b = 0;
		}

		/*
		 * ２回続けて同じものを書き込む
		 */
		buffer[1][x * 48 + i * 6 + 0] =
		    (BYTE) truecolorbrightness[b];
		buffer[1][x * 48 + i * 6 + 1] =
		    (BYTE) truecolorbrightness[g];
		buffer[1][x * 48 + i * 6 + 2] =
		    (BYTE) truecolorbrightness[r];
		buffer[1][x * 48 + i * 6 + 3] =
		    (BYTE) truecolorbrightness[b];
		buffer[1][x * 48 + i * 6 + 4] =
		    (BYTE) truecolorbrightness[g];
		buffer[1][x * 48 + i * 6 + 5] =
		    (BYTE) truecolorbrightness[r];

		/*
		 * 次のビットへ
		 */
		bit >>= 1;
	    }
	    offset++;
	}

	/*
	 * フルスキャン補間
	 */
	if (fullscan) {
	    memcpy(buffer[0], buffer[1], sizeof(buffer[1]));
	}

	/*
	 * 書き込み
	 */
	if (!file_write(fileh, (BYTE *) buffer, sizeof(buffer))) {
	    return FALSE;
	}

	/*
	 * 次のyへ(戻る)
	 */
	offset -= (40 * 2);
    }

    return TRUE;
}
#endif

/*
 *  BMPデータ書き込み(640モード)
 */
static BOOL     FASTCALL
bmp_640_sub(SDL_RWops *fileh, BOOL fullscan)
{
    int             x,
                    y;
    int             i;
    int             offset;
    BYTE            bit;
    BYTE            buffer[2][320];
    BYTE           *vramptr;
#if XM7_VER >= 3
    WORD            dx1,
                    dx2;
    BOOL            winy;
#endif

    ASSERT(fileh >= 0);

    /*
     * 初期オフセット設定
     */
    offset = 80 * 199;

#if XM7_VER >= 3
    /*
     * ウィンドウ領域のクリッピングを行う
     */
    window_clip(0);
    dx1 = (WORD) (window_dx1 >> 3);
    dx2 = (WORD) (window_dx2 >> 3);

    /*
     * カラー9で書き込み
     */
    memset(buffer[0], 0x99, sizeof(buffer[0]));

    /*
     * yループ
     */
    for (y = 0; y < 200; y++) {
	winy = (((199 - y) >= window_dy1) && ((199 - y) <= window_dy2));

	/*
	 * 一旦クリア
	 */
	memset(buffer[1], 0, sizeof(buffer[1]));

	/*
	 * xループ
	 */
	for (x = 0; x < 80; x++) {
	    bit = 0x80;
	    if (winy && (x >= dx1) && (x < dx2)) {
		/*
		 * ウィンドウ内(裏ブロック)
		 */
		vramptr = vram_bdptr;
	    } else {
		/*
		 * ウィンドウ外(表ブロック)
		 */
		vramptr = vram_dptr;
	    }
#else
    /*
     * カラー9で書き込み
     */
    memset(buffer[0], 0x99, sizeof(buffer[0]));

    /*
     * yループ
     */
    for (y = 0; y < 200; y++) {

	/*
	 * 一旦クリア
	 */
	memset(buffer[1], 0, sizeof(buffer[1]));

	/*
	 * xループ
	 */
	for (x = 0; x < 80; x++) {
	    bit = 0x80;
#if XM7_VER >= 2
	    vramptr = vram_dptr;
#else
	    vramptr = vram_c;
#endif
#endif

	    /*
	     * bitループ
	     */
	    for (i = 0; i < 4; i++) {
#if XM7_VER >= 3
		if (vramptr[offset + 0x00000] & bit) {
		    buffer[1][x * 4 + i] |= 0x10;
		}
		if (vramptr[offset + 0x08000] & bit) {
		    buffer[1][x * 4 + i] |= 0x20;
		}
		if (vramptr[offset + 0x10000] & bit) {
		    buffer[1][x * 4 + i] |= 0x40;
		}
		bit >>= 1;

		if (vramptr[offset + 0x00000] & bit) {
		    buffer[1][x * 4 + i] |= 0x01;
		}
		if (vramptr[offset + 0x08000] & bit) {
		    buffer[1][x * 4 + i] |= 0x02;
		}
		if (vramptr[offset + 0x10000] & bit) {
		    buffer[1][x * 4 + i] |= 0x04;
		}
		bit >>= 1;
#else
		if (vramptr[offset + 0x0000] & bit) {
		    buffer[1][x * 4 + i] |= 0x10;
		}
		if (vramptr[offset + 0x4000] & bit) {
		    buffer[1][x * 4 + i] |= 0x20;
		}
		if (vramptr[offset + 0x8000] & bit) {
		    buffer[1][x * 4 + i] |= 0x40;
		}
		bit >>= 1;

		if (vramptr[offset + 0x0000] & bit) {
		    buffer[1][x * 4 + i] |= 0x01;
		}
		if (vramptr[offset + 0x4000] & bit) {
		    buffer[1][x * 4 + i] |= 0x02;
		}
		if (vramptr[offset + 0x8000] & bit) {
		    buffer[1][x * 4 + i] |= 0x04;
		}
		bit >>= 1;
#endif
	    }
	    offset++;
	}

	/*
	 * フルスキャン補間
	 */
	if (fullscan) {
	    memcpy(buffer[0], buffer[1], sizeof(buffer[1]));
	}

	/*
	 * 書き込み
	 */
	if (!file_write(fileh, (BYTE *) buffer, sizeof(buffer))) {
	    return FALSE;
	}

	/*
	 * 次のyへ(戻る)
	 */
	offset -= (80 * 2);
    }

    return TRUE;
}

#if XM7_VER >= 3
/*
 *  BMPデータ書き込み(400ラインモード)
 */
static BOOL     FASTCALL
bmp_400l_sub(SDL_RWops *fileh)
{
    int             x,
                    y;
    int             i;
    int             offset;
    BYTE            bit;
    BYTE            buffer[320];
    BYTE           *vramptr;
    WORD            dx1,
                    dx2;
    BOOL            winy;

    ASSERT(fileh >= 0);

    /*
     * 初期オフセット設定
     */
    offset = 80 * 399;

    /*
     * ウィンドウ領域のクリッピングを行う
     */
    window_clip(2);
    dx1 = (WORD) (window_dx1 >> 3);
    dx2 = (WORD) (window_dx2 >> 3);

    /*
     * yループ
     */
    for (y = 0; y < 400; y++) {
	winy = (((399 - y) >= window_dy1) && ((399 - y) <= window_dy2));

	/*
	 * 一旦クリア
	 */
	memset(buffer, 0, sizeof(buffer));

	/*
	 * xループ
	 */
	for (x = 0; x < 80; x++) {
	    bit = 0x80;
	    if (winy && (x >= dx1) && (x < dx2)) {
		/*
		 * ウィンドウ内(裏ブロック)
		 */
		vramptr = vram_bdptr;
	    } else {
		/*
		 * ウィンドウ外(表ブロック)
		 */
		vramptr = vram_dptr;
	    }

	    /*
	     * bitループ
	     */
	    for (i = 0; i < 4; i++) {
		if (vramptr[offset + 0x00000] & bit) {
		    buffer[x * 4 + i] |= 0x10;
		}
		if (vramptr[offset + 0x08000] & bit) {
		    buffer[x * 4 + i] |= 0x20;
		}
		if (vramptr[offset + 0x10000] & bit) {
		    buffer[x * 4 + i] |= 0x40;
		}
		bit >>= 1;

		if (vramptr[offset + 0x00000] & bit) {
		    buffer[x * 4 + i] |= 0x01;
		}
		if (vramptr[offset + 0x08000] & bit) {
		    buffer[x * 4 + i] |= 0x02;
		}
		if (vramptr[offset + 0x10000] & bit) {
		    buffer[x * 4 + i] |= 0x04;
		}
		bit >>= 1;
	    }
	    offset++;
	}

	/*
	 * 書き込み
	 */
	if (!file_write(fileh, buffer, sizeof(buffer))) {
	    return FALSE;
	}

	/*
	 * 次のyへ(戻る)
	 */
	offset -= (80 * 2);
    }

    return TRUE;
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *  BMPデータ書き込み(L4 400ラインモード)
 */
static BOOL     FASTCALL
bmp_400l4_sub(int fileh)
{
    int             x,
                    y;
    int             i;
    int             offset;
    BYTE            bit,
                    bit2;
    BYTE            buffer[320];
    WORD            taddr,
                    gaddr,
                    textbase;
    BYTE            csr_st,
                    csr_ed,
                    csr_type;
    BYTE            raster,
                    lines;
    BYTE            col,
                    chr,
                    atr,
                    chr_dat;
    BOOL            enable_page;

    ASSERT(fileh >= 0);

    /*
     * 初期オフセット設定
     */
    offset = 80 * 399;

    /*
     * テキスト展開 初期設定(全体)
     */
    csr_st = (BYTE) (crtc_register[10] & 0x1f);
    csr_ed = (BYTE) (crtc_register[11] & 0x1f);
    csr_type = (BYTE) ((crtc_register[10] & 0x60) >> 5);
    lines = (BYTE) ((crtc_register[9] & 0x1f) + 1);

    /*
     * yループ
     */
    for (y = 0; y < 400; y++) {
	/*
	 * 一旦クリア
	 */
	memset(buffer, 0, sizeof(buffer));

	/*
	 * テキスト展開 初期設定(ラスタ単位)
	 */
	textbase = (WORD) text_start_addr;
	textbase += (WORD) (((399 - y) / lines) * (crtc_register[1] << 2));
	textbase &= 0xffe;
	raster = (BYTE) ((399 - y) % lines);

	/*
	 * xループ
	 */
	for (x = 0; x < 80; x++) {
	    bit = 0x80;
	    if (!width40_flag || !(x & 1)) {
		bit2 = 0x80;

		/*
		 * キャラクタコード・アトリビュートを取得
		 */
		if (width40_flag) {
		    taddr = (WORD) ((textbase + (x & ~1)) & 0xffe);
		} else {
		    taddr = (WORD) ((textbase + (x << 1)) & 0xffe);
		}
		chr = tvram_c[taddr + 0];
		atr = tvram_c[taddr + 1];

		/*
		 * アトリビュートから描画色を設定
		 */
		col = (BYTE) ((atr & 0x07) | ((atr & 0x20) >> 2));

		/*
		 * フォントデータ取得(アトリビュート/カーソル処理を含む)
		 */
		if ((!(atr & 0x10) || text_blink) && (raster < 16)) {
		    chr_dat = subcg_l4[(WORD) (chr << 4) + raster];
		} else {
		    chr_dat = 0x00;
		}
		if (atr & 0x08) {
		    chr_dat ^= (BYTE) 0xff;
		}
		if (csr_type != 1) {
		    if (((taddr == cursor_addr) &&
			 (cursor_blink || !csr_type)) &&
			((raster >= csr_st) && (raster <= csr_ed))) {
			chr_dat ^= (BYTE) 0xff;
		    }
		}
	    }

	    /*
	     * GVRAM 実アドレスを取得
	     */
	    gaddr = (WORD) ((offset + vram_offset[0]) & 0x7fff);
	    enable_page = FALSE;
	    if (gaddr >= 0x4000) {
		if (!(multi_page & 0x10)) {
		    enable_page = TRUE;
		}
	    } else {
		if (!(multi_page & 0x20)) {
		    enable_page = TRUE;
		}
	    }

	    /*
	     * bitループ
	     */
	    for (i = 0; i < 4; i++) {
		if (chr_dat & bit2) {
		    buffer[x * 4 + i] |= (BYTE) (col << 4);
		} else if ((vram_c[gaddr] & bit) && enable_page) {
		    buffer[x * 4 + i] |= (BYTE) (ttl_palet[1] << 4);
		} else {
		    buffer[x * 4 + i] |= (BYTE) (ttl_palet[0] << 4);
		}
		bit >>= 1;
		if (!width40_flag) {
		    bit2 >>= 1;
		}

		if (chr_dat & bit2) {
		    buffer[x * 4 + i] |= (BYTE) col;
		} else if ((vram_c[gaddr] & bit) && enable_page) {
		    buffer[x * 4 + i] |= (BYTE) ttl_palet[1];
		} else {
		    buffer[x * 4 + i] |= (BYTE) ttl_palet[0];
		}
		bit >>= 1;
		bit2 >>= 1;
	    }
	    offset++;
	}

	/*
	 * 書き込み
	 */
	if (!file_write(fileh, buffer, sizeof(buffer))) {
	    return FALSE;
	}

	/*
	 * 次のyへ(戻る)
	 */
	offset -= (80 * 2);
    }

    return TRUE;
}
#endif

/*
 *  BMPデータ書き込み(320モード・縮小画像)
 */
#if XM7_VER >= 2
static BOOL     FASTCALL
bmp_320_sub2(SDL_RWops *fileh)
{
    int             x,
                    y;
    int             offset;
    int             i;
    BYTE            buffer[640];
    int             dat;
    BYTE            bit;
    WORD            color;
    int             mask;
    BYTE           *vramptr;
#if XM7_VER >= 3
    WORD            dx1,
                    dx2;
    BOOL            winy;
#endif

    ASSERT(fileh >= 0);

    /*
     * 初期オフセット設定
     */
    offset = 40 * 199;

#if XM7_VER >= 3
    /*
     * ウィンドウ領域のクリッピングを行う
     */
    window_clip(1);
    dx1 = (WORD) (window_dx1 >> 3);
    dx2 = (WORD) (window_dx2 >> 3);
#endif

    /*
     * マスク取得
     */
    mask = 0;
    if (!(multi_page & 0x10)) {
	mask |= 0x000f;
    }
    if (!(multi_page & 0x20)) {
	mask |= 0x00f0;
    }
    if (!(multi_page & 0x40)) {
	mask |= 0x0f00;
    }

    /*
     * yループ
     */
    for (y = 0; y < 200; y++) {
#if XM7_VER >= 3
	winy = (((199 - y) >= window_dy1) && ((199 - y) <= window_dy2));

	/*
	 * xループ
	 */
	for (x = 0; x < 40; x++) {
	    bit = 0x80;
	    if (winy && (x >= dx1) && (x < dx2)) {
		/*
		 * ウィンドウ内(裏ブロック)
		 */
		vramptr = vram_bdptr;
	    } else {
		/*
		 * ウィンドウ外(表ブロック)
		 */
		vramptr = vram_dptr;
	    }
#else
	/*
	 * xループ
	 */
	for (x = 0; x < 40; x++) {
	    bit = 0x80;
	    vramptr = vram_dptr;
#endif

	    /*
	     * ビットループ
	     */
	    for (i = 0; i < 8; i++) {
		dat = 0;

#if XM7_VER >= 3
		/*
		 * G評価
		 */
		if (vramptr[offset + 0x10000] & bit) {
		    dat |= 0x800;
		}
		if (vramptr[offset + 0x12000] & bit) {
		    dat |= 0x400;
		}
		if (vramptr[offset + 0x14000] & bit) {
		    dat |= 0x200;
		}
		if (vramptr[offset + 0x16000] & bit) {
		    dat |= 0x100;
		}

		/*
		 * R評価
		 */
		if (vramptr[offset + 0x08000] & bit) {
		    dat |= 0x080;
		}
		if (vramptr[offset + 0x0a000] & bit) {
		    dat |= 0x040;
		}
		if (vramptr[offset + 0x0c000] & bit) {
		    dat |= 0x020;
		}
		if (vramptr[offset + 0x0e000] & bit) {
		    dat |= 0x010;
		}

		/*
		 * B評価
		 */
		if (vramptr[offset + 0x00000] & bit) {
		    dat |= 0x008;
		}
		if (vramptr[offset + 0x02000] & bit) {
		    dat |= 0x004;
		}
		if (vramptr[offset + 0x04000] & bit) {
		    dat |= 0x002;
		}
		if (vramptr[offset + 0x06000] & bit) {
		    dat |= 0x001;
		}
#else
		/*
		 * G評価
		 */
		if (vramptr[offset + 0x08000] & bit) {
		    dat |= 0x800;
		}
		if (vramptr[offset + 0x0a000] & bit) {
		    dat |= 0x400;
		}
		if (vramptr[offset + 0x14000] & bit) {
		    dat |= 0x200;
		}
		if (vramptr[offset + 0x16000] & bit) {
		    dat |= 0x100;
		}

		/*
		 * R評価
		 */
		if (vramptr[offset + 0x04000] & bit) {
		    dat |= 0x080;
		}
		if (vramptr[offset + 0x06000] & bit) {
		    dat |= 0x040;
		}
		if (vramptr[offset + 0x10000] & bit) {
		    dat |= 0x020;
		}
		if (vramptr[offset + 0x12000] & bit) {
		    dat |= 0x010;
		}

		/*
		 * B評価
		 */
		if (vramptr[offset + 0x00000] & bit) {
		    dat |= 0x008;
		}
		if (vramptr[offset + 0x02000] & bit) {
		    dat |= 0x004;
		}
		if (vramptr[offset + 0x0c000] & bit) {
		    dat |= 0x002;
		}
		if (vramptr[offset + 0x0e000] & bit) {
		    dat |= 0x001;
		}
#endif

		/*
		 * アナログパレットよりデータ取得
		 */
		dat &= mask;
		color = apalet_r[dat];
		color <<= 1;
		if (apalet_r[dat] > 0) {
		    color |= 1;
		}
		color <<= 4;

		color |= apalet_g[dat];
		color <<= 1;
		if (apalet_g[dat] > 0) {
		    color |= 1;
		}
		color <<= 4;

		color |= apalet_b[dat];
		color <<= 1;
		if (apalet_b[dat] > 0) {
		    color |= 1;
		}

		/*
		 * CRTフラグ
		 */
		if (!crt_flag) {
		    color = 0;
		}

		buffer[x * 16 + i * 2 + 0] = (BYTE) (color & 255);
		buffer[x * 16 + i * 2 + 1] = (BYTE) (color >> 8);

		/*
		 * 次のビットへ
		 */
		bit >>= 1;
	    }
	    offset++;
	}

	/*
	 * 書き込み
	 */
	if (!file_write(fileh, buffer, sizeof(buffer))) {
	    return FALSE;
	}

	/*
	 * 次のyへ(戻る)
	 */
	offset -= (40 * 2);
    }

    return TRUE;
}
#endif

#if XM7_VER >= 3
/*
 *  BMPデータ書き込み(26万色モード・縮小画像)
 */
static BOOL     FASTCALL
bmp_256k_sub2(SDL_RWops *fileh)
{
    int             x,
                    y;
    int             offset;
    int             i;
    BYTE            buffer[960];
    BYTE            bit;
    BYTE            r,
                    g,
                    b;

    ASSERT(fileh >= 0);

    /*
     * 初期オフセット設定
     */
    offset = 40 * 199;

    /*
     * yループ
     */
    for (y = 0; y < 200; y++) {

	/*
	 * xループ
	 */
	for (x = 0; x < 40; x++) {
	    bit = 0x80;
	    /*
	     * ビットループ
	     */
	    for (i = 0; i < 8; i++) {
		r = g = b = 0;

		if (!(multi_page & 0x40)) {
		    /*
		     * G評価
		     */
		    if (vram_c[offset + 0x10000] & bit) {
			g |= 0x20;
		    }
		    if (vram_c[offset + 0x12000] & bit) {
			g |= 0x10;
		    }
		    if (vram_c[offset + 0x14000] & bit) {
			g |= 0x08;
		    }
		    if (vram_c[offset + 0x16000] & bit) {
			g |= 0x04;
		    }
		    if (vram_c[offset + 0x28000] & bit) {
			g |= 0x02;
		    }
		    if (vram_c[offset + 0x2a000] & bit) {
			g |= 0x01;
		    }
		}

		if (!(multi_page & 0x20)) {
		    /*
		     * R評価
		     */
		    if (vram_c[offset + 0x08000] & bit) {
			r |= 0x20;
		    }
		    if (vram_c[offset + 0x0a000] & bit) {
			r |= 0x10;
		    }
		    if (vram_c[offset + 0x0c000] & bit) {
			r |= 0x08;
		    }
		    if (vram_c[offset + 0x0e000] & bit) {
			r |= 0x04;
		    }
		    if (vram_c[offset + 0x20000] & bit) {
			r |= 0x02;
		    }
		    if (vram_c[offset + 0x22000] & bit) {
			r |= 0x01;
		    }
		}

		if (!(multi_page & 0x10)) {
		    /*
		     * B評価
		     */
		    if (vram_c[offset + 0x00000] & bit) {
			b |= 0x20;
		    }
		    if (vram_c[offset + 0x02000] & bit) {
			b |= 0x10;
		    }
		    if (vram_c[offset + 0x04000] & bit) {
			b |= 0x08;
		    }
		    if (vram_c[offset + 0x06000] & bit) {
			b |= 0x04;
		    }
		    if (vram_c[offset + 0x18000] & bit) {
			b |= 0x02;
		    }
		    if (vram_c[offset + 0x1a000] & bit) {
			b |= 0x01;
		    }
		}

		/*
		 * CRTフラグ
		 */
		if (!crt_flag) {
		    r = g = b = 0;
		}

		buffer[x * 24 + i * 3 + 0] = (BYTE) truecolorbrightness[b];
		buffer[x * 24 + i * 3 + 1] = (BYTE) truecolorbrightness[g];
		buffer[x * 24 + i * 3 + 2] = (BYTE) truecolorbrightness[r];

		/*
		 * 次のビットへ
		 */
		bit >>= 1;
	    }
	    offset++;
	}

	/*
	 * 書き込み
	 */
	if (!file_write(fileh, buffer, sizeof(buffer))) {
	    return FALSE;
	}

	/*
	 * 次のyへ(戻る)
	 */
	offset -= (40 * 2);
    }

    return TRUE;
}
#endif

/*
 *  BMPデータ書き込み(640モード・縮小画像)
 */
static BOOL     FASTCALL
bmp_640_sub2(SDL_RWops *fileh)
{
    int             x,
                    y;
    int             i;
    int             offset;
    BYTE            bit;
    BYTE            buffer[640];
    BYTE           *vramptr;
    BYTE            pal[8];
    BYTE            col[2];
    WORD            color;
#if XM7_VER >= 3
    WORD            dx1,
                    dx2;
    BOOL            winy;
#endif

    ASSERT(fileh >= 0);

    /*
     * 色混合テーブル初期化
     */
    mix_color_init(GAMMA200L);

    /*
     * 色データ生成
     */
    for (i = 0; i < 8; i++) {
	if (crt_flag) {
	    pal[i] =
		(BYTE) (ttl_palet[i & ((~(multi_page >> 4)) & 7)] & 7);
	} else {
	    pal[i] = 0;
	}
    }

    /*
     * 初期オフセット設定
     */
    offset = 80 * 199;

#if XM7_VER >= 3
    /*
     * ウィンドウ領域のクリッピングを行う
     */
    window_clip(0);
    dx1 = (WORD) (window_dx1 >> 3);
    dx2 = (WORD) (window_dx2 >> 3);

    /*
     * yループ
     */
    for (y = 0; y < 200; y++) {
	winy = (((199 - y) >= window_dy1) && ((199 - y) <= window_dy2));

	/*
	 * xループ
	 */
	for (x = 0; x < 80; x++) {
	    bit = 0x80;
	    if (winy && (x >= dx1) && (x < dx2)) {
		/*
		 * ウィンドウ内(裏ブロック)
		 */
		vramptr = vram_bdptr;
	    } else {
		/*
		 * ウィンドウ外(表ブロック)
		 */
		vramptr = vram_dptr;
	    }
#else
    /*
     * yループ
     */
    for (y = 0; y < 200; y++) {

	/*
	 * xループ
	 */
	for (x = 0; x < 80; x++) {
	    bit = 0x80;
#if XM7_VER >= 2
	    vramptr = vram_dptr;
#else
	    vramptr = vram_c;
#endif
#endif

	    /*
	     * bitループ
	     */
	    for (i = 0; i < 4; i++) {
		col[0] = 0;
		col[1] = 0;

#if XM7_VER >= 3
		if (vramptr[offset + 0x00000] & bit) {
		    col[0] |= 1;
		}
		if (vramptr[offset + 0x08000] & bit) {
		    col[0] |= 2;
		}
		if (vramptr[offset + 0x10000] & bit) {
		    col[0] |= 4;
		}
		bit >>= 1;

		if (vramptr[offset + 0x00000] & bit) {
		    col[1] |= 1;
		}
		if (vramptr[offset + 0x08000] & bit) {
		    col[1] |= 2;
		}
		if (vramptr[offset + 0x10000] & bit) {
		    col[1] |= 4;
		}
		bit >>= 1;
#else
		if (vramptr[offset + 0x0000] & bit) {
		    col[0] |= 1;
		}
		if (vramptr[offset + 0x4000] & bit) {
		    col[0] |= 2;
		}
		if (vramptr[offset + 0x8000] & bit) {
		    col[0] |= 4;
		}
		bit >>= 1;

		if (vramptr[offset + 0x0000] & bit) {
		    col[1] |= 1;
		}
		if (vramptr[offset + 0x4000] & bit) {
		    col[1] |= 2;
		}
		if (vramptr[offset + 0x8000] & bit) {
		    col[1] |= 4;
		}
		bit >>= 1;
#endif

		col[0] = pal[col[0]];
		col[1] = pal[col[1]];
		color = mix_color(col, 2);
		buffer[x * 8 + i * 2 + 0] = (BYTE) (color & 255);
		buffer[x * 8 + i * 2 + 1] = (BYTE) (color >> 8);
	    }
	    offset++;
	}

	/*
	 * 書き込み
	 */
	if (!file_write(fileh, buffer, sizeof(buffer))) {
	    return FALSE;
	}

	/*
	 * 次のyへ(戻る)
	 */
	offset -= (80 * 2);
    }

    return TRUE;
}

#if XM7_VER >= 3
/*
 *  BMPデータ書き込み(400ラインモード・縮小画像)
 */
static BOOL     FASTCALL
bmp_400l_sub2(SDL_RWops *fileh)
{
    int             x,
                    y;
    int             i;
    int             offset;
    BYTE            bit;
    BYTE            buffer[640];
    BYTE           *vramptr;
    BYTE            pal[8];
    BYTE            lbuf[640];
    BYTE            pbuf[4];
    WORD            color;
    BYTE            col1,
                    col2;
    WORD            dx1,
                    dx2;
    BOOL            winy;

    ASSERT(fileh >= 0);

    /*
     * 色混合テーブル初期化
     */
    mix_color_init(GAMMA400L);

    /*
     * 色データ生成
     */
    for (i = 0; i < 8; i++) {
	if (crt_flag) {
	    pal[i] =
		(BYTE) (ttl_palet[i & ((~(multi_page >> 4)) & 7)] & 7);
	} else {
	    pal[i] = 0;
	}
    }

    /*
     * 初期オフセット設定
     */
    offset = 80 * 399;

    /*
     * ウィンドウ領域のクリッピングを行う
     */
    window_clip(2);
    dx1 = (WORD) (window_dx1 >> 3);
    dx2 = (WORD) (window_dx2 >> 3);

    /*
     * yループ
     */
    for (y = 0; y < 400; y++) {
	winy = (((399 - y) >= window_dy1) && ((399 - y) <= window_dy2));

	if ((y % 2) == 0) {
	    /*
	     * パレットバッファ初期化
	     */
	    memset(lbuf, 0, sizeof(pbuf));
	}

	/*
	 * xループ
	 */
	for (x = 0; x < 80; x++) {
	    bit = 0x80;
	    if (winy && (x >= dx1) && (x < dx2)) {
		/*
		 * ウィンドウ内(裏ブロック)
		 */
		vramptr = vram_bdptr;
	    } else {
		/*
		 * ウィンドウ外(表ブロック)
		 */
		vramptr = vram_dptr;
	    }

	    /*
	     * bitループ
	     */
	    for (i = 0; i < 4; i++) {
		col1 = 0;
		col2 = 0;

		if (vramptr[offset + 0x00000] & bit) {
		    col1 |= 1;
		}
		if (vramptr[offset + 0x08000] & bit) {
		    col1 |= 2;
		}
		if (vramptr[offset + 0x10000] & bit) {
		    col1 |= 4;
		}
		bit >>= 1;

		if (vramptr[offset + 0x00000] & bit) {
		    col2 |= 1;
		}
		if (vramptr[offset + 0x08000] & bit) {
		    col2 |= 2;
		}
		if (vramptr[offset + 0x10000] & bit) {
		    col2 |= 4;
		}
		bit >>= 1;

		if ((y % 2) == 0) {
		    lbuf[(x * 4 + i) * 2 + 0] = col1;
		    lbuf[(x * 4 + i) * 2 + 1] = col2;
		} else {
		    pbuf[0] = pal[lbuf[(x * 4 + i) * 2 + 0]];
		    pbuf[1] = pal[lbuf[(x * 4 + i) * 2 + 1]];
		    pbuf[2] = pal[col1];
		    pbuf[3] = pal[col2];

		    color = mix_color(pbuf, 4);
		    buffer[x * 8 + i * 2 + 0] = (BYTE) (color & 255);
		    buffer[x * 8 + i * 2 + 1] = (BYTE) (color >> 8);
		}
	    }
	    offset++;
	}

	/*
	 * 書き込み
	 */
	if ((y % 2) == 1) {
	    if (!file_write(fileh, buffer, sizeof(buffer))) {
		return FALSE;
	    }
	}

	/*
	 * 次のyへ(戻る)
	 */
	offset -= (80 * 2);
    }

    return TRUE;
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *  BMPデータ書き込み(L4 400ラインモード・縮小画像)
 */
static BOOL     FASTCALL
bmp_400l4_sub2(SDL_RWops *fileh)
{
    int             x,
                    y;
    int             i;
    int             offset;
    BYTE            bit,
                    bit2;
    BYTE            buffer[640];
    BYTE            lbuf[640];
    BYTE            pbuf[4];
    WORD            color;
    BYTE            col1,
                    col2;
    WORD            taddr,
                    gaddr,
                    textbase;
    BYTE            csr_st,
                    csr_ed,
                    csr_type;
    BYTE            raster,
                    lines;
    BYTE            col,
                    chr,
                    atr,
                    chr_dat;
    BOOL            enable_page;

    ASSERT(fileh >= 0);

    /*
     * 初期オフセット設定
     */
    offset = 80 * 399;

    /*
     * テキスト展開 初期設定(全体)
     */
    csr_st = (BYTE) (crtc_register[10] & 0x1f);
    csr_ed = (BYTE) (crtc_register[11] & 0x1f);
    csr_type = (BYTE) ((crtc_register[10] & 0x60) >> 5);
    lines = (BYTE) ((crtc_register[9] & 0x1f) + 1);

    /*
     * パレットバッファ初期化
     */
    memset(lbuf, 0, sizeof(pbuf));

    /*
     * yループ
     */
    for (y = 0; y < 400; y++) {
	/*
	 * テキスト展開 初期設定(ラスタ単位)
	 */
	textbase = (WORD) text_start_addr;
	textbase += (WORD) (((399 - y) / lines) * (crtc_register[1] << 2));
	textbase &= 0xffe;
	raster = (BYTE) ((399 - y) % lines);

	/*
	 * xループ
	 */
	for (x = 0; x < 80; x++) {
	    bit = 0x80;
	    if (!width40_flag || !(x & 1)) {
		bit2 = 0x80;

		/*
		 * キャラクタコード・アトリビュートを取得
		 */
		if (width40_flag) {
		    taddr = (WORD) ((textbase + (x & ~1)) & 0xffe);
		} else {
		    taddr = (WORD) ((textbase + (x << 1)) & 0xffe);
		}
		chr = tvram_c[taddr + 0];
		atr = tvram_c[taddr + 1];

		/*
		 * アトリビュートから描画色を設定
		 */
		col = (BYTE) ((atr & 0x07) | ((atr & 0x20) >> 2));

		/*
		 * フォントデータ取得(アトリビュート/カーソル処理を含む)
		 */
		if ((!(atr & 0x10) || text_blink) && (raster < 16)) {
		    chr_dat = subcg_l4[(WORD) (chr << 4) + raster];
		} else {
		    chr_dat = 0x00;
		}
		if (atr & 0x08) {
		    chr_dat ^= (BYTE) 0xff;
		}
		if (csr_type != 1) {
		    if (((taddr == cursor_addr) &&
			 (cursor_blink || !csr_type)) &&
			((raster >= csr_st) && (raster <= csr_ed))) {
			chr_dat ^= (BYTE) 0xff;
		    }
		}
	    }

	    /*
	     * GVRAM 実アドレスを取得
	     */
	    gaddr = (WORD) ((offset + vram_offset[0]) & 0x7fff);
	    enable_page = FALSE;
	    if (gaddr >= 0x4000) {
		if (!(multi_page & 0x10)) {
		    enable_page = TRUE;
		}
	    } else {
		if (!(multi_page & 0x20)) {
		    enable_page = TRUE;
		}
	    }

	    /*
	     * bitループ
	     */
	    for (i = 0; i < 4; i++) {
		if (chr_dat & bit2) {
		    col1 = col;
		} else if ((vram_c[gaddr] & bit) && enable_page) {
		    col1 = ttl_palet[1];
		} else {
		    col1 = ttl_palet[0];
		}
		bit >>= 1;
		if (!width40_flag) {
		    bit2 >>= 1;
		}

		if (chr_dat & bit2) {
		    col2 = col;
		} else if ((vram_c[gaddr] & bit) && enable_page) {
		    col2 = ttl_palet[1];
		} else {
		    col2 = ttl_palet[0];
		}
		bit >>= 1;
		bit2 >>= 1;

		if ((y % 2) == 0) {
		    lbuf[(x * 4 + i) * 2 + 0] = col1;
		    lbuf[(x * 4 + i) * 2 + 1] = col2;
		} else {
		    if (crt_flag) {
			pbuf[0] = lbuf[(x * 4 + i) * 2 + 0];
			pbuf[1] = lbuf[(x * 4 + i) * 2 + 1];
			pbuf[2] = col1;
			pbuf[3] = col2;

			color = mix_color_16(GAMMA400L, pbuf, 4);
			buffer[x * 8 + i * 2 + 0] = (BYTE) (color & 255);
			buffer[x * 8 + i * 2 + 1] = (BYTE) (color >> 8);
		    } else {
			buffer[x * 8 + i * 2 + 0] = 0;
			buffer[x * 8 + i * 2 + 1] = 0;
		    }
		}
	    }
	    offset++;
	}

	/*
	 * 2ラインごとに書き込み
	 */
	if (y & 1) {
	    if (!file_write(fileh, buffer, sizeof(buffer))) {
		return FALSE;
	    }
	}

	/*
	 * 次のyへ(戻る)
	 */
	offset -= (80 * 2);
    }

    return TRUE;
}
#endif

/*
 *      画面キャプチャ(BMP)
 */
BOOL            FASTCALL
capture_to_bmp(char *fname, BOOL fullscan)
{
    SDL_RWops   *fileh;

    ASSERT(fname);

    /*
     * ファイルオープン
     */
    fileh = file_open(fname, OPEN_W);
    if (fileh == NULL) {
	return FALSE;
    }

    /*
     * ヘッダ書き込み
     */
    if (!bmp_header_sub(fileh)) {
	file_close(fileh);
	return FALSE;
    }

    /*
     * パレット書き込み
     */
#if XM7_VER >= 2
#if XM7_VER >= 3
    if (!(screen_mode & SCR_ANALOG)) {
#else
    if (!mode320) {
#endif
	if (!bmp_palette_sub(fileh)) {
	    file_close(fileh);
	    return FALSE;
	}
    }
#else
    if (!bmp_palette_sub(fileh)) {
	file_close(fileh);
	return FALSE;
    }
#endif

    /*
     * 本体書き込み
     */
#if XM7_VER >= 3
    switch (screen_mode) {
    case SCR_400LINE:		/* 640×400 8色モード */
	if (!bmp_400l_sub(fileh)) {
	    file_close(fileh);
	    return FALSE;
	}
	break;
    case SCR_262144:		/* 320×200 26万色モード */
	if (!bmp_256k_sub(fileh, fullscan)) {
	    file_close(fileh);
	    return FALSE;
	}
	break;
    case SCR_4096:		/* 320×200 4096色モード */
	if (!bmp_320_sub(fileh, fullscan)) {
	    file_close(fileh);
	    return FALSE;
	}
	break;
    case SCR_200LINE:		/* 640×200 8色モード */
	if (!bmp_640_sub(fileh, fullscan)) {
	    file_close(fileh);
	    return FALSE;
	}
	break;
    }
#elif XM7_VER >= 2
    if (mode320) {
	/*
	 * 320×200 4096色モード
	 */
	if (!bmp_320_sub(fileh, fullscan)) {
	    file_close(fileh);
	    return FALSE;
	}
    } else {
	/*
	 * 640×200 8色モード
	 */
	if (!bmp_640_sub(fileh, fullscan)) {
	    file_close(fileh);
	    return FALSE;
	}
    }
#elif XM7_VER == 1 && defined(L4CARD)
    if (enable_400line) {
	/*
	 * 640×400 単色モード
	 */
	if (!bmp_400l4_sub(fileh)) {
	    file_close(fileh);
	    return FALSE;
	}
    } else {
	/*
	 * 640×200 8色モード
	 */
	if (!bmp_640_sub(fileh, fullscan)) {
	    file_close(fileh);
	    return FALSE;
	}
    }
#else
    /*
     * 640×200 8色モード
     */
    if (!bmp_640_sub(fileh, fullscan)) {
	file_close(fileh);
	return FALSE;
    }
#endif

    /*
     * 成功
     */
    file_close(fileh);
    return TRUE;
}

/*
 *      画面キャプチャ(BMP・縮小画像)
 */
BOOL            FASTCALL
capture_to_bmp2(char *fname)
{
    SDL_RWops    *fileh;

    ASSERT(fname);

    /*
     * ファイルオープン
     */
    fileh = file_open(fname, OPEN_W);
    if (fileh == NULL) {
	return FALSE;
    }

    /*
     * ヘッダ書き込み
     */
    if (!bmp_header_sub2(fileh)) {
	file_close(fileh);
	return FALSE;
    }

    /*
     * 本体書き込み
     */
#if XM7_VER >= 3
    switch (screen_mode) {
    case SCR_400LINE:		/* 640×400 8色モード */
	if (!bmp_400l_sub2(fileh)) {
	    file_close(fileh);
	    return FALSE;
	}
	break;
    case SCR_262144:		/* 320×200 26万色モード */
	if (!bmp_256k_sub2(fileh)) {
	    file_close(fileh);
	    return FALSE;
	}
	break;
    case SCR_4096:		/* 320×200 4096色モード */
	if (!bmp_320_sub2(fileh)) {
	    file_close(fileh);
	    return FALSE;
	}
	break;
    case SCR_200LINE:		/* 640×200 8色モード */
	if (!bmp_640_sub2(fileh)) {
	    file_close(fileh);
	    return FALSE;
	}
	break;
    }
#elif XM7_VER >= 2
    if (mode320) {
	/*
	 * 320×200 4096色モード
	 */
	if (!bmp_320_sub2(fileh)) {
	    file_close(fileh);
	    return FALSE;
	}
    } else {
	/*
	 * 640×200 8色モード
	 */
	if (!bmp_640_sub2(fileh)) {
	    file_close(fileh);
	    return FALSE;
	}
    }
#elif XM7_VER == 1 && defined(L4CARD)
    if (enable_400line) {
	/*
	 * 640×400 単色モード
	 */
	if (!bmp_400l4_sub2(fileh)) {
	    file_close(fileh);
	    return FALSE;
	}
    } else {
	/*
	 * 640×200 8色モード
	 */
	if (!bmp_640_sub2(fileh)) {
	    file_close(fileh);
	    return FALSE;
	}
    }
#else
    /*
     * 640×200 8色モード
     */
    if (!bmp_640_sub2(fileh)) {
	file_close(fileh);
	return FALSE;
    }
#endif

    /*
     * 成功
     */
    file_close(fileh);
    return TRUE;
}
