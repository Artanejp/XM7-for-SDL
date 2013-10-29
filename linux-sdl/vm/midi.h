/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2013 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2013 Ryu Takegami
 *
 *      [ MIDIアダプタ (Oh!FM掲載MIDIカード?)]
 */

#ifndef _midi_h_
#define _midi_h_

#ifdef MIDI

/*
 * ステータスレジスタ
 */
//#define	RSS_DSR			0x80
//#define	RSS_SYNDET		0x40
//#define	RSS_FRAMEERR	0x20
//#define	RSS_OVERRUN		0x10
//#define	RSS_PARITYERR	0x08
//#define	RSS_TXEMPTY		0x04
//#define	RSS_RXRDY		0x02
//#define	RSS_TXRDY		0x01

/*
 * モードコマンドレジスタ
 */
//#define	RSM_STOPBITM	0xc0
//#define	RSM_STOPBIT1	0x40
//#define	RSM_STOPBIT15	0x80
//#define	RSM_STOPBIT2	0xc0
//#define	RSM_PARITYM		0x30
//#define	RSM_PARITYEVEN	0x20
//#define	RSM_PARITYEN	0x10
//#define	RSM_CHARLENM	0x0c
//#define	RSM_CHARLEN5	0x00
//#define	RSM_CHARLEN6	0x04
//#define	RSM_CHARLEN7	0x08
//#define	RSM_CHARLEN8	0x0c
//#define	RSM_BAUDDIVM	0x03
//#define	RSM_BAUDDIV1	0x01
//#define	RSM_BAUDDIV16	0x02
//#define	RSM_BAUDDIV64	0x03

/*
 * コマンドレジスタ
 */
//#define	RSC_EH			0x80
//#define	RSC_IR			0x40
//#define	RSC_RTS			0x20
//#define	RSC_ER			0x10
//#define	RSC_SBRK		0x08
//#define	RSC_RXE			0x04
//#define	RSC_DTR			0x02
//#define	RSC_TXEN		0x01


#ifdef __cplusplus
extern          "C" {
#endif
    /*
     *      主要エントリ
     */
    BOOL FASTCALL   midi_init(void);
    /*
     * 初期化
     */
    void FASTCALL   midi_cleanup(void);
    /*
     * クリーンアップ
     */
    void FASTCALL   midi_reset(void);
    /*
     * リセット
     */
    BOOL FASTCALL   midi_readb(WORD addr, BYTE * dat);
    /*
     * メモリ読み出し
     */
    BOOL FASTCALL   midi_writeb(WORD addr, BYTE dat);
    /*
     * メモリ書き込み
     */
    BOOL FASTCALL   midi_save(SDL_RWops *fileh);
    /*
     * セーブ
     */
    BOOL FASTCALL   midi_load(SDL_RWops *fileh, int ver);
    /*
     * ロード
     */

    /*
     *      主要ワーク
     */
    extern BOOL     midi_busy;
    /*
     * MIDIバッファオーバーフロー
     */
#ifdef __cplusplus
}
#endif
#endif				/* MIDI */
#endif				/* _midi_h_ */
