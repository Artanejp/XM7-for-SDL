/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ MMR,TWR / I/O型RAMディスク ]
 */

#ifndef _mmr_h_
#define _mmr_h_

#ifdef __cplusplus
extern          "C" {
#endif
    /*
     *      主要エントリ
     */
    BOOL FASTCALL   mmr_init(void);
    /*
     * 初期化 
     */
    void FASTCALL   mmr_cleanup(void);
    /*
     * クリーンアップ 
     */
    void FASTCALL   mmr_reset(void);
    /*
     * リセット 
     */
    BOOL FASTCALL   mmr_readb(WORD addr, BYTE * dat);
    /*
     * メモリ読み出し 
     */
    BOOL FASTCALL   mmr_writeb(WORD addr, BYTE dat);
    /*
     * メモリ書き込み 
     */
    BOOL FASTCALL   mmr_extrb(WORD * addr, BYTE * dat);
    /*
     * MMR経由読み出し 
     */
    BOOL FASTCALL   mmr_extbnio(WORD * addr, BYTE * dat);
    /*
     * MMR経由読み出し(I/Oなし) 
     */
    BOOL FASTCALL   mmr_extwb(WORD * addr, BYTE dat);
    /*
     * MMR経由書き込み 
     */
    BOOL FASTCALL   mmr_save(int fileh);
    /*
     * セーブ 
     */
    BOOL FASTCALL   mmr_load(int fileh, int ver);
    /*
     * ロード 
     */

    /*
     *      主要ワーク
     */
    extern BOOL     mmr_flag;
    /*
     * MMR有効フラグ 
     */
    extern BYTE     mmr_seg;
    /*
     * MMRセグメント 
     */
    extern BOOL     mmr_modify;
    /*
     * MMR状態変更フラグ 
     */
#if XM7_VER >= 3
    extern BYTE     mmr_reg[0x80];
    /*
     * MMRレジスタ 
     */
    extern BOOL     mmr_ext;
    /*
     * 拡張MMR有効フラグ 
     */
    extern BOOL     mmr_fastmode;
    /*
     * MMR高速フラグ 
     */
    extern BOOL     mmr_extram;
    /*
     * 拡張RAM有効フラグ 
     */
    extern BOOL     mmr_fast_refresh;
    /*
     * 高速リフレッシュフラグ 
     */
#else
    extern BYTE     mmr_reg[0x40];
    /*
     * MMRレジスタ 
     */
#endif

    extern BOOL     twr_flag;
    /*
     * TWR有効フラグ 
     */
    extern BYTE     twr_reg;
    /*
     * TWRレジスタ 
     */
#ifdef __cplusplus
}
#endif
#endif				/* _mmr_h_ */
