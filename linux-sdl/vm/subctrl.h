/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ サブCPUコントロール ]
 */

#ifndef _subctrl_h_
#define _subctrl_h_

#ifdef __cplusplus
extern          "C" {
#endif
    /*
     *      主要エントリ
     */
    BOOL FASTCALL   subctrl_init(void);
    /*
     * 初期化
     */
    void FASTCALL   subctrl_cleanup(void);
    /*
     * クリーンアップ
     */
    void FASTCALL   subctrl_reset(void);
    /*
     * リセット
     */
    void FASTCALL   subctrl_halt_ack(void);
    /*
     * HALT/CANCELアクノリッジ
     */
    BOOL FASTCALL   subctrl_readb(WORD addr, BYTE * dat);
    /*
     * メモリ読み出し
     */
    BOOL FASTCALL   subctrl_writeb(WORD addr, BYTE dat);
    /*
     * メモリ書き込み
     */
    BOOL FASTCALL   subctrl_save(SDL_RWops *fileh);
    /*
     * セーブ
     */
    BOOL FASTCALL   subctrl_load(SDL_RWops *fileh, int ver);
    /*
     * ロード
     */

    /*
     *      主要ワーク
     */
    extern BOOL     subhalt_flag;
    /*
     * サブHALTフラグ
     */
    extern BOOL     subbusy_flag;
    /*
     * サブBUSYフラグ
     */
    extern BOOL     subcancel_flag;
    /*
     * サブキャンセルフラグ
     */
    extern BOOL     subattn_flag;
    /*
     * サブアテンションフラグ
     */
    extern BOOL     subhalt_request;
    /*
     * サブHALTリクエストフラグ
     */
    extern BOOL     subcancel_request;
    /*
     * サブキャンセルリクエストフラグ
     */
    extern BYTE     shared_ram[0x80];
    /*
     * 共有RAM
     */
    extern BOOL     subreset_flag;
    /*
     * サブ再起動フラグ
     */
    extern BYTE     busy_CLR_count;
    /*
     * BUSY($D40A) CLR命令実行時カウンタ
     */
    extern BOOL     mode320;
    /*
     * 320x200モード
     */
#if XM7_VER >= 3
    extern BOOL     mode400l;
    /*
     * 640x400モード
     */
    extern BOOL     mode256k;
    /*
     * 26万色モード
     */
    extern BOOL     subram_protect;
    /*
     * サブモニタRAMプロテクト
     */
    extern BOOL     subreset_halt;
    /*
     * HALT中再起動フラグ
     */
    extern BOOL     subkanji_flag;
    /*
     * 漢字ROM サブCPU接続フラグ
     */
#endif
#if XM7_VER == 1 && defined(L4CARD)
    extern BOOL     select_400line;
    /*
     * 400ラインカードモード
     */
    extern BOOL     subkanji_flag;
    /*
     * 漢字ROM サブCPU接続フラグ
     */
#endif
#ifdef __cplusplus
}
#endif
#endif				/* _subctrl_h_ */
