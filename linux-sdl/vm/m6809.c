/*** m6809: Portable 6809 emulator ******************************************

    Copyright John Butler

    References:

        6809 Simulator V09, By L.C. Benschop, Eidnhoven The Netherlands.

        m6809: Portable 6809 emulator, DS (6809 code in MAME, derived from
            the 6809 Simulator V09)

        6809 Microcomputer Programming & Interfacing with Experiments"
            by Andrew C. Staugaard, Jr.; Howard W. Sams & Co., Inc.

    System dependencies:    WORD must be 16 bit unsigned int
                            BYTE must be 8 bit unsigned int
                            DWORD must be more than 16 bits
                            arrays up to 65536 bytes must be supported
                            machine must be twos complement

    History:
991026 HJB:
    Fixed missing calls to cpu_changepc() for the TFR and EXG ocpodes.
    Replaced m6809_slapstic checks by a macro (CHANGE_PC). ESB still
    needs the tweaks.

991024 HJB:
    Tried to improve speed: Using bit7 of cycles1/2 as flag for multi
    byte opcodes is gone, those opcodes now call fetch_effective_address().
    Got rid of the slow/fast flags for stack (S and U) memory accesses.
    Minor changes to use 32 bit values as arguments to memory functions
    and added defines for that purpose (e.g. X = 16bit XD = 32bit).

990312 HJB:
    Added bugfixes according to Aaron's findings.
    Reset only sets CC_II and CC_IF, DP to zero and PC from reset vector.
990311 HJB:
    Added _info functions. Now uses static m6808_Regs struct instead
    of single statics. Changed the 16 bit registers to use the generic
    PAIR union. Registers defined using macros. Split the core into
    four execution loops for M6802, M6803, M6808 and HD63701.
    TST, TSTA and TSTB opcodes reset carry flag.
    Modified the read/write stack handlers to push LSB first then MSB
    and pull MSB first then LSB.

990228 HJB:
    Changed the interrupt handling again. Now interrupts are taken
    either right at the moment the lines are asserted or whenever
    an interrupt is enabled and the corresponding line is still
    asserted. That way the pending_interrupts checks are not
    needed anymore. However, the CWAI and SYNC flags still need
    some flags, so I changed the name to 'int_state'.
    This core also has the code for the old interrupt system removed.

990225 HJB:
    Cleaned up the code here and there, added some comments.
    Slightly changed the SAR opcodes (similiar to other CPU cores).
    Added symbolic names for the flag bits.
    Changed the way CWAI/Interrupt() handle CPU state saving.
    A new flag M6809_STATE in pending_interrupts is used to determine
    if a state save is needed on interrupt entry or already done by CWAI.
    Added M6809_IRQ_LINE and M6809_FIRQ_LINE defines to m6809.h
    Moved the internal interrupt_pending flags from m6809.h to m6809.c
    Changed CWAI cycles2[0x3c] to be 2 (plus all or at least 19 if
    CWAI actually pushes the entire state).
    Implemented undocumented TFR/EXG for undefined source and mixed 8/16
    bit transfers (they should transfer/exchange the constant $ff).
    Removed unused jmp/jsr _slap functions from 6809ops.c,
    m6809_slapstick check moved into the opcode functions.

*****************************************************************************/
//#define CPU_DEBUG

//#include "debugger.h"
//#include "m6809.h"
#include "xm7.h"
#define INLINE inline

//#define BIG_SWITCH 1
/* Enable big switch statement for the main opcodes */
//#ifndef BIG_SWITCH
//#define BIG_SWITCH  1
//#endif

#define VERBOSE 0

//#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

/* 6809 Registers */

//static void check_irq_lines( cpu6809_t *m68_state );
static void IIError(cpu6809_t *m68_state);
static void cpu_execline(cpu6809_t *m68_state);
static void cpu_exec(cpu6809_t *m68_state);

INLINE void fetch_effective_address( cpu6809_t *m68_state );

/* flag bits in the cc register */
#define CC_C	0x01        /* Carry */
#define CC_V	0x02        /* Overflow */
#define CC_Z    0x04        /* Zero */
#define CC_N    0x08        /* Negative */
#define CC_II   0x10        /* Inhibit IRQ */
#define CC_H    0x20        /* Half (auxiliary) carry */
#define CC_IF   0x40        /* Inhibit FIRQ */
#define CC_E    0x80        /* entire state pushed */


#define pPC 	m68_state->pc
#define pU		m68_state->u
#define pS		m68_state->s
#define pX		m68_state->x
#define pY		m68_state->y
#define pD		m68_state->d

#define PC  	m68_state->pc
#define PCD 	m68_state->pc
#define U		m68_state->u
#define UD		m68_state->u
#define S		m68_state->s
#define SD		m68_state->s
#define X		m68_state->x
#define XD		m68_state->x
#define Y		m68_state->y
#define YD		m68_state->y
#define D   	m68_state->acc.d
#define A   	m68_state->acc.h.a
#define B		m68_state->acc.h.b
#define DP		m68_state->dp
#define DPD 	m68_state->dp
#define CC  	m68_state->cc

#define EA	m68_state->ea
#define EAD m68_state->ea
#define EAP m68_state->ea




/*
 * */

INLINE BYTE READB(cpu6809_t *t, WORD addr)
{
       return t->readmem(addr);
}


INLINE WORD READW(cpu6809_t *t, WORD addr)
{

       return (WORD)((t->readmem(addr)<<8) + t->readmem((addr+1)&0xffff));
}


INLINE void WRITEB(cpu6809_t *t, WORD addr, BYTE data)
{

      t->writemem(addr, data);
}


INLINE void WRITEW(cpu6809_t *t, WORD addr, WORD data)
{

      t->writemem(addr,(BYTE)(data >>8));
      t->writemem((addr+1)&0xffff,(BYTE)(data & 0xff));
}




/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define RM(Addr) READB(m68_state, Addr)

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define WM(Addr,Value) WRITEB(m68_state,Addr,Value)

/****************************************************************************/
/* Z80_RDOP() is identical to Z80_RDMEM() except it is used for reading     */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define ROP(Addr) READB(m68_state, Addr)

/****************************************************************************/
/* Z80_RDOP_ARG() is identical to Z80_RDOP() except it is used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define ROP_ARG(Addr) READB(m68_state, Addr)

/* macros to access memory */
#define IMMBYTE(b)	b = ROP_ARG(PCD); PC++
#define IMMWORD(w)	w = (ROP_ARG(PCD)<<8) + ROP_ARG((PC+1)&0xffff); PC+=2
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define PUSHBYTE(b) --S; WM(SD,b)
#define PUSHWORD(w) --S; WM(SD,(w&0xff)); --S; WM(SD,(w>>8))
#define PULLBYTE(b) b = RM(SD); S++
#define PULLWORD(w) w = RM(SD)<<8; S++; w |= RM(SD); S++

#define PSHUBYTE(b) --U; WM(UD,b);
#define PSHUWORD(w) --U; WM(UD,(w&0xff)); --U; WM(UD,(w>>8))
#define PULUBYTE(b) b = RM(UD); U++
#define PULUWORD(w) w = RM(UD)<<8; U++; w |= RM(UD); U++
#else
#define PUSHBYTE(b) --S; WM(SD,b)
#define PUSHWORD(w) --S; WM(SD,(w&0xff)); --S; WM(SD,(w>>8))
#define PULLBYTE(b) b = RM(SD); S++
#define PULLWORD(w) w = RM(SD)<<8; S++; w |= RM(SD); S++

#define PSHUBYTE(b) --U; WM(UD,b);
#define PSHUWORD(w) --U; WM(UD,(w&0xff)); --U; WM(UD,(w>>8))
#define PULUBYTE(b) b = RM(UD); U++
#define PULUWORD(w) w = RM(UD)<<8; U++; w |= RM(UD); U++

#endif
#define CLR_HNZVC   CC&=~(CC_H|CC_N|CC_Z|CC_V|CC_C)
#define CLR_NZV 	CC&=~(CC_N|CC_Z|CC_V)
#define CLR_NZ		CC&=~(CC_N|CC_Z)
#define CLR_HNZC	CC&=~(CC_H|CC_N|CC_Z|CC_C)
#define CLR_NZVC	CC&=~(CC_N|CC_Z|CC_V|CC_C)
#define CLR_Z		CC&=~(CC_Z)
#define CLR_NZC 	CC&=~(CC_N|CC_Z|CC_C)
#define CLR_ZC		CC&=~(CC_Z|CC_C)

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z(a)		if(!a)SEZ
#define SET_Z8(a)		SET_Z((BYTE)a)
#define SET_Z16(a)		SET_Z((WORD)a)
//#define SET_N8(a)		CC|=((a&0x80)>>4)
//#define SET_N16(a)		CC|=((a&0x8000)>>12)
#define SET_N8(a)       if(a & 0x80)CC|=CC_N
#define SET_N16(a)       if(a & 0x8000)CC|=CC_N

//#define SET_H(a,b,r)	CC|=(((a^b^r)&0x10)<<1)
#define SET_H(a,b,r)	if((a^b^r)&0x10)CC|=CC_H
//#define SET_C8(a)		CC|=((a&0x100)>>8)
//#define SET_C16(a)		CC|=((a&0x10000)>>16)
#define SET_C8(a)		if(a&0x0100)CC|=CC_C
#define SET_C16(a)		if(a&0x010000)CC|=CC_C
#define SET_V8(a,b,r)	if((a^b^r^(r>>1))&0x80)CC|=CC_V
#define SET_V16(a,b,r)	if((a^b^r^(r>>1))&0x8000)CC|=CC_V

#define SET_FLAGS8I(a)		{CC|=flags8i[a&0xff];}
#define SET_FLAGS8D(a)		{CC|=flags8d[a&0xff];}

/* combos */
#define SET_NZ8(a)			{SET_N8(a);SET_Z8(a);}
#define SET_NZ16(a)			{SET_N16(a);SET_Z16(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_FLAGS16(a,b,r)	{SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}

#define SET_HNZVC8(a,b,r)	{SET_H(a,b,r);SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_HNZVC16(a,b,r)	{SET_H(a,b,r);SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}

#define SET_NZVC8(a,b,r)	{SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_NZVC16(a,b,r)	{SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}


#define NXORV			(((CC&CC_N)^((CC&CC_V)<<2)) !=0)

/* for treating an unsigned byte as a signed word */
#define SIGNED(b) ((WORD)(b&0x80?b|0xff00:b))

/* macros for addressing modes (postbytes have their own code) */
#define DIRECT	{BYTE tmpt; EAD = DP<<8; IMMBYTE(tmpt); EAD = EAD + tmpt; }
#define IMM8	EAD = PC; PC+=1
#define IMM16	EAD = PC; PC+=2
#define EXTENDED IMMWORD(EA)

/* macros to set status flags */
#if defined(SEC)
#undef SEC
#endif
#define SEC CC|=CC_C
#define CLC CC&=~CC_C
#define SEZ CC|=CC_Z
#define CLZ CC&=~CC_Z
#define SEN CC|=CC_N
#define CLN CC&=~CC_N
#define SEV CC|=CC_V
#define CLV CC&=~CC_V
#define SEH CC|=CC_H
#define CLH CC&=~CC_H

/* macros for convenience */
#define DIRBYTE(b) {DIRECT;b=RM(EA);}
#define DIRWORD(w) {DIRECT;w=RM16(m68_state, EA);}
#define EXTBYTE(b) {EXTENDED;b=RM(EA);}
#define EXTWORD(w) {EXTENDED;w=RM16(m68_state, EA);}

/* macros for branch instructions */
INLINE void BRANCH(cpu6809_t *m68_state, int f)
{
	BYTE t;
    IMMBYTE(t);

	if( f )
	{
		if(t >= 0x80) {
			PC = PC - (0x0100-t);
		} else {
			PC = PC + t;
		}
	}
}

INLINE void LBRANCH(cpu6809_t *m68_state,int f)
 {
	WORD t;
	IMMWORD(t);
	if( f )
	{
		m68_state->cycle += 1;
		PC = (PC + t) & 0xffff;
	}
}
/* macros for setting/getting registers in TFR/EXG instructions */

INLINE WORD RM16(cpu6809_t *m68_state, WORD Addr )
{
	WORD result = RM(Addr) << 8;
        result |= RM((Addr+1)&0xffff);
        return result;
}

INLINE void WM16(cpu6809_t *m68_state, WORD Addr, WORD data )
{
   WM(Addr, (data & 0xff00)>>8 );
   WM((Addr+1)&0xffff, (data & 0xff) );
}

static void UpdateState(cpu6809_t *m68_state)
{
	/* compatibility with 6309 */
}



/****************************************************************************/
/* Reset registers to their initial values                                  */
/****************************************************************************/
static void cpu_reset(cpu6809_t *m68_state)
{
//	cpu6809_t *m68_state = get_safe_token(device);

	m68_state->intr = 0;
    D = 0;
    X = 0;
    Y = 0;
    U = 0;
    S = 0;
	DP = 0;			/* Reset direct page register */

	CC = CC_II | CC_IF;        /* IRQ + FIRQ disabled */

    m68_state->cycle = 0;
    m68_state->total = 0;
    m68_state->ea = 0;
    m68_state->intr = 0x0000;
	m68_state->pc = RM16(m68_state, 0xfffe);
#ifdef CPU_DEBUG
    printf("DEBUG: Reset %04x %02x\n",m68_state->pc ,READB(m68_state, m68_state->pc));
#endif

	UpdateState(m68_state);
}




/****************************************************************************
 * includes the actual opcode implementations
 ****************************************************************************/
#include "m6809tbl.h"

#include "m6809ops.h"

static void cpu_nmi(cpu6809_t *m68_state)
{
   //printf("NMI occured PC=0x%04x VECTOR=%04x SP=%04x \n",PC,RM16(m68_state, 0xfffc),S);
   m68_state->intr |= INTR_CWAI_OUT; /* CWAI */
   CC |= CC_E;
   PUSHWORD(pPC);
   PUSHWORD(pU);
   PUSHWORD(pY);
   PUSHWORD(pX);
   PUSHBYTE(DP);
   PUSHBYTE(B);
   PUSHBYTE(A);
   PUSHBYTE(CC);
   CC = CC | CC_II | CC_IF;  // 0x50
   PC = RM16(m68_state, 0xfffc);
   m68_state->intr &= ~INTR_NMI; // 0xfffe /* NMIクリア */
}



static void cpu_firq(cpu6809_t *m68_state)
{
//	printf("Firq occured PC=0x%04x VECTOR=%04x SP=%04x \n",PC,RM16(m68_state, 0xfff6),S);
   if( m68_state->intr & INTR_CWAI_IN) {
      /* CWAI */
      CC |= CC_E;
      m68_state->intr |= INTR_CWAI_OUT; /* CWAI */
      PUSHWORD(pPC);
      PUSHWORD(pU);
      PUSHWORD(pY);
      PUSHWORD(pX);
      PUSHBYTE(DP);
      PUSHBYTE(B);
      PUSHBYTE(A);
      PUSHBYTE(CC);
      CC = CC | CC_II | CC_IF;
      PC = RM16(m68_state, 0xfff6);
   } else {
      /* NORMAL */
      CC &= ~CC_E;
      PUSHWORD(pPC);
      PUSHBYTE(CC);
      CC = CC | CC_II | CC_IF;
      PC = RM16(m68_state, 0xfff6);
//      m68state->intr &= 0xfffd;
   }
}

static void cpu_irq(cpu6809_t *m68_state)
{
 //  printf("Irq occured PC=0x%04x VECTOR=%04x SP=%04x \n",PC,RM16(m68_state, 0xfff8),S);
   m68_state->intr |= INTR_CWAI_OUT; /* CWAI */
   CC |= CC_E;
   PUSHWORD(pPC);
   PUSHWORD(pU);
   PUSHWORD(pY);
   PUSHWORD(pX);
   PUSHBYTE(DP);
   PUSHBYTE(B);
   PUSHBYTE(A);
   PUSHBYTE(CC);
   CC |= CC_II;
   PC = RM16(m68_state, 0xfff8);
//   m68_state->intr &= 0xfffb;
}



static void cpu_exec(cpu6809_t *m68_state)
{
   WORD intr = m68_state->intr;
   WORD cycle = 0;
   BYTE cc = CC;
   /*
    * Check HALT State
    */
   if(intr & INTR_HALT) { // 0x8000
        READB(m68_state, PC);
	    m68_state->cycle = 2;
        PC++;
        return;
  }
   /*
    * Check Interrupt
    */
check_nmi:
   if((intr & (INTR_NMI | INTR_FIRQ | INTR_IRQ)) != 0) { // 0x0007
	   if((intr & INTR_NMI) == 0) goto check_firq;
	   m68_state->intr |= INTR_SYNC_OUT;
	   if((intr & INTR_SLOAD) != 0) {
		   cpu_nmi(m68_state);
		   cpu_execline(m68_state);
		   cycle = 19;
		   goto int_cycle;
	   } else {
		   goto check_firq;
	   }
	} else {
		goto check_ok;
	}

check_firq:
    if((intr & INTR_FIRQ) != 0) {
        m68_state->intr |= INTR_SYNC_OUT;
        if((cc & CC_IF) != 0) goto check_irq;
        cpu_firq(m68_state);
        cpu_execline(m68_state);
        cycle = 10;
        goto int_cycle;
    }

check_irq:
    if((intr & INTR_IRQ) != 0) {
        m68_state->intr |= INTR_SYNC_OUT;
        if((cc & CC_II) != 0) goto check_ok;
        cpu_irq(m68_state);
        cpu_execline(m68_state);
        cycle = 19;
        cc |= CC_II;
        goto int_cycle;
    }
   /*
    * NO INTERRUPT
    */
check_ok:
   cpu_execline(m68_state);
   return;
    /*
    * INTERRUPT
    */
int_cycle:
   if((m68_state->intr & INTR_CWAI_IN) == 0) {
        m68_state->cycle += cycle;
    }
    return;
}


static void cpu_execline(cpu6809_t *m68_state)
{
        BYTE ireg;
//			debugger_instruction_hook(device, PCD);
            ireg = ROP(PC);
            fetch_op = ireg;
	        PC++;
            m68_state->cycle = cycles1[ireg];
#if BIG_SWITCH
            switch( ireg )
			{
			case 0x00: neg_di(m68_state);   break;
			case 0x01: neg_di(m68_state);   break;	/* undocumented */
			case 0x02: ngc_di(m68_state);   break; /* undocumented */
			case 0x03: com_di(m68_state);   break;
			case 0x04: lsr_di(m68_state);   break;
			case 0x05: lsr_di(m68_state);   break; /* Undefined */
			case 0x06: ror_di(m68_state);   break;
			case 0x07: asr_di(m68_state);   break;
			case 0x08: asl_di(m68_state);   break;
			case 0x09: rol_di(m68_state);   break;
			case 0x0a: dec_di(m68_state);   break;
			case 0x0b: dcc_di(m68_state);   break; /* undocumented */
			case 0x0c: inc_di(m68_state);   break;
			case 0x0d: tst_di(m68_state);   break;
			case 0x0e: jmp_di(m68_state);   break;
			case 0x0f: clr_di(m68_state);   break;
			case 0x10: pref10(m68_state);   break;
			case 0x11: pref11(m68_state);   break;
			case 0x12: nop(m68_state);      break;
			case 0x13: sync_09(m68_state);  break; // Rename 20101110
			case 0x14: trap(m68_state);     break;
			case 0x15: trap(m68_state);     break;
			case 0x16: lbra(m68_state);     break;
			case 0x17: lbsr(m68_state);     break;
			case 0x18: aslcc_in(m68_state); break; /* undocumented */
			case 0x19: daa(m68_state);      break;
			case 0x1a: orcc(m68_state);     break;
			case 0x1b: nop(m68_state);      break; /* undocumented */
			case 0x1c: andcc(m68_state);    break;
			case 0x1d: sex(m68_state);      break;
			case 0x1e: exg(m68_state);      break;
			case 0x1f: tfr(m68_state);      break;
			case 0x20: bra(m68_state);      break;
			case 0x21: brn(m68_state);      break;
			case 0x22: bhi(m68_state);      break;
			case 0x23: bls(m68_state);	    break;
			case 0x24: bcc(m68_state);	    break;
			case 0x25: bcs(m68_state);	    break;
			case 0x26: bne(m68_state);	    break;
			case 0x27: beq(m68_state);	    break;
			case 0x28: bvc(m68_state);	    break;
			case 0x29: bvs(m68_state);	    break;
			case 0x2a: bpl(m68_state);	    break;
			case 0x2b: bmi(m68_state);	    break;
			case 0x2c: bge(m68_state);	    break;
			case 0x2d: blt(m68_state);	    break;
			case 0x2e: bgt(m68_state);	    break;
			case 0x2f: ble(m68_state);	    break;
			case 0x30: leax(m68_state);	    break;
			case 0x31: leay(m68_state);	    break;
			case 0x32: leas(m68_state);	    break;
			case 0x33: leau(m68_state);	    break;
			case 0x34: pshs(m68_state);	    break;
			case 0x35: puls(m68_state);	    break;
			case 0x36: pshu(m68_state);	    break;
			case 0x37: pulu(m68_state);	    break;
			case 0x38: andcc(m68_state);   break; /* undocumented */
			case 0x39: rts(m68_state);	    break;
			case 0x3a: abx(m68_state);	    break;
			case 0x3b: rti(m68_state);	    break;
			case 0x3c: cwai(m68_state);	    break;
			case 0x3d: mul(m68_state);	    break;
			case 0x3e: rst(m68_state);   break; /* undocumented */
			case 0x3f: swi(m68_state);	    break;
			case 0x40: nega(m68_state);	    break;
			case 0x41: nega(m68_state);   break; /* undocumented */
			case 0x42: ngca(m68_state);   break; /* undocumented */
			case 0x43: coma(m68_state);	    break;
			case 0x44: lsra(m68_state);	    break;
			case 0x45: lsra(m68_state);   break; /* undocumented */
			case 0x46: rora(m68_state);	    break;
			case 0x47: asra(m68_state);	    break;
			case 0x48: asla(m68_state);	    break;
			case 0x49: rola(m68_state);	    break;
			case 0x4a: deca(m68_state);	    break;
			case 0x4b: dcca(m68_state);   break; /* undocumented */
			case 0x4c: inca(m68_state);	    break;
			case 0x4d: tsta(m68_state);	    break;
			case 0x4e: clca(m68_state);   break; /* undocumented */
			case 0x4f: clra(m68_state);	    break;
			case 0x50: negb(m68_state);	    break;
			case 0x51: negb(m68_state);   break; /* undocumented */
			case 0x52: ngcb(m68_state);   break; /* undocumented */
			case 0x53: comb(m68_state);	    break;
			case 0x54: lsrb(m68_state);	    break;
			case 0x55: lsrb(m68_state);   break; /* undocumented */
			case 0x56: rorb(m68_state);	    break;
			case 0x57: asrb(m68_state);	    break;
			case 0x58: aslb(m68_state);	    break;
			case 0x59: rolb(m68_state);	    break;
			case 0x5a: decb(m68_state);	    break;
			case 0x5b: dccb(m68_state);   break; /* undocumented */
			case 0x5c: incb(m68_state);	    break;
			case 0x5d: tstb(m68_state);	    break;
			case 0x5e: clcb(m68_state);   break; /* undocumented */
			case 0x5f: clrb(m68_state);	    break;
			case 0x60: neg_ix(m68_state);    break;
			case 0x61: neg_ix(m68_state);   break;/* undocumented */
			case 0x62: ngc_ix(m68_state);   break;/* undocumented */
			case 0x63: com_ix(m68_state);    break;
			case 0x64: lsr_ix(m68_state);    break;
			case 0x65: lsr_ix(m68_state);   break;/* undocumented */
			case 0x66: ror_ix(m68_state);    break;
			case 0x67: asr_ix(m68_state);    break;
			case 0x68: asl_ix(m68_state);    break;
			case 0x69: rol_ix(m68_state);    break;
			case 0x6a: dec_ix(m68_state);    break;
			case 0x6b: dcc_ix(m68_state);   break;/* undocumented */
			case 0x6c: inc_ix(m68_state);    break;
			case 0x6d: tst_ix(m68_state);    break;
			case 0x6e: jmp_ix(m68_state);    break;
			case 0x6f: clr_ix(m68_state);    break;
			case 0x70: neg_ex(m68_state);    break;
			case 0x71: neg_ex(m68_state);   break;/* undocumented */
			case 0x72: ngc_ex(m68_state);   break;/* undocumented */
			case 0x73: com_ex(m68_state);    break;
			case 0x74: lsr_ex(m68_state);    break;
			case 0x75: lsr_ex(m68_state);   break;/* undocumented */
			case 0x76: ror_ex(m68_state);    break;
			case 0x77: asr_ex(m68_state);    break;
			case 0x78: asl_ex(m68_state);    break;
			case 0x79: rol_ex(m68_state);    break;
			case 0x7a: dec_ex(m68_state);    break;
			case 0x7b: dcc_ex(m68_state);   break;/* undocumented */
			case 0x7c: inc_ex(m68_state);    break;
			case 0x7d: tst_ex(m68_state);    break;
			case 0x7e: jmp_ex(m68_state);    break;
			case 0x7f: clr_ex(m68_state);    break;
			case 0x80: suba_im(m68_state);   break;
			case 0x81: cmpa_im(m68_state);   break;
			case 0x82: sbca_im(m68_state);   break;
			case 0x83: subd_im(m68_state);   break;
			case 0x84: anda_im(m68_state);   break;
			case 0x85: bita_im(m68_state);   break;
			case 0x86: lda_im(m68_state);    break;
			case 0x87: flag8_im(m68_state); break;/* undocumented *///sta_im(m68_state);    break;
			case 0x88: eora_im(m68_state);   break;
			case 0x89: adca_im(m68_state);   break;
			case 0x8a: ora_im(m68_state);    break;
			case 0x8b: adda_im(m68_state);   break;
			case 0x8c: cmpx_im(m68_state);   break;
			case 0x8d: bsr(m68_state);	    break;
			case 0x8e: ldx_im(m68_state);    break;
		        case 0x8f: flag16_im(m68_state); break; /* undocumented */ //stx_im(m68_state);    break;
			case 0x90: suba_di(m68_state);   break;
			case 0x91: cmpa_di(m68_state);   break;
			case 0x92: sbca_di(m68_state);   break;
			case 0x93: subd_di(m68_state);   break;
			case 0x94: anda_di(m68_state);   break;
			case 0x95: bita_di(m68_state);   break;
			case 0x96: lda_di(m68_state);    break;
			case 0x97: sta_di(m68_state);    break;
			case 0x98: eora_di(m68_state);   break;
			case 0x99: adca_di(m68_state);   break;
			case 0x9a: ora_di(m68_state);    break;
			case 0x9b: adda_di(m68_state);   break;
			case 0x9c: cmpx_di(m68_state);   break;
			case 0x9d: jsr_di(m68_state);    break;
			case 0x9e: ldx_di(m68_state);    break;
			case 0x9f: stx_di(m68_state);    break;
			case 0xa0: suba_ix(m68_state);   break;
			case 0xa1: cmpa_ix(m68_state);   break;
			case 0xa2: sbca_ix(m68_state);   break;
			case 0xa3: subd_ix(m68_state);   break;
			case 0xa4: anda_ix(m68_state);   break;
			case 0xa5: bita_ix(m68_state);   break;
			case 0xa6: lda_ix(m68_state);    break;
			case 0xa7: sta_ix(m68_state);    break;
			case 0xa8: eora_ix(m68_state);   break;
			case 0xa9: adca_ix(m68_state);   break;
			case 0xaa: ora_ix(m68_state);    break;
			case 0xab: adda_ix(m68_state);   break;
			case 0xac: cmpx_ix(m68_state);   break;
			case 0xad: jsr_ix(m68_state);    break;
			case 0xae: ldx_ix(m68_state);    break;
			case 0xaf: stx_ix(m68_state);    break;
			case 0xb0: suba_ex(m68_state);   break;
			case 0xb1: cmpa_ex(m68_state);   break;
			case 0xb2: sbca_ex(m68_state);   break;
			case 0xb3: subd_ex(m68_state);   break;
			case 0xb4: anda_ex(m68_state);   break;
			case 0xb5: bita_ex(m68_state);   break;
			case 0xb6: lda_ex(m68_state);    break;
			case 0xb7: sta_ex(m68_state);    break;
			case 0xb8: eora_ex(m68_state);   break;
			case 0xb9: adca_ex(m68_state);   break;
			case 0xba: ora_ex(m68_state);    break;
			case 0xbb: adda_ex(m68_state);   break;
			case 0xbc: cmpx_ex(m68_state);   break;
			case 0xbd: jsr_ex(m68_state);    break;
			case 0xbe: ldx_ex(m68_state);    break;
			case 0xbf: stx_ex(m68_state);    break;
			case 0xc0: subb_im(m68_state);   break;
			case 0xc1: cmpb_im(m68_state);   break;
			case 0xc2: sbcb_im(m68_state);   break;
			case 0xc3: addd_im(m68_state);   break;
			case 0xc4: andb_im(m68_state);   break;
			case 0xc5: bitb_im(m68_state);   break;
			case 0xc6: ldb_im(m68_state);    break;
			case 0xc7: flag8_im(m68_state); break; /* undocumented *///stb_im(m68_state);    break;
			case 0xc8: eorb_im(m68_state);   break;
			case 0xc9: adcb_im(m68_state);   break;
			case 0xca: orb_im(m68_state);    break;
			case 0xcb: addb_im(m68_state);   break;
			case 0xcc: ldd_im(m68_state);    break;
			case 0xcd: trap(m68_state); break; /* undocumented */ //std_im(m68_state);    break;
			case 0xce: ldu_im(m68_state);    break;
			case 0xcf: flag16_im(m68_state); break; /* undocumented */ //stu_im(m68_state);    break;
			case 0xd0: subb_di(m68_state);   break;
			case 0xd1: cmpb_di(m68_state);   break;
			case 0xd2: sbcb_di(m68_state);   break;
			case 0xd3: addd_di(m68_state);   break;
			case 0xd4: andb_di(m68_state);   break;
			case 0xd5: bitb_di(m68_state);   break;
			case 0xd6: ldb_di(m68_state);    break;
			case 0xd7: stb_di(m68_state);    break;
			case 0xd8: eorb_di(m68_state);   break;
			case 0xd9: adcb_di(m68_state);   break;
			case 0xda: orb_di(m68_state);    break;
			case 0xdb: addb_di(m68_state);   break;
			case 0xdc: ldd_di(m68_state);    break;
			case 0xdd: std_di(m68_state);    break;
			case 0xde: ldu_di(m68_state);    break;
			case 0xdf: stu_di(m68_state);    break;
			case 0xe0: subb_ix(m68_state);   break;
			case 0xe1: cmpb_ix(m68_state);   break;
			case 0xe2: sbcb_ix(m68_state);   break;
			case 0xe3: addd_ix(m68_state);   break;
			case 0xe4: andb_ix(m68_state);   break;
			case 0xe5: bitb_ix(m68_state);   break;
			case 0xe6: ldb_ix(m68_state);    break;
			case 0xe7: stb_ix(m68_state);    break;
			case 0xe8: eorb_ix(m68_state);   break;
			case 0xe9: adcb_ix(m68_state);   break;
			case 0xea: orb_ix(m68_state);    break;
			case 0xeb: addb_ix(m68_state);   break;
			case 0xec: ldd_ix(m68_state);    break;
			case 0xed: std_ix(m68_state);    break;
			case 0xee: ldu_ix(m68_state);    break;
			case 0xef: stu_ix(m68_state);    break;
			case 0xf0: subb_ex(m68_state);   break;
			case 0xf1: cmpb_ex(m68_state);   break;
			case 0xf2: sbcb_ex(m68_state);   break;
			case 0xf3: addd_ex(m68_state);   break;
			case 0xf4: andb_ex(m68_state);   break;
			case 0xf5: bitb_ex(m68_state);   break;
			case 0xf6: ldb_ex(m68_state);    break;
			case 0xf7: stb_ex(m68_state);    break;
			case 0xf8: eorb_ex(m68_state);   break;
			case 0xf9: adcb_ex(m68_state);   break;
			case 0xfa: orb_ex(m68_state);    break;
			case 0xfb: addb_ex(m68_state);   break;
			case 0xfc: ldd_ex(m68_state);    break;
			case 0xfd: std_ex(m68_state);    break;
			case 0xfe: ldu_ex(m68_state);    break;
			case 0xff: stu_ex(m68_state);    break;
			}
#else
            		(*m6809_main[ireg])(m68_state);
#endif    /* BIG_SWITCH */

//    return cycles ;   /* NS 970908 */
}
#if 1
// fetch_effective_address ($80-$FF)
INLINE void fetchsub_IDX(cpu6809_t *m68_state, WORD upper, WORD lower)
{
   WORD indirect = (upper & 0x01);
   WORD *reg;
   
   switch((upper >> 1) & 0x03){ // $8-$f >> 1 = $4 - $7 : delete bit2 
    case 0: // $8x,$9x
      reg = &(X);
      break;
    case 1: // $ax,$bx
      reg = &(Y);
      break;
    case 2: // $cx,$dx
      reg = &(U);
      break;
    case 3: // $ex,$fx
      reg = &(S);
      break;
   }
   
   switch(lower) {
    case 0: // ,r+ 
      EA = *reg;
      *reg = *reg + 1;
      break;
    case 1: // ,r++
      EA = *reg;
      *reg = *reg + 2;
      break;
    case 2: // ,-r
      *reg = *reg - 1;
      EA = *reg;
      break;
    case 3: // ,--r
      *reg = *reg - 2;
      EA = *reg;
      break;
    case 4: // ,r
      EA = *reg;
      break;
    case 5: // b,r
      EA = *reg + SIGNED(B);
      break;
    case 6: // a,r
    case 7:
      EA = *reg + SIGNED(A);
      break;
    case 8: // $xx,r
      IMMBYTE(EA);
      EA = *reg + SIGNED(EA);
      break;
    case 9: // $xxxx, r
      IMMWORD(EAP);
      EA = EA + *reg;
      break;
    case 0x0a: // Undocumented
      EA = PC;
      EA++;
      EA |= 0x00ff;
      break;
    case 0x0b: // D,r
      EA = *reg + D;
      break;
    case 0x0c: // xx,pc
      IMMBYTE(EA);
      EA = PC + SIGNED(EA);
      break;
    case 0x0d: // xxxx,pc
      IMMWORD(EAP);
      EA = EA + PC;
      break;
    case 0x0e: // Undocumented
      EA = 0xffff;
      break;
    case 0x0f:
      IMMWORD(EAP);
      break;
   }
   // $9x,$bx,$dx,$fx = INDIRECT
   if(indirect != 0) {
      EAD = RM16(m68_state, EAD);
   }
}

// fetch_effective_address ($00-$7F)
INLINE void fetch_effective_address( cpu6809_t *m68_state )
{
   BYTE postbyte;
   WORD upper,lower;
   
   IMMBYTE(postbyte);
   upper = (postbyte >> 4) & 0x0f;
   lower = postbyte & 0x0f;
   switch(upper){
    case 0x00:
      EA = X + lower;
      break;
    case 0x01:
      EA = X - 16 + lower;
      break;
    case 0x02:
      EA = Y + lower;
      break;
    case 0x03:
      EA = Y - 16 + lower;
      break;
    case 0x04:
      EA = U + lower;
      break;
    case 0x05:
      EA = U - 16 + lower;
      break;
    case 0x06:
      EA = S + lower;
      break;
    case 0x07:
      EA = S - 16 + lower;
      break;
    default:
      fetchsub_IDX(m68_state, upper, lower);
      break;
   }
   m68_state->cycle += index_cycle_em[postbyte];
   
}

#else
INLINE void fetch_effective_address( cpu6809_t *m68_state )
{
	BYTE postbyte;
	IMMBYTE(postbyte);
	switch(postbyte)
	{
	case 0x00: EA=X;												   break;
	case 0x01: EA=X+1;												   break;
	case 0x02: EA=X+2;												   break;
	case 0x03: EA=X+3;												   break;
	case 0x04: EA=X+4;												   break;
	case 0x05: EA=X+5;												   break;
	case 0x06: EA=X+6;												   break;
	case 0x07: EA=X+7;												   break;
	case 0x08: EA=X+8;												   break;
	case 0x09: EA=X+9;												   break;
	case 0x0a: EA=X+10; 											   break;
	case 0x0b: EA=X+11; 											   break;
	case 0x0c: EA=X+12; 											   break;
	case 0x0d: EA=X+13; 											   break;
	case 0x0e: EA=X+14; 											   break;
	case 0x0f: EA=X+15; 											   break;

	case 0x10: EA=X-16; 											   break;
	case 0x11: EA=X-15; 											   break;
	case 0x12: EA=X-14; 											   break;
	case 0x13: EA=X-13; 											   break;
	case 0x14: EA=X-12; 											   break;
	case 0x15: EA=X-11; 											   break;
	case 0x16: EA=X-10; 											   break;
	case 0x17: EA=X-9;												   break;
	case 0x18: EA=X-8;												   break;
	case 0x19: EA=X-7;												   break;
	case 0x1a: EA=X-6;												   break;
	case 0x1b: EA=X-5;												   break;
	case 0x1c: EA=X-4;												   break;
	case 0x1d: EA=X-3;												   break;
	case 0x1e: EA=X-2;												   break;
	case 0x1f: EA=X-1;												   break;

	case 0x20: EA=Y;												   break;
	case 0x21: EA=Y+1;												   break;
	case 0x22: EA=Y+2;												   break;
	case 0x23: EA=Y+3;												   break;
	case 0x24: EA=Y+4;												   break;
	case 0x25: EA=Y+5;												   break;
	case 0x26: EA=Y+6;												   break;
	case 0x27: EA=Y+7;												   break;
	case 0x28: EA=Y+8;												   break;
	case 0x29: EA=Y+9;												   break;
	case 0x2a: EA=Y+10; 											   break;
	case 0x2b: EA=Y+11; 											   break;
	case 0x2c: EA=Y+12; 											   break;
	case 0x2d: EA=Y+13; 											   break;
	case 0x2e: EA=Y+14; 											   break;
	case 0x2f: EA=Y+15; 											   break;

	case 0x30: EA=Y-16; 											   break;
	case 0x31: EA=Y-15; 											   break;
	case 0x32: EA=Y-14; 											   break;
	case 0x33: EA=Y-13; 											   break;
	case 0x34: EA=Y-12; 											   break;
	case 0x35: EA=Y-11; 											   break;
	case 0x36: EA=Y-10; 											   break;
	case 0x37: EA=Y-9;												   break;
	case 0x38: EA=Y-8;												   break;
	case 0x39: EA=Y-7;												   break;
	case 0x3a: EA=Y-6;												   break;
	case 0x3b: EA=Y-5;												   break;
	case 0x3c: EA=Y-4;												   break;
	case 0x3d: EA=Y-3;												   break;
	case 0x3e: EA=Y-2;												   break;
	case 0x3f: EA=Y-1;												   break;

	case 0x40: EA=U;												   break;
	case 0x41: EA=U+1;												   break;
	case 0x42: EA=U+2;												   break;
	case 0x43: EA=U+3;												   break;
	case 0x44: EA=U+4;												   break;
	case 0x45: EA=U+5;												   break;
	case 0x46: EA=U+6;												   break;
	case 0x47: EA=U+7;												   break;
	case 0x48: EA=U+8;												   break;
	case 0x49: EA=U+9;												   break;
	case 0x4a: EA=U+10; 											   break;
	case 0x4b: EA=U+11; 											   break;
	case 0x4c: EA=U+12; 											   break;
	case 0x4d: EA=U+13; 											   break;
	case 0x4e: EA=U+14; 											   break;
	case 0x4f: EA=U+15; 											   break;

	case 0x50: EA=U-16; 											   break;
	case 0x51: EA=U-15; 											   break;
	case 0x52: EA=U-14; 											   break;
	case 0x53: EA=U-13; 											   break;
	case 0x54: EA=U-12; 											   break;
	case 0x55: EA=U-11; 											   break;
	case 0x56: EA=U-10; 											   break;
	case 0x57: EA=U-9;												   break;
	case 0x58: EA=U-8;												   break;
	case 0x59: EA=U-7;												   break;
	case 0x5a: EA=U-6;												   break;
	case 0x5b: EA=U-5;												   break;
	case 0x5c: EA=U-4;												   break;
	case 0x5d: EA=U-3;												   break;
	case 0x5e: EA=U-2;												   break;
	case 0x5f: EA=U-1;												   break;

	case 0x60: EA=S;												   break;
	case 0x61: EA=S+1;												   break;
	case 0x62: EA=S+2;												   break;
	case 0x63: EA=S+3;												   break;
	case 0x64: EA=S+4;												   break;
	case 0x65: EA=S+5;												   break;
	case 0x66: EA=S+6;												   break;
	case 0x67: EA=S+7;												   break;
	case 0x68: EA=S+8;												   break;
	case 0x69: EA=S+9;												   break;
	case 0x6a: EA=S+10; 											   break;
	case 0x6b: EA=S+11; 											   break;
	case 0x6c: EA=S+12; 											   break;
	case 0x6d: EA=S+13; 											   break;
	case 0x6e: EA=S+14; 											   break;
	case 0x6f: EA=S+15; 											   break;

	case 0x70: EA=S-16; 											   break;
	case 0x71: EA=S-15; 											   break;
	case 0x72: EA=S-14; 											   break;
	case 0x73: EA=S-13; 											   break;
	case 0x74: EA=S-12; 											   break;
	case 0x75: EA=S-11; 											   break;
	case 0x76: EA=S-10; 											   break;
	case 0x77: EA=S-9;												   break;
	case 0x78: EA=S-8;												   break;
	case 0x79: EA=S-7;												   break;
	case 0x7a: EA=S-6;												   break;
	case 0x7b: EA=S-5;												   break;
	case 0x7c: EA=S-4;												   break;
	case 0x7d: EA=S-3;												   break;
	case 0x7e: EA=S-2;												   break;
	case 0x7f: EA=S-1;												   break;

	case 0x80: EA=X;	X++;										   break;
	case 0x81: EA=X;	X+=2;										   break;
	case 0x82: X--; 	EA=X;										   break;
	case 0x83: X-=2;	EA=X;										   break;
	case 0x84: EA=X;												   break;
	case 0x85: EA=X+SIGNED(B);										   break;
	case 0x86: EA=X+SIGNED(A);										   break;
	case 0x87: EA=X+SIGNED(A);												   break; /*   ILLEGAL*/
	case 0x88: IMMBYTE(EA); 	EA=X+SIGNED(EA);					   break; /* this is a hack to make Vectrex work. It should be m68_state->icount-=1. Dunno where the cycle was lost :( */
	case 0x89: IMMWORD(EAP);	EA+=X;								   break;
	case 0x8a: EA=PC; EA++ ; EA |= 0x00ff;							   break; /*   IIError*/
	case 0x8b: EA=X+D;												   break;
	case 0x8c: IMMBYTE(EA); 	EA=PC+SIGNED(EA);					   break;
	case 0x8d: IMMWORD(EAP);	EA+=PC; 							   break;
	case 0x8e: EA=0xffff;											   break; /*   ILLEGAL*/
	case 0x8f: IMMWORD(EAP);										   break;
	case 0x90: EA=X;	X++;						EAD=RM16(m68_state, EAD);	   break; /* Indirect ,R+ not in my specs */
	case 0x91: EA=X;	X+=2;						EAD=RM16(m68_state, EAD);	   break;
	case 0x92: X--; 	EA=X;						EAD=RM16(m68_state, EAD);	   break;
	case 0x93: X-=2;	EA=X;						EAD=RM16(m68_state, EAD);	   break;
	case 0x94: EA=X;								EAD=RM16(m68_state, EAD);	   break;
	case 0x95: EA=X+SIGNED(B);						EAD=RM16(m68_state, EAD);	   break;
	case 0x96: EA=X+SIGNED(A);						EAD=RM16(m68_state, EAD);	   break;
	case 0x97: EA=X+SIGNED(A);						EAD=RM16(m68_state, EAD);	   break; /*   ILLEGAL*/
	case 0x98: IMMBYTE(EA); 	EA=X+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0x99: IMMWORD(EAP);	EA+=X;				EAD=RM16(m68_state, EAD);	   break;
	case 0x9a: EA=PC; EA++ ; EA |= 0x00ff; EAD=RM16(m68_state,EAD);				   break; /*   ILLEGAL*/
	case 0x9b: EA=X+D;								EAD=RM16(m68_state, EAD);	   break;
	case 0x9c: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0x9d: IMMWORD(EAP);	EA+=PC; 			EAD=RM16(m68_state, EAD);	   break;
	case 0x9e: EA=0xffff;	    EAD=RM16(m68_state, EAD);						   break; /*   ILLEGAL*/
	case 0x9f: IMMWORD(EAP);						EAD=RM16(m68_state, EAD);	   break;

	case 0xa0: EA=Y;	Y++;										   break;
	case 0xa1: EA=Y;	Y+=2;										   break;
	case 0xa2: Y--; 	EA=Y;										   break;
	case 0xa3: Y-=2;	EA=Y;										   break;
	case 0xa4: EA=Y;												   break;
	case 0xa5: EA=Y+SIGNED(B);										   break;
	case 0xa6: EA=Y+SIGNED(A);										   break;
	case 0xa7: EA=Y+SIGNED(A);												   break; /*   ILLEGAL*/
	case 0xa8: IMMBYTE(EA); 	EA=Y+SIGNED(EA);					   break;
	case 0xa9: IMMWORD(EAP);	EA+=Y;								   break;
	case 0xaa: EA=PC; EA++ ; EA |= 0x00ff;							   break; /*   ILLEGAL*/
	case 0xab: EA=Y+D;												   break;
	case 0xac: IMMBYTE(EA); 	EA=PC+SIGNED(EA);					   break;
	case 0xad: IMMWORD(EAP);	EA+=PC; 							   break;
	case 0xae: EA=0xffff;												   break; /*   ILLEGAL*/
	case 0xaf: IMMWORD(EAP);										   break;

	case 0xb0: EA=Y;	Y++;						EAD=RM16(m68_state, EAD);	   break;
	case 0xb1: EA=Y;	Y+=2;						EAD=RM16(m68_state, EAD);	   break;
	case 0xb2: Y--; 	EA=Y;						EAD=RM16(m68_state, EAD);	   break;
	case 0xb3: Y-=2;	EA=Y;						EAD=RM16(m68_state, EAD);	   break;
	case 0xb4: EA=Y;								EAD=RM16(m68_state, EAD);	   break;
	case 0xb5: EA=Y+SIGNED(B);						EAD=RM16(m68_state, EAD);	   break;
	case 0xb6: EA=Y+SIGNED(A);						EAD=RM16(m68_state, EAD);	   break;
	case 0xb7: EA=Y+SIGNED(A);						EAD=RM16(m68_state, EAD);	   break; /*   ILLEGAL*/
	case 0xb8: IMMBYTE(EA); 	EA=Y+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0xb9: IMMWORD(EAP);	EA+=Y;				EAD=RM16(m68_state, EAD);	   break;
	case 0xba: EA=PC; EA++ ; EA |= 0x00ff;			EAD=RM16(m68_state, EAD);												   break; /*   ILLEGAL*/
	case 0xbb: EA=Y+D;								EAD=RM16(m68_state, EAD);	   break;
	case 0xbc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0xbd: IMMWORD(EAP);	EA+=PC; 			EAD=RM16(m68_state, EAD);	   break;
	case 0xbe: EA=0xffff;							EAD=RM16(m68_state, EAD);	   break; /*   ILLEGAL*/
	case 0xbf: IMMWORD(EAP);						EAD=RM16(m68_state, EAD);	   break;

	case 0xc0: EA=U;			U++;								   break;
	case 0xc1: EA=U;			U+=2;								   break;
	case 0xc2: U--; 			EA=U;								   break;
	case 0xc3: U-=2;			EA=U;								   break;
	case 0xc4: EA=U;												   break;
	case 0xc5: EA=U+SIGNED(B);										   break;
	case 0xc6: EA=U+SIGNED(A);										   break;
	case 0xc7: EA=U+SIGNED(A);										   break; /*ILLEGAL*/
	case 0xc8: IMMBYTE(EA); 	EA=U+SIGNED(EA);					   break;
	case 0xc9: IMMWORD(EAP);	EA+=U;								   break;
	case 0xca: EA=PC; EA++ ; EA |= 0x00ff;							   break; /*ILLEGAL*/
	case 0xcb: EA=U+D;												   break;
	case 0xcc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);					   break;
	case 0xcd: IMMWORD(EAP);	EA+=PC; 							   break;
	case 0xce: EA=0xffff;											   break; /*ILLEGAL*/
	case 0xcf: IMMWORD(EAP);										   break;

	case 0xd0: EA=U;	U++;						EAD=RM16(m68_state, EAD);	   break;
	case 0xd1: EA=U;	U+=2;						EAD=RM16(m68_state, EAD);	   break;
	case 0xd2: U--; 	EA=U;						EAD=RM16(m68_state, EAD);	   break;
	case 0xd3: U-=2;	EA=U;						EAD=RM16(m68_state, EAD);	   break;
	case 0xd4: EA=U;								EAD=RM16(m68_state, EAD);	   break;
	case 0xd5: EA=U+SIGNED(B);						EAD=RM16(m68_state, EAD);	   break;
	case 0xd6: EA=U+SIGNED(A);						EAD=RM16(m68_state, EAD);	   break;
	case 0xd7: EA=U+SIGNED(A);						EAD=RM16(m68_state, EAD);	   break; /*ILLEGAL*/
	case 0xd8: IMMBYTE(EA); 	EA=U+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0xd9: IMMWORD(EAP);	EA+=U;				EAD=RM16(m68_state, EAD);	   break;
	case 0xda: EA=PC; EA++ ; EA |= 0x00ff;			EAD=RM16(m68_state, EAD);	   break; /*ILLEGAL*/
	case 0xdb: EA=U+D;								EAD=RM16(m68_state, EAD);	   break;
	case 0xdc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0xdd: IMMWORD(EAP);	EA+=PC; 			EAD=RM16(m68_state, EAD);	   break;
	case 0xde: EA=0xffff;							EAD=RM16(m68_state, EAD);	   break; /*ILLEGAL*/
	case 0xdf: IMMWORD(EAP);						EAD=RM16(m68_state, EAD);	   break;

	case 0xe0: EA=S;	S++;										   break;
	case 0xe1: EA=S;	S+=2;										   break;
	case 0xe2: S--; 	EA=S;										   break;
	case 0xe3: S-=2;	EA=S;										   break;
	case 0xe4: EA=S;												   break;
	case 0xe5: EA=S+SIGNED(B);										   break;
	case 0xe6: EA=S+SIGNED(A);										   break;
	case 0xe7: EA=S+SIGNED(A);										   break; /*ILLEGAL*/
	case 0xe8: IMMBYTE(EA); 	EA=S+SIGNED(EA);					   break;
	case 0xe9: IMMWORD(EAP);	EA+=S;								   break;
	case 0xea: EA=PC; EA++ ; EA |= 0x00ff;							   break; /*ILLEGAL*/
	case 0xeb: EA=S+D;												   break;
	case 0xec: IMMBYTE(EA); 	EA=PC+SIGNED(EA);					   break;
	case 0xed: IMMWORD(EAP);	EA+=PC; 							   break;
	case 0xee: EA=0xffff;											   break;  /*ILLEGAL*/
	case 0xef: IMMWORD(EAP);										   break;

	case 0xf0: EA=S;	S++;						EAD=RM16(m68_state, EAD);	   break;
	case 0xf1: EA=S;	S+=2;						EAD=RM16(m68_state, EAD);	   break;
	case 0xf2: S--; 	EA=S;						EAD=RM16(m68_state, EAD);	   break;
	case 0xf3: S-=2;	EA=S;						EAD=RM16(m68_state, EAD);	   break;
	case 0xf4: EA=S;								EAD=RM16(m68_state, EAD);	   break;
	case 0xf5: EA=S+SIGNED(B);						EAD=RM16(m68_state, EAD);	   break;
	case 0xf6: EA=S+SIGNED(A);						EAD=RM16(m68_state, EAD);	   break;
	case 0xf7: EA=S+SIGNED(A);						EAD=RM16(m68_state, EAD); 		break; /*ILLEGAL*/
	case 0xf8: IMMBYTE(EA); 	EA=S+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0xf9: IMMWORD(EAP);	EA+=S;				EAD=RM16(m68_state, EAD);	   break;
	case 0xfa: EA=PC; EA++ ; EA |= 0x00ff;			EAD=RM16(m68_state, EAD);	   break; /*ILLEGAL*/
	case 0xfb: EA=S+D;								EAD=RM16(m68_state, EAD);	   break;
	case 0xfc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(m68_state, EAD);	   break;
	case 0xfd: IMMWORD(EAP);	EA+=PC; 			EAD=RM16(m68_state, EAD);	   break;
	case 0xfe: EA=0xffff;							EAD=RM16(m68_state, EAD);	   break; /*ILLEGAL*/
	case 0xff: IMMWORD(EAP);						EAD=RM16(m68_state, EAD);	   break;
	}
   m68_state->cycle += index_cycle_em[postbyte];
}
#endif 


//extern int  FASTCALL disline(int cpu, WORD pcreg, char *buffer);

static void debugreg(cpu6809_t *p)
{
//	if((maincpu.pc <0xff40)||(maincpu.pc>0xff53)) return;
   printf("DEBUG: %04x %02x %02x %02x %02x EA=%04x CYCLE=%03d ",p->pc ,READB(p, p->pc),READB(p, p->pc+1),READB(p, p->pc+2),READB(p, p->pc+3),p->ea,p->cycle);
   printf("AB=%04x X=%04x Y=%04x U=%04x S=%04x DP=%04x CC=%04x INT=%04x",p->acc.d, p->x, p->y, p->u, p->s, p->dp, p->cc, p->intr);
   printf("\n");
}

void main_exec(void)
{
	WORD nPC = maincpu.pc;
	cpu6809_t *p=&maincpu;
	if(disasm_main_flag) {
		char disasmline[1024];
		memset(disasmline, 0x00, 1023);
		disline(MAINCPU, maincpu.pc, disasmline);
		printf("MAIN:DIS:%s ",disasmline);
	} else {
		disasm_main_count = 0;
	}
   cpu_exec(&maincpu);
	if(disasm_main_flag) {
		printf("AB=%04x X=%04x Y=%04x U=%04x S=%04x DP=%04x CC=%04x INT=%04x \n",p->acc.d, p->x, p->y, p->u, p->s, p->dp, p->cc, p->intr);
	}
   maincpu.total += maincpu.cycle;
}

void main_line(void)
{
   cpu_execline(&maincpu);
}

void main_reset(void)
{
   cpu_reset(&maincpu);
}

void sub_exec(void)
{
	cpu6809_t *p=&subcpu;
	if(disasm_sub_flag) {
		char disasmline[1024];
		memset(disasmline, 0x00, 1023);
		disline(SUBCPU, subcpu.pc, disasmline);
		printf("SUB:DISASM:%s ",disasmline);
	}

   cpu_exec(&subcpu);
	if(disasm_sub_flag) {
		printf("AB=%04x X=%04x Y=%04x U=%04x S=%04x DP=%04x CC=%04x INT=%04x \n",p->acc.d, p->x, p->y, p->u, p->s, p->dp, p->cc, p->intr);
	}
   subcpu.total += subcpu.cycle;

}

void sub_line(void)
{
   cpu_execline(&subcpu);
}

void sub_reset(void)
{
   cpu_reset(&subcpu);
}
#if 0
void jsub_line(void)
{
   cpu_execline(&jsubcpu);
}

void jsub_exec(void)
{
   cpu_exec(&jsubcpu);
   jsubcpu.total += jsubcpu.cycle;
}

void jsub_reset(void)
{
   cpu_reset(&jsubcpu);
}
#endif
