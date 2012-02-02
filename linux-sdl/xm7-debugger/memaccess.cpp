/*
* Memory Access for Debugger
*/

#include "../vm/xm7.h"
#include "../vm/mmr.h"

#include "memdef.h"

/*
* READ FROM Memory (for Debugger)
*/
BYTE rb_main(WORD addr)
{
    WORD a;
    BYTE b;

    a = addr;

    if((a < 0xfd00) || (a>= 0xfe00)) {
#if XM7_VER >=2
        if(mmr_extbnio(&a, &b)) return b;
#endif
        // else
        return  mainmem_readbnio(a);
    }
//        return  mainmen_readbnio(a);
    a = a & 0x00ff;
    if(main_io == NULL) return 0xff;
    return main_io[a];
}

void memread_main(BYTE *buf, DWORD start, DWORD bytes)
{
    DWORD a;
    int i;

    if(bytes > 0x10000) bytes = 0x10000; // Trap
    a = start & 0x0ffff;
    for(i = 0 ; i < bytes; i++) {
        buf[i] = rb_main((WORD)a);
        a++;
        if(a > 0x10000) a -= 0x10000;
    }

}


BYTE rb_sub(WORD ADDR)
{
    WORD a;
    a = ADDR;
    if((a >= 0xd400) || (a < 0xd000)) {
        return submem_readbnio(a);
    }
    /* From vm/submem.c */
#if XM7_VER == 1
	/* $D410〜$D7FFは$D400〜$D40Fのミラー */
	a &= 0x000f;
#else
	if (fm7_ver == 1) {
		/* $D410〜$D7FFは$D400〜$D40Fのミラー */
		a &= 0x000f;
	} else 	if (fm7_ver == 2) {
        /* $D440〜$D4FFは$D400〜$D43Fのミラー */
		a &= 0x003f;
    } else { // V3
        a = a & 0x00ff;
    }
#endif
    if(sub_io == NULL) return 0xff;
    return sub_io[a];
}

void memread_sub(BYTE *buf, DWORD start, DWORD bytes)
{
    DWORD a;
    int i;

    if(bytes > 0x10000) bytes = 0x10000; // Trap
    a = start & 0x0ffff;
    for(i = 0 ; i < bytes; i++) {
        buf[i] = rb_sub((WORD)a);
        a++;
        if(a > 0x10000) a -= 0x10000;
    }

}
