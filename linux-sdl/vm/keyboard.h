/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ キーボード ]
 */

#ifndef _keyboard_h_
#define _keyboard_h_

/*
 *      定数定義
 */
#define KEY_FORMAT_9BIT		0	/* FM-7互換 */
#define KEY_FORMAT_FM16B	1	/* FM-16β互換 */
#define KEY_FORMAT_SCAN		2	/* 物理コードモード */

#ifdef __cplusplus
extern          "C" {
#endif
    /*
     *      主要エントリ
     */
    BOOL FASTCALL   keyboard_init(void);
    /*
     * 初期化 
     */
    void FASTCALL   keyboard_cleanup(void);
    /*
     * クリーンアップ 
     */
    void FASTCALL   keyboard_reset(void);
    /*
     * リセット 
     */
    BOOL FASTCALL   keyboard_readb(WORD addr, BYTE * dat);
    /*
     * メモリ読み出し 
     */
    BOOL FASTCALL   keyboard_writeb(WORD addr, BYTE dat);
    /*
     * メモリ書き込み 
     */
    BOOL FASTCALL   keyboard_save(int fileh);
    /*
     * セーブ 
     */
    BOOL FASTCALL   keyboard_load(int fileh, int ver);
    /*
     * ロード 
     */
    void FASTCALL   keyboard_repeat(void);
    /*
     * キーリピートタイマ変更 
     */
    void FASTCALL   keyboard_make(BYTE dat);
    /*
     * キー押下 
     */
    void FASTCALL   keyboard_break(BYTE dat);
    /*
     * キー離した 
     */

    /*
     *      主要ワーク
     */
    extern BOOL     caps_flag;
    /*
     * CAPS フラグ 
     */
    extern BOOL     kana_flag;
    /*
     * カナ フラグ 
     */
    extern BOOL     ins_flag;
    /*
     * INS フラグ 
     */
    extern BOOL     shift_flag;
    /*
     * SHIFTキーフラグ 
     */
    extern BOOL     lshift_flag;
    /*
     * 左SHIFTキーフラグ 
     */
    extern BOOL     rshift_flag;
    /*
     * 右SHIFTキーフラグ 
     */
    extern BOOL     ctrl_flag;
    /*
     * CTRLキーフラグ 
     */
    extern BOOL     graph_flag;
    /*
     * GRAPHキーフラグ 
     */
    extern BOOL     break_flag;
    /*
     * Breakキーフラグ 
     */
    extern BYTE     key_scan;
    /*
     * キーコード(物理, Make/Break兼用) 
     */
    extern WORD     key_fm7;
    /*
     * キーコード(FM-7互換) 
     */
    extern BOOL     key_repeat_flag;
    /*
     * キーリピート有効フラグ 
     */

    extern BYTE     key_format;
    /*
     * コードフォーマット 
     */
    extern DWORD    key_repeat_time1;
    /*
     * キーリピート開始時間 
     */
    extern DWORD    key_repeat_time2;
    /*
     * キーリピート間隔 
     */

    extern BYTE     simpose_mode;
    /*
     * スーパーインポーズ モード 
     */
    extern BOOL     simpose_half;
    /*
     * スーパーインポーズ ハーフトーン 
     */
    extern BOOL     digitize_enable;
    /*
     * ディジタイズ有効・無効フラグ 
     */
    extern BOOL     digitize_keywait;
    /*
     * ディジタイズキー待ち 
     */
#ifdef __cplusplus
}
#endif
#endif				/* _keyboard_h_ */
