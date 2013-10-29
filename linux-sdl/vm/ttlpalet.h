/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2013 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2013 Ryu Takegami
 *
 *      [ TTLパレット(MB15021) ]
 */

#ifndef _ttlpalet_h_
#define _ttlpalet_h_

#ifdef __cplusplus
extern          "C" {
#endif
    /*
     *      主要エントリ
     */
    BOOL FASTCALL   ttlpalet_init(void);
    /*
     * 初期化
     */
    void FASTCALL   ttlpalet_cleanup(void);
    /*
     * クリーンアップ
     */
    void FASTCALL   ttlpalet_reset(void);
    /*
     * リセット
     */
    BOOL FASTCALL   ttlpalet_readb(WORD addr, BYTE * dat);
    /*
     * メモリ読み出し
     */
    BOOL FASTCALL   ttlpalet_writeb(WORD addr, BYTE dat);
    /*
     * メモリ書き込み
     */
    BOOL FASTCALL   ttlpalet_save(SDL_RWops *fileh);
    /*
     * セーブ
     */
    BOOL FASTCALL   ttlpalet_load(SDL_RWops *fileh, int ver);
    /*
     * ロード
     */

    /*
     *      主要ワーク
     */
    extern BYTE     ttl_palet[8];
    /*
     * TTLパレットデータ
     */
#ifdef __cplusplus
}
#endif
#endif				/* _ttlpalet_h_ */
