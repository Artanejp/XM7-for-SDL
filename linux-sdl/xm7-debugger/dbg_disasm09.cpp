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

