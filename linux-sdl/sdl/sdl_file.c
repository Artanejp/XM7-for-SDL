/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN ファイルI/O ]
 */



#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "xm7.h"
#include "device.h"
#include <SDL/SDL_rwops.h>
#include "agar_xm7.h"

    /*
     *  ファイルロード(ROM専用)
     */
BOOL file_load(char *fname, BYTE * buf, int size)
{
    char           path[MAXPATHLEN];
//    int            handle;
    SDL_RWops *handle;
	/*
	 * assert
	 */
	ASSERT(fname);
    ASSERT(buf);
    ASSERT(size > 0);
#ifdef CONFPATH
    strcpy(path, CONFPATH);
#else
    strcpy(path, ModuleDir);
#endif
    strcat(path, fname);
    handle = SDL_RWFromFile(path, "r");
    if (handle == NULL) {
        return FALSE;
    }

    if (SDL_RWread(handle, buf, 1, size) != size) {
        SDL_RWclose(handle);
//        SDL_FreeRW(handle);
        return FALSE;
    }
    SDL_RWclose(handle);
//    SDL_FreeRW(handle);
    return TRUE;
}


    /*
     *  ファイルセーブ(学習RAM専用)
     */
BOOL file_save(char *fname, BYTE * buf, int size)
{
    char           path[MAXPATHLEN];
    SDL_RWops      *handle;

	/*
	 * assert
	 */
     ASSERT(fname);
    ASSERT(buf);
    ASSERT(size > 0);
#ifdef CONFPATH
     strcpy(path, CONFPATH);
#else
     strcpy(path, ModuleDir);
#endif
    strcat(path, fname);
    handle = SDL_RWFromFile(path, "W+");
    if (handle == NULL) {
        return FALSE;
    }
    if (SDL_RWwrite(handle, buf, 1, size) != size) {
        SDL_RWclose(handle);
  //      SDL_FreeRW(handle);
        return FALSE;
    }
    SDL_RWclose(handle);
 //   SDL_FreeRW(handle);
    return TRUE;
}


    /*
     *  ファイルオープン
     */
SDL_RWops *file_open(char *fname, int mode)
{

	/*
	 * assert
	 */
	ASSERT(fname);
    switch (mode) {
    case OPEN_R:
        return SDL_RWFromFile(fname, "rb");
        break;
    case OPEN_W:
        return SDL_RWFromFile(fname, "wb");
        break;
    case OPEN_RW:
        return SDL_RWFromFile(fname, "r+b");
        break;
	defalt:
        return NULL;
        break;
    }
}


/*
 *  ファイルの属性を変える(Linuxなど)
 */
void file_chmod(char *fname, int mode)
{
    switch (mode) {
    case OPEN_R:		/* 読み込み専用 */
        chmod(fname, S_IRUSR);
        break;
    case OPEN_W:
        chmod(fname, S_IRUSR | S_IWUSR);	/* 読み書き属性に変更する
						 */
        break;
    case OPEN_RW:
        chmod(fname, S_IRUSR | S_IWUSR);
        break;
    default:
        break;
    }
}


    /*
     *  ファイルクローズ
     */
void file_close(SDL_RWops *handle)
{

	/*
	 * assert
	 */
	ASSERT(handle == NULL);
    SDL_RWclose(handle);
}
    /*
     *  ファイルサイズ取得
     */
DWORD file_getsize(SDL_RWops *handle)
{
    long           now;
    long           end;

	/*
	 * assert
	 */
	ASSERT(handle == NULL);
    now = SDL_RWtell(handle);
    if (now == -1) {
        return 0;
    }
    end = SDL_RWseek(handle, 0L, SEEK_END);
    SDL_RWseek(handle, now, SEEK_SET);
    return end;
}


/*
 *  ファイルシーク
 */
BOOL file_seek(SDL_RWops *handle, DWORD offset)
{
    long           now;

	/*
	 * assert
	 */
	ASSERT(handle >= 0);
    now = SDL_RWseek(handle, (off_t) offset, SEEK_SET);
    if (now != offset) {
	return FALSE;
    }
    return TRUE;
}


    /*
     *  ファイル読み出し
     */
BOOL file_read(SDL_RWops *handle, BYTE * ptr, DWORD size)
{
    unsigned int   cnt;

	/*
	 * assert
	 */
    ASSERT(handle == NULL);
    ASSERT(ptr);
    ASSERT(size > 0);
    cnt = SDL_RWread(handle, ptr,  size, 1);
    if (cnt != 1) {
        return FALSE;
    }
    return TRUE;
}


    /*
     *  ファイル書き込み
     */
BOOL file_write(SDL_RWops *handle, BYTE * ptr, DWORD size)
{
    unsigned int   cnt;

	/*
	 * assert
	 */
	ASSERT(handle == NULL);
    ASSERT(ptr);
    ASSERT(size > 0);
    cnt = SDL_RWwrite(handle, ptr, size, 1);
    if (cnt != 1) {
        return FALSE;
    }
    return TRUE;
}
