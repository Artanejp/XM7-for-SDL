/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 2004 GIMONS  [ XWIN
 * INIファイルアクセス ] 
 */  
    
#ifdef _XWIN
    
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "xm7.h"
#ifdef USE_AGAR
#include "agar_xm7.h"
#else
#include "sdl.h"
#endif
#include "sdl_inifile.h"

    
    /*
     *  INIファイル名 
     */ 
static char     ini[MAXPATHLEN];

    /*
     *  ＩＮＩファイル中の設定項目 
     */ 
static char     entries[MAX_ENTRIES][MAX_LEN];

/*
 *  INIファイル中の項目数
 */ 
static int      max_entries = 0;
static int     current_pos = 0;

/*
 *  初期化 
 */ 
void
INI_init(char *inifile)
{
   strcpy(ini, ModuleDir);
   strcat(ini, inifile);
   max_entries = 0;
   current_pos = 0;
} 
/*
 *  ＩＮＩファイルのロード 
 */ 
BOOL INI_load(void)
{
    FILE * fp;
    char           string[MAX_LEN];
    int            n = 0;
    if ((fp = fopen(ini, "r")) == NULL) {
	return FALSE;
    }
    while ((fgets(string, MAX_LEN - 1, fp)) != NULL) {
	string[strlen(string) - 1] = '\0';
	strcpy(entries[n++], string);
    }
    fclose(fp);
    max_entries = n;
    return TRUE;
}

/*
 * ＩＮＩファイルへのセーブ 
 */ 
BOOL INI_save(void)
{
    FILE * fp;
    int            i;
    if ((fp = fopen(ini, "w")) == NULL) {
	return FALSE;
    }
    for (i = 0; i < max_entries; i++) {
	fputs(entries[i], fp);
	fputc('\n', fp);
    }
    fclose(fp);
    return TRUE;
}


/*
 *  セクションとキーに一致した値を取得 
 */ 
char *
INI_get(char *section, char *key)
{
    char           secstr[MAX_LEN];
    char           keystr[MAX_LEN];
    BOOL inner = FALSE;
    char          *line;
    int            i;
    sprintf(secstr, "[%s]", section);
    sprintf(keystr, "%s=", key);
    for (i = 0; i < max_entries; i++) {
	line = entries[i];
	if (inner) {
	    if (strncmp(keystr, line, strlen(keystr)) == 0) {
		return line + strlen(keystr);
	    } else if (line[0] == '[') {
		inner = FALSE;
	    }
	}
	if (strcmp(line, secstr) == 0)
	    inner = TRUE;
    }
    return NULL;
}


    /*
     * セクションとキーに一致した値を設定 
     */ 
void
INI_set(char *section, char *key, char *value)
{
    char           secstr[MAX_LEN];
    char           keystr[MAX_LEN];
    BOOL inner = FALSE;
    char          *line;
    int            i,
                    j;
    sprintf(secstr, "[%s]", section);
    sprintf(keystr, "%s=", key);
    for (i = 0; i < max_entries; i++) {
	line = entries[i];
	if (inner) {
	    if (strncmp(keystr, line, strlen(keystr)) == 0) {
		sprintf(entries[i], "%s%s", keystr, value);
		return;
	    } else if (line[0] == '[') {
		for (j = max_entries - 1; j >= i; j--) {
		    strcpy(entries[j + 1], entries[j]);
		}
		sprintf(entries[i], "%s%s", keystr, value);
		max_entries++;
		return;
	    }
	}
	if (strcmp(line, secstr) == 0)
	    inner = TRUE;
    }
    if (!inner) {
	strcpy(entries[max_entries], secstr);
	max_entries++;
    }
    sprintf(entries[max_entries], "%s%s", keystr, value);
    max_entries++;
}

void
debug(void)
{
    int            i;
    printf("-----%d\n", max_entries);
    for (i = 0; i < max_entries; i++) {
	printf("%s\n", entries[i]);
    }
}


    /*
     *  セクションの削除 
     */ 
    void
INI_clearSection(char *section)
{
    char           secstr[MAX_LEN];
    BOOL inner = FALSE;
    char          *line;
    int            i,
                    j;
    sprintf(secstr, "[%s]", section);
    for (i = 0; i < max_entries;) {
	line = entries[i];
	if (inner) {
	    if (line[0] == '[')
		return;
	    for (j = i; j < max_entries - 1; j++) {
		strcpy(entries[j], entries[j + 1]);
	    }
	    max_entries--;
	} else if (strcmp(line, secstr) == 0) {
	    inner = TRUE;
	    for (j = i; j < max_entries - 1; j++) {
		strcpy(entries[j], entries[j + 1]);
	    }
	    max_entries--;
	} else
	    i++;
    }
}


    /*
     *  キーの削除 
     */ 
    void
INI_clearKey(char *section, char *key)
{
    char           secstr[MAX_LEN];
    char           keystr[MAX_LEN];
    BOOL inner = FALSE;
    char          *line;
    int            i,
                    j;
    sprintf(secstr, "[%s]", section);
    sprintf(keystr, "%s=", key);
    for (i = 0; i < max_entries; i++) {
	line = entries[i];
	if (inner) {
	    if (strncmp(keystr, line, strlen(keystr)) == 0) {
		for (j = i; j < max_entries - 1; j++) {
		    strcpy(entries[j], entries[j + 1]);
		}
		max_entries--;
		return;
	    } else if (line[0] == '[') {
		return;
	    }
	}
	if (strcmp(line, secstr) == 0)
	    inner = TRUE;
    }
}


    /*
     *  ブーリアン値を取得 
     */ 
    BOOL INI_getBool(char *section, char *key, BOOL value)
{
    char          *str = INI_get(section, key);
    if (str == NULL || strcmp(str, "") == 0)
	return value;
    return strcmp(str, "0") == 0 ? FALSE : TRUE;
}


    /*
     *  ブーリアン値を設定 
     */ 
    void
INI_setBool(char *section, char *key, BOOL value)
{
    INI_set(section, key, value == TRUE ? "1" : "0");
} 
    /*
     *  整数値を取得 
     */ 
    int
INI_getInt(char *section, char *key, int defvalue)
{
    char          *str = INI_get(section, key);
    if (str == NULL)
	return defvalue;
    return atoi(str);
}


    /*
     *  整数値を設定 
     */ 
    void
INI_setInt(char *section, char *key, int value)
{
    char           str[16];
    sprintf(str, "%d", value);
    INI_set(section, key, str);
} 
    /*
     *  文字列を取得 
     */ 
char           *
INI_getString(char *section, char *key, char *defvalue)
{
    char          *str = INI_get(section, key);
    if (str == NULL)
	return defvalue;
    return str;
}


    /*
     *  文字列を設定 
     */ 
void
INI_setString(char *section, char *key, char *value)
{
    INI_set(section, key, value);
} 
#endif	/* _XWIN */
