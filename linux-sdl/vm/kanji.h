/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ 漢字ROM ]
 */

#ifndef _kanji_h_
#define _kanji_h_

#ifdef __cplusplus
extern          "C" {
#endif
    /*
     *      主要エントリ
     */
    BOOL FASTCALL   kanji_init(void);
    /*
     * 初期化 
     */
    void FASTCALL   kanji_cleanup(void);
    /*
     * クリーンアップ 
     */
    void FASTCALL   kanji_reset(void);
    /*
     * リセット 
     */
    BOOL FASTCALL   kanji_readb(WORD addr, BYTE * dat);
    /*
     * メモリ読み出し 
     */
    BOOL FASTCALL   kanji_writeb(WORD addr, BYTE dat);
    /*
     * メモリ書き込み 
     */
    BOOL FASTCALL   kanji_save(int fileh);
    /*
     * セーブ 
     */
    BOOL FASTCALL   kanji_load(int fileh, int ver);
    /*
     * ロード 
     */

    /*
     *      主要ワーク
     */
    extern WORD     kanji_addr;
    /*
     * アドレスレジスタ(共通) 
     */
    extern BYTE    *kanji_rom;
    /*
     * 第１水準ROM 
     */
#if XM7_VER >= 3
    extern BYTE *kanji_rom_jis78; /* 第１水準ROM(JIS78準拠) */
    extern BYTE    *kanji_rom2;
    /*
     * 第２水準ROM 
     */
#endif
#ifdef __cplusplus
}
#endif
#endif				/* _kanji_h_ */
