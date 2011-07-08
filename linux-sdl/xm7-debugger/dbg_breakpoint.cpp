/*
 * breakpoint
 */

#include "xm7.h"

#define BRKP_NOOP  0x00000000
#define BRKP_READ  0x00000001
#define BRKP_WRITE 0x00000002
#define BRKP_FETCH 0x00000004
#define BRKP_REG   0x00000008

#define BRKP_JMP   0x00000020
#define BRKP_JSR   0x00000040
#define BRKP_UNDEF 0x00000080 // 未定義命令

#define BRKP_NMI   0x00000100
#define BRKP_IRQ   0x00000200
#define BRKP_FIRQ  0x00000400
#define BRKP_HALT  0x00000800

#define BRKP_REGA  0x000001000
#define BRKP_REGB  0x000002000
#define BRKP_REGCC 0x000004000
#define BRKP_REGDP 0x000008000
#define BRKP_REGX  0x000010000
#define BRKP_REGY  0x000020000
#define BRKP_REGS  0x000040000
#define BRKP_REGU  0x000080000
#define BRKP_PCR   0x000100000
#define BRKP_EA    0x000200000
#define BRKP_REGD  0x000400000


#define BRKP_COUNT 0x01000000
#define BRKP_AND   0x02000000
#define BRKP_OR    0x04000000
#define BRKP_XOR   0x08000000

#define BRKP_SET   0x10000000
#define BRKP_RESET 0x20000000
 struct DBG_BreakPoint {
    Uint32 addr;
    Uint32 flags;
    void *child;
 };

#define BRK_MODE_READ1  0x01
#define BRK_MODE_READ2  0x02
#define BRK_MODE_WRITE1 0x10
#define BRK_MODE_WRITE2 0x20


 BOOL DBG_BreakPoint_Fetch(struct DBG_BreakPoint *p, int fetchbytes, cpu6809_t *cpu)
 {
     Uint32 tmp;
     BOOL a = FALSE;

     if(p == NULL) return FALSE;
     if(cpu == NULL) return FALSE;

     if(p->flags & BRKP_PCR) {
         tmp = (Uint32) cpu->pc;
         if(tmp == p->addr) {
             a |= TRUE;
         } else {
             a |= FALSE;
         }
     }
     if(p->flags & BRKP_REGX) {
         tmp = (Uint32) cpu->x;
         if(tmp == p->addr) {
             a |= TRUE;
         } else {
             a |= FALSE;
         }
     }
     if(p->flags & BRKP_REGY) {
         tmp = (Uint32) cpu->y;
         if(tmp == p->addr) {
             a |= TRUE;
         } else {
             a |= FALSE;
         }
     }

     if(p->flags & BRKP_REGS) {
         tmp = (Uint32) cpu->s;
         if(tmp == p->addr) {
             a |= TRUE;
         } else {
             a |= FALSE;
         }
     }
     if(p->flags & BRKP_REGU) {
         tmp = (Uint32) cpu->u;
         if(tmp == p->addr) {
             a |= TRUE;
         } else {
             a |= FALSE;
         }
     }
     if(p->flags & BRKP_REGD) {
         tmp = ((Uint32)cpu->acc.h.a << 8) + (Uint32)cpu->acc.h.b;
         if(tmp == p->addr) {
             a |= TRUE;
         } else {
             a |= FALSE;
         }
     }
     if(p->flags & BRKP_REGA) {
         tmp = p->addr & 0x000000ff;
         if(tmp == (Uint8)cpu->acc.h.a) {
             a |= TRUE;
         } else {
             a |= FALSE;
         }
     }
     if(p->flags & BRKP_REGB) {
         tmp = p->addr & 0x000000ff;
         if(tmp == (Uint8)cpu->acc.h.b) {
             a |= TRUE;
         } else {
             a |= FALSE;
         }
     }
     if(p->flags & BRKP_REGCC) {
         tmp = p->addr & 0x000000ff;
         if(tmp == (Uint8)cpu->cc) {
             a |= TRUE;
         } else {
             a |= FALSE;
         }
     }

     if(p->flags & BRKP_EA) {
         Uint32 ea;
        // DPフラグを見る
        if(p->flags & BRKP_REGDP) {
            ea = ((Uint32)cpu->dp << 8) + (Uint32)(cpu->ea &  0x00ff);
        } else {
            ea = (Uint32)cpu->ea;
        }
        if(ea == p->addr) {
            a |= TRUE;
        } else {
            a |= FALSE;
        }
     } else {
         if(p->flags & BRKP_REGDP) {
            tmp = p->addr & 0x000000ff;
            if(tmp == (Uint8)cpu->dp) {
                a |= TRUE;
            } else {
                a |= FALSE;
            }
        }
     }
     if(p->flags & BRKP_FETCH) {
         if(fetchbytes > 0) {
             Uint32 pc1, pc2;
             Uint32 ma;
             pc1 = ((Uint32)cpu->pc) & 0xffff;
             pc2 = (pc1 + fetchbytes) & 0xffff;
             ma = (p->addr) & 0xffff;
             if(pc1 > pc2) {
                 if(pc1 <= ma) {
                     a |= TRUE;
                 } else if(pc2 > ma) {
                     a |= TRUE;
                 }
             } else {
                 if((ma >= pc1) && (ma < pc2)) {
                     a |= TRUE;
                 }
             }
         }
     }
     if(p->flags & BRKP_SET) {
             a = TRUE;
     }
     if(p->flags & BRKP_RESET) {
             a = FALSE;
     }

    /*
     * 割込は他のステータスに優先する
     */
    if(p->flags & BRKP_NMI) {
        if(cpu->intr & 0x0001){
            a |= TRUE;
        }
    }
    if(p->flags & BRKP_FIRQ) {
        if(cpu->intr & 0x0002){
            a |= TRUE;
        }
    }
    if(p->flags & BRKP_IRQ) {
        if(cpu->intr & 0x0004){
            a |= TRUE;
        }
    }

    if((p->child != NULL) && (p->child != p)){ // Loop設定の排除
         if(p->flags & BRKP_OR) {
            a |= DBG_BreakPoint_Fetch((struct DBG_BreakPoint *)(p->child), fetchbytes, cpu);
         } else if(p->flags & BRKP_AND) {
            a &= DBG_BreakPoint_Fetch((struct DBG_BreakPoint *)(p->child), fetchbytes, cpu);
         } else if(p->flags & BRKP_XOR) {
             a ^= DBG_BreakPoint_Fetch((struct DBG_BreakPoint *)(p->child), fetchbytes, cpu);
         }
     }

     return a;
 }

BOOL DBG_CheckBreakPoint_Fetch(struct DBG_BreakPoint *p[], int fetchbytes, cpu6809_t *cpu)
{
    BOOL a;
    int i;
    if(p == NULL) return FALSE;
    if(cpu == NULL) return FALSE;

    a = FALSE;
    i = 0;
    while(p[i] != NULL) {
        a |= DBG_BreakPoint_Fetch(p[i], fetchbytes, cpu);
        i++;
    }
    return a;
}


BOOL DBG_BreakPoint_Read(struct DBG_BreakPoint *p, Uint32 addr)
{
    BOOL a;
    if(p == NULL) return FALSE;

    a = FALSE;
    if(p->flags & BRKP_READ){
        if(p->addr == addr) {
            a |= TRUE;
        }
    }
    if((p->child != NULL) && (p->child != p)) {
        a |= DBG_BreakPoint_Read((struct DBG_BreakPoint *)p->child, addr);
    }
    return a;
}

BOOL DBG_CheckBreakPoint_Read(struct DBG_BreakPoint *p[], Uint32 addr)
{
    BOOL a;
    int i;

    if(p == NULL) return FALSE;
    i = 0;
    a = FALSE;
    while(p[i] != NULL) {
        a |= DBG_BreakPoint_Read(p[i], addr);
    }
    return a;
}

BOOL DBG_BreakPoint_Write(struct DBG_BreakPoint *p, Uint32 addr)
{
    BOOL a;
    if(p == NULL) return FALSE;

    a = FALSE;
    if(p->flags & BRKP_WRITE){
        if(p->addr == addr) {
            a |= TRUE;
        }
    }
    if((p->child != NULL) && (p->child != p)) {
        a |= DBG_BreakPoint_Write((struct DBG_BreakPoint *)p->child, addr);
    }
    return a;
}

BOOL DBG_CheckBreakPoint_Write(struct DBG_BreakPoint *p[], Uint32 addr)
{
    BOOL a;
    int i;

    if(p == NULL) return FALSE;
    i = 0;
    a = FALSE;
    while(p[i] != NULL) {
        a |= DBG_BreakPoint_Write(p[i], addr);
    }
    return a;
}
