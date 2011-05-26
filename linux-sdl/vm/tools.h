/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ 補助ツール ]
 */

#ifndef _tools_h_
#define _tools_h_

#ifdef __cplusplus
extern          "C" {
#endif
    /*
     *      主要エントリ
     */
    BOOL FASTCALL   make_new_d77(char *fname, char *name, BOOL mode2dd);
    /*
     * ブランクディスク作成 
     */
    BOOL FASTCALL   make_new_userdisk(char *fname, char *name,
				      BOOL mode2dd);
    /*
     * ユーザディスク作成 
     */
    BOOL FASTCALL   make_new_t77(char *fname);
    /*
     * ブランクテープ作成 
     */
    BOOL FASTCALL   conv_vfd_to_d77(char *src, char *dst, char *name);
    /*
     * VFD→D77変換 
     */
    BOOL FASTCALL   conv_2d_to_d77(char *src, char *dst, char *name);
    /*
     * 2D/2DD→D77変換 
     */
    BOOL FASTCALL   conv_vtp_to_t77(char *src, char *dst);
    /*
     * VTP→T77変換 
     */
    BOOL FASTCALL   capture_to_bmp(char *fname, BOOL fullscan);
    /*
     * 画面キャプチャ(BMP) 
     */
    BOOL FASTCALL   capture_to_bmp2(char *fname);
    /*
     * 画面キャプチャ(BMP・縮小画像) 
     */
    void FASTCALL   mix_color_init(double gamma);
    /*
     * 画像縮小カラー混合テーブル初期化 
     */
    WORD FASTCALL   mix_color(BYTE * palet_table, BYTE palet_count);
    /*
     * 画像縮小カラー混合 
     */
#ifdef __cplusplus
}
#endif
#endif				/* _tools_h_ */
