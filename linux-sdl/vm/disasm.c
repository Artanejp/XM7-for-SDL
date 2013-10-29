/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2013 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2013 Ryu Takegami
 *
 *      [ 逆アセンブラ ]
 */

#include <string.h>
#include "xm7.h"
#if (XM7_VER == 1 && defined(JSUB))
#include "jsubsys.h"
#endif

/*
 *      スタティック ワーク
 */
//static int      cputype;	/* CPU種別 */
//static BYTE     opc;		/* オペコード */
//static WORD     pc;		/* 実行前PC */
//static WORD     addpc;		/* PC加算値(命令長) */
//static char     linebuf[32];	/* 逆アセンブル出力バッファ */

/*
 *      例外系1(0x00)・インヘレントA(0x40)・インヘレントB(0x50)テーブル
 *        インヘレントモード時はレジスタ名が追加される
 *        0x0e のみ例外
 */
static const char *inh_tbl[] = {
    "NEG",
    "NEG",
    "NGC",
    "COM",
    "LSR",
    "LSR",
    "ROR",
    "ASR",
    "LSL",
    "ROL",
    "DEC",
    "DCC",
    "INC",
    "TST",
    "CLC",			/* 0x0e = JMP */
    "CLR"
};

/*
 *      例外系2(0x10)テーブル
 */
static const char *except2_tbl[] = {
    NULL,
    NULL,
    "NOP",
    "SYNC",
    "HALT",
    "HALT",
    "LBRA",
    "LBSR",
    "ASLCC",
    "DAA",
    "ORCC",
    "NOP",
    "ANDCC",
    "SEX",
    "EXG",
    "TFR"
};

/*
 *      ブランチ系(0x20)・ロングブランチ系(0x10 0x20)テーブル
 *        ロングブランチ時は先頭に L が追加される
 */
static const char *branch_tbl[] = {
    "BRA",
    "BRN",
    "BHI",
    "BLS",
    "BCC",
    "BCS",
    "BNE",
    "BEQ",
    "BVC",
    "BVS",
    "BPL",
    "BMI",
    "BGE",
    "BLT",
    "BGT",
    "BLE"
};

/*
 *      LEA、スタック系(0x30)テーブル
 */
static const char *leastack_tbl[] = {
    "LEAX",
    "LEAY",
    "LEAS",
    "LEAU",
    "PSHS",
    "PULS",
    "PSHU",
    "PULU",
    "ANDCC",
    "RTS",
    "ABX",
    "RTI",
    "CWAI",
    "MUL",
    "RST",
    "SWI"
};

/*
 *      インヘレントM(0x60, 0x70)テーブル
 */
static const char *inhm_tbl[] = {
    "NEG",
    "NEG",
    "NGC",
    "COM",
    "LSR",
    "LSR",
    "ROR",
    "ASR",
    "LSL",
    "ROL",
    "DEC",
    "DCC",
    "INC",
    "TST",
    "JMP",			/* 0x6e, 0x7e = JMP */
    "CLR"
};

/*
 *      Aレジスタ、Xレジスタ系(0x80)テーブル
 */
static const char *regax_tbl[] = {
    "SUBA",
    "CMPA",
    "SBCA",
    "SUBD",
    "ANDA",
    "BITA",
    "LDA",
    "STA",
    "EORA",
    "ADCA",
    "ORA",
    "ADDA",
    "CMPX",
    "JSR",
    "LDX",
    "STX"
};

/*
 *      Bレジスタ、Dレジスタ、Uレジスタ系(0xc0)テーブル
 */
static const char *regbdu_tbl[] = {
    "SUBB",
    "CMPB",
    "SBCB",
    "ADDD",
    "ANDB",
    "BITB",
    "LDB",
    "STB",
    "EORB",
    "ADCB",
    "ORB",
    "ADDB",
    "LDD",
    "STD",
    "LDU",
    "STU"
};

/*
 *      TFR/EXGテーブル (6809)
 */
static char    *tfrexg_tbl[] = {
    "D",
    "X",
    "Y",
    "U",
    "S",
    "PC",
    NULL,
    NULL,
    "A",
    "B",
    "CC",
    "DP",
    NULL,
    NULL
};

/*
 *      PSH/PULテーブル
 */
static char    *pshpul_tbl[] = {
    "CC",
    "A",
    "B",
    "DP",
    "X",
    "Y",
    NULL,			/* S or U */
    "PC"
};

/*
 *      インデックステーブル
 */
static char    *idx_tbl[] = {
    "X",
    "Y",
    "U",
    "S"
};

/*-[ 汎用サブ ]-------------------------------------------------------------*/

/*
 *      データフェッチ
 */
static BYTE     FASTCALL
fetch(int cputype, WORD *pc, WORD *addpc)
{
    BYTE            dat;

    switch (cputype) {
    case MAINCPU:
	dat = mainmem_readbnio((WORD) (*pc + *addpc));
	break;

    case SUBCPU:
	dat = submem_readbnio((WORD) (*pc + *addpc));
	break;

#if (XM7_VER == 1) && defined(JSUB)
    case JSUBCPU:
	dat = jsubmem_readbnio((WORD) (*pc + *addpc));
	break;
#endif

    default:
	ASSERT(FALSE);
	break;
    }

    *addpc = *addpc + 1;
    return dat;
}

/*
 *      データ読み出し
 */
static BYTE     FASTCALL
read_byte(int cputype, WORD addr)
{
    BYTE            dat;

    switch (cputype) {
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

/*
 *      16進1桁セット サブ
 */
static void     FASTCALL
sub1hex(BYTE dat, char *buffer)
{
    char            buf[2];

    /*
     * assert 
     */
    ASSERT(buffer);

    buf[0] = (char) (dat + 0x30);
    if (dat > 9) {
	buf[0] = (char) (dat + 0x37);
    }

    buf[1] = '\0';
    strcat(buffer, buf);
}

/*
 *      16進2桁セット サブ
 */
static void     FASTCALL
sub2hex(BYTE dat, char *buffer)
{
    sub1hex((BYTE) (dat >> 4), buffer);
    sub1hex((BYTE) (dat & 0x0f), buffer);
}

/*
 *      16進2桁セット
 */
static void     FASTCALL
set2hex(BYTE dat, char *linebuf)
{
    strcat(linebuf, "$");

    sub2hex(dat, linebuf);
}

/*
 *      16進4桁セット サブ
 */
static void     FASTCALL
sub4hex(WORD dat, char *buffer)
{
    sub2hex((BYTE) (dat >> 8), buffer);
    sub2hex((BYTE) (dat & 0xff), buffer);
}

/*
 *      16進4桁セット
 */
static void     FASTCALL
set4hex(WORD dat, char *linebuf)
{
    strcat(linebuf, "$");
#if 1
    sub4hex(dat, linebuf);
#else
    sub2hex((BYTE) (dat >> 8), linebuf);
    sub2hex((BYTE) (dat & 0xff), linebuf);
#endif
}

/*
 *      10進2桁セット
 */
static void     FASTCALL
set2dec(BYTE dat, char *linebuf)
{
    char            buf[2];

    buf[1] = '\0';

    /*
     * 上位桁 
     */
    buf[0] = (char) (dat / 10);
    if (buf[0] > 0) {
	buf[0] += (char) 0x30;
	strcat(linebuf, buf);
    }

    /*
     * 下位桁 
     */
    buf[0] = (char) (dat % 10);
    buf[0] += (char) 0x30;
    strcat(linebuf, buf);
}

/*
 *      未定義
 */
static void     FASTCALL
notdef(int cpu, WORD *pc, WORD *addpc, char *linebuf)
{
    strcat(linebuf, "?");
}

/*-[ アドレッシングモード処理 ]---------------------------------------------*/

/*
 *      リラティブモード(1バイト)
 */
static void     FASTCALL
rel1(int cpu, WORD *pc, WORD *addpc, char *linebuf)
{
    BYTE            opr;

    /*
     * オペランド取得 
     */
    opr = fetch(cpu, pc, addpc);

    /*
     * セット 
     */
    if (opr <= 0x7f) {
	set4hex((WORD) (*pc + *addpc + (BYTE) opr), linebuf);
    } else {
	set4hex((WORD) (*pc + *addpc - (BYTE) (~opr + 1)), linebuf);
    }
}

/*
 *      リラティブモード(2バイト)
 */
static void     FASTCALL
rel2(int cpu, WORD *pc, WORD *addpc, char *linebuf)
{
    WORD            dat;

    /*
     * オペランド取得 
     */
    dat = (WORD) (fetch(cpu, pc, addpc) << 8);
    dat |= (WORD) fetch(cpu, pc, addpc);

    /*
     * セット 
     */
    if (dat <= 0x7fff) {
	set4hex((WORD) (*pc + *addpc + dat), linebuf);
    } else {
	set4hex((WORD) (*pc + *addpc - (~dat + 1)), linebuf);
    }
}

/*
 *      ダイレクトモード
 */
static void     FASTCALL
direct(int cpu, WORD *pc, WORD *addpc, char *linebuf)
{
    BYTE            opr;

    /*
     * オペランド取得 
     */
    opr = fetch(cpu, pc, addpc);

    /*
     * セット 
     */
    strcat(linebuf, "<");
    set2hex(opr, linebuf);
}

/*
 *      エクステンドモード
 */
static void     FASTCALL
extend(int cpu, WORD *pc, WORD *addpc, char *linebuf)
{
    WORD            dat;

    /*
     * オペランド取得 
     */
    dat = (WORD) (fetch(cpu, pc, addpc) << 8);
    dat |= (WORD) fetch(cpu, pc, addpc);

    /*
     * セット 
     */
    set4hex(dat, linebuf);
}

/*
 *      イミディエイトモード(1バイト)
 */
static void     FASTCALL
imm1(int cpu, WORD *pc, WORD *addpc, char *linebuf)
{
    BYTE            opr;

    /*
     * オペランド取得 
     */
    opr = fetch(cpu, pc, addpc);

    /*
     * セット 
     */
    strcat(linebuf, "#");
    set2hex(opr, linebuf);
}

/*
 *      イミディエイトモード(2バイト)
 */
static void     FASTCALL
imm2(int cpu, WORD *pc, WORD *addpc, char *linebuf)
{
    WORD            dat;

    /*
     * オペランド取得 
     */
    dat = (WORD) (fetch(cpu, pc, addpc) << 8);
    dat |= (WORD) (fetch(cpu, pc, addpc));

    /*
     * セット 
     */
    strcat(linebuf, "#");
    set4hex(dat, linebuf);
}

/*
 *      インデックスモード
 */
static void     FASTCALL
idx(int cpu, WORD *pc, WORD *addpc, char *linebuf)
{
    BYTE            opr;
    BYTE            high,
                    low;
    BYTE            offset;
    WORD            woffset;

    /*
     * オペランド取得 
     */
    opr = fetch(cpu, pc, addpc);
    high = (BYTE) (opr & 0xf0);
    low = (BYTE) (opr & 0x0f);

    /*
     * 0x00〜0x7fは5bitオフセット 
     */
    if (opr < 0x80) {
	if (opr & 0x10) {
	    /*
	     * マイナス 
	     */
	    offset = (BYTE) (~((opr & 0x0f) | 0xf0) + 1);
	    strcat(linebuf, "-");
	    set2dec(offset, linebuf);
	} else {
	    /*
	     * プラス 
	     */
	    offset = low;
	    set2dec(offset, linebuf);
	}

	strcat(linebuf, ",");

	/*
	 * X, Y, U, S 
	 */
	offset = (BYTE) ((opr & 0x60) >> 5);
	ASSERT(offset <= 3);
	strcat(linebuf, idx_tbl[offset]);
	return;
    }

    /*
     * 0x8e,0xae,0xce,0xeeは例外 $FFFF 
     */
    /*
     * 0x9e,0xbe,0xde,0xfeは例外[$FFFF] 
     */
    if (low == 0x0e) {
	if (high & 0x10) {
	    strcat(linebuf, "[");
	}
	woffset = 0xffff;
	set4hex(woffset, linebuf);
	if (high & 0x10) {
	    strcat(linebuf, "]");
	}
	return;
    }
    /*
     * 0x8f,0xaf,0xcf,0xefは例外 addr 
     */
    /*
     * 0x9f,0xbf,0xdf,0xffは例外[addr] 
     */
    if (low == 0x0f) {
	if (high & 0x10) {
	    strcat(linebuf, "[");
	}
	woffset = (WORD) fetch(cpu, pc, addpc);
	woffset = (WORD) ((woffset << 8) + (WORD) fetch(cpu, pc, addpc));
	set4hex(woffset, linebuf);
	if (high & 0x10) {
	    strcat(linebuf, "]");
	}
	return;
    }

    /*
     * 0x80以上で、下位が0,1,2,3はオートインクリメントorデクリメント 
     */
    if (low < 4) {
	if (high & 0x10) {
	    strcat(linebuf, "[");
	}
	strcat(linebuf, ",");

	/*
	 * オートデクリメント 
	 */
	if (low >= 2) {
	    strcat(linebuf, "-");
	}
	if (low == 3) {
	    strcat(linebuf, "-");
	}

	/*
	 * X, Y, U, S 
	 */
	offset = (BYTE) ((opr & 0x60) >> 5);
	ASSERT(offset <= 3);
	strcat(linebuf, idx_tbl[offset]);

	/*
	 * オートインクリメント 
	 */
	if (low < 2) {
	    strcat(linebuf, "+");
	}
	if (low == 1) {
	    strcat(linebuf, "+");
	}

	if (high & 0x10) {
	    strcat(linebuf, "]");
	}
	return;
    }

    /*
     * 下位4,5,6,Bはレジスタオフセット 
     */
    if ((low == 4) || (low == 5) || (low == 6) || (low == 11)) {
	if (high & 0x10) {
	    strcat(linebuf, "[");
	}

	switch (low) {
	case 4:
	    strcat(linebuf, ",");
	    break;
	case 5:
	    strcat(linebuf, "B,");
	    break;
	case 6:
	    strcat(linebuf, "A,");
	    break;
	case 11:
	    strcat(linebuf, "D,");
	    break;
	default:
	    ASSERT(FALSE);
	    break;
	}

	/*
	 * X, Y, U, S 
	 */
	offset = (BYTE) ((opr & 0x60) >> 5);
	ASSERT(offset <= 3);
	strcat(linebuf, idx_tbl[offset]);

	if (high & 0x10) {
	    strcat(linebuf, "]");
	}
	return;
    }

    /*
     * 下位8,Cは8bitオフセット 
     */
    if ((low == 8) || (low == 12)) {
	if (high & 0x10) {
	    strcat(linebuf, "[");
	}

	offset = fetch(cpu, pc, addpc);

	/*
	 * X, Y, U, S, PCR 
	 */
	if (low == 12) {
	    if (offset >= 0x80) {
		woffset = (WORD) (0xff00 + offset);
	    } else {
		woffset = offset;
	    }
	    set4hex((WORD) (*pc + *addpc + woffset), linebuf);
	    strcat(linebuf, ",");
	    strcat(linebuf, "PCR");
	} else {
	    if (offset >= 0x80) {
		offset = (BYTE) (~offset + 1);
		strcat(linebuf, "-");
	    }
	    set2hex(offset, linebuf);
	    strcat(linebuf, ",");
	    offset = (BYTE) ((opr & 0x60) >> 5);
	    ASSERT(offset <= 3);
	    strcat(linebuf, idx_tbl[offset]);
	}

	if (high & 0x10) {
	    strcat(linebuf, "]");
	}
	return;
    }

    /*
     * 下位9,Dは16bitオフセット 
     */
    if ((low == 9) || (low == 13)) {
	if (high & 0x10) {
	    strcat(linebuf, "[");
	}

	woffset = (WORD) fetch(cpu, pc, addpc);
	woffset = (WORD) ((woffset << 8) + (WORD) fetch(cpu, pc, addpc));

	/*
	 * X, Y, U, S, PCR 
	 */
	if (low == 13) {
	    set4hex((WORD) (woffset + *pc + *addpc), linebuf);
	    strcat(linebuf, ",");
	    strcat(linebuf, "PCR");
	} else {
	    if (woffset >= 0x8000) {
		woffset = (WORD) (~woffset + 1);
		strcat(linebuf, "-");
	    }
	    set4hex(woffset, linebuf);
	    strcat(linebuf, ",");
	    offset = (BYTE) ((opr & 0x60) >> 5);
	    ASSERT(offset <= 3);
	    strcat(linebuf, idx_tbl[offset]);
	}

	if (high & 0x10) {
	    strcat(linebuf, "]");
	}
	return;
    }

    /*
     * それ以外は未定義 
     */
    notdef(cpu, pc, addpc, linebuf);
}

/*
 *      TFR,EXG,レジスタ間演算
 */
static void     FASTCALL
tfrexg(int cpu, WORD *pc, WORD *addpc, char *linebuf)
{
    BYTE            opr;
    const char    **tfrexgtbl;

    /*
     * オペランド取得 
     */
    opr = fetch(cpu, pc, addpc);

    tfrexgtbl = (const char **) tfrexg_tbl;

    /*
     * 作成 
     */
    if (tfrexgtbl[(opr & 0xf0) >> 4] == NULL) {
	linebuf[0] = '\0';
	notdef(cpu, pc, addpc, linebuf);
	return;
    }
    strcat(linebuf, tfrexgtbl[(opr & 0xf0) >> 4]);
    strcat(linebuf, ",");
    if (tfrexgtbl[opr & 0x0f] == NULL) {
	linebuf[0] = '\0';
	notdef(cpu, pc, addpc, linebuf);
	return;
    }
    strcat(linebuf, tfrexgtbl[opr & 0x0f]);
}

/*
 *      PSH,PUL
 */
static void     FASTCALL
pshpul(int cpu, WORD *pc, WORD *addpc, char *linebuf)
{
    BYTE            opr;
    char            sreg[2];
    int             i;
    int             flag;

    /*
     * オペランド取得 
     */
    opr = fetch(cpu, pc, addpc);

    /*
     * S,Uを決定する 
     */
    if (linebuf[3] == 'S') {
	sreg[0] = 'U';
    } else {
	sreg[0] = 'S';
    }
    sreg[1] = '\0';

    /*
     * 8回評価 
     */
    flag = FALSE;
    for (i = 0; i < 8; i++) {
	if (opr & 0x01) {
	    if (flag) {
		strcat(linebuf, ",");
	    }
	    if (i == 6) {
		/*
		 * S,U 
		 */
		strcat(linebuf, sreg);
	    } else {
		/*
		 * それ以外 
		 */
		strcat(linebuf, pshpul_tbl[i]);
	    }
	    flag = TRUE;
	}
	opr >>= 1;
    }
}

/*
 *      ニーモニック連結
 */
static void     FASTCALL
strcat_mnemonic(const char *s, char *linebuf)
{
    char            tmp[7];

    tmp[6] = '\0';
    memset(tmp, 0x20, sizeof(tmp) - 1);
    memcpy(tmp, s, strlen(s));
    strcat(linebuf, tmp);
}

/*-[ オペコード処理 ]-------------------------------------------------------*/

/*
 *      ページ2(0x10)
 */
static void     FASTCALL
page2(int cpu, WORD *pc, WORD *addpc, char *linebuf)
{
    BYTE            high,
                    low;
    char            tmp[5];
    BYTE             opc;

    /*
     * オペコードを再度取得 
     */
    opc = fetch(cpu, pc, addpc);
    high = (BYTE) (opc & 0xf0);
    low = (BYTE) (opc & 0x0f);

    /*
     * 0x20台はロングブランチ 
     */
    if (high == 0x20) {
	tmp[0] = 'L';
	strcpy(&tmp[1], branch_tbl[low]);
	strcat_mnemonic(tmp, linebuf);
	rel2(cpu, pc, addpc, linebuf);
	return;
    }

    /*
     * 0x3fはSWI2 
     */
    if (opc == 0x3f) {
	strcat_mnemonic("SWI2", linebuf);
	return;
    }

    /*
     * 0x8dはLBSR 
     */
    if (opc == 0x8d) {
	strcat_mnemonic("LBSR", linebuf);
	return;
    }

    /*
     * 0xc0以上はSレジスタ 
     */
    if (opc >= 0xc0) {
	if (low <= 0x0d) {
	    notdef(cpu, pc, addpc, linebuf);
	    return;
	}
	if (opc == 0xcf) {
	    notdef(cpu, pc, addpc, linebuf);
	    return;
	}
	/*
	 * 0x?eはLDS、0x?fはSTS 
	 */
	if (low == 0x0e) {
	    strcat_mnemonic("LDS", linebuf);
	}
	if (low == 0x0f) {
	    strcat_mnemonic("STS", linebuf);
	}
	/*
	 * LDS,STS アドレッシングモード 
	 */
	switch (high) {
	case 0xc0:
	    imm2(cpu, pc, addpc, linebuf);
	    break;
	case 0xd0:
	    direct(cpu, pc, addpc, linebuf);
	    break;
	case 0xe0:
	    idx(cpu, pc, addpc, linebuf);
	    break;
	case 0xf0:
	    extend(cpu, pc, addpc, linebuf);
	    break;
	default:
	    ASSERT(FALSE);
	    break;
	}
	return;
    }

    /*
     * 0x80以上はCMPD, CMPY, LDY, STY 
     */
    if (opc >= 0x80) {
	switch (low) {
	case 0x03:
	    strcat_mnemonic("CMPD", linebuf);
	    break;
	case 0x0c:
	    strcat_mnemonic("CMPY", linebuf);
	    break;
	case 0x0e:
	    strcat_mnemonic("LDY", linebuf);
	    break;
	case 0x0f:
	    if (high == 0x80) {
		notdef(cpu, pc, addpc, linebuf);
		return;
	    } else {
		strcat_mnemonic("STY", linebuf);
	    }
	    break;
	default:
	    notdef(cpu, pc, addpc, linebuf);
	    return;
	}

	/*
	 * アドレッシングモード 
	 */
	switch (high) {
	case 0x80:
	    imm2(cpu, pc, addpc, linebuf);
	    break;
	case 0x90:
	    direct(cpu, pc, addpc, linebuf);
	    break;
	case 0xa0:
	    idx(cpu, pc, addpc, linebuf);
	    break;
	case 0xb0:
	    extend(cpu, pc, addpc, linebuf);
	    break;
	default:
	    ASSERT(FALSE);
	    break;
	}

	return;
    }

    /*
     * それ以外は未定義 
     */
    notdef(cpu, pc, addpc, linebuf);
}

/*
 *      ページ3(0x11)
 */
static void     FASTCALL
page3(int cpu, WORD *pc, WORD *addpc, char *linebuf)
{
    BYTE            high,
                    low;
    BYTE             opc;

    /*
     * オペコードを再度取得 
     */
    opc = fetch(cpu, pc, addpc);
    high = (BYTE) (opc & 0xf0);
    low = (BYTE) (opc & 0x0f);

    /*
     * 0x3fはSWI3 
     */
    if (opc == 0x3f) {
	strcat_mnemonic("SWI3" , linebuf);
	return;
    }

    /*
     * 上位が8,9,A,B 
     */
    if ((high >= 0x80) && (high <= 0xb0)) {
	/*
	 * 下位チェック 
	 */
	switch (low) {
	case 3:
	    strcat_mnemonic("CMPU", linebuf);
	    break;
	case 12:
	    strcat_mnemonic("CMPS", linebuf);
	    break;
	default:
	    notdef(cpu, pc, addpc, linebuf);
	    return;
	}

	/*
	 * アドレッシングモード 
	 */
	switch (high) {
	case 0x80:
	    imm2(cpu, pc, addpc, linebuf);
	    break;
	case 0x90:
	    direct(cpu, pc, addpc, linebuf);
	    break;
	case 0xa0:
	    idx(cpu, pc, addpc, linebuf);
	    break;
	case 0xb0:
	    extend(cpu, pc, addpc, linebuf);
	    break;
	default:
	    ASSERT(FALSE);
	    break;
	}

	return;
    }

    /*
     * それ以外は未定義 
     */
    notdef(cpu, pc, addpc, linebuf);
}

/*
 *      例外系1(0x00)
 */
static void     FASTCALL
except1(int cpu, BYTE opc, WORD *pc, WORD *addpc, char *linebuf)
{
    if ((opc & 0x0f) == 0x0e) {
	/*
	 * 0x0eはJMP 
	 */
	strcat_mnemonic("JMP", linebuf);
    } else {
	strcat_mnemonic(inh_tbl[opc & 0x0f], linebuf);
    }

    /*
     * すべてダイレクト 
     */
    direct(cpu, pc, addpc, linebuf);
}

/*
 *      例外系2(0x10)
 */
static void     FASTCALL
except2(int cpu, BYTE opc, WORD *pc, WORD *addpc, char *linebuf)
{
    /*
     * 0x10, 0x11で始まるページ 
     */
    if (opc == 0x10) {
	page2(cpu, pc, addpc, linebuf);
	return;
    }
    if (opc == 0x11) {
	page3(cpu, pc, addpc, linebuf);
	return;
    }

    /*
     * 未定義チェック 
     */
    if (except2_tbl[opc & 0x0f] == NULL) {
	notdef(cpu, pc, addpc, linebuf);
	return;
    }
    strcat_mnemonic(except2_tbl[opc & 0x0f], linebuf);

    /*
     * 0x16, 0x17はロングブランチ 
     */
    if ((opc == 0x16) || (opc == 0x17)) {
	rel2(cpu, pc, addpc, linebuf);;
	return;
    }

    /*
     * 0x1a, 0x1cはイミディエイト 
     */
    if ((opc == 0x1a) || (opc == 0x1c)) {
	imm1(cpu, pc, addpc, linebuf);
	return;
    }

    /*
     * 0x1e, 0x1fはTFR/EXG 
     */
    if ((opc == 0x1e) || (opc == 0x1f)) {
	tfrexg(cpu, pc, addpc, linebuf);
	return;
    }
}

/*
 *      ブランチ系(0x20)
 */
static void     FASTCALL
branch(int cpu, BYTE opc, WORD *pc, WORD *addpc, char *linebuf)
{
    strcat_mnemonic(branch_tbl[opc - 0x20], linebuf);
    rel1(cpu, pc, addpc, linebuf);
}

/*
 *      LEA,スタック系(0x30)
 */
static void     FASTCALL
leastack(int cpu, BYTE opc, WORD *pc, WORD *addpc, char *linebuf)
{
    /*
     * オペコード 
     */
    strcat_mnemonic(leastack_tbl[opc & 0x0f], linebuf);

    /*
     * LEAはインデックスのみ 
     */
    if (opc < 0x34) {
	idx(cpu, pc, addpc, linebuf);
	return;
    }

    /*
     * PSH,PULは専用 
     */
    if (opc < 0x38) {
	pshpul(cpu, pc, addpc, linebuf);
	return;
    }

    /*
     * CWAI/隠し命令ANDCCはイミディエイト 
     */
    if ((opc == 0x38) || (opc == 0x3c)) {
	imm1(cpu, pc, addpc, linebuf);
    }
}

/*
 *      インヘレントA,B,M系(0x40,0x50,0x60,0x70)
 */
static void     FASTCALL
inhabm(int cpu, BYTE opc, WORD *pc, WORD *addpc, char *linebuf)
{
    char            tmp[5];

    switch (opc >> 4) {
	/*
	 * A/Bレジスタ 
	 */
    case 0x4:
    case 0x5:
	strcpy(tmp, inh_tbl[opc & 0x0f]);
	tmp[3] = (char) ('A' + ((opc & 0x10) >> 4));
	tmp[4] = '\0';
	strcat_mnemonic(tmp, linebuf);
	break;
	/*
	 * メモリ、インデックス/エクステンド 
	 */
    case 0x6:
    case 0x7:
	strcat_mnemonic(inhm_tbl[opc & 0x0f], linebuf);
	if ((opc & 0xf0) == 0x60) {
	    idx(cpu, pc, addpc, linebuf);
	} else {
	    extend(cpu, pc, addpc, linebuf);
	}
	break;
    default:
	ASSERT(FALSE);
	break;
    }
}

/*
 *      Aレジスタ、Xレジスタ系(0x80, 0x90, 0xa0, 0xb0)
 *      Bレジスタ、Dレジスタ、Uレジスタ系(0xc0, 0xd0, 0xe0, 0xf0)
 */
static void     FASTCALL
regaxbdu(int cpu, BYTE opc, WORD *pc, WORD *addpc, char *linebuf)
{
    BYTE            opc2;

    /*
     * 内部処理の都合上、bit6を0に固定 
     */
    opc2 = (BYTE) (opc & 0xbf);

    /*
     * 0x87, 0x8f, 0xc7, 0xcfは隠し命令 
     */
    if ((opc2 == 0x87) || (opc2 == 0x8f)) {
	strcat_mnemonic("FLAG", linebuf);
	if (opc2 == 0x87) {
	    imm1(cpu, pc, addpc, linebuf);
	} else {
	    imm2(cpu, pc, addpc, linebuf);
	}
	return;
    }

    /*
     * 0x8dはBSR 
     */
    if (opc == 0x8d) {
	strcat_mnemonic("BSR", linebuf);
	rel1(cpu, pc, addpc, linebuf);
	return;
    }

    /*
     * 0xcdはHALT 
     */
    if (opc == 0xcd) {
	strcat_mnemonic("HALT", linebuf);
	return;
    }

    /*
     * それ以外 
     */
    if (opc & 0x40) {
	strcat_mnemonic(regbdu_tbl[opc & 0x0f], linebuf);
    } else {
	strcat_mnemonic(regax_tbl[opc & 0x0f], linebuf);
    }

    /*
     * アドレッシングモード別 
     */
    switch (opc2 >> 4) {
    case 0x8:
	if ((opc2 == 0x83) || (opc2 == 0x8c) || (opc2 == 0x8e)) {
	    imm2(cpu, pc, addpc, linebuf);
	} else {
	    imm1(cpu, pc, addpc, linebuf);
	}
	break;
    case 0x9:
	direct(cpu, pc, addpc, linebuf);
	break;
    case 0xa:
	idx(cpu, pc, addpc, linebuf);
	break;
    case 0xb:
	extend(cpu, pc, addpc, linebuf);
	break;
    default:
	ASSERT(FALSE);
	break;
    }
}

/*-[ メイン ]---------------------------------------------------------------*/

/*
 *      逆アセンブラ本体
 */
int             FASTCALL
disline(int cpu, WORD pcreg, char *buffer)
{
    int             i;
    int             j;
    BYTE            opc;
    WORD pc;
    WORD addpc;
    int cputype;
    char linebuf[128];

    /*
     * assert 
     */
#if XM7_VER == 1 && defined(JSUB)
    ASSERT((cpu == MAINCPU) || (cpu == SUBCPU) || (cpu == JSUBCPU));
#else
    ASSERT((cpu == MAINCPU) || (cpu == SUBCPU));
#endif
    ASSERT(buffer);

    /*
     * 初期設定 
     */
    cputype = cpu;
    pc = pcreg;
    addpc = 0;
    linebuf[0] = '\0';

    /*
     * 先頭のバイトを読み出し 
     */
    opc = fetch(cputype, &pc, &addpc);

    /*
     * グループ別 
     */
    switch ((int) (opc >> 4)) {
    case 0x0:
	except1(cputype, opc, &pc, &addpc, linebuf);
	break;
    case 0x1:
	except2(cputype, opc, &pc, &addpc, linebuf);
	break;
    case 0x2:
	branch(cputype, opc, &pc, &addpc, linebuf);
	break;
    case 0x3:
	leastack(cputype, opc, &pc, &addpc, linebuf);
	break;
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
	inhabm(cputype, opc, &pc, &addpc, linebuf);
	break;
    case 0x8:
    case 0x9:
    case 0xa:
    case 0xb:
    case 0xc:
    case 0xd:
    case 0xe:
    case 0xf:
	regaxbdu(cputype, opc, &pc, &addpc, linebuf);
	break;
    default:
	ASSERT(FALSE);
    }

    /*
     * 命令データをセット 
     */
    buffer[0] = '\0';

    sub4hex(pcreg, buffer);
    strcat(buffer, " ");
    for (i = 0; i < addpc; i++) {
	sub2hex(read_byte(cpu, (WORD) (pcreg + i)), buffer);
	strcat(buffer, " ");
    }

    /*
     * bufferが20バイト+'\0'になるよう調整する 
     */
    j = strlen(buffer);
    if (j < 20) {
	for (i = 0; i < 20 - j; i++) {
	    buffer[i + j] = ' ';
	}
	buffer[i + j] = '\0';
    }

    strcat(buffer, linebuf);
    return (int) addpc + pc;
}

/*
 * 逆アセンブル(デバッガ用)
 */
int FASTCALL disline2(int cpu, WORD pcreg, cpu6809_t *cpuset, char *buffer)
{
    int             i;
    int             j;
    BYTE opc;
    WORD pc;
    WORD addpc;
    int cputype;
    char linebuf[128];
   

    /*
     * assert 
     */
    ASSERT((cpu == MAINCPU) || (cpu == SUBCPU));
    ASSERT(buffer);
    ASSERT(cpuset);
    /*
     * 初期設定 
     */
    cputype = cpu;
    pc = pcreg;
    addpc = 0;
    linebuf[0] = '\0';

    /*
     * 先頭のバイトを読み出し 
     */
    opc = fetch(cputype, &pc, &addpc);

    /*
     * グループ別 
     */
    switch ((int) (opc >> 4)) {
    case 0x0:
	except1(cputype, opc, &pc, &addpc, linebuf);
	break;
    case 0x1:
	except2(cputype, opc, &pc, &addpc, linebuf);
	break;
    case 0x2:
	branch(cputype, opc, &pc, &addpc, linebuf);
	break;
    case 0x3:
	leastack(cputype, opc, &pc, &addpc, linebuf);
	break;
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
	inhabm(cputype, opc, &pc, &addpc, linebuf);
	break;
    case 0x8:
    case 0x9:
    case 0xa:
    case 0xb:
    case 0xc:
    case 0xd:
    case 0xe:
    case 0xf:
	regaxbdu(cputype, opc, &pc, &addpc, linebuf);
	break;
    default:
	ASSERT(FALSE);
    }

    /*
     * 命令データをセット 
     */
    buffer[0] = '\0';

    sub4hex(pcreg, buffer);
    strcat(buffer, " ");
    for (i = 0; i < addpc; i++) {
	sub2hex(read_byte(cputype, (WORD) (pcreg + i)), buffer);
	strcat(buffer, " ");
    }

    /*
     * bufferが20バイト+'\0'になるよう調整する 
     */
    j = strlen(buffer);
    if (j < 20) {
	for (i = 0; i < 20 - j; i++) {
	    buffer[i + j] = ' ';
	}
	buffer[i + j] = '\0';
    }

    strcat(buffer, linebuf);
    return (int) addpc + pc;
}
