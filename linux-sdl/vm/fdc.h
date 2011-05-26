/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ フロッピーディスク コントローラ(MB8877A) ]
 */

#ifndef _fdc_h_
#define _fdc_h_

/*
 *      定数定義
 */
#define FDC_DRIVES			4	/* サポートドライブ数 
						 */
#define FDC_MEDIAS			16	/* D77ファイルに含まれる最大枚数 
						 */

#define FDC_ST_BUSY			0x01	/* BUSY */
#define FDC_ST_INDEX		0x02	/* INDEXホール検出 */
#define FDC_ST_DRQ			0x02	/* データ要求 */
#define FDC_ST_TRACK00		0x04	/* トラック0 */
#define FDC_ST_LOSTDATA		0x04	/* データロスト */
#define FDC_ST_CRCERR		0x08	/* CRCエラー */
#define FDC_ST_SEEKERR		0x10	/* シークエラー */
#define FDC_ST_RECNFND		0x10	/* セクタ検索失敗 */
#define FDC_ST_HEADENG		0x20	/* ヘッド押し付け */
#define FDC_ST_RECTYPE		0x20	/* レコードタイプ異常 */
#define FDC_ST_WRITEFAULT	0x20	/* 書き込み失敗 */
#define FDC_ST_WRITEP		0x40	/* 書き込み保護 */
#define FDC_ST_NOTREADY		0x80	/* メディア未挿入 */

#define FDC_TYPE_NOTREADY	0	/* ファイルなし */
#define FDC_TYPE_2D			1	/* 2Dファイルをマウント 
						 */
#define FDC_TYPE_D77		2	/* D77ファイルをマウント */
#define FDC_TYPE_VFD		3	/* VFDファイルをマウント */
#define FDC_TYPE_2DD		4	/* 2DDファイルをマウント */

#define FDC_ACCESS_READY	0	/* アクセスなし */
#define FDC_ACCESS_SEEK		1	/* シーク系アクセス */
#define FDC_ACCESS_READ		2	/* 読み込み系アクセス */
#define FDC_ACCESS_WRITE	3	/* 書き込み系アクセス */
#define FDC_ACCESS_NOTREADY	4	/* ドライブの準備ができていない */

#define FDC_LOST_TIME		48	/* wait時のLOST DATAまでの時間 */

#ifdef __cplusplus
extern          "C" {
#endif
    /*
     *      主要エントリ
     */
    BOOL    fdc_init(void);
    /*
     * 初期化 
     */
    void    fdc_cleanup(void);
    /*
     * クリーンアップ 
     */
    void    fdc_reset(void);
    /*
     * リセット 
     */
    BOOL    fdc_readb(WORD addr, BYTE * dat);
    /*
     * メモリ読み出し 
     */
    BOOL    fdc_writeb(WORD addr, BYTE dat);
    /*
     * メモリ書き込み 
     */
    BOOL    fdc_save(int fileh);
    /*
     * セーブ 
     */
    BOOL    fdc_load(int fileh, int ver);
    /*
     * ロード 
     */
    int     fdc_setdisk(int drive, char *fname);
    /*
     * ディスクセット 
     */
    BOOL    fdc_setmedia(int drive, int index);
    /*
     * ディスクファイル内メディア指定 
     */
    BOOL    fdc_setwritep(int drive, BOOL writep);
    /*
     * ライトプロテクト指定 
     */

    /*
     *      主要ワーク
     */
    extern BYTE     fdc_command;
    /*
     * $FD18 FDCコマンド 
     */
    extern BYTE     fdc_status;
    /*
     * $FD18 FDCステータス 
     */
    extern BYTE     fdc_trkreg;
    /*
     * $FD19 トラックレジスタ 
     */
    extern BYTE     fdc_secreg;
    /*
     * $FD1A セクタレジスタ 
     */
    extern BYTE     fdc_datareg;
    /*
     * $FD1B データレジスタ 
     */
    extern BYTE     fdc_sidereg;
    /*
     * $FD1C サイドレジスタ 
     */
    extern BYTE     fdc_motor;
    /*
     * $FD1D モータ(on:0x80 off:0x00) 
     */
    extern BYTE     fdc_drvreg;
    /*
     * $FD1D ドライブ(0-3) 
     */
    extern BYTE     fdc_drqirq;
    /*
     * $FD1F DRQ, IRQ, その他フラグ 
     */


#if XM7_VER >= 3
    extern BYTE     fdc_logidrv;
    /*
     * $FD1E 論理ドライブ番号 
     */
    extern BYTE     fdc_physdrv[FDC_DRIVES];
    /*
     * $FD1E 論理/物理ドライブの対応 
     */
    extern BYTE     fdc_2ddmode;
    /*
     * $FD1E 2DDモードセレクト 
     */
#endif

    extern BYTE     fdc_cmdtype;
    /*
     * コマンドタイプ 
     */
    extern WORD     fdc_totalcnt;
    /*
     * トータルカウンタ 
     */
    extern WORD     fdc_nowcnt;
    /*
     * カレントカウンタ 
     */
    extern BYTE     fdc_ready[FDC_DRIVES];
    /*
     * レディ状態 
     */
    extern BOOL     fdc_teject[FDC_DRIVES];
    /*
     * 一時イジェクト 
     */
    extern BOOL     fdc_writep[FDC_DRIVES];
    /*
     * 書き込み禁止状態 
     */
    extern BYTE     fdc_track[FDC_DRIVES];
    /*
     * 実トラック 
     */

    extern char     fdc_fname[FDC_DRIVES][256 + 1];
    /*
     * ファイルネーム 
     */
    extern char     fdc_name[FDC_DRIVES][FDC_MEDIAS][17];
    /*
     * メディアごとの名前 
     */
    extern BOOL     fdc_fwritep[FDC_DRIVES];
    /*
     * 書き込み禁止状態(ファイル単位) 
     */
    extern BYTE     fdc_header[FDC_DRIVES][0x2b0];
    /*
     * D77ファイルヘッダ 
     */
    extern BYTE     fdc_medias[FDC_DRIVES];
    /*
     * メディア枚数 
     */
    extern BYTE     fdc_media[FDC_DRIVES];
    /*
     * メディアセレクト状態 
     */
    extern BYTE     fdc_access[FDC_DRIVES];
    /*
     * アクセスLED 
     */

#ifdef FDDSND
    extern BOOL     fdc_waitmode;
    /*
     * FDCアクセスウェイト 
     */
    extern BOOL     fdc_sound;
    /*
     * FDDシーク音発生フラグ 
     */
#endif

#ifdef __cplusplus
}
#endif
#endif				/* _fdc_h_ */
