/*
 * Disassembler 6809 : Debugger側Hook
 */

#include "xm7.h"
#include <stdlib.h>
#include <string.h>

struct DBG_StrRingBuf{
    Uint32 size;
    Uint32 readp;
    Uint32 writep;
    char *buf;
};

extern int FASTCALL disline(int cpu, WORD pcreg, char *buffer);


struct DBG_StrRingBuf *DBG_CreateStrRingBuf(int chars)
{
    struct DBG_StrRingBuf *p;
    if(chars <= 0) return NULL;

    p = (struct DBG_StrRingBuf *)malloc(sizeof(struct DBG_StrRingBuf));
    if(p == NULL) return NULL;

    p->buf = (char *)malloc(sizeof(char) * chars);
    if(p->buf == NULL) {
        free(p);
        return NULL;
    }
    return p;
}

void DBG_DestroyStrRingBuf(DBG_StrRingBuf *p)
{
    if(p != NULL) {
        if(p->buf != NULL) free((void *)p->buf);
        free((void *)p);
    }
}

void DBG_PushLine(char *str, struct DBG_StrRingBuf *buf)
{
     int l;

     if(str == NULL) return;
     if(buf == NULL) return;

     l = strlen(str);
     if((l + buf->writep) < (buf->size - 1)) {
     // Not Wrap
        memcpy(&(buf->buf[buf->writep]), str, l - 1);
        buf->buf[buf->writep + l] = '\n';
        buf->buf[buf->writep + l + 1] = '\0';
        buf->writep += l ;
     } else {
        // Wrap!
        int ll;
        ll = buf->size - 1 - buf->writep;
        memcpy(&(buf->buf[buf->writep]), str, ll);
        memcpy(&(buf->buf[0]), &str[ll], l - ll - 1);
        buf->buf[l - ll] = '\n';
        buf->buf[l - ll + 1] = '\0';
        buf->writep = l - ll + 1;
    }
}


void DBG_PopLine(struct DBG_StrRingBuf *buf, char *str)
{
     int i;
     int j;

     if(str == NULL) return;
     if(buf == NULL) return;

     i = buf->readp;
     j = 0;
     do {
        str[j] = buf->buf[i];
        if(str[j] == '\0') break;
        j++;
        i++;
        if(i >= buf->size) {
            i = 0;
        }
        if(j >= buf->size) {
            str[j - 1] = '\0';
            break;
        }
     } while(1);

}

/*
 *      データ読み出し
 */
static BYTE FASTCALL read_byte(int cpuno, WORD addr)
{
    BYTE            dat;

    switch (cpuno) {
    case MAINCPU:
        dat = mainmem_readbnio(addr);
	break;

    case SUBCPU:
        dat = submem_readbnio(addr);
	break;

#if (XM7_VER == 1) && defined(JSUB)
    case JSUBCPU:
        dat = jsubmem_readbnio(addr);
	break;
#endif

    default:
	ASSERT(FALSE);
	break;
    }

    return dat;
}


int DBG_DisAsm1op(int cpuno, Uint16 pc, char *s, Uint8 *membuf)
{
    int bytes;
    int i;
    if(s == NULL) return 0;
    bytes = disline(cpuno, pc, s);
    if(membuf != NULL) {
        for(i = 0; i < bytes; i++) {
            membuf[i] = read_byte(cpuno, pc + i);
        }
    }
    return bytes;
}

int DBG_DisAsm(struct DBG_StrRingBuf *sr, int cpuno, Uint16 pc, Uint8 *membuf,  int count)
{
    int bytes;
    int total;
    char str[128];
    int i;
    Uint8 *q;

    if(sr == NULL) return 0;
    if(membuf != NULL) {
        q = membuf;
    } else {
        q = NULL;
    }
    total = 0;
    for(i = 0; i < count; i++) {
        str[0] = '\0';
        bytes = DBG_DisAsm1op(cpuno, pc, str, q);
        DBG_PushLine(str, sr);
        if(q != NULL) {
            q += bytes;
        }
        total += bytes;
    }
    return total;
}
