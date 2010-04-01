/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN コントロールバー
 * ] 
 */  
    
#ifdef _XWIN
    
#include <gtk/gtk.h>
#include "xm7.h"
#include "keyboard.h"
#include "tapelp.h"
#include "display.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "fdc.h"
#include "sdl.h"
#include "sdl_bar.h"
#include "sdl_sch.h"
#include "sdl_draw.h"
#include "SDL/SDL_ttf.h"
#include <iconv.h>

#define COL_BLACK       0xff000000
#define COL_BLACKMASK   0xff010101
#define COL_RED         0xff0000ff
#define COL_BLUE        0xffff0000
#define COL_GREEN       0xff00ff00
#define COL_NORM        0xffffffff



/* デフォルトフォントが設定されてないときはフォントとしてIPAゴシックを使う */
#ifndef FUNC_FONT
#define FUNC_FONT "./ipagui.ttf" 
#endif

#ifndef STAT_FONT
#define STAT_FONT "./ipagui.ttf" 
#endif

#define FUNC_PT 16
#define STAT_PT 16
    
/*
 *  スタティック ワーク 
 */ 
static char     szCaption[132];	/* キャプション */
static char     szOldCaption[132];	/* キャプション */
static int     nCAP;		/* CAPキー */
static int     nKANA;		/* かなキー */
static int     nINS;		/* INSキー */
static int     nDrive[2];	/* フロッピードライブ */
static char    szDrive[2][16 + 1];	/* フロッピードライブ */
static char    szOldDrive[2][16+1];       /* フロッピードライブ(過去) */

static int     nTape;           /* テープ */
static int     nOldTape;        /* テープ(過去) */
static int     nInitialDrawFlag; /* 強制再表示フラグ */
static SDL_Surface      *pInsOn; /* INSキープリレンダリング(ON) */
static SDL_Surface      *pInsOff; /* INSキープリレンダリング(OFF) */
static SDL_Surface      *pKanaOn; /* カナキープリレンダリング(ON) */
static SDL_Surface      *pKanaOff; /* カナキープリレンダリング(OFF) */
static SDL_Surface      *pCapsOn; /* Capsキープリレンダリング(ON) */
static SDL_Surface      *pCapsOff; /* Capsキープリレンダリング(OFF) */
static SDL_Surface      *pFDRead[2]; /* Drive0 Read */
static SDL_Surface      *pFDWrite[2]; /* Drive0 Write */
static SDL_Surface      *pFDNorm[2]; /* Drive0 Normal */
static SDL_Surface      *pCMTRead; /* Tape Read */
static SDL_Surface      *pCMTWrite; /* Tape Write */
static SDL_Surface      *pCMTNorm; /* Tape Normal */
static SDL_Surface      *pCaption; /* Caption */
static SDL_Surface      *pStatusBar; /* ステータス表示バー */
static TTF_Font         *pStatusFont;
static TTF_Font         *pVFDFont;


/*-[ ステータスバー ]-------------------------------------------------------*/ 
/*
 *  ステータスバーの生成 
 */ 
void
CreateStatus(void) 
{
        SDL_Surface *p;
        SDL_Surface *tmp;
        SDL_Rect rec;
        SDL_Rect drec;
        TTF_Font *f;
        SDL_Color r, b, n , black;
        Uint32 rmask, gmask, bmask, amask;
        int i;

        /*
         * RECT INS
         */
        r.r = 255;
        r.g = 0;
        r.b = 0;

        b.r = 0;
        b.g = 0;
        b.b = 255;

        n.r = 255;
        n.g = 255;
        n.b = 255;

        black.r = 0;
        black.g = 0;
        black.b = 0;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        amask = 0xff000000;
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
#endif


        TTF_Init();
        pVFDFont = TTF_OpenFont(STAT_FONT, STAT_PT);
        f = pVFDFont;

        rec.x = 0;
        rec.y = 0;
        rec.w = 50;
        rec.h = 20;

        drec.x = 8;
        drec.y = 2;
        drec.w = 50 - 8;
        drec.h = 20 - 2;

        pInsOn = SDL_CreateRGBSurface(SDL_SWSURFACE, 50, 20,
                                       32, rmask, gmask, bmask, amask);

        pInsOff = SDL_CreateRGBSurface(SDL_SWSURFACE, 50, 20,
                                       32, rmask, gmask, bmask, amask);
        SDL_FillRect(pInsOn, &rec, COL_RED);
        tmp = TTF_RenderUTF8_Blended(f, "INS", n );
        SDL_BlitSurface(tmp,NULL,pInsOn,NULL);
        SDL_FreeSurface(tmp);

        SDL_FillRect(pInsOff, &rec, COL_BLACK);
        tmp = TTF_RenderUTF8_Blended(f, "INS", r );
        SDL_BlitSurface(tmp,NULL,pInsOff,NULL);
        SDL_FreeSurface(tmp);
        /*
         * RECT CAPS
         */
        rec.x = 0;
        rec.y = 0;
        rec.w = 50;
        rec.h = 20;

        drec.x = 2;
        drec.y = 2;
        drec.w = 50 - 2;
        drec.h = 20 - 2;

        pCapsOn = SDL_CreateRGBSurface(SDL_SWSURFACE, 50, 20,
                                       32, rmask, gmask, bmask, amask);
        pCapsOff = SDL_CreateRGBSurface(SDL_SWSURFACE, 50, 20,
                                       32, rmask, gmask, bmask, amask);
        SDL_FillRect(pCapsOn, &rec, COL_RED);
        tmp  = TTF_RenderUTF8_Blended(f, "CAPS", n);
        SDL_BlitSurface(tmp, &rec, pCapsOn, &drec);
        SDL_FreeSurface(tmp);


        SDL_FillRect(pCapsOff, &rec, COL_BLACK);
        tmp  = TTF_RenderUTF8_Blended(f, "CAPS" , r);
        SDL_BlitSurface(tmp, &rec, pCapsOff, &drec);
        SDL_FreeSurface(tmp);


        /*
         * RECT KANA
         */
        rec.x = 0;
        rec.y = 0;
        rec.w = 50;
        rec.h = 20;

        drec.x = 12;
        drec.y = 2;
        drec.w = 50 - 12;
        drec.h = 20 - 2;

        pKanaOn = SDL_CreateRGBSurface(SDL_SWSURFACE, 50, 20,
                                       32, rmask, gmask, bmask, amask);
        pKanaOff = SDL_CreateRGBSurface(SDL_SWSURFACE, 50, 20,
                                       32, rmask, gmask, bmask, amask);
        SDL_FillRect(pKanaOn, &rec, COL_RED);
        tmp  = TTF_RenderUTF8_Blended(f, "かな", n);
        SDL_BlitSurface(tmp, &rec, pKanaOn, &drec);
        SDL_FreeSurface(tmp);

        SDL_FillRect(pKanaOff, &rec, COL_BLACK);
        tmp  = TTF_RenderUTF8_Blended(f, "かな", r);
        SDL_BlitSurface(tmp, &rec, pKanaOff, &drec);
        SDL_FreeSurface(tmp);



        /*
         * RECT Drive1
         */
        rec.x = 0;
        rec.y = 0;
        rec.w = 160;
        rec.h = 20;
        for(i = 0 ; i < 2 ; i++) {
                pFDRead[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                                  32, rmask, gmask, bmask, amask);
                pFDWrite[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                                   32, rmask, gmask, bmask, amask);
                pFDNorm[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                                  32, rmask, gmask, bmask, amask);
                SDL_FillRect(pFDRead[i], &rec, COL_RED);
                SDL_FillRect(pFDWrite[i], &rec, COL_BLUE);
                SDL_FillRect(pFDNorm[i], &rec, COL_NORM);
        }

        /*
         * RECT Tape
         */
        pCMTRead = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                       32, rmask, gmask, bmask, amask);
        pCMTWrite = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                       32, rmask, gmask, bmask, amask);
        pCMTNorm = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                       32, rmask, gmask, bmask, amask);

        SDL_FillRect(pCMTRead, &rec, COL_RED);
        SDL_FillRect(pCMTWrite, &rec, COL_BLUE);
        SDL_FillRect(pCMTNorm, &rec, COL_NORM);

        pCaption = SDL_CreateRGBSurface(SDL_SWSURFACE, 300, 20,
                                       32, rmask, gmask, bmask, amask);
        rec.x = 0;
        rec.y = 0;
        rec.h = 300;
        rec.w = 20;
        SDL_FillRect(pCaption, &rec, COL_BLACKMASK);
 
        /*
         * Draw
         */
        p = SDL_GetVideoSurface();
        if(p == NULL) return;
        DrawStatus();
        rec.x = 0;
        rec.y = 0;
        rec.w = 50;
        rec.h = 20;
        SDL_UpdateRect(p, 0, 0, p->w, p->h);
} 

void
DestroyDraw(void)
{
        int i;
        if(pInsOn !=NULL ) {
                SDL_FreeSurface(pInsOn);
                pInsOn = NULL;
        }
        if(pInsOff != NULL) {
                SDL_FreeSurface(pInsOff);
                pInsOff = NULL;
        }
        if(pCapsOn !=NULL ) {
                SDL_FreeSurface(pCapsOn);
                pCapsOn = NULL;
        }
        if(pCapsOff != NULL) {
                SDL_FreeSurface(pCapsOff);
                pCapsOff = NULL;
        }
        if(pKanaOn !=NULL ) {
                SDL_FreeSurface(pKanaOn);
                pKanaOn = NULL;
        }
        if(pKanaOff != NULL) {
                SDL_FreeSurface(pKanaOff);
                pKanaOff = NULL;
        }
        for(i = 0 ; i < 2 ; i++ ) {
                if(pFDRead[i] != NULL) {
                        SDL_FreeSurface(pFDRead[i]);
                        pFDRead[i] = NULL;
                }
                if(pFDWrite[i] != NULL) {
                        SDL_FreeSurface(pFDWrite[i]);
                        pFDWrite[i] = NULL;
                }
                if(pFDNorm[i] != NULL) {
                        SDL_FreeSurface(pFDNorm[i]);
                        pFDNorm[i] = NULL;
                }
        }
        if(pCMTRead !=NULL ) {
                SDL_FreeSurface(pCMTRead);
                pCMTRead = NULL;
        }
        if(pCMTWrite !=NULL ) {
                SDL_FreeSurface(pCMTWrite);
                pCMTWrite = NULL;
        }
        if(pCMTNorm !=NULL ) {
                SDL_FreeSurface(pCMTNorm);
                pCMTNorm = NULL;
        }
        if(pVFDFont != NULL) {
                TTF_CloseFont(pVFDFont);
                pVFDFont = NULL;
        }
        if(pStatusFont != NULL) {
                TTF_CloseFont(pStatusFont);
                pStatusFont = NULL;
        }
}
    /*
     *  キャプション描画 
     */ 
static void
DrawMainCaption(void) 
{
    char           string[1024];
    char           tmp[128];
    char          *p;
    SDL_Surface   *pS, *tmpSurface;
    SDL_Color     n;
    SDL_Rect      rec,drec;
    TTF_Font      *f;
    
    
    /*
     * 動作状況に応じて、コピー 
     */ 
    if (run_flag) {
	strcpy(string, "XM7[実行]");
    }    
    else {
	strcpy(string, "XM7[停止]");
    }
    strcat(string, " ");
    
    /*
     * CPU速度比率 
     */ 
    if (bAutoSpeedAdjust) {
            sprintf(tmp, "(%3d%%) ", speed_ratio / 100);
            strcat(string, tmp);
    }
    
    /*
     * フロッピーディスクドライブ 0 
     */ 
    if (fdc_ready[0] != FDC_TYPE_NOTREADY) {
            
            /*
	     * ファイルネーム＋拡張子のみ取り出す 
	     */ 
	    p = strrchr(fdc_fname[0], '/');
	if (p == NULL) {
	    p = fdc_fname[0];
	} else {
	    p++;
	}
	sprintf(tmp, "- %s ", p);
	strcat(string, tmp);
    }
    
	/*
	 * フロッピーディスクドライブ 1 
	 */ 
	if (fdc_ready[1] != FDC_TYPE_NOTREADY) {
	if ((strcmp(fdc_fname[0], fdc_fname[1]) != 0) || 
	     (fdc_ready[0] == FDC_TYPE_NOTREADY)) {
	    
		/*
		 * ファイルネーム＋拡張子のみ取り出す 
		 */ 
		p = strrchr(fdc_fname[1], '/');
	    if (p == NULL) {
		p = fdc_fname[1];
	    } else {
		p++;
	    }
	    sprintf(tmp, "(%s) ", p);
	    strcat(string, tmp);
	}
    }
    
	/*
	 * テープ 
	 */ 
	if (tape_fileh != -1) {
	
	    /*
	     * ファイルネーム＋拡張子のみ取り出す 
	     */ 
	    p = strrchr(tape_fname, '/');
	if (p == NULL) {
	    p = tape_fname;
	} else {
	    p++;
	}
	sprintf(tmp, "- %s ", p);
	strcat(string, tmp);
    }
    
	/*
	 * 比較描画 
	 */ 

    strncpy(szCaption, string, 128);
    if((strncmp(szOldCaption, szCaption, 128) != 0) || (nInitialDrawFlag == TRUE)) {
                    rec.x = 0;
                    rec.y = 0;
                    rec.w = 300;
                    rec.h = 20;
                    
                    drec.x = nDrawWidth - (300 + 8); /* 320モードの時は変える */
                    drec.y = nDrawHeight + 40;
                    drec.w = 300;
                    drec.h = 20;
                    

                    strncpy(szOldCaption, szCaption, 128);
                    f = pVFDFont; /* メモリに余裕があるならば pStatusFontにする */
                    if(f == NULL) return;
                    n.r = 255;
                    n.g = 255;
                    n.b = 255;
                    if(pCaption == NULL) return;
                    SDL_FillRect(pCaption, &rec, COL_BLACKMASK);
                    tmpSurface = TTF_RenderUTF8_Blended(f, szCaption, n);
                    SDL_BlitSurface(tmpSurface, NULL, pCaption, &rec);
                    SDL_FreeSurface(tmpSurface);
                    pS = SDL_GetVideoSurface();
                    if(pS == NULL) return;
                    SDL_BlitSurface(pCaption, NULL, pS, &drec);
                    SDL_UpdateRect(pS, drec.x, drec.y, drec.w, drec.h);
                    SDL_Flip(pS);

    }    
            

}


    /*
     *  CAPキー描画 
     */ 
static void
DrawCAP(void) 
{
        int            num;
        SDL_Surface *p;
        SDL_Rect rec,drec;


    
        /*
         * 番号決定 
         */ 
        if (caps_flag) {
                num = 1;
        } else {
                num = 0;
        }    
       /*
        * 同じなら何もしない 
        */ 
        if ((nCAP == num) && (nInitialDrawFlag != TRUE)) {
                return;
        }
    
/*
 * 描画、ワーク更新 
 */ 
        nCAP = num;
        rec.x = 0;
        rec.y = 0;
        rec.w = 50;
        rec.h = 20;

        drec.x = nDrawWidth - (60 * 2);
        drec.y = nDrawHeight + 0;
        drec.w = 50;
        drec.h = 20;

        p = SDL_GetVideoSurface();
        if(p == NULL) return;

        if (nCAP) {
                SDL_BlitSurface(pCapsOn, NULL, p, &drec);
        } else {
                SDL_BlitSurface(pCapsOff, NULL, p, &drec); 
        }
        SDL_UpdateRect(p, drec.x, drec.y, drec.w, drec.h);
        SDL_Flip(p);
}


    /*
     *  かなキー描画 
     */ 
static void
DrawKANA(void) 
{
        int            num;
        SDL_Surface *p;
        SDL_Rect rec, drec;
    
        /*
         * 番号決定 
         */ 
        if (kana_flag) {
                num = 1;
        } else {
                num = 0;
        }
    
/*
 * 同じなら何もしない 
 */ 
        if ((nKANA == num) && (nInitialDrawFlag != TRUE)){
                return;
        }
    
/*
 * 描画、ワーク更新 
 */ 
        nKANA = num;

        rec.x = 0;
        rec.y = 0;
        rec.w = 50;
        rec.h = 20;


        drec.x = nDrawWidth - (60 * 1);
        drec.y = nDrawHeight + 0;
        drec.w = 50;
        drec.h = 20;
        p = SDL_GetVideoSurface();
        if(p == NULL) return;
        if (nKANA) {
                SDL_BlitSurface(pKanaOn, &rec, p, &drec);                 
        } else {
                SDL_BlitSurface(pKanaOff, &rec, p, &drec); 
        }
        SDL_UpdateRect(p, drec.x, drec.y, drec.w, drec.h);

}


    /*
     *  INSキー描画 
     */ 
static void
DrawINS(void) 
{
        int            num;
        SDL_Surface *p;
        SDL_Rect rec, drec;
        /*
         * 番号決定 
         */ 
        if (ins_flag) {
                num = 1;
        }  else {
                num = 0;
        }
    
/*
 * 同じなら何もしない 
 */ 
        if ((nINS == num) && (nInitialDrawFlag != TRUE)){
                return;
        }
    
/*
 * 描画、ワーク更新 
 */ 
        nINS = num;
        rec.x = 0;
        rec.y = 0;
        rec.w = 50;
        rec.h = 20;

        drec.x = nDrawWidth - (60 * 3);
        drec.y = nDrawHeight + 0;
        drec.w = 50;
        drec.h = 20;
        p = SDL_GetVideoSurface();
        if(p == NULL) return;
        if (nINS) {
                SDL_BlitSurface(pInsOn, &rec, p, &drec);
        } else {
                SDL_BlitSurface(pInsOff, &rec, p, &drec);
        }
        SDL_UpdateRect(p, drec.x, drec.y, drec.w, drec.h);
        SDL_Flip(p);
}


    /*
     *  ドライブ描画 
     */ 
static void 
DrawDrive(int drive) 
{
    int            num;
    char          *name;
    char          string[64];
    char          utf8[256];
    char          *pIn, *pOut;
    iconv_t       hd;
    size_t        in, out;

    SDL_Rect rec,drec;
    SDL_Surface *p, *tmp;
    SDL_Color r, b, n , black;
    TTF_Font *f;

    ASSERT((drive >= 0) && (drive <= 1));
    rec.x = 0;
    rec.y = 0;
    rec.w = 160;
    rec.h = 20;
  

    /*
     * 番号セット 
     */ 
    if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
            num = 255;
    }  else {
            num = fdc_access[drive];
            if (num == FDC_ACCESS_SEEK) {
                    num = FDC_ACCESS_READY;
            }
    }
    
    /*
     * 名前取得 
     */ 
    name = "";
    utf8[0] = '\0';
    if (fdc_ready[drive] == FDC_TYPE_D77) {
            name = fdc_name[drive][fdc_media[drive]];
    }
    if (fdc_ready[drive] == FDC_TYPE_2D) {
	name = "2D DISK";
    }
    if (fdc_ready[drive] == FDC_TYPE_VFD) {
	name = "VFD DISK";
    }
    
    /*
     * 番号比較 
     */ 
    if ((nDrive[drive] == num) && (nInitialDrawFlag != TRUE)) {
            if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
                    if(strlen(szDrive[drive]) > 0) {
                            SDL_FillRect(pFDRead[drive], &rec, COL_RED);
                            SDL_FillRect(pFDWrite[drive], &rec, COL_BLUE);                            
                            SDL_FillRect(pFDNorm[drive], &rec, COL_NORM);
                    }
                    szDrive[drive][0] = '\0';
                    szOldDrive[drive][0] = '\0';
                    return;
            }
            if (strcmp(szDrive[drive], name) == 0) {
                    return;
            }
    }
    
    /*
     * 描画 
     */ 
    nDrive[drive] = num;
    strncpy(szDrive[drive], name, 16);
    if (nDrive[drive] == 255) {
            strcpy(string, "");
    } else {
            strcpy(string, szDrive[drive]);
    }

    if(strcmp(szDrive[drive], szOldDrive[drive]) != 0) {
            /* 
             * 過去のファイルネームと違うのでフォントレンダリングする
             */

            n.r = 255;
            n.g = 255;
            n.b = 255;

            black.r = 0;
            black.g = 0;
            black.b = 0;
            drec.x = rec.x + 8;
            drec.y = rec.y + 2;
            drec.w  = rec.w - 8;
            drec.h  = rec.h - 2;

            pIn = string;
            pOut = utf8;
            in = strlen(pIn);
            out = 256;
            hd = iconv_open("utf8", "cp932");
            if(hd >= 0) {
                    while(in>0) {
                            iconv(hd, &pIn, &in, &pOut, &out);
                    }   
                    iconv_close(hd);
            }

            f = pVFDFont;
            SDL_FillRect(pFDRead[drive], &rec, COL_RED);
            tmp = TTF_RenderUTF8_Blended(f, utf8, n );
            SDL_BlitSurface(tmp, &rec, pFDRead[drive], &drec);
            SDL_FreeSurface(tmp);

            SDL_FillRect(pFDWrite[drive], &rec, COL_BLUE);
            tmp = TTF_RenderUTF8_Blended(f, utf8, n );
            SDL_BlitSurface(tmp, &rec, pFDWrite[drive], &drec);
            SDL_FreeSurface(tmp);

            SDL_FillRect(pFDNorm[drive], &rec, COL_NORM);
            tmp = TTF_RenderUTF8_Blended(f, utf8, black );
            SDL_BlitSurface(tmp, &rec, pFDNorm[drive], &drec);
            SDL_FreeSurface(tmp);
            
            strncpy(szOldDrive[drive], szDrive[drive], 16);
//            strcpy(szOldDrive[drive], string);
    }



    p = SDL_GetVideoSurface();  
    drec.x = nDrawWidth - (160 * (drive + 2));
    drec.y = nDrawHeight + 20;
    drec.w = 160;
    drec.h = 20;


    if (nDrive[drive] == FDC_ACCESS_READ) {
            /*
             * READ
             */
            SDL_BlitSurface(pFDRead[drive], &rec, p, &drec);
    } else if (nDrive[drive] == FDC_ACCESS_WRITE) {
            SDL_BlitSurface(pFDWrite[drive], &rec, p, &drec);
    } else {
            SDL_BlitSurface(pFDNorm[drive], &rec, p, &drec);
    }
    SDL_UpdateRect(p, drec.x, drec.y, drec.w, drec.h);

}


    /*
     *  テープ描画 
     */ 
static void
DrawTape(void) 
{
    int             num;
    char            string[128];
    SDL_Surface     *p, *tmp;
    SDL_Rect        rec,drec;
    SDL_Color r, b, n , black;
    TTF_Font *f;

    rec.x = 0;
    rec.y = 0;
    rec.w = 160;
    rec.h = 20;

    /*
     * ナンバー計算 
     */ 
    num = 30000;
    if (tape_fileh != -1) {
            num = (int) ((tape_offset >> 8) % 10000);
            if (tape_motor) {
                    if (tape_rec) {
                            num += 20000;
	    }
                    
                    else {
                            num += 10000;
                    }
            }
    }
    
    /*
     * 番号比較 
     */ 
    if ((nTape == num) && (nInitialDrawFlag != TRUE)){
            return;
    }
    
    /*
     * 描画 
     */ 
    nTape = num;
    if (nTape >= 30000) {
            string[0] = '\0';
            //strcpy(string, "OVER");
    }
    else {
	sprintf(string, "%04d", nTape % 10000);
    }
    if(nOldTape != nTape) {
            /*
             * カウンタ番号レンダリング(仮)
             */
            drec.x = rec.x + 32;
            drec.y = rec.y + 2;
            drec.w = rec.w - 32;
            drec.h = rec.h - 2;

            n.r = 255;
            n.g = 255;
            n.b = 255;

            black.r = 0;
            black.g = 0;
            black.b = 0;


            f = pVFDFont;

            SDL_FillRect(pCMTRead, &rec, COL_RED);
            tmp = TTF_RenderUTF8_Blended(f, string, n );
            SDL_BlitSurface(tmp, &rec, pCMTRead, &drec);
            SDL_FreeSurface(tmp);

            SDL_FillRect(pCMTWrite, &rec, COL_BLUE);
            tmp = TTF_RenderUTF8_Blended(f, string, n );
            SDL_BlitSurface(tmp, &rec, pCMTWrite, &drec);
            SDL_FreeSurface(tmp);

            SDL_FillRect(pCMTNorm, &rec, COL_NORM);
            tmp = TTF_RenderUTF8_Blended(f, string, black );
            SDL_BlitSurface(tmp, &rec, pCMTNorm, &drec);
            SDL_FreeSurface(tmp);
            nOldTape = nTape;
    }

    drec.x = nDrawWidth - (160 * 1);
    drec.y = nDrawHeight + 20;
    drec.w = 160;
    drec.h = 20;


    p = SDL_GetVideoSurface();
    if(p == NULL) return;

    if ((nTape >= 10000) && (nTape < 30000)) {
            if (nTape >= 20000) {
                    SDL_BlitSurface(pCMTWrite, &rec, p, &drec);
            }   else {
                    SDL_BlitSurface(pCMTRead, &rec, p, &drec);
            }
    } else {
                    SDL_BlitSurface(pCMTNorm, &rec, p, &drec);
    }
    SDL_UpdateRect(p, drec.x, drec.y, drec.w, drec.h);
}


/*
 *  描画 
 */ 
void  
DrawStatus(void) 
{
        DrawMainCaption();
        DrawCAP();
        DrawKANA();
        DrawINS();
        DrawDrive(0);
        DrawDrive(1);
        DrawTape();
        nInitialDrawFlag = FALSE;
} 

        
/*
 *  強制描画 
 */ 
void  
DrawStatusForce(void) 
{
        nInitialDrawFlag = TRUE;
        DrawStatus();
} 


/*
 *  再描画 
 */ 
void
PaintStatus(void) 
{
        SDL_Surface *p;
/*
 * 記憶ワークをすべてクリアする 
 */ 
        nInitialDrawFlag = TRUE;
        szCaption[0] = '\0';
        szOldCaption[0] = '\0';
        nCAP = -1;
        nKANA = -1;
        nINS = -1;
        nDrive[0] = -1;
        nDrive[1] = -1;
        szDrive[0][0] = '\0';
        szDrive[1][0] = '\0';
        szOldDrive[0][0] = '\0';
        szOldDrive[1][0] = '\0';
        nTape = -1;
        nOldTape = 0;
    
        /*
         * 描画 
         */ 
        p = SDL_GetVideoSurface();
        if(p == NULL) return;
        DrawStatus();
} 
#endif	/* _XWIN */
