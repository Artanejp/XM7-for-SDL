/*
 * FM-7 EMULATOR "XM7" Copyright (C) 2004 GIMONS [ XWIN
 * INIファイルアクセス ]
 */


#ifndef _xw_inifile_h_
#define _xw_inifile_h_

    /*
     * 定数、型定義
     */
#define MAX_LEN 256
#define MAX_ENTRIES 256

    /*
     * 初期化
     */
#ifdef __cplusplus
extern "C" {
#endif
void            INI_init(char *inifile);

    /*
     * ＩＮＩファイルのロード
     */
    BOOL INI_load(void);

    /*
     * ＩＮＩファイルへのセーブ
     */
    BOOL INI_save(void);

    /*
     * セクションとキーに一致した値を取得
     */
char           *INI_get(const char *section,const char *key);

    /*
     * セクションとキーに一致した値を設定
     */
void            INI_set(const char *section,const char *key, char *value);

    /*
     * セクションの削除
     */
void            INI_clearSection(const char *section);

    /*
     * キーの削除
     */
void            INI_clearKey(const char *section,const char *key);

    /*
     * ブーリアン値を取得
     */
    BOOL INI_getBool(const char *section,const char *key, BOOL value);

    /*
     * ブーリアン値を設定
     */
void            INI_setBool(const char *section,const char *key, BOOL value);

    /*
     * 整数値を取得
     */
int             INI_getInt(const char *section,const char *key, int defvalue);

    /*
     * 整数値を設定
     */
void            INI_setInt(const char *section,const char *key, int value);

    /*
     * 文字列を取得
     */
char           *INI_getString(const char *section,const char *key, char *defvalue);

    /*
     * 文字列を設定
     */
void            INI_setString(const char *section,const char *key, char *value);
#ifdef __cplusplus
}
#endif
#endif	/* _xw_inifile_h_ */
