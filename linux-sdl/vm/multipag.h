/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ マルチページ ]
 */

#ifndef _multipag_h_
#define _multipag_h_

#ifdef __cplusplus
extern          "C" {
#endif
    /*
     *      主要エントリ
     */
    BOOL FASTCALL   multipag_init(void);
    /*
     * 初期化 
     */
    void FASTCALL   multipag_cleanup(void);
    /*
     * クリーンアップ 
     */
    void FASTCALL   multipag_reset(void);
    /*
     * リセット 
     */
    BOOL FASTCALL   multipag_readb(WORD addr, BYTE * dat);
    /*
     * メモリ読み出し 
     */
    BOOL FASTCALL   multipag_writeb(WORD addr, BYTE dat);
    /*
     * メモリ書き込み 
     */
    BOOL FASTCALL   multipag_save(int fileh);
    /*
     * セーブ 
     */
    BOOL FASTCALL   multipag_load(int fileh, int ver);
    /*
     * ロード 
     */

    /*
     *      主要ワーク
     */
    extern BYTE     multi_page;
    /*
     * マルチページ 
     */
#ifdef __cplusplus
}
#endif
#endif				/* _multipag_h_ */
