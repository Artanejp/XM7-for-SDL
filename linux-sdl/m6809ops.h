/*

HNZVC

? = undefined
* = affected
- = unaffected
0 = cleared
1 = set
# = CCr directly affected by instruction
@ = special - carry set if bit 7 is set

*/

//#define OP_HANDLER(_name) INLINE void _name (m68_state_t *m68_state)
#define OP_HANDLER(_name) INLINE void _name (cpu6809_t *m68_state)

OP_HANDLER( illegal )
{
	//logerror("M6809: illegal opcode at %04x\n",PC);
        printf("M6809: illegal opcode at %04x %02x %02x %02x %02x \n",PC-1,RM(PC-1),RM(PC),RM(PC+1),RM(PC+2));
        //PC+=1;
}

static void IIError(cpu6809_t *m68_state)
{
	illegal(m68_state);		// Vector to Trap handler
}

/* $00 NEG direct ?**** */
OP_HANDLER( neg_di )
{
	WORD r,t;
	DIRBYTE(t);
	r = -t;
	CLR_NZVC;
	SET_NZVC8(0,t,r);
	WM(EAD,r);
}

/* $01 Undefined Neg */
/* $03 COM direct -**01 */
OP_HANDLER( com_di )
{
	BYTE t;
	DIRBYTE(t);
	t = ~t;
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	WM(EAD,t);
}

/* $02 NGC Direct (Undefined) */
OP_HANDLER(ngc_di)
{
     if((CC & CC_C) == 0) {
	neg_di(m68_state);
     } else {
       com_di(m68_state);
     }
   
}


/* $04 LSR direct -0*-* */
OP_HANDLER( lsr_di )
{
	BYTE t;
	DIRBYTE(t);
	CLR_NZC;
	CC |= (t & CC_C);
	t >>= 1;
	SET_Z(t);
	WM(EAD,t);
}

/* $05 ILLEGAL */

/* $06 ROR direct -**-* */
OP_HANDLER( ror_di )
{
	BYTE t,r;
	DIRBYTE(t);
	r= (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	r |= t>>1;
	SET_NZ8(r);
	WM(EAD,r);
}

/* $07 ASR direct ?**-* */
OP_HANDLER( asr_di )
{
	BYTE t;
	DIRBYTE(t);
	CLR_NZC;
	CC |= (t & CC_C);
	t = (t & 0x80) | (t >> 1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $08 ASL direct ?**** */
OP_HANDLER( asl_di )
{
	WORD t,r;
	DIRBYTE(t);
	r = t << 1;
	CLR_NZVC;
	SET_NZVC8(t,t,r);
	WM(EAD,r);
}

/* $09 ROL direct -**** */
OP_HANDLER( rol_di )
{
	WORD t,r;
	DIRBYTE(t);
	r = (CC & CC_C) | (t << 1);
	CLR_NZVC;
	SET_NZVC8(t,t,r);
	WM(EAD,r);
}

/* $0A DEC direct -***- */
OP_HANDLER( dec_di )
{
	BYTE t;
	DIRBYTE(t);
	--t;
	CLR_NZV;
	SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $0B DCC direct */
OP_HANDLER( dcc_di )
{
	BYTE t,s;
	DIRBYTE(t);
	--t;
	CLR_NZV;
	SET_FLAGS8D(t);
        s = CC;
        s >>= 2;
        s=~s;
        s=s & 0x01;
        CC = s | CC;
	WM(EAD,t);
}


/* $OC INC direct -***- */
OP_HANDLER( inc_di )
{
	BYTE t;
	DIRBYTE(t);
	++t;
	CLR_NZV;
	SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $OD TST direct -**0- */
OP_HANDLER( tst_di )
{
	BYTE t;
	DIRBYTE(t);
	CLR_NZV;
	SET_NZ8(t);
}

/* $0E JMP direct ----- */
OP_HANDLER( jmp_di )
{
	DIRECT;
	PC = EA;
}

/* $0F CLR direct -0100 */
OP_HANDLER( clr_di )
{
	DIRECT;
	(void)RM(EAD);
	WM(EAD,0);
	CLR_NZVC;
	SEZ;
}

/* $10 FLAG */

/* $11 FLAG */

/* $12 NOP inherent ----- */
OP_HANDLER( nop )
{
	;
}

/* $13 SYNC inherent ----- */
OP_HANDLER( sync )
{
   cpu6809_t *t = m68_state;
     if((m68_state->intr & INTR_SYNC_IN) == 0)
     {
	
	// SYNC命令初めて
	m68_state->intr |= INTR_SYNC_IN;
	m68_state->intr &= 0xffbf;
	PC -= 1; // 次のサイクルも同じ命令
	return;
     } else {
	// SYNC実行中
    	 if((m68_state->intr & INTR_SYNC_OUT) != 0) {
    		 // 割込が来たのでSYNC抜ける
    		 m68_state->intr &= 0xff9f;
    		 return;
    	 }
    	 PC -= 1;  // 割込こないと次のサイクルも同じ命令
     }
}
   


/* $14 trap(HALT) */
OP_HANDLER( trap )
{
   
       m68_state->intr |= 0x8000; // HALTフラグ
       // Debug: トラップ要因
       printf("INSN: TRAP @%04x %02x %02x\n",PC-1, RM(PC-1), RM(PC));
}

/* $15 trap */

/* $16 LBRA relative ----- */
OP_HANDLER( lbra )
{
	IMMWORD(EAD);
	PC += EAD;

}

/* $17 LBSR relative ----- */
OP_HANDLER( lbsr )
{
	IMMWORD(EAD);
	PUSHWORD(PC);
	PC += EAD;
}

/* $18 ASLCC */

OP_HANDLER( aslcc_in )
{
   
     BYTE cc = CC;
   if((cc & 0x04) != 0x00) //20100824 Fix
   {
	         cc |= 0x01;
   }
   cc <<=1;
   cc &= 0x3e;
   CC = cc;
}
 

/* $19 DAA inherent (A) -**0* */
OP_HANDLER( daa )
{
	BYTE msn, lsn;
	WORD t, cf = 0;
	msn = A & 0xf0; lsn = A & 0x0f;
	if( lsn>0x09 || CC & CC_H) cf |= 0x06;
	if( msn>0x80 && lsn>0x09 ) cf |= 0x60;
	if( msn>0x90 || CC & CC_C) cf |= 0x60;
	t = cf + A;
	CLR_NZV; /* keep carry from previous operation */
	SET_NZ8((BYTE)t); SET_C8(t);
	A = t;
}

/* $1A ORCC immediate ##### */
OP_HANDLER( orcc )
{
	BYTE t;
	IMMBYTE(t);
	CC |= t;

}

/* $1B ILLEGAL */



/* $1C ANDCC immediate ##### */
OP_HANDLER( andcc )
{
	BYTE t;
	IMMBYTE(t);
	CC &= t;
//	check_irq_lines(m68_state);	/* HJB 990116 */
}

/* $1D SEX inherent -**-- */
OP_HANDLER( sex )
{
	WORD t;
	t = SIGNED(B);
	D = t;
	//  CLR_NZV;    Tim Lindner 20020905: verified that V flag is not affected
	CLR_NZ;
	SET_NZ16(t);
}

/* $1E EXG inherent ----- */
OP_HANDLER( exg )
{
	WORD t1,t2;
	BYTE tb;

	IMMBYTE(tb);
	if( (tb^(tb>>4)) & 0x08 )	/* HJB 990225: mixed 8/16 bit case? */
	{
		/* transfer $ff to both registers */
		t1 = t2 = 0xff;
	}
	else
	{
		switch(tb>>4) {
			case  0: t1 = D;  break;
			case  1: t1 = X;  break;
			case  2: t1 = Y;  break;
			case  3: t1 = U;  break;
			case  4: t1 = S;  break;
			case  5: t1 = PC; break;
			case  8: t1 = A;  break;
			case  9: t1 = B;  break;
			case 10: t1 = CC; break;
			case 11: t1 = DP; break;
			default: t1 = 0xff;
		}
		switch(tb&15) {
			case  0: t2 = D;  break;
			case  1: t2 = X;  break;
			case  2: t2 = Y;  break;
			case  3: t2 = U;  break;
			case  4: t2 = S;  break;
			case  5: t2 = PC; break;
			case  8: t2 = A;  break;
			case  9: t2 = B;  break;
			case 10: t2 = CC; break;
			case 11: t2 = DP; break;
			default: t2 = 0xff;
        }
	}
	switch(tb>>4) {
		case  0: D = t2;  break;
		case  1: X = t2;  break;
		case  2: Y = t2;  break;
		case  3: U = t2;  break;
		case  4: S = t2;  m68_state->intr |= INTR_SLOAD; break;
		case  5: PC = t2; break;
		case  8: A = t2;  break;
		case  9: B = t2;  break;
		case 10: CC = t2; break;
		case 11: DP = t2; break;
	}
	switch(tb&15) {
		case  0: D = t1;  break;
		case  1: X = t1;  break;
		case  2: Y = t1;  break;
		case  3: U = t1;  break;
		case  4: S = t1;  m68_state->intr |= INTR_SLOAD; break;
		case  5: PC = t1; break;
		case  8: A = t1;  break;
		case  9: B = t1;  break;
		case 10: CC = t1; break;
		case 11: DP = t1; break;
	}
}

/* $1F TFR inherent ----- */
OP_HANDLER( tfr )
{
	BYTE tb;
	WORD t;

	IMMBYTE(tb);
	if( (tb^(tb>>4)) & 0x08 )	/* HJB 990225: mixed 8/16 bit case? */
	{
		/* transfer $ff to register */
		t = 0xff;
        }
	else
	{
		switch(tb>>4) {
			case  0: t = D;  break;
			case  1: t = X;  break;
			case  2: t = Y;  break;
			case  3: t = U;  break;
			case  4: t = S;  break;
			case  5: t = PC; break;
			case  8: t = A;  break;
			case  9: t = B;  break;
			case 10: t = CC; break;
			case 11: t = DP; break;
			default: t = 0xff;
        }
	}
	switch(tb&15) {
		case  0: D = t;  break;
		case  1: X = t;  break;
		case  2: Y = t;  break;
		case  3: U = t;  break;
		case  4: S = t;  m68_state->intr |= INTR_SLOAD; break;
		case  5: PC = t; break;
		case  8: A = t;  break;
		case  9: B = t;  break;
		case 10: CC = t; break;
		case 11: DP = t; break;
    }
}

/* $20 BRA relative ----- */
OP_HANDLER( bra )
{
	BYTE t;
	IMMBYTE(t);
	PC += SIGNED(t);
	/* JB 970823 - speed up busy loops */
//	if( t == 0xfe )
//		if( m68_state->icount > 0 ) m68_state->icount = 0;
}

/* $21 BRN relative ----- */
OP_HANDLER( brn )
{
	BYTE t;
	IMMBYTE(t);
}

/* $1021 LBRN relative ----- */
OP_HANDLER( lbrn )
{
	IMMWORD(EAD);
}

/* $22 BHI relative ----- */
OP_HANDLER( bhi )
{
	BRANCH(m68_state,  !(CC & (CC_Z|CC_C)) );
}

/* $1022 LBHI relative ----- */
OP_HANDLER( lbhi )
{
	LBRANCH(m68_state,  !(CC & (CC_Z|CC_C)) );
}

/* $23 BLS relative ----- */
OP_HANDLER( bls )
{
	BRANCH(m68_state,  (CC & (CC_Z|CC_C)) );
}

/* $1023 LBLS relative ----- */
OP_HANDLER( lbls )
{
	LBRANCH(m68_state,  (CC&(CC_Z|CC_C)) );
}

/* $24 BCC relative ----- */
OP_HANDLER( bcc )
{
	BRANCH(m68_state,  !(CC&CC_C) );
}

/* $1024 LBCC relative ----- */
OP_HANDLER( lbcc )
{
	LBRANCH(m68_state,  !(CC&CC_C) );
}

/* $25 BCS relative ----- */
OP_HANDLER( bcs )
{
	BRANCH(m68_state,  (CC&CC_C) );
}

/* $1025 LBCS relative ----- */
OP_HANDLER( lbcs )
{
	LBRANCH(m68_state,  (CC&CC_C) );
}

/* $26 BNE relative ----- */
OP_HANDLER( bne )
{
	int cond=!(CC&CC_Z);
	BRANCH(m68_state, cond  );
}

/* $1026 LBNE relative ----- */
OP_HANDLER( lbne )
{
	LBRANCH(m68_state,  !(CC&CC_Z) );
}

/* $27 BEQ relative ----- */
OP_HANDLER( beq )
{
	BRANCH(m68_state,  (CC&CC_Z) );
}

/* $1027 LBEQ relative ----- */
OP_HANDLER( lbeq )
{
	LBRANCH(m68_state,  (CC&CC_Z) );
}

/* $28 BVC relative ----- */
OP_HANDLER( bvc )
{
	BRANCH(m68_state,  !(CC&CC_V) );
}

/* $1028 LBVC relative ----- */
OP_HANDLER( lbvc )
{
	LBRANCH(m68_state,  !(CC&CC_V) );
}

/* $29 BVS relative ----- */
OP_HANDLER( bvs )
{
	BRANCH(m68_state,  (CC&CC_V) );
}

/* $1029 LBVS relative ----- */
OP_HANDLER( lbvs )
{
	LBRANCH(m68_state,  (CC&CC_V) );
}

/* $2A BPL relative ----- */
OP_HANDLER( bpl )
{
	BRANCH(m68_state,  !(CC&CC_N) );
}

/* $102A LBPL relative ----- */
OP_HANDLER( lbpl )
{
	LBRANCH(m68_state,  !(CC&CC_N) );
}

/* $2B BMI relative ----- */
OP_HANDLER( bmi )
{
	BRANCH(m68_state,  (CC&CC_N) );
}

/* $102B LBMI relative ----- */
OP_HANDLER( lbmi )
{
	LBRANCH(m68_state,  (CC&CC_N) );
}

/* $2C BGE relative ----- */
OP_HANDLER( bge )
{
	BRANCH(m68_state,  !NXORV );
}

/* $102C LBGE relative ----- */
OP_HANDLER( lbge )
{
	LBRANCH(m68_state,  !NXORV );
}

/* $2D BLT relative ----- */
OP_HANDLER( blt )
{
	BRANCH(m68_state,  NXORV );
}

/* $102D LBLT relative ----- */
OP_HANDLER( lblt )
{
	LBRANCH(m68_state,  NXORV );
}

/* $2E BGT relative ----- */
OP_HANDLER( bgt )
{
	BRANCH(m68_state,  !(NXORV || (CC&CC_Z)) );
}

/* $102E LBGT relative ----- */
OP_HANDLER( lbgt )
{
	LBRANCH(m68_state,  !(NXORV || (CC&CC_Z)) );
}

/* $2F BLE relative ----- */
OP_HANDLER( ble )
{
	BRANCH(m68_state,  (NXORV || (CC&CC_Z)) );
}

/* $102F LBLE relative ----- */
OP_HANDLER( lble )
{
	LBRANCH(m68_state,  (NXORV || (CC&CC_Z)) );
}

/* $30 LEAX indexed --*-- */
OP_HANDLER( leax )
{
	fetch_effective_address(m68_state);
	X = EA;
	CLR_Z;
	SET_Z16(X);
}

/* $31 LEAY indexed --*-- */
OP_HANDLER( leay )
{
	fetch_effective_address(m68_state);
	Y = EA;
	CLR_Z;
	SET_Z16(Y);

}

/* $32 LEAS indexed ----- */
OP_HANDLER( leas )
{
	fetch_effective_address(m68_state);
	S = EA;
        m68_state->intr |= INTR_SLOAD;
//	m68_state->int_state |= M6809_LDS;
}

/* $33 LEAU indexed ----- */
OP_HANDLER( leau )
{
	fetch_effective_address(m68_state);
	U = EA;
}

/* $34 PSHS inherent ----- */
OP_HANDLER( pshs )
{
	BYTE t;
	IMMBYTE(t);
	if( t&0x80 ) { PUSHWORD(pPC); m68_state->cycle += 2; }
	if( t&0x40 ) { PUSHWORD(pU);  m68_state->cycle += 2; }
	if( t&0x20 ) { PUSHWORD(pY);  m68_state->cycle += 2; }
	if( t&0x10 ) { PUSHWORD(pX);  m68_state->cycle += 2; }
	if( t&0x08 ) { PUSHBYTE(DP);  m68_state->cycle += 1; }
	if( t&0x04 ) { PUSHBYTE(B);   m68_state->cycle += 1; }
	if( t&0x02 ) { PUSHBYTE(A);   m68_state->cycle += 1; }
	if( t&0x01 ) { PUSHBYTE(CC);  m68_state->cycle += 1; }
}

/* 35 PULS inherent ----- */
OP_HANDLER( puls )
{
	BYTE t;
	IMMBYTE(t);
	if( t&0x01 ) { PULLBYTE(CC); m68_state->cycle += 1; }
	if( t&0x02 ) { PULLBYTE(A);  m68_state->cycle += 1; }
	if( t&0x04 ) { PULLBYTE(B);  m68_state->cycle += 1; }
	if( t&0x08 ) { PULLBYTE(DP); m68_state->cycle += 1; }
	if( t&0x10 ) { PULLWORD(XD); m68_state->cycle += 2; }
	if( t&0x20 ) { PULLWORD(YD); m68_state->cycle += 2; }
	if( t&0x40 ) { PULLWORD(UD); m68_state->cycle += 2; }
	if( t&0x80 ) { PULLWORD(PCD); m68_state->cycle += 2; }

	/* HJB 990225: moved check after all PULLs */
//	if( t&0x01 ) { check_irq_lines(m68_state); }
}

/* $36 PSHU inherent ----- */
OP_HANDLER( pshu )
{
	BYTE t;
	IMMBYTE(t);
	if( t&0x80 ) { PSHUWORD(pPC); m68_state->cycle += 2; }
	if( t&0x40 ) { PSHUWORD(pS);  m68_state->cycle += 2; }
	if( t&0x20 ) { PSHUWORD(pY);  m68_state->cycle += 2; }
	if( t&0x10 ) { PSHUWORD(pX);  m68_state->cycle += 2; }
	if( t&0x08 ) { PSHUBYTE(DP);  m68_state->cycle += 1; }
	if( t&0x04 ) { PSHUBYTE(B);   m68_state->cycle += 1; }
	if( t&0x02 ) { PSHUBYTE(A);   m68_state->cycle += 1; }
	if( t&0x01 ) { PSHUBYTE(CC);  m68_state->cycle += 1; }
}

/* 37 PULU inherent ----- */
OP_HANDLER( pulu )
{
	BYTE t;
	IMMBYTE(t);
	if( t&0x01 ) { PULUBYTE(CC); m68_state->cycle += 1; }
	if( t&0x02 ) { PULUBYTE(A);  m68_state->cycle += 1; }
	if( t&0x04 ) { PULUBYTE(B);  m68_state->cycle += 1; }
	if( t&0x08 ) { PULUBYTE(DP); m68_state->cycle += 1; }
	if( t&0x10 ) { PULUWORD(XD); m68_state->cycle += 2; }
	if( t&0x20 ) { PULUWORD(YD); m68_state->cycle += 2; }
	if( t&0x40 ) { PULUWORD(SD); m68_state->cycle += 2; }
	if( t&0x80 ) { PULUWORD(PCD); m68_state->cycle += 2; }

	/* HJB 990225: moved check after all PULLs */
	//if( t&0x01 ) { check_irq_lines(m68_state); }
}

/* $38 ILLEGAL */

/* $39 RTS inherent ----- */
OP_HANDLER( rts )
{
	PULLWORD(PCD);
}

/* $3A ABX inherent ----- */
OP_HANDLER( abx )
{
	X += B;
}

/* $3B RTI inherent ##### */
OP_HANDLER( rti )
{
	BYTE t;
	PULLBYTE(CC);
	t = CC & CC_E;		/* HJB 990225: entire state saved? */
	if(t)
	{
		m68_state->cycle += 9;
		PULLBYTE(A);
		PULLBYTE(B);
		PULLBYTE(DP);
		PULLWORD(XD);
		PULLWORD(YD);
		PULLWORD(UD);
	}
	PULLWORD(PCD);
//	check_irq_lines(m68_state);	/* HJB 990116 */
}

/* $3C CWAI inherent ----1 */
OP_HANDLER( cwai )
{
	BYTE t;
    if(m68_state->intr & INTR_CWAI_IN){
	/* CWAI実行中 */
       if(m68_state->intr & INTR_CWAI_OUT) {
    	   /* 割込がかかって、RTIの後 */
    	   m68_state->intr &= 0xfe7f; /* CWAIフラグクリア */
    	   PC += 1;
    	   return;
       } else {
    	   PC -= 1;
    	   return;
       }
    }
	/* 今回初めてCWAI実行 */
first:
     IMMBYTE(t);
     CC = CC & t;
     m68_state->intr = (m68_state->intr | INTR_CWAI_IN) & 0xfeff;
     PC -= 2;
     return;
 }

/* $3D MUL inherent --*-@ */
OP_HANDLER( mul )
{
	WORD t;
	t = A * B;
	CLR_ZC; SET_Z(t); if(t&0x80) SEC;
	D = t;
}

/* $3E RST */
OP_HANDLER( rst )
{
      cpu_reset(m68_state);
}


/* $3F SWI (SWI2 SWI3) absolute indirect ----- */
OP_HANDLER( swi )
{
	CC |= CC_E; 			/* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	CC |= CC_IF | CC_II;	/* inhibit FIRQ and IRQ */
	PCD=RM16(m68_state, 0xfffa);
}

/* $103F SWI2 absolute indirect ----- */
OP_HANDLER( swi2 )
{
	CC |= CC_E; 			/* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	PCD = RM16(m68_state, 0xfff4);
}

/* $113F SWI3 absolute indirect ----- */
OP_HANDLER( swi3 )
{
	CC |= CC_E; 			/* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	PCD = RM16(m68_state, 0xfff2);
}

/* $40 NEGA inherent ?**** */
OP_HANDLER( nega )
{
	WORD r;
	r = -A;
	CLR_NZVC;
	SET_NZVC8(0,A,r);
	A = r;
}

/* $41 NEGA */


/* $43 COMA inherent -**01 */
OP_HANDLER( coma )
{
	A = ~A;
	CLR_NZV;
	SET_NZ8(A);
	SEC;
}

/* $42 NGCA */
OP_HANDLER( ngca )
{
        if((CC & 0x01) == 0) {
	   nega(m68_state);
	} else {
	   coma(m68_state);
	}
}

/* $44 LSRA inherent -0*-* */
OP_HANDLER( lsra )
{
	CLR_NZC;
	CC |= (A & CC_C);
	A >>= 1;
	SET_Z(A);
}

/* $45 LSRA */

/* $46 RORA inherent -**-* */
OP_HANDLER( rora )
{
	BYTE r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (A & CC_C);
	r |= A >> 1;
	SET_NZ8(r);
	A = r;
}

/* $47 ASRA inherent ?**-* */
OP_HANDLER( asra )
{
	CLR_NZC;
	CC |= (A & CC_C);
	A = (A & 0x80) | (A >> 1);
	SET_NZ8(A);
}

/* $48 ASLA inherent ?**** */
OP_HANDLER( asla )
{
	WORD r;
	r = A << 1;
	CLR_NZVC;
	SET_NZVC8(A,A,r);
	A = r;
}

/* $49 ROLA inherent -**** */
OP_HANDLER( rola )
{
	WORD t,r;
	t = A;
	r = (CC & CC_C) | (t<<1);
	CLR_NZVC;
	SET_NZVC8(t,t,r);
	A = r;
}

/* $4A DECA inherent -***- */
OP_HANDLER( deca )
{
	--A;
	CLR_NZV;
	SET_FLAGS8D(A);
}


/* $4B DCCA */
OP_HANDLER( dcca )
{
	BYTE t,s;
	--A;
	CLR_NZV;
	SET_FLAGS8D(A);
        s = CC;
        s >>= 2;
        s=~s;
        s=s & 0x01;
        CC = s | CC;
}

/* $4C INCA inherent -***- */
OP_HANDLER( inca )
{
	++A;
	CLR_NZV;
	SET_FLAGS8I(A);
}

/* $4D TSTA inherent -**0- */
OP_HANDLER( tsta )
{
	CLR_NZV;
	SET_NZ8(A);
}

/* $4E ILLEGAL */
OP_HANDLER( clca )
{
   A = 0;
   CLR_NZV;
   SET_Z(A);
}

/* $4F CLRA inherent -0100 */
OP_HANDLER( clra )
{
	A = 0;
	CLR_NZVC; SEZ;
}

/* $50 NEGB inherent ?**** */
OP_HANDLER( negb )
{
	WORD r;
	r = -B;
	CLR_NZVC;
	SET_NZVC8(0,B,r);
	B = r;
}

/* $51 NEGB */

/* $52 NGCB */

/* $53 COMB inherent -**01 */
OP_HANDLER( comb )
{
	B = ~B;
	CLR_NZV;
	SET_NZ8(B);
	SEC;
}

/* $52 NGCB */
OP_HANDLER( ngcb )
{
        if((CC & 0x01) == 0) {
	   negb(m68_state);
	} else {
	   comb(m68_state);
	}
}

/* $54 LSRB inherent -0*-* */
OP_HANDLER( lsrb )
{
	CLR_NZC;
	CC |= (B & CC_C);
	B >>= 1;
	SET_Z(B);
}

/* $55 LSRB */

/* $56 RORB inherent -**-* */
OP_HANDLER( rorb )
{
	BYTE r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (B & CC_C);
	r |= B >> 1;
	SET_NZ8(r);
	B = r;
}

/* $57 ASRB inherent ?**-* */
OP_HANDLER( asrb )
{
	CLR_NZC;
	CC |= (B & CC_C);
	B= (B & 0x80) | (B >> 1);
	SET_NZ8(B);
}

/* $58 ASLB inherent ?**** */
OP_HANDLER( aslb )
{
	WORD r;
	r = B << 1;
	CLR_NZVC;
	SET_NZVC8(B,B,r);
	B = r;
}

/* $59 ROLB inherent -**** */
OP_HANDLER( rolb )
{
	WORD t,r;
	t = B;
	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_NZVC8(t,t,r);
	B = r;
}

/* $5A DECB inherent -***- */
OP_HANDLER( decb )
{
	BYTE t;
	t = B;
	CLR_NZV;
	B-=1;
	SET_FLAGS8D(B);
}

/* $5B ILLEGAL */
/* $5B DCCB */
OP_HANDLER( dccb )
{
	BYTE t,s;
	--B;
	CLR_NZV;
	SET_FLAGS8D(B);
        s = CC;
        s >>= 2;
        s=~s;
        s=s & 0x01;
        CC = s | CC;
}

/* $5C INCB inherent -***- */
OP_HANDLER( incb )
{
	++B;
	CLR_NZV;
	SET_FLAGS8I(B);
}

/* $5D TSTB inherent -**0- */
OP_HANDLER( tstb )
{
	CLR_NZV;
	SET_NZ8(B);
}

/* $5E ILLEGAL */
OP_HANDLER( clcb )
{
   B = 0;
   CLR_NZV;
   SET_Z(B);
}

/* $5F CLRB inherent -0100 */
OP_HANDLER( clrb )
{
	B = 0;
	CLR_NZVC; SEZ;
}

/* $60 NEG indexed ?**** */
OP_HANDLER( neg_ix )
{
	WORD r,t;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r=-t;
	CLR_NZVC;
	SET_NZVC8(0,t,r);
	WM(EAD,r);
}

/* $61 ILLEGAL */


/* $63 COM indexed -**01 */
OP_HANDLER( com_ix )
{
	BYTE t;
	fetch_effective_address(m68_state);
	t = ~RM(EAD);
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	WM(EAD,t);
}
/* $62 ILLEGAL */
OP_HANDLER( ngc_ix )
{
        if((CC & 0x01) == 0) {
	   neg_ix(m68_state);
	} else {
	   com_ix(m68_state);
	}
}

/* $64 LSR indexed -0*-* */
OP_HANDLER( lsr_ix )
{
	BYTE t;
	fetch_effective_address(m68_state);
	t=RM(EAD);
	CLR_NZC;
	CC |= (t & CC_C);
	t>>=1; SET_Z(t);
	WM(EAD,t);
}

/* $65 ILLEGAL */

/* $66 ROR indexed -**-* */
OP_HANDLER( ror_ix )
{
	BYTE t,r;
	fetch_effective_address(m68_state);
	t=RM(EAD);
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	r |= t>>1; SET_NZ8(r);
	WM(EAD,r);
}

/* $67 ASR indexed ?**-* */
OP_HANDLER( asr_ix )
{
	BYTE t;
	fetch_effective_address(m68_state);
	t=RM(EAD);
	CLR_NZC;
	CC |= (t & CC_C);
	t=(t&0x80)|(t>>1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $68 ASL indexed ?**** */
OP_HANDLER( asl_ix )
{
	WORD t,r;
	fetch_effective_address(m68_state);
	t=RM(EAD);
	r = t << 1;
	CLR_NZVC;
	SET_NZVC8(t,t,r);
	WM(EAD,r);
}

/* $69 ROL indexed -**** */
OP_HANDLER( rol_ix )
{
	WORD t,r;
	fetch_effective_address(m68_state);
	t=RM(EAD);
	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_NZVC8(t,t,r);
	WM(EAD,r);
}

/* $6A DEC indexed -***- */
OP_HANDLER( dec_ix )
{
	BYTE t;
	fetch_effective_address(m68_state);
	t = RM(EAD) - 1;
	CLR_NZV;
	SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $6B DCC index */
OP_HANDLER( dcc_ix )
{
	BYTE t,s;
   	fetch_effective_address(m68_state);
	t = RM(EAD) - 1;
	CLR_NZV;
	SET_FLAGS8D(t);
        s = CC;
        s >>= 2;
        s=~s;
        s=s & 0x01;
        CC = s | CC;
	WM(EAD,t);
}

/* $6C INC indexed -***- */
OP_HANDLER( inc_ix )
{
	BYTE t;
	fetch_effective_address(m68_state);
	t = RM(EAD) + 1;
	CLR_NZV; SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $6D TST indexed -**0- */
OP_HANDLER( tst_ix )
{
	BYTE t;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	CLR_NZV;
	SET_NZ8(t);
}

/* $6E JMP indexed ----- */
OP_HANDLER( jmp_ix )
{
	fetch_effective_address(m68_state);
	PCD = EAD;
}

/* $6F CLR indexed -0100 */
OP_HANDLER( clr_ix )
{
	fetch_effective_address(m68_state);
	(void)RM(EAD);
	WM(EAD,0);
	CLR_NZVC; SEZ;
}

/* $70 NEG extended ?**** */
OP_HANDLER( neg_ex )
{
	WORD r,t;
	EXTBYTE(t); r=-t;
	CLR_NZVC;
	SET_NZVC8(0,t,r);
	WM(EAD,r);
}

/* $71 ILLEGAL */

/* $72 ILLEGAL */

/* $73 COM extended -**01 */
OP_HANDLER( com_ex )
{
	BYTE t;
	EXTBYTE(t); t = ~t;
	CLR_NZV; SET_NZ8(t); SEC;
	WM(EAD,t);
}

/* $72 ILLEGAL */
OP_HANDLER( ngc_ex )
{
        if((CC & 0x01) == 0) {
	   neg_ex(m68_state);
	} else {
	   com_ex(m68_state);
	}
}

/* $74 LSR extended -0*-* */
OP_HANDLER( lsr_ex )
{
	BYTE t;
	EXTBYTE(t); CLR_NZC; CC |= (t & CC_C);
	t>>=1; SET_Z(t);
	WM(EAD,t);
}

/* $75 ILLEGAL */

/* $76 ROR extended -**-* */
OP_HANDLER( ror_ex )
{
	BYTE t,r;
	EXTBYTE(t); r=(CC & CC_C) << 7;
	CLR_NZC; CC |= (t & CC_C);
	r |= t>>1; SET_NZ8(r);
	WM(EAD,r);
}

/* $77 ASR extended ?**-* */
OP_HANDLER( asr_ex )
{
	BYTE t;
	EXTBYTE(t); CLR_NZC; CC |= (t & CC_C);
	t=(t&0x80)|(t>>1);
	SET_NZ8(t);
	WM(EAD,t);
}

/* $78 ASL extended ?**** */
OP_HANDLER( asl_ex )
{
	WORD t,r;
	EXTBYTE(t); r=t<<1;
	CLR_NZVC; SET_NZVC8(t,t,r);
	WM(EAD,r);
}

/* $79 ROL extended -**** */
OP_HANDLER( rol_ex )
{
	WORD t,r;
	EXTBYTE(t); r = (CC & CC_C) | (t << 1);
	CLR_NZVC; SET_NZVC8(t,t,r);
	WM(EAD,r);
}

/* $7A DEC extended -***- */
OP_HANDLER( dec_ex )
{
	BYTE t;
	EXTBYTE(t); --t;
	CLR_NZV; SET_FLAGS8D(t);
	WM(EAD,t);
}

/* $7B ILLEGAL */
/* $6B DCC index */
OP_HANDLER( dcc_ex )
{
	BYTE t,s;
        EXTBYTE(t);
        --t;
	CLR_NZV;
	SET_FLAGS8D(t);
        s = CC;
        s >>= 2;
        s=~s;
        s=s & 0x01;
        CC = s | CC;
	WM(EA,t);
}

/* $7C INC extended -***- */
OP_HANDLER( inc_ex )
{
	BYTE t;
	EXTBYTE(t); ++t;
	CLR_NZV; SET_FLAGS8I(t);
	WM(EAD,t);
}

/* $7D TST extended -**0- */
OP_HANDLER( tst_ex )
{
	BYTE t;
	EXTBYTE(t); CLR_NZV; SET_NZ8(t);
}

/* $7E JMP extended ----- */
OP_HANDLER( jmp_ex )
{
	EXTENDED;
	PCD = EAD;
}

/* $7F CLR extended -0100 */
OP_HANDLER( clr_ex )
{
	EXTENDED;
	(void)RM(EAD);
	WM(EAD,0);
	CLR_NZVC; SEZ;
}

/* $80 SUBA immediate ?**** */
OP_HANDLER( suba_im )
{
	WORD t,r;
	IMMBYTE(t);
	r = A - t;
	CLR_HNZVC;
	SET_NZVC8(A,t,r);
	A = r;
}

/* $81 CMPA immediate ?**** */
OP_HANDLER( cmpa_im )
{
	WORD	  t,r;
	IMMBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_NZVC8(A,t,r);
}

/* $82 SBCA immediate ?**** */
OP_HANDLER( sbca_im )
{
	WORD	  t,r;
	IMMBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_HNZVC;
	SET_NZVC8(A,t,r);
	A = r;
}

/* $83 SUBD (CMPD CMPU) immediate -**** */
OP_HANDLER( subd_im )
{
	DWORD r,d;
	WORD b;
	IMMWORD(b);
	d = D;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
	D = r;
}

/* $1083 CMPD immediate -**** */
OP_HANDLER( cmpd_im )
{
	DWORD r,d;
	WORD b;
	IMMWORD(b);
	d = D;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $1183 CMPU immediate -**** */
OP_HANDLER( cmpu_im )
{
	DWORD r, d;
	WORD b;
	IMMWORD(b);
	d = U;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $84 ANDA immediate -**0- */
OP_HANDLER( anda_im )
{
	BYTE t;
	IMMBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $85 BITA immediate -**0- */
OP_HANDLER( bita_im )
{
	BYTE t,r;
	IMMBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $86 LDA immediate -**0- */
OP_HANDLER( lda_im )
{
	IMMBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* is this a legal instruction? */
/* $87 STA immediate -**0- */
OP_HANDLER( sta_im )
{
	CLR_NZV;
	SET_NZ8(A);
	IMM8;
	WM(EAD,A);
}

/*
 * $87 , $C7: FLAG8
 */
OP_HANDLER( flag8_im )
{
   IMM8;
   CLR_NZV;
   CC |= 0x08;
}


/* $88 EORA immediate -**0- */
OP_HANDLER( eora_im )
{
	BYTE t;
	IMMBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $89 ADCA immediate ***** */
OP_HANDLER( adca_im )
{
	WORD t,r;
	IMMBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_HNZVC8(A,t,r);
	A = r;
}

/* $8A ORA immediate -**0- */
OP_HANDLER( ora_im )
{
	BYTE t;
	IMMBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $8B ADDA immediate ***** */
OP_HANDLER( adda_im )
{
	WORD t,r;
	IMMBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_HNZVC8(A,t,r);
	A = r;
}

/* $8C CMPX (CMPY CMPS) immediate -**** */
OP_HANDLER( cmpx_im )
{
	DWORD r,d;
	WORD b;
	IMMWORD(b);
	d = X;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $108C CMPY immediate -**** */
OP_HANDLER( cmpy_im )
{
	DWORD r,d;
	WORD b;
	IMMWORD(b);
	d = Y;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $118C CMPS immediate -**** */
OP_HANDLER( cmps_im )
{
	DWORD r,d;
	WORD b;
	IMMWORD(b);
	d = S;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $8D BSR ----- */
OP_HANDLER( bsr )
{
	BYTE t;
	IMMBYTE(t);
	PUSHWORD(pPC);
	PC += SIGNED(t);
}

/* $8E LDX (LDY) immediate -**0- */
OP_HANDLER( ldx_im )
{
	IMMWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $108E LDY immediate -**0- */
OP_HANDLER( ldy_im )
{
	IMMWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* is this a legal instruction? */
/* $8F STX (STY) immediate -**0- */
OP_HANDLER( stx_im )
{
	CLR_NZV;
	SET_NZ16(X);
	IMM16;
	WM16(m68_state, EAD, X);
}

/*
 * $8F , $CF: FLAG16
 */
OP_HANDLER( flag16_im )
{
   IMM16;
   CLR_NZV;
   CC |= 0x08;
}


/* is this a legal instruction? */
/* $108F STY immediate -**0- */
OP_HANDLER( sty_im )
{
	CLR_NZV;
	SET_NZ16(Y);
	IMM16;
	WM16(m68_state, EAD, Y);
}

/* $90 SUBA direct ?**** */
OP_HANDLER( suba_di )
{
	WORD	  t,r;
	DIRBYTE(t);
	r = A - t;
	CLR_HNZVC;
	SET_NZVC8(A,t,r);
	A = r;
}

/* $91 CMPA direct ?**** */
OP_HANDLER( cmpa_di )
{
	WORD	  t,r;
	DIRBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_NZVC8(A,t,r);
}

/* $92 SBCA direct ?**** */
OP_HANDLER( sbca_di )
{
	WORD	  t,r;
	DIRBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_HNZVC;
	SET_NZVC8(A,t,r);
	A = r;
}

/* $93 SUBD (CMPD CMPU) direct -**** */
OP_HANDLER( subd_di )
{
	DWORD r,d;
	WORD b;
	DIRWORD(b);
	d = D;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
	D = r;
}

/* $1093 CMPD direct -**** */
OP_HANDLER( cmpd_di )
{
	DWORD r,d;
	WORD b;
	DIRWORD(b);
	d = D;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $1193 CMPU direct -**** */
OP_HANDLER( cmpu_di )
{
	DWORD r,d;
	WORD b;
	DIRWORD(b);
	d = U;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(U,b,r);
}

/* $94 ANDA direct -**0- */
OP_HANDLER( anda_di )
{
	BYTE t;
	DIRBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $95 BITA direct -**0- */
OP_HANDLER( bita_di )
{
	BYTE t,r;
	DIRBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $96 LDA direct -**0- */
OP_HANDLER( lda_di )
{
	DIRBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $97 STA direct -**0- */
OP_HANDLER( sta_di )
{
	CLR_NZV;
	SET_NZ8(A);
	DIRECT;
	WM(EAD,A);
}

/* $98 EORA direct -**0- */
OP_HANDLER( eora_di )
{
	BYTE t;
	DIRBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $99 ADCA direct ***** */
OP_HANDLER( adca_di )
{
	WORD t,r;
	DIRBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_HNZVC8(A,t,r);
	A = r;
}

/* $9A ORA direct -**0- */
OP_HANDLER( ora_di )
{
	BYTE t;
	DIRBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $9B ADDA direct ***** */
OP_HANDLER( adda_di )
{
	WORD t,r;
	DIRBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_HNZVC8(A,t,r);
	A = r;
}

/* $9C CMPX (CMPY CMPS) direct -**** */
OP_HANDLER( cmpx_di )
{
	DWORD r,d;
	WORD b;
	DIRWORD(b);
	d = X;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $109C CMPY direct -**** */
OP_HANDLER( cmpy_di )
{
	DWORD r,d;
	WORD b;
	DIRWORD(b);
	d = Y;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $119C CMPS direct -**** */
OP_HANDLER( cmps_di )
{
	DWORD r,d;
	WORD b;
	DIRWORD(b);
	d = S;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $9D JSR direct ----- */
OP_HANDLER( jsr_di )
{
	DIRECT;
	PUSHWORD(pPC);
	PCD = EAD;
}

/* $9E LDX (LDY) direct -**0- */
OP_HANDLER( ldx_di )
{
	DIRWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $109E LDY direct -**0- */
OP_HANDLER( ldy_di )
{
	DIRWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $9F STX (STY) direct -**0- */
OP_HANDLER( stx_di )
{
	CLR_NZV;
	SET_NZ16(X);
	DIRECT;
	WM16(m68_state, EAD, X);
}

/* $109F STY direct -**0- */
OP_HANDLER( sty_di )
{
	CLR_NZV;
	SET_NZ16(Y);
	DIRECT;
	WM16(m68_state, EAD, Y);
}

/* $a0 SUBA indexed ?**** */
OP_HANDLER( suba_ix )
{
	WORD t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = A - t;
	CLR_HNZVC;
	SET_NZVC8(A,t,r);
	A = r;
}

/* $a1 CMPA indexed ?**** */
OP_HANDLER( cmpa_ix )
{
	WORD t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = A - t;
	CLR_NZVC;
	SET_NZVC8(A,t,r);
}

/* $a2 SBCA indexed ?**** */
OP_HANDLER( sbca_ix )
{
	WORD	  t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = A - t - (CC & CC_C);
	CLR_HNZVC;
	SET_NZVC8(A,t,r);
	A = r;
}

/* $a3 SUBD (CMPD CMPU) indexed -**** */
OP_HANDLER( subd_ix )
{
	DWORD r,d;
	WORD b;
	fetch_effective_address(m68_state);
	b=RM16(m68_state, EAD);
	d = D;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
	D = r;
}

/* $10a3 CMPD indexed -**** */
OP_HANDLER( cmpd_ix )
{
	DWORD r,d;
	WORD b;
	fetch_effective_address(m68_state);
	b=RM16(m68_state, EAD);
	d = D;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $11a3 CMPU indexed -**** */
OP_HANDLER( cmpu_ix )
{
	DWORD r;
	WORD b;
	fetch_effective_address(m68_state);
	b=RM16(m68_state, EAD);
	r = U - b;
	CLR_NZVC;
	SET_NZVC16(U,b,r);
}

/* $a4 ANDA indexed -**0- */
OP_HANDLER( anda_ix )
{
	fetch_effective_address(m68_state);
	A &= RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a5 BITA indexed -**0- */
OP_HANDLER( bita_ix )
{
	BYTE r;
	fetch_effective_address(m68_state);
	r = A & RM(EAD);
	CLR_NZV;
	SET_NZ8(r);
}

/* $a6 LDA indexed -**0- */
OP_HANDLER( lda_ix )
{
	fetch_effective_address(m68_state);
	A = RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a7 STA indexed -**0- */
OP_HANDLER( sta_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ8(A);
	WM(EAD,A);
}

/* $a8 EORA indexed -**0- */
OP_HANDLER( eora_ix )
{
	fetch_effective_address(m68_state);
	A ^= RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a9 ADCA indexed ***** */
OP_HANDLER( adca_ix )
{
	WORD t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_NZVC8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $aA ORA indexed -**0- */
OP_HANDLER( ora_ix )
{
	fetch_effective_address(m68_state);
	A |= RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $aB ADDA indexed ***** */
OP_HANDLER( adda_ix )
{
	WORD t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = A + t;
	CLR_HNZVC;
	SET_NZVC8(A,t,r);
	SET_H(A,t,r);
	A = r;
}

/* $aC CMPX (CMPY CMPS) indexed -**** */
OP_HANDLER( cmpx_ix )
{
	DWORD r,d;
	WORD b;
	fetch_effective_address(m68_state);
	b=RM16(m68_state, EAD);
	d = X;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $10aC CMPY indexed -**** */
OP_HANDLER( cmpy_ix )
{
	DWORD r,d;
	WORD b;
	fetch_effective_address(m68_state);
	b=RM16(m68_state, EAD);
	d = Y;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $11aC CMPS indexed -**** */
OP_HANDLER( cmps_ix )
{
	DWORD r,d;
	WORD b;
	fetch_effective_address(m68_state);
	b = RM16(m68_state, EAD);
	d = S;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $aD JSR indexed ----- */
OP_HANDLER( jsr_ix )
{
	fetch_effective_address(m68_state);
	PUSHWORD(pPC);
	PCD = EAD;
}

/* $aE LDX (LDY) indexed -**0- */
OP_HANDLER( ldx_ix )
{
	fetch_effective_address(m68_state);
	X=RM16(m68_state, EAD);
	CLR_NZV;
	SET_NZ16(X);
}

/* $10aE LDY indexed -**0- */
OP_HANDLER( ldy_ix )
{
	fetch_effective_address(m68_state);
	Y=RM16(m68_state, EAD);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $aF STX (STY) indexed -**0- */
OP_HANDLER( stx_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ16(X);
	WM16(m68_state, EAD, X);
}

/* $10aF STY indexed -**0- */
OP_HANDLER( sty_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ16(Y);
	WM16(m68_state, EAD, Y);
}

/* $b0 SUBA extended ?**** */
OP_HANDLER( suba_ex )
{
	WORD	  t,r;
	EXTBYTE(t);
	r = A - t;
	CLR_HNZVC;
	SET_NZVC8(A,t,r);
	A = r;
}

/* $b1 CMPA extended ?**** */
OP_HANDLER( cmpa_ex )
{
	WORD	  t,r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_NZVC8(A,t,r);
}

/* $b2 SBCA extended ?**** */
OP_HANDLER( sbca_ex )
{
	WORD	  t,r;
	EXTBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_HNZVC;
	SET_NZVC8(A,t,r);
	A = r;
}

/* $b3 SUBD (CMPD CMPU) extended -**** */
OP_HANDLER( subd_ex )
{
	DWORD r,d;
	WORD b;
	EXTWORD(b);
	d = D;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
	D = r;
}

/* $10b3 CMPD extended -**** */
OP_HANDLER( cmpd_ex )
{
	DWORD r,d;
	WORD b;
	EXTWORD(b);
	d = D;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $11b3 CMPU extended -**** */
OP_HANDLER( cmpu_ex )
{
	DWORD r,d;
	WORD b;
	EXTWORD(b);
	d = U;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $b4 ANDA extended -**0- */
OP_HANDLER( anda_ex )
{
	BYTE t;
	EXTBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b5 BITA extended -**0- */
OP_HANDLER( bita_ex )
{
	BYTE t,r;
	EXTBYTE(t);
	r = A & t;
	CLR_NZV; SET_NZ8(r);
}

/* $b6 LDA extended -**0- */
OP_HANDLER( lda_ex )
{
	EXTBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $b7 STA extended -**0- */
OP_HANDLER( sta_ex )
{
	CLR_NZV;
	SET_NZ8(A);
	EXTENDED;
	WM(EAD,A);
}

/* $b8 EORA extended -**0- */
OP_HANDLER( eora_ex )
{
	BYTE t;
	EXTBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b9 ADCA extended ***** */
OP_HANDLER( adca_ex )
{
	WORD t,r;
	EXTBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_HNZVC8(A,t,r);
	A = r;
}

/* $bA ORA extended -**0- */
OP_HANDLER( ora_ex )
{
	BYTE t;
	EXTBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $bB ADDA extended ***** */
OP_HANDLER( adda_ex )
{
	WORD t,r;
	EXTBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_HNZVC8(A,t,r);
	A = r;
}

/* $bC CMPX (CMPY CMPS) extended -**** */
OP_HANDLER( cmpx_ex )
{
	DWORD r,d;
	WORD b;
	EXTWORD(b);
	d = X;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $10bC CMPY extended -**** */
OP_HANDLER( cmpy_ex )
{
	DWORD r,d;
	WORD b;
	EXTWORD(b);
	d = Y;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $11bC CMPS extended -**** */
OP_HANDLER( cmps_ex )
{
	DWORD r,d;
	WORD b;
	EXTWORD(b);
	d = S;
	r = d - b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
}

/* $bD JSR extended ----- */
OP_HANDLER( jsr_ex )
{
	EXTENDED;
	PUSHWORD(pPC);
	PCD = EAD;
}

/* $bE LDX (LDY) extended -**0- */
OP_HANDLER( ldx_ex )
{
	EXTWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $10bE LDY extended -**0- */
OP_HANDLER( ldy_ex )
{
	EXTWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $bF STX (STY) extended -**0- */
OP_HANDLER( stx_ex )
{
	CLR_NZV;
	SET_NZ16(X);
	EXTENDED;
	WM16(m68_state, EAD, X);
}

/* $10bF STY extended -**0- */
OP_HANDLER( sty_ex )
{
	CLR_NZV;
	SET_NZ16(Y);
	EXTENDED;
	WM16(m68_state, EAD, Y);
}

/* $c0 SUBB immediate ?**** */
OP_HANDLER( subb_im )
{
	WORD	  t,r;
	IMMBYTE(t);
	r = B - t;
	CLR_HNZVC;
	SET_NZVC8(B,t,r);
	B = r;
}

/* $c1 CMPB immediate ?**** */
OP_HANDLER( cmpb_im )
{
	WORD	  t,r;
	IMMBYTE(t);
	r = B - t;
	CLR_NZVC; SET_NZVC8(B,t,r);
}

/* $c2 SBCB immediate ?**** */
OP_HANDLER( sbcb_im )
{
	WORD	  t,r;
	IMMBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_HNZVC;
	SET_NZVC8(B,t,r);
	B = r;
}

/* $c3 ADDD immediate -**** */
OP_HANDLER( addd_im )
{
	DWORD r,d;
	WORD b;
	IMMWORD(b);
	d = D;
	r = d + b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
	D = r;
}

/* $c4 ANDB immediate -**0- */
OP_HANDLER( andb_im )
{
	BYTE t;
	IMMBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $c5 BITB immediate -**0- */
OP_HANDLER( bitb_im )
{
	BYTE t,r;
	IMMBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $c6 LDB immediate -**0- */
OP_HANDLER( ldb_im )
{
	IMMBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* is this a legal instruction? */
/* $c7 STB immediate -**0- */
OP_HANDLER( stb_im )
{
	CLR_NZV;
	SET_NZ8(B);
	IMM8;
	WM(EAD,B);
}

/* $c8 EORB immediate -**0- */
OP_HANDLER( eorb_im )
{
	BYTE t;
	IMMBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $c9 ADCB immediate ***** */
OP_HANDLER( adcb_im )
{
	WORD t,r;
	IMMBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_HNZVC8(B,t,r);
	B = r;
}

/* $cA ORB immediate -**0- */
OP_HANDLER( orb_im )
{
	BYTE t;
	IMMBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $cB ADDB immediate ***** */
OP_HANDLER( addb_im )
{
	WORD t,r;
	IMMBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_HNZVC8(B,t,r);
	B = r;
}

/* $cC LDD immediate -**0- */
OP_HANDLER( ldd_im )
{
	IMMWORD(D);
	CLR_NZV;
	SET_NZ16(D);
}

/* is this a legal instruction? */
/* $cD STD immediate -**0- */
OP_HANDLER( std_im )
{
	CLR_NZV;
	SET_NZ16(D);
        IMM16;
	WM(EAD, D);
}

/* $cE LDU (LDS) immediate -**0- */
OP_HANDLER( ldu_im )
{
	IMMWORD(pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10cE LDS immediate -**0- */
OP_HANDLER( lds_im )
{
	IMMWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
	m68_state->intr |= 0x0010;
}

/* is this a legal instruction? */
/* $cF STU (STS) immediate -**0- */
OP_HANDLER( stu_im )
{
	CLR_NZV;
	SET_NZ16(U);
    IMM16;
	WM16(m68_state, EAD, U);
}

/* is this a legal instruction? */
/* $10cF STS immediate -**0- */
OP_HANDLER( sts_im )
{
	CLR_NZV;
	SET_NZ16(S);
    IMM16;
	WM16(m68_state, EAD, S);
}

/* $d0 SUBB direct ?**** */
OP_HANDLER( subb_di )
{
	WORD	  t,r;
	DIRBYTE(t);
	r = B - t;
	CLR_HNZVC;
	SET_NZVC8(B,t,r);
	B = r;
}

/* $d1 CMPB direct ?**** */
OP_HANDLER( cmpb_di )
{
	WORD	  t,r;
	DIRBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_NZVC8(B,t,r);
}

/* $d2 SBCB direct ?**** */
OP_HANDLER( sbcb_di )
{
	WORD	  t,r;
	DIRBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_HNZVC;
	SET_NZVC8(B,t,r);
	B = r;
}

/* $d3 ADDD direct -**** */
OP_HANDLER( addd_di )
{
	DWORD r,d;
	WORD b;
	DIRWORD(b);
	d = D;
	r = d + b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
	D = r;
}

/* $d4 ANDB direct -**0- */
OP_HANDLER( andb_di )
{
	BYTE t;
	DIRBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $d5 BITB direct -**0- */
OP_HANDLER( bitb_di )
{
	BYTE t,r;
	DIRBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $d6 LDB direct -**0- */
OP_HANDLER( ldb_di )
{
	DIRBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $d7 STB direct -**0- */
OP_HANDLER( stb_di )
{
	CLR_NZV;
	SET_NZ8(B);
	DIRECT;
	WM(EAD,B);
}

/* $d8 EORB direct -**0- */
OP_HANDLER( eorb_di )
{
	BYTE t;
	DIRBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $d9 ADCB direct ***** */
OP_HANDLER( adcb_di )
{
	WORD t,r;
	DIRBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_HNZVC8(B,t,r);
	B = r;
}

/* $dA ORB direct -**0- */
OP_HANDLER( orb_di )
{
	BYTE t;
	DIRBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $dB ADDB direct ***** */
OP_HANDLER( addb_di )
{
	WORD t,r;
	DIRBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_HNZVC8(B,t,r);
	B = r;
}

/* $dC LDD direct -**0- */
OP_HANDLER( ldd_di )
{
	DIRWORD(D);
	CLR_NZV;
	SET_NZ16(D);
}

/* $dD STD direct -**0- */
OP_HANDLER( std_di )
{
	CLR_NZV;
	SET_NZ16(D);
	DIRECT;
	WM16(m68_state, EAD, D);
}

/* $dE LDU (LDS) direct -**0- */
OP_HANDLER( ldu_di )
{
	DIRWORD(pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10dE LDS direct -**0- */
OP_HANDLER( lds_di )
{
	DIRWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
        m68_state->intr |= INTR_SLOAD;
//	m68_state->int_state |= M6809_LDS;
}

/* $dF STU (STS) direct -**0- */
OP_HANDLER( stu_di )
{
	CLR_NZV;
	SET_NZ16(U);
	DIRECT;
	WM16(m68_state, EAD, U);
}

/* $10dF STS direct -**0- */
OP_HANDLER( sts_di )
{
	CLR_NZV;
	SET_NZ16(S);
	DIRECT;
	WM16(m68_state, EAD, S);
}

/* $e0 SUBB indexed ?**** */
OP_HANDLER( subb_ix )
{
	WORD	  t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = B - t;
	CLR_HNZVC;
	SET_NZVC8(B,t,r);
	B = r;
}

/* $e1 CMPB indexed ?**** */
OP_HANDLER( cmpb_ix )
{
	WORD	  t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = B - t;
	CLR_NZVC;
	SET_NZVC8(B,t,r);
}

/* $e2 SBCB indexed ?**** */
OP_HANDLER( sbcb_ix )
{
	WORD	  t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = B - t - (CC & CC_C);
	CLR_HNZVC;
	SET_NZVC8(B,t,r);
	B = r;
}

/* $e3 ADDD indexed -**** */
OP_HANDLER( addd_ix )
{
	DWORD r,d;
    WORD b;
    fetch_effective_address(m68_state);
	b=RM16(m68_state, EAD);
	d = D;
	r = d + b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
	D = r;
}

/* $e4 ANDB indexed -**0- */
OP_HANDLER( andb_ix )
{
	fetch_effective_address(m68_state);
	B &= RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e5 BITB indexed -**0- */
OP_HANDLER( bitb_ix )
{
	BYTE r;
	fetch_effective_address(m68_state);
	r = B & RM(EAD);
	CLR_NZV;
	SET_NZ8(r);
}

/* $e6 LDB indexed -**0- */
OP_HANDLER( ldb_ix )
{
	fetch_effective_address(m68_state);
	B = RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e7 STB indexed -**0- */
OP_HANDLER( stb_ix )
{
	fetch_effective_address(m68_state);
    CLR_NZV;
	SET_NZ8(B);
	WM(EAD,B);
}

/* $e8 EORB indexed -**0- */
OP_HANDLER( eorb_ix )
{
	fetch_effective_address(m68_state);
	B ^= RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e9 ADCB indexed ***** */
OP_HANDLER( adcb_ix )
{
	WORD t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_HNZVC8(B,t,r);
	B = r;
}

/* $eA ORB indexed -**0- */
OP_HANDLER( orb_ix )
{
	fetch_effective_address(m68_state);
	B |= RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $eB ADDB indexed ***** */
OP_HANDLER( addb_ix )
{
	WORD t,r;
	fetch_effective_address(m68_state);
	t = RM(EAD);
	r = B + t;
	CLR_HNZVC;
	SET_HNZVC8(B,t,r);
	B = r;
}

/* $eC LDD indexed -**0- */
OP_HANDLER( ldd_ix )
{
	fetch_effective_address(m68_state);
	D=RM16(m68_state, EAD);
	CLR_NZV; SET_NZ16(D);
}

/* $eD STD indexed -**0- */
OP_HANDLER( std_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ16(D);
	WM16(m68_state, EAD, D);
}

/* $eE LDU (LDS) indexed -**0- */
OP_HANDLER( ldu_ix )
{
	fetch_effective_address(m68_state);
	U=RM16(m68_state, EAD);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10eE LDS indexed -**0- */
OP_HANDLER( lds_ix )
{
	fetch_effective_address(m68_state);
	S=RM16(m68_state, EAD);
	CLR_NZV;
	SET_NZ16(S);
        m68_state->intr |= INTR_SLOAD;
//	m68_state->int_state |= M6809_LDS;
}

/* $eF STU (STS) indexed -**0- */
OP_HANDLER( stu_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ16(U);
	WM16(m68_state, EAD, U);
}

/* $10eF STS indexed -**0- */
OP_HANDLER( sts_ix )
{
	fetch_effective_address(m68_state);
	CLR_NZV;
	SET_NZ16(S);
	WM16(m68_state, EAD, S);
}

/* $f0 SUBB extended ?**** */
OP_HANDLER( subb_ex )
{
	WORD	  t,r;
	EXTBYTE(t);
	r = B - t;
	CLR_HNZVC;
	SET_NZVC8(B,t,r);
	B = r;
}

/* $f1 CMPB extended ?**** */
OP_HANDLER( cmpb_ex )
{
	WORD	  t,r;
	EXTBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_NZVC8(B,t,r);
}

/* $f2 SBCB extended ?**** */
OP_HANDLER( sbcb_ex )
{
	WORD	  t,r;
	EXTBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_HNZVC;
	SET_NZVC8(B,t,r);
	B = r;
}

/* $f3 ADDD extended -**** */
OP_HANDLER( addd_ex )
{
	DWORD r,d;
	WORD b;
	EXTWORD(b);
	d = D;
	r = d + b;
	CLR_NZVC;
	SET_NZVC16(d,b,r);
	D = r;
}

/* $f4 ANDB extended -**0- */
OP_HANDLER( andb_ex )
{
	BYTE t;
	EXTBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $f5 BITB extended -**0- */
OP_HANDLER( bitb_ex )
{
	BYTE t,r;
	EXTBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $f6 LDB extended -**0- */
OP_HANDLER( ldb_ex )
{
	EXTBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $f7 STB extended -**0- */
OP_HANDLER( stb_ex )
{
	CLR_NZV;
	SET_NZ8(B);
	EXTENDED;
	WM(EAD,B);
}

/* $f8 EORB extended -**0- */
OP_HANDLER( eorb_ex )
{
	BYTE t;
	EXTBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $f9 ADCB extended ***** */
OP_HANDLER( adcb_ex )
{
	WORD t,r;
	EXTBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_HNZVC8(B,t,r);
	B = r;
}

/* $fA ORB extended -**0- */
OP_HANDLER( orb_ex )
{
	BYTE t;
	EXTBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $fB ADDB extended ***** */
OP_HANDLER( addb_ex )
{
	WORD t,r;
	EXTBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_HNZVC8(B,t,r);
	B = r;
}

/* $fC LDD extended -**0- */
OP_HANDLER( ldd_ex )
{
	EXTWORD(D);
	CLR_NZV;
	SET_NZ16(D);
}

/* $fD STD extended -**0- */
OP_HANDLER( std_ex )
{
	CLR_NZV;
	SET_NZ16(D);
	EXTENDED;
	WM16(m68_state, EAD, D);
}

/* $fE LDU (LDS) extended -**0- */
OP_HANDLER( ldu_ex )
{
	EXTWORD(pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10fE LDS extended -**0- */
OP_HANDLER( lds_ex )
{
	EXTWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
        m68_state->intr |= INTR_SLOAD;
//	m68_state->int_state |= M6809_LDS;
}

/* $fF STU (STS) extended -**0- */
OP_HANDLER( stu_ex )
{
	CLR_NZV;
	SET_NZ16(U);
	EXTENDED;
	WM16(m68_state, EAD, U);
}

/* $10fF STS extended -**0- */
OP_HANDLER( sts_ex )
{
	CLR_NZV;
	SET_NZ16(S);
	EXTENDED;
	WM16(m68_state, EAD, S);
}

/* $10xx opcodes */
OP_HANDLER( pref10 )
{
	BYTE ireg2 = ROP_ARG(PCD);
	PC++;
	switch( ireg2 )
	{
		case 0x21: lbrn(m68_state);		m68_state->cycle=5;	break;
		case 0x22: lbhi(m68_state);		m68_state->cycle=5;	break;
		case 0x23: lbls(m68_state);		m68_state->cycle=5;	break;
		case 0x24: lbcc(m68_state);		m68_state->cycle=5;	break;
		case 0x25: lbcs(m68_state);		m68_state->cycle=5;	break;
		case 0x26: lbne(m68_state);		m68_state->cycle=5;	break;
		case 0x27: lbeq(m68_state);		m68_state->cycle=5;	break;
		case 0x28: lbvc(m68_state);		m68_state->cycle=5;	break;
		case 0x29: lbvs(m68_state);		m68_state->cycle=5;	break;
		case 0x2a: lbpl(m68_state);		m68_state->cycle=5;	break;
		case 0x2b: lbmi(m68_state);		m68_state->cycle=5;	break;
		case 0x2c: lbge(m68_state);		m68_state->cycle=5;	break;
		case 0x2d: lblt(m68_state);		m68_state->cycle=5;	break;
		case 0x2e: lbgt(m68_state);		m68_state->cycle=5;	break;
		case 0x2f: lble(m68_state);		m68_state->cycle=5;	break;

		case 0x3f: swi2(m68_state);		m68_state->cycle=19;	break;

		case 0x83: cmpd_im(m68_state);	m68_state->cycle=5;	break;
		case 0x8c: cmpy_im(m68_state);	m68_state->cycle=5;	break;
		case 0x8e: ldy_im(m68_state);	m68_state->cycle=4;	break;
		case 0x8f: sty_im(m68_state);	m68_state->cycle=4;	break;

		case 0x93: cmpd_di(m68_state);	m68_state->cycle=7;	break;
		case 0x9c: cmpy_di(m68_state);	m68_state->cycle=7;	break;
		case 0x9e: ldy_di(m68_state);	m68_state->cycle=6;	break;
		case 0x9f: sty_di(m68_state);	m68_state->cycle=6;	break;

		case 0xa3: cmpd_ix(m68_state);	m68_state->cycle=7;	break;
		case 0xac: cmpy_ix(m68_state);	m68_state->cycle=7;	break;
		case 0xae: ldy_ix(m68_state);	m68_state->cycle=6;	break;
		case 0xaf: sty_ix(m68_state);	m68_state->cycle=6;	break;

		case 0xb3: cmpd_ex(m68_state);	m68_state->cycle=8;	break;
		case 0xbc: cmpy_ex(m68_state);	m68_state->cycle=8;	break;
		case 0xbe: ldy_ex(m68_state);	m68_state->cycle=7;	break;
		case 0xbf: sty_ex(m68_state);	m68_state->cycle=7;	break;

		case 0xce: lds_im(m68_state);	m68_state->cycle=4;	break;
		case 0xcf: sts_im(m68_state);	m68_state->cycle=4;	break;

		case 0xde: lds_di(m68_state);	m68_state->cycle=6;	break;
		case 0xdf: sts_di(m68_state);	m68_state->cycle=6;	break;

		case 0xee: lds_ix(m68_state);	m68_state->cycle=6;	break;
		case 0xef: sts_ix(m68_state);	m68_state->cycle=6;	break;

		case 0xfe: lds_ex(m68_state);	m68_state->cycle=7;	break;
		case 0xff: sts_ex(m68_state);	m68_state->cycle=7;	break;

		default:   IIError(m68_state);						break;
	}
}

/* $11xx opcodes */
OP_HANDLER( pref11 )
{
	BYTE ireg2 = ROP_ARG(PCD);
	PC++;
	switch( ireg2 )
	{
		case 0x3f: swi3(m68_state);		m68_state->cycle=19;	break;

		case 0x83: cmpu_im(m68_state);	m68_state->cycle=5;	break;
		case 0x8c: cmps_im(m68_state);	m68_state->cycle=5;	break;

		case 0x93: cmpu_di(m68_state);	m68_state->cycle=7;	break;
		case 0x9c: cmps_di(m68_state);	m68_state->cycle=7;	break;

		case 0xa3: cmpu_ix(m68_state);	m68_state->cycle=7;	break;
		case 0xac: cmps_ix(m68_state);	m68_state->cycle=7;	break;

		case 0xb3: cmpu_ex(m68_state);	m68_state->cycle=8;	break;
		case 0xbc: cmps_ex(m68_state);	m68_state->cycle=8;	break;

		default:   IIError(m68_state);						break;
	}
}

