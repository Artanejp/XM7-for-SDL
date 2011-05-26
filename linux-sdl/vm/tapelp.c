/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ カセットテープ＆プリンタ ]
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "tapelp.h"
#include "mainetc.h"
#include "device.h"
#include "event.h"

/*
 *      グローバル ワーク
 */
BOOL            tape_in;	/* テープ 入力データ */
BOOL            tape_out;	/* テープ 出力データ */
BOOL            tape_motor;	/* テープ モータ */
BOOL            tape_rec;	/* テープ RECフラグ */
BOOL            tape_writep;	/* テープ 書き込み禁止 */
WORD            tape_count;	/* テープ サイクルカウンタ */
DWORD           tape_subcnt;	/* テープ サブカウンタ */
int             tape_fileh;	/* テープ ファイルハンドル */
DWORD           tape_offset;	/* テープ ファイルオフセット */
char            tape_fname[128 + 1];	/* テープ ファイルネーム 
					 */

WORD            tape_incnt;	/* テープ 読み込みカウンタ */
DWORD           tape_fsize;	/* テープ ファイルサイズ */
BOOL            tape_fetch;	/* テープ
				 * データフェッチフラグ */
BYTE           *tape_savebuf;	/* テープ 書き込みバッファ */
WORD            tape_saveptr;	/* テープ 書き込みポインタ */

BOOL            tape_monitor;	/* テープ
				 * テープ音モニタフラグ */
#ifdef FDDSND
BOOL            tape_sound;	/* テープ リレー音出力フラグ */
#endif

BYTE            lp_data;	/* プリンタ 出力データ */
BOOL            lp_busy;	/* プリンタ BUSYフラグ */
BOOL            lp_error;	/* プリンタ エラーフラグ */
BOOL            lp_pe;		/* プリンタ PEフラグ */
BOOL            lp_ackng;	/* プリンタ ACKフラグ */
BOOL            lp_online;	/* プリンタ オンライン */
BOOL            lp_strobe;	/* プリンタ ストローブ */
int             lp_fileh;	/* プリンタ ファイルハンドル */

char            lp_fname[128 + 1];	/* プリンタ
					 * ファイルネーム */

/*
 *      プロトタイプ宣言
 */
static void FASTCALL tape_flush(void);	/* テープ書き込みバッファフラッシュ 
					 */


/*
 *      カセットテープ＆プリンタ
 *      初期化
 */
BOOL            FASTCALL
tapelp_init(void)
{
    /*
     * ワークエリア初期化 
     */
    tape_savebuf = NULL;

    /*
     * テープ 
     */
    tape_fileh = -1;
    tape_fname[0] = '\0';
    tape_offset = 0;
    tape_fsize = 0;
    tape_writep = FALSE;
    tape_fetch = FALSE;
    tape_saveptr = 0;
    tape_monitor = FALSE;
    tape_motor = FALSE;
#ifdef FDDSND
    tape_sound = FALSE;
#endif

    /*
     * プリンタ 
     */
    lp_fileh = -1;
    lp_fname[0] = '\0';

    /*
     * テープ書き込みバッファ 
     */
    tape_savebuf = (BYTE *) malloc(TAPE_SAVEBUFSIZE);
    if (tape_savebuf == NULL) {
	return FALSE;
    }

    return TRUE;
}

/*
 *      カセットテープ＆プリンタ
 *      クリーンアップ
 */
void            FASTCALL
tapelp_cleanup(void)
{
    ASSERT(tape_savebuf);

    /*
     * ファイルを開いていれば、閉じる 
     */
    if (tape_fileh != -1) {
	tape_flush();
	file_close(tape_fileh);
	tape_fileh = -1;
    }

    /*
     * モータOFF 
     */
    tape_motor = FALSE;

    /*
     * ファイルを開いていれば、閉じる 
     */
    if (lp_fileh != -1) {
	file_close(lp_fileh);
	lp_fileh = -1;
    }

    /*
     * 初期化途中で失敗した場合を考慮 
     */
    if (tape_savebuf) {
	free(tape_savebuf);
    }
}

/*
 *      カセットテープ＆プリンタ
 *      リセット
 */
void            FASTCALL
tapelp_reset(void)
{
    /*
     * 未出力データがあればフラッシュする 
     */
    tape_flush();

#ifdef FDDSND
    /*
     * カセットモータOFF 
     */
    if (tape_motor && tape_sound) {
	wav_notify(SOUND_CMTMOTOROFF);
    }
#endif

    tape_motor = FALSE;
    tape_rec = FALSE;
    tape_count = 0;
    tape_in = TRUE;
    tape_out = FALSE;
    tape_incnt = 0;
    tape_subcnt = 0;
    tape_fetch = FALSE;

    lp_busy = FALSE;
    lp_error = FALSE;
    lp_ackng = TRUE;
    lp_pe = FALSE;
    lp_online = FALSE;
    lp_strobe = FALSE;
}

/*-[ プリンタ ]-------------------------------------------------------------*/

/*
 *      プリンタ
 *      データ出力
 */
static void     FASTCALL
lp_output(BYTE dat)
{
    /*
     * オープンしていなければ，開く 
     */
    if (lp_fileh == -1) {
	if (lp_fname[0] != '\0') {
	    lp_fileh = file_open(lp_fname, OPEN_W);
	}
    }

    /*
     * オープンチェック 
     */
    if (lp_fileh == -1) {
	return;
    }

    /*
     * アペンド 
     */
    file_write(lp_fileh, &dat, 1);
}

/*
 *      プリンタ
 *      ファイル名設定
 */
void            FASTCALL
lp_setfile(char *fname)
{
    /*
     * 一度開いていれば、閉じる 
     */
    if (lp_fileh != -1) {
	file_close(lp_fileh);
	lp_fileh = -1;
    }

    /*
     * ファイル名セット 
     */
    if (fname == NULL) {
	lp_fname[0] = '\0';
	return;
    }

    if (strlen(fname) < sizeof(lp_fname)) {
	strcpy(lp_fname, fname);
    } else {
	lp_fname[0] = '\0';
    }
}

/*-[ テープ ]---------------------------------------------------------------*/

/*
 *      テープ
 *      書き込みバッファのフラッシュ
 */
static void     FASTCALL
tape_flush(void)
{
    if (tape_fileh != -1) {
	/*
	 * 録音状態でバッファ内にデータがあればファイルに書き出す 
	 */
	if ((tape_rec) && (tape_saveptr > 0)) {
	    file_write(tape_fileh, tape_savebuf, tape_saveptr);
	}
    }

    /*
     * 書き込みポインタを初期化 
     */
    tape_saveptr = 0;
}

/*
 *      テープ
 *      １バイト書き込み
 */
static void     FASTCALL
tape_byte_write(BYTE dat)
{
    /*
     * バッファにデータを追加 
     */
    tape_savebuf[tape_saveptr++] = dat;

    /*
     * バッファがいっぱいになったらデータを書き出す 
     */
    if (tape_saveptr >= TAPE_SAVEBUFSIZE) {
	tape_flush();
    }
}

/*
 *      テープ
 *      データ入力
 */
static void     FASTCALL
tape_input(BOOL flag)
{
    BYTE            high;
    BYTE            low;
    WORD            dat;

    /*
     * モータが回っているか 
     */
    if (!tape_motor) {
	return;
    }

    /*
     * 録音されていれば入力できない 
     */
    if (tape_rec) {
	return;
    }

    /*
     * 本番でない場合、既にデータフェッチしている場合は何もしない 
     */
    if (!flag && tape_fetch) {
	return;
    }

    /*
     * シングルカウンタが入力カウンタを越えていれば、0にする 
     */
    while (tape_count >= tape_incnt) {
	tape_count -= tape_incnt;
	tape_incnt = 0;

	/*
	 * データフェッチ 
	 */
	tape_in = FALSE;

	if (tape_fileh == -1) {
	    return;
	}

	if (tape_offset >= tape_fsize) {
	    return;
	}

	if (!file_seek(tape_fileh, tape_offset)) {
	    return;
	}
	if (!file_read(tape_fileh, &high, 1)) {
	    return;
	}
	if (!file_read(tape_fileh, &low, 1)) {
	    return;
	}

	/*
	 * データ設定 
	 */
	dat = (WORD) (high * 256 + low);
	if (dat > 0x7fff) {
	    tape_in = TRUE;
	}

	/*
	 * データフェッチ済みフラグを設定 
	 */
	tape_fetch = !flag;

	/*
	 * 本番の入力でない場合はここまで 
	 */
	if (!flag) {
	    return;
	}

	/*
	 * カウンタ設定 
	 */
	tape_incnt = (WORD) (dat & 0x7fff);

	/*
	 * カウンタを先繰りする 
	 */
	if (tape_count > tape_incnt) {
	    tape_count -= tape_incnt;
	    tape_incnt = 0;
	} else {
	    tape_incnt -= tape_count;
	    tape_count = 0;
	}

	/*
	 * オフセット更新 
	 */
	tape_offset += 2;
    }
}

/*
 *      テープ
 *      データ出力
 */
static void     FASTCALL
tape_output(BOOL flag)
{
    WORD            dat;
    BYTE            high,
                    low;

    /*
     * テープが回っているか 
     */
    if (!tape_motor) {
	return;
    }

    /*
     * 録音中か 
     */
    if (!tape_rec) {
	return;
    }

    /*
     * カウンタが回っているか 
     */
    if (tape_count == 0) {
	return;
    }

    /*
     * 書き込み可能か 
     */
    if (tape_writep) {
	return;
    }

    /*
     * ファイルがオープンされていれば、データ書き込み 
     */
    dat = tape_count;
    if (dat >= 0x8000) {
	dat = 0x7fff;
    }
    if (flag) {
	dat |= 0x8000;
    }
    high = (BYTE) (dat >> 8);
    low = (BYTE) (dat & 0xff);
    if (tape_fileh != -1) {
	tape_byte_write(high);
	tape_byte_write(low);

	tape_offset += 2;
	if (tape_offset >= tape_fsize) {
	    tape_fsize = tape_offset;
	}
    }

    /*
     * カウンタをリセット 
     */
    tape_count = 0;
    tape_subcnt = 0;
}

/*
 *      テープ
 *      マーカ出力
 */
static void     FASTCALL
tape_mark(void)
{
    /*
     * テープが回っているか 
     */
    if (!tape_motor) {
	return;
    }

    /*
     * 録音中か 
     */
    if (!tape_rec) {
	return;
    }

    /*
     * 書き込み可能か 
     */
    if (tape_writep) {
	return;
    }

    /*
     * ファイルがオープンされていれば、データ書き込み 
     */
    if (tape_fileh != -1) {
	tape_byte_write(0);
	tape_byte_write(0);

	tape_offset += 2;
	if (tape_offset >= tape_fsize) {
	    tape_fsize = tape_offset;
	}
    }
}

/*
 *      テープ
 *      巻き戻し
 */
void            FASTCALL
tape_rew(void)
{
    WORD            dat;

    /*
     * 条件判定 
     */
    if (tape_fileh == -1) {
	return;
    }

    /*
     * assert 
     */
    ASSERT(tape_fsize >= 16);
    ASSERT(tape_offset >= 16);
    ASSERT(!(tape_fsize & 0x01));
    ASSERT(!(tape_offset & 0x01));

    /*
     * 録音中ならいったんフラッシュ 
     */
    tape_flush();

    while (tape_offset > 16) {
	/*
	 * ２バイト前に戻り、読み込み 
	 */
	tape_offset -= 2;
	if (!file_seek(tape_fileh, tape_offset)) {
	    return;
	}
	file_read(tape_fileh, (BYTE *) & dat, 2);

	/*
	 * $0000なら、そこに設定 
	 */
	if (dat == 0) {
	    file_seek(tape_fileh, tape_offset);
	    return;
	}

	/*
	 * いま読み込んだ分だけ戻す 
	 */
	if (!file_seek(tape_fileh, tape_offset)) {
	    return;
	}
    }
}

/*
 *      テープ
 *      早送り
 */
void            FASTCALL
tape_ff(void)
{
    WORD            dat;

    /*
     * 条件判定 
     */
    if (tape_fileh == -1) {
	return;
    }

    /*
     * assert 
     */
    ASSERT(tape_fsize >= 16);
    ASSERT(tape_offset >= 16);
    ASSERT(!(tape_fsize & 0x01));
    ASSERT(!(tape_offset & 0x01));

    /*
     * 録音中ならいったんフラッシュ 
     */
    tape_flush();

    while (tape_offset < tape_fsize) {
	/*
	 * 先へ進める 
	 */
	tape_offset += 2;
	if (tape_offset >= tape_fsize) {
	    return;
	}
	if (!file_seek(tape_fileh, tape_offset)) {
	    return;
	}
	file_read(tape_fileh, (BYTE *) & dat, 2);

	/*
	 * $0000なら、その次に設定 
	 */
	if (dat == 0) {
	    tape_offset += 2;
	    if (tape_offset >= tape_fsize) {
		tape_fsize = tape_offset;
	    }
	    return;
	}
    }
}

#if 0
/*
 *      テープ
 *      巻き戻し(無音部分検索)
 */
void            FASTCALL
tape_rew2(void)
{
    WORD            dat;
    WORD            flag;

    /*
     * 条件判定 
     */
    if (tape_fileh == -1) {
	return;
    }

    /*
     * assert 
     */
    ASSERT(tape_fsize >= 16);
    ASSERT(tape_offset >= 16);
    ASSERT(!(tape_fsize & 0x01));
    ASSERT(!(tape_offset & 0x01));

    /*
     * 録音中ならいったんフラッシュ 
     */
    tape_flush();

    /*
     * データ状態を初期化 
     */
    flag = -1;

    while (tape_offset > 16) {
	/*
	 * ２バイト前に戻り、読み込み 
	 */
	tape_offset -= 2;
	if (!file_seek(tape_fileh, tape_offset)) {
	    return;
	}
	file_read(tape_fileh, (BYTE *) & dat, 2);

	/*
	 * しばらくデータが変化していなければ変化するまで戻す 
	 */
	if (((WORD) (dat & 0x8000) == flag) &&
	    ((WORD) (dat & 0x7fff) == 0x7fff)) {
	    while ((WORD) (dat & 0x8000) == flag) {
		tape_offset -= 2;
		if (!file_seek(tape_fileh, tape_offset)) {
		    return;
		}
		file_read(tape_fileh, (BYTE *) & dat, 2);
	    }
	    tape_offset += 2;
	    if (!file_seek(tape_fileh, tape_offset)) {
		return;
	    }
	    return;
	}

	/*
	 * データ状態を保存 
	 */
	flag = (WORD) (dat & 0x8000);
    }
}

/*
 *      テープ
 *      早送り(無音部分検索)
 */
void            FASTCALL
tape_ff2(void)
{
    WORD            dat;
    WORD            flag;

    /*
     * 条件判定 
     */
    if (tape_fileh == -1) {
	return;
    }

    /*
     * assert 
     */
    ASSERT(tape_fsize >= 16);
    ASSERT(tape_offset >= 16);
    ASSERT(!(tape_fsize & 0x01));
    ASSERT(!(tape_offset & 0x01));

    /*
     * 録音中ならいったんフラッシュ 
     */
    tape_flush();

    /*
     * データ状態を初期化 
     */
    flag = -1;

    while (tape_offset < tape_fsize) {
	/*
	 * 先へ進める 
	 */
	tape_offset += 2;
	if (tape_offset >= tape_fsize) {
	    return;
	}
	if (!file_seek(tape_fileh, tape_offset)) {
	    return;
	}
	file_read(tape_fileh, (BYTE *) & dat, 2);

	/*
	 * しばらくデータが変化してければ変化するまで進める 
	 */
	if (((WORD) (dat & 0x8000) == flag) &&
	    ((WORD) (dat & 0x7fff) == 0x7fff)) {
	    do {
		tape_offset += 2;
		if (tape_offset >= tape_fsize) {
		    return;
		}
		if (!file_seek(tape_fileh, tape_offset)) {
		    return;
		}
		file_read(tape_fileh, (BYTE *) & dat, 2);
	    }
	    while ((WORD) (dat & 0x8000) == flag);
	    tape_offset += 2;
	    if (tape_offset >= tape_fsize) {
		tape_fsize = tape_offset;
	    }
	    return;
	}

	/*
	 * データ状態を保存 
	 */
	flag = (WORD) (dat & 0x8000);
    }
}
#endif

/*
 *      テープ
 *      ファイル名設定
 */
void            FASTCALL
tape_setfile(char *fname)
{
    char           *header = "XM7 TAPE IMAGE 0";
    char            buf[17];

    /*
     * 一度開いていれば、閉じる 
     */
    if (tape_fileh != -1) {
	tape_flush();
	file_close(tape_fileh);
	tape_fileh = -1;
	tape_writep = FALSE;
    }

    /*
     * ファイル名セット 
     */
    if (fname == NULL) {
	tape_fname[0] = '\0';
    } else {
	if (strlen(fname) < sizeof(tape_fname)) {
	    strcpy(tape_fname, fname);
	} else {
	    tape_fname[0] = '\0';
	}
    }

    /*
     * ファイルオープンを試みる 
     */
    if (tape_fname[0] != '\0') {
	tape_fileh = file_open(tape_fname, OPEN_RW);
	if (tape_fileh != -1) {
	    tape_writep = FALSE;
	} else {
	    tape_fileh = file_open(tape_fname, OPEN_R);
	    tape_writep = TRUE;
	}
    }

    /*
     * 開けていれば、ヘッダを読み込みチェック 
     */
    if (tape_fileh != -1) {
	memset(buf, 0, sizeof(buf));
	file_read(tape_fileh, (BYTE *) buf, 16);
	if (strcmp(buf, header) != 0) {
	    file_close(tape_fileh);
	    tape_fileh = -1;
	    tape_writep = FALSE;
	}
    }

    /*
     * フラグの処理 
     */
    tape_setrec(FALSE);
    tape_count = 0;
    tape_incnt = 0;
    tape_subcnt = 0;

    /*
     * ファイルが開けていれば、ファイルサイズ、オフセットを決定 
     */
    if (tape_fileh != -1) {
	tape_fsize = file_getsize(tape_fileh);
	tape_offset = 16;
    }
}

/*
 *      テープ
 *      録音フラグ設定
 */
void            FASTCALL
tape_setrec(BOOL flag)
{
    /*
     * モータが回っていれば、マーカを書き込む 
     */
    if (tape_motor && !tape_rec) {
	if (flag) {
	    tape_rec = TRUE;
	    tape_mark();
	    return;
	}
    } else {
	/*
	 * 録音終了なら、書き込みバッファをフラッシュ 
	 */
	if (tape_motor && tape_rec) {
	    if (!flag) {
		tape_flush();
	    }
	}
    }

    tape_rec = flag;
}

/*
 *      テープ
 *      サウンド生成
 */
static BOOL     FASTCALL
tape_outsnd(void)
{
    if (tape_motor) {
	if (tape_rec) {
	    /*
	     * 録音 
	     */
	    if (!tape_writep) {
		tape_notify(tape_out);
	    }
	} else {
	    /*
	     * 再生 
	     */
	    tape_input(FALSE);
	    tape_notify(tape_in);
	}
    }

    return TRUE;
}

/*-[ メモリR/W ]------------------------------------------------------------*/

/*
 *      カセットテープ＆プリンタ
 *      １バイト読み出し
 */
BOOL            FASTCALL
tapelp_readb(WORD addr, BYTE * dat)
{
    BYTE            ret;
    BYTE            joy;

    /*
     * アドレスチェック 
     */
    if (addr != 0xfd02) {
	return FALSE;
    }

    /*
     * プリンタ ステータス作成 
     */
    ret = 0x70;
    if (lp_busy) {
	ret |= 0x01;
    }
    if (!lp_error) {
	ret |= 0x02;
    }
    if (!lp_ackng) {
	ret |= 0x04;
    }
    if (lp_pe) {
	ret |= 0x08;
    }

    /*
     * プリンタ未接続なら、電波新聞社ジョイスティック 
     */
    if ((lp_fileh == -1) || (lp_fname[0] == '\0')) {
	/*
	 * 初期化、取得 
	 */
	ret |= 0x0f;
	joy = joy_request(2);

	/*
	 * 右 
	 */
	if (!(lp_data & 0x01) && (joy & 0x08)) {
	    ret &= ~0x08;
	}
	/*
	 * 左 
	 */
	if (!(lp_data & 0x02) && (joy & 0x04)) {
	    ret &= ~0x08;
	}
	/*
	 * 上 
	 */
	if (!(lp_data & 0x04) && (joy & 0x01)) {
	    ret &= ~0x08;
	}
	/*
	 * 下 
	 */
	if (!(lp_data & 0x08) && (joy & 0x02)) {
	    ret &= ~0x08;
	}
	/*
	 * J2 
	 */
	if (!(lp_data & 0x10) && (joy & 0x20)) {
	    ret &= ~0x08;
	}
	/*
	 * J1 
	 */
	if (!(lp_data & 0x20) && (joy & 0x10)) {
	    ret &= ~0x08;
	}
    }

    /*
     * カセット データ作成 
     */
    tape_input(TRUE);
    if (tape_in) {
	ret |= 0x80;
    }

    /*
     * ok 
     */
    *dat = ret;
    return TRUE;
}

/*
 *      カセットテープ＆プリンタ
 *      １バイト書き込み
 */
BOOL            FASTCALL
tapelp_writeb(WORD addr, BYTE dat)
{
    switch (addr) {
	/*
	 * カセット制御、プリンタ制御 
	 */
    case 0xfd00:
	/*
	 * プリンタ オンライン 
	 */
	if (dat & 0x80) {
	    lp_online = FALSE;
	} else {
	    lp_online = TRUE;
	}

	/*
	 * プリンタ ストローブ 
	 */
	if (dat & 0x40) {
	    lp_strobe = TRUE;
	} else {
	    if (lp_strobe && lp_online) {
		lp_output(lp_data);
		mainetc_lp();
	    }
	    lp_strobe = FALSE;
	}

	/*
	 * テープ 出力データ 
	 */
	if (dat & 0x01) {
	    if (!tape_out) {
		tape_output(FALSE);
	    }
	    tape_out = TRUE;
	} else {
	    if (tape_out) {
		tape_output(TRUE);
	    }
	    tape_out = FALSE;
	}

	/*
	 * テープ モータ 
	 */
	if (dat & 0x02) {
	    if (!tape_motor) {
		/*
		 * 新規スタート 
		 */
		tape_count = 0;
		tape_subcnt = 0;
		tape_motor = TRUE;
		if (tape_rec) {
		    tape_mark();
		}
#ifdef FDDSND
		if (tape_sound) {
		    wav_notify(SOUND_CMTMOTORON);
		}
#endif
		schedule_setevent(EVENT_TAPEMON, 40, tape_outsnd);
	    }
	} else {
#ifdef FDDSND
	    if (tape_motor && tape_sound) {
		wav_notify(SOUND_CMTMOTOROFF);
	    }
#endif
	    schedule_delevent(EVENT_TAPEMON);

	    /*
	     * モータ停止 
	     */
	    tape_motor = FALSE;
	    tape_flush();
	}

	return TRUE;

	/*
	 * プリンタ出力データ 
	 */
    case 0xfd01:
	lp_data = dat;
	return TRUE;
    }

    return FALSE;
}

/*
 *      カセットテープ＆プリンタ
 *      セーブ
 */
BOOL            FASTCALL
tapelp_save(int fileh)
{
    BOOL            tmp;

    /*
     * ステートセーブ前に書き込みバッファをフラッシュ 
     */
    tape_flush();

    if (!file_bool_write(fileh, tape_in)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, tape_out)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, tape_motor)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, tape_rec)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, tape_writep)) {
	return FALSE;
    }
    if (!file_word_write(fileh, tape_count)) {
	return FALSE;
    }
    if (!file_dword_write(fileh, tape_subcnt)) {
	return FALSE;
    }

    if (!file_dword_write(fileh, tape_offset)) {
	return FALSE;
    }
    if (!file_write(fileh, (BYTE *) tape_fname, 128 + 1)) {
	return FALSE;
    }

    tmp = (tape_fileh != -1);
    if (!file_bool_write(fileh, tmp)) {
	return FALSE;
    }

    if (!file_word_write(fileh, tape_incnt)) {
	return FALSE;
    }
    if (!file_dword_write(fileh, tape_fsize)) {
	return FALSE;
    }

    if (!file_byte_write(fileh, lp_data)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, lp_busy)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, lp_error)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, lp_pe)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, lp_ackng)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, lp_online)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, lp_strobe)) {
	return FALSE;
    }

    if (!file_write(fileh, (BYTE *) lp_fname, 128 + 1)) {
	return FALSE;
    }

    return TRUE;
}

/*
 *      カセットテープ＆プリンタ
 *      ロード
 */
BOOL            FASTCALL
tapelp_load(int fileh, int ver)
{
    DWORD           offset;
    char            fname[128 + 1];
    BYTE            tmp;
    BOOL            flag;

    /*
     * バージョンチェック 
     */
    if (ver < 200) {
	return FALSE;
    }

    if (!file_bool_read(fileh, &tape_in)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &tape_out)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &tape_motor)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &tape_rec)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &tape_writep)) {
	return FALSE;
    }
    if (!file_word_read(fileh, &tape_count)) {
	return FALSE;
    }
#if XM7_VER >= 2
#if XM7_VER >= 3
    if ((ver >= 906) || ((ver >= 706) && (ver <= 799))) {
#else
    if ((ver >= 706) && (ver <= 799)) {
#endif
	if (!file_dword_read(fileh, &tape_subcnt)) {
	    return FALSE;
	}
    } else {
	if (!file_byte_read(fileh, &tmp)) {
	    return FALSE;
	}
	tape_subcnt = (tmp << 4);
    }
#else
    if (!file_dword_read(fileh, &tape_subcnt)) {
	return FALSE;
    }
#endif

    if (!file_dword_read(fileh, &offset)) {
	return FALSE;
    }
    if (!file_read(fileh, (BYTE *) fname, 128 + 1)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &flag)) {
	return FALSE;
    }

    /*
     * マウント 
     */
    tape_setfile(NULL);
    if (flag) {
	tape_setfile(fname);
	if ((tape_fileh != -1) && ((tape_fsize + 1) >= offset)) {
	    file_seek(tape_fileh, offset);
	    tape_offset = offset;
	}
    }

    if (!file_word_read(fileh, &tape_incnt)) {
	return FALSE;
    }
    /*
     * tape_fsizeは無効 
     */
    if (!file_dword_read(fileh, &offset)) {
	return FALSE;
    }

    if (!file_byte_read(fileh, &lp_data)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &lp_busy)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &lp_error)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &lp_pe)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &lp_ackng)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &lp_online)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &lp_strobe)) {
	return FALSE;
    }

    if (!file_read(fileh, (BYTE *) lp_fname, 128 + 1)) {
	return FALSE;
    }

    schedule_handle(EVENT_TAPEMON, tape_outsnd);

    /*
     * その他のワークエリアを初期化 
     */
    tape_saveptr = 0;
    tape_fetch = FALSE;

    return TRUE;
}
