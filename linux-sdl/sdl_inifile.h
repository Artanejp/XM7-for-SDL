/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 2004 GIMONS  [ XWIN
 * INIファイルアクセス ] 
 */  
    
#ifdef _XWIN
    
#ifndef _xw_inifile_h_
#define _xw_inifile_h_
    
    /*
     *  定数、型定義 
     */
#define MAX_LEN 256
#define MAX_ENTRIES 256
    
    /*
     *  初期化 
     */ 
void            INI_init(char *inifile);

    /*
     *  ＩＮＩファイルのロード 
     */ 
    BOOL INI_load(void);

    /*
     *  ＩＮＩファイルへのセーブ 
     */ 
    BOOL INI_save(void);

    /*
     *  セクションとキーに一致した値を取得 
     */ 
char           *INI_get(char *section, char *key);

    /*
     *  セクションとキーに一致した値を設定 
     */ 
void            INI_set(char *section, char *key, char *value);

    /*
     *  セクションの削除 
     */ 
void            INI_clearSection(char *section);

    /*
     *  キーの削除 
     */ 
void            INI_clearKey(char *section, char *key);

    /*
     *  ブーリアン値を取得 
     */ 
    BOOL INI_getBool(char *section, char *key, BOOL value);

    /*
     *  ブーリアン値を設定 
     */ 
void            INI_setBool(char *section, char *key, BOOL value);

    /*
     *  整数値を取得 
     */ 
int             INI_getInt(char *section, char *key, int defvalue);

    /*
     *  整数値を設定 
     */ 
void            INI_setInt(char *section, char *key, int value);

    /*
     *  文字列を取得 
     */ 
char           *INI_getString(char *section, char *key, char *defvalue);

    /*
     *  文字列を設定 
     */ 
void            INI_setString(char *section, char *key, char *value);

#endif	/* _xw_inifile_h_ */
#endif	/* _XWIN */
