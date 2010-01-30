/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN ファイルI/O ] 
 */  
    
#ifdef _XWIN
    
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "xm7.h"
#include "device.h"
#include "sdl.h"
    
    /*
     *  ファイルロード(ROM専用) 
     */ 
    BOOL FASTCALL file_load(char *fname, BYTE * buf, int size) 
{
    char           path[MAXPATHLEN];
    int            handle;
    
	/*
	 * assert 
	 */ 
	ASSERT(fname);
    ASSERT(buf);
    ASSERT(size > 0);
    strcpy(path, ModuleDir);
    strcat(path, fname);
    handle = open(path, O_RDONLY);
    if (handle == -1) {
	return FALSE;
    }
    if (read(handle, buf, size) != size) {
	close(handle);
	return FALSE;
    }
    close(handle);
    return TRUE;
}


    /*
     *  ファイルセーブ(学習RAM専用) 
     */ 
    BOOL FASTCALL file_save(char *fname, BYTE * buf, int size) 
{
    char           path[MAXPATHLEN];
    int            handle;
    
	/*
	 * assert 
	 */ 
	ASSERT(fname);
    ASSERT(buf);
    ASSERT(size > 0);
    strcpy(path, ModuleDir);
    strcat(path, fname);
    handle = open(path, O_CREAT | O_WRONLY, S_IREAD | S_IWRITE);
    if (handle == -1) {
	return FALSE;
    }
    if (write(handle, buf, size) != size) {
	close(handle);
	return FALSE;
    }
    close(handle);
    return TRUE;
}


    /*
     *  ファイルオープン 
     */ 
int             FASTCALL
file_open(char *fname, int mode) 
{
    
	/*
	 * assert 
	 */ 
	ASSERT(fname);
    switch (mode) {
    case OPEN_R:
	return open(fname, O_RDONLY);
	break;
    case OPEN_W:
	return open(fname, O_CREAT | O_TRUNC | O_WRONLY,
		     S_IWRITE | S_IRUSR | S_IWUSR);
	break;
    case OPEN_RW:
	return open(fname, O_RDWR);
	break;
    }
    ASSERT(FALSE);
    return -1;
}


    /*
     *  ファイルの属性を変える(Linuxなど) 
     */ 
    void
file_chmod(char *fname, int mode) 
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
     *  ファイルクローズ 
     */ 
void            FASTCALL
file_close(int handle) 
{
    
	/*
	 * assert 
	 */ 
	ASSERT(handle >= 0);
    close(handle);
} 
    /*
     *  ファイルサイズ取得 
     */ 
    DWORD FASTCALL file_getsize(int handle) 
{
    long           now;
    long           end;
    
	/*
	 * assert 
	 */ 
	ASSERT(handle >= 0);
    now = lseek(handle, 0L, SEEK_CUR);
    if (now == -1) {
	return 0;
    }
    end = lseek(handle, 0L, SEEK_END);
    lseek(handle, now, SEEK_SET);
    return end;
}


    /*
     *  ファイルシーク 
     */ 
    BOOL FASTCALL file_seek(int handle, DWORD offset) 
{
    long           now;
    
	/*
	 * assert 
	 */ 
	ASSERT(handle >= 0);
    now = lseek(handle, (off_t) offset, SEEK_SET);
    if (now != offset) {
	return FALSE;
    }
    return TRUE;
}


    /*
     *  ファイル読み出し 
     */ 
    BOOL FASTCALL file_read(int handle, BYTE * ptr, DWORD size) 
{
    unsigned int   cnt;
    
	/*
	 * assert 
	 */ 
	ASSERT(handle >= 0);
    ASSERT(ptr);
    ASSERT(size > 0);
    cnt = read(handle, ptr, size);
    if (cnt != size) {
	return FALSE;
    }
    return TRUE;
}


    /*
     *  ファイル書き込み 
     */ 
    BOOL FASTCALL file_write(int handle, BYTE * ptr, DWORD size) 
{
    unsigned int   cnt;
    
	/*
	 * assert 
	 */ 
	ASSERT(handle >= 0);
    ASSERT(ptr);
    ASSERT(size > 0);
    cnt = write(handle, ptr, size);
    if (cnt != size) {
	return FALSE;
    }
    return TRUE;
}


#endif	/* _XWIN */
