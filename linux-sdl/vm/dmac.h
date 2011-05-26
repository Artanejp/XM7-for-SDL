/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ DMAC(HD68B44) AV40仕様対応版 ]
 */

#ifndef _dmac_h_
#define _dmac_h_

#if XM7_VER >= 3

#ifdef __cplusplus
extern          "C" {
#endif
    /*
     *      主要エントリ
     */
    BOOL FASTCALL   dmac_init(void);
    /*
     * 初期化 
     */
    void FASTCALL   dmac_cleanup(void);
    /*
     * クリーンアップ 
     */
    void FASTCALL   dmac_reset(void);
    /*
     * リセット 
     */
    void FASTCALL   dmac_start(void);
    /*
     * スタート 
     */
    void FASTCALL   dmac_exec(void);
    /*
     * １ステップ転送 
     */
    BOOL FASTCALL   dmac_readb(WORD addr, BYTE * dat);
    /*
     * メモリ読み出し 
     */
    BOOL FASTCALL   dmac_writeb(WORD addr, BYTE dat);
    /*
     * メモリ書き込み 
     */
    BOOL FASTCALL   dmac_save(int fileh);
    /*
     * セーブ 
     */
    BOOL FASTCALL   dmac_load(int fileh, int ver);
    /*
     * ロード 
     */

    /*
     *      グローバル ワーク
     */
    extern WORD     dma_adr[4];
    /*
     * アドレスレジスタ 
     */
    extern WORD     dma_bcr[4];
    /*
     * 転送語数レジスタ 
     */
    extern BYTE     dma_chcr[4];
    /*
     * チャネル制御レジスタ 
     */
    extern BYTE     dma_dcr;
    /*
     * データチェイン制御レジスタ 
     */
    extern BYTE     dma_pcr;
    /*
     * 優先制御レジスタ 
     */
    extern BYTE     dma_icr;
    /*
     * 割り込み制御レジスタ 
     */
    extern BYTE     dmac_reg;
    /*
     * 現在選択されているレジスタ番号 
     */
    extern BYTE     dma_comp;
    /*
     * DMA転送完了フラグ 
     */
    extern BOOL     dma_flag;
    /*
     * DMA動作フラグ 
     */
    extern BOOL     dma_burst_transfer;
    /*
     * DMAバースト転送動作フラグ 
     */
#ifdef __cplusplus
}
#endif
#endif				/* XM7_VER >= 3 */
#endif				/* _dmac_h_ */
