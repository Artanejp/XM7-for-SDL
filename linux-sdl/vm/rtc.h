/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ リアルタイムクロック (MS58321) ]
 */

#ifndef _rtc_h_
#define _rtc_h_

#if XM7_VER >= 2

#ifdef __cplusplus
extern          "C" {
#endif
    /*
     *      主要エントリ
     */
    BOOL FASTCALL   rtc_init(void);
    /*
     * 初期化
     */
    void FASTCALL   rtc_cleanup(void);
    /*
     * クリーンアップ
     */
    void FASTCALL   rtc_reset(void);
    /*
     * リセット
     */
    void FASTCALL   rtc_time_adjust(void);
    /*
     * 時刻アジャスト
     */
    void FASTCALL   rtc_set(BYTE * packet);
    /*
     * 時計セット
     */
    void FASTCALL   rtc_get(BYTE * packet);
    /*
     * 時計取得
     */
    BOOL FASTCALL   rtc_save(SDL_RWops *fileh);
    /*
     * セーブ
     */
    BOOL FASTCALL   rtc_load(SDL_RWops *fileh, int ver);
    /*
     * ロード
     */

    /*
     *      主要ワーク
     */
    extern BYTE     rtc_year;
    /*
     * 時計 年(00〜99)
     */
    extern BYTE     rtc_month;
    /*
     * 時計 月(1〜12)
     */
    extern BYTE     rtc_day;
    /*
     * 時計 日(0〜31)
     */
    extern BYTE     rtc_week;
    /*
     * 時計 曜日(0〜6)
     */
    extern BYTE     rtc_hour;
    /*
     * 時計 時(0〜12 or 0〜24h)
     */
    extern BYTE     rtc_minute;
    /*
     * 時計 分(0〜59)
     */
    extern BYTE     rtc_second;
    /*
     * 時計 秒(0〜59)
     */
    extern BOOL     rtc_24h;
    /*
     * 時計 12h/24h切り替え
     */
    extern BOOL     rtc_pm;
    /*
     * 時計 AM/PMフラグ
     */
    extern BYTE     rtc_leap;
    /*
     * 時計 閏年判定端数
     */

#ifdef __cplusplus
}
#endif
#endif				/* XM7_VER >= 2 */
#endif				/* _rtc_h_ */
