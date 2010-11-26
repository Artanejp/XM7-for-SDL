/*
 * agar_osd.cpp
 *
 *  Created on: 2010/11/26
 *      Author: whatisthis
 */

#include <SDL.h>
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>

#include "xm7.h"
#include "keyboard.h"
#include "tapelp.h"
#include "display.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "fdc.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#else
#include "sdl.h"
#endif



#include "sdl_bar.h"
#include "sdl_sch.h"
#include "api_draw.h"

#include <iconv.h>




/* デフォルトフォントが設定されてないときはフォントとしてIPAゴシックを使う */
#ifndef FUNC_FONT
#define FUNC_FONT "ipagui.ttf"
#endif

#ifndef STAT_FONT
#define STAT_FONT "ipagui.ttf"
#endif

#define FUNC_PT 16
#define STAT_PT 16

/*
 * Global変数
 */
char StatusFont[MAXPATHLEN];

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
static AG_Surface      *pInsOn; /* INSキープリレンダリング(ON) */
static AG_Surface      *pInsOff; /* INSキープリレンダリング(OFF) */
static AG_Surface      *pKanaOn; /* カナキープリレンダリング(ON) */
static AG_Surface      *pKanaOff; /* カナキープリレンダリング(OFF) */
static AG_Surface      *pCapsOn; /* Capsキープリレンダリング(ON) */
static AG_Surface      *pCapsOff; /* Capsキープリレンダリング(OFF) */
static AG_Surface      *pFDRead[2]; /* Drive0 Read */
static AG_Surface      *pFDWrite[2]; /* Drive0 Write */
static AG_Surface      *pFDNorm[2]; /* Drive0 Normal */
static AG_Surface      *pCMTRead; /* Tape Read */
static AG_Surface      *pCMTWrite; /* Tape Write */
static AG_Surface      *pCMTNorm; /* Tape Normal */
static AG_Surface      *pCaption; /* Caption */
static AG_Surface      *pStatusBar; /* ステータス表示バー */
static AG_Font         *pStatusFont;
static AG_Font         *pVFDFont;
extern SDL_mutex        *DrawMutex;

extern "C" {
AG_Window *BarWin;
AG_Box *OsdArea;
}
static AG_Box *StatusBox;
static AG_Box *FD1Box;
static AG_Box *FD0Box;
static AG_Box *CMTBox;
static AG_Box *INSBox;
static AG_Box *CAPSBox;
static AG_Box *KANABox;



/*-[ ステータスバー ]-------------------------------------------------------*/
/*
 *  ステータスバーの生成
 */
void CreateStatus(void)
{
        AG_Rect rec;
        AG_SizeAlloc barrect;
        AG_Color r, b, n , black;
        AG_Box *box;
        AG_Box *hbox;
        Uint32 rmask, gmask, bmask, amask;
        int i;

        if(BarWin != NULL) {
        	return;
        }

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
#if AG_BYTEORDER == AG_BIG_ENDIAN
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

        AG_PushTextState();
        pStatusFont = AG_FetchFont("mona.ttf", 12, -1);
        if(pStatusFont == NULL) {
        	return;
        }
        AG_PopTextState();
 //       BarWin = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_PLAIN);
        BarWin = AG_WindowNew(0);

#if 1
        AG_TextColor(black);
        AG_TextBGColor(r);
        pInsOn = AG_TextRender("Ins");
        pCapsOn = AG_TextRender("CAPS");
        pKanaOn = AG_TextRender("かな");
        AG_TextColor(black);
        AG_TextBGColor(b);
        pInsOff = AG_TextRender("Ins");
        pCapsOff = AG_TextRender("CAPS");
        pKanaOff = AG_TextRender("かな");

        /*
         * RECT Drive1
         */
        rec.x = 0;
        rec.y = 0;
        rec.w = 120;
        rec.h = 20;
        for(i = 0 ; i < 2 ; i++) {
                pFDRead[i] = AG_SurfaceRGBA(rec.w, rec.h, 32, AG_SRCALPHA,  rmask, gmask, bmask, amask);
                pFDWrite[i] = AG_SurfaceRGBA(rec.w, rec.h, 32, AG_SRCALPHA, rmask, gmask, bmask, amask);
                pFDNorm[i] = AG_SurfaceRGBA(rec.w, rec.h, 32, AG_SRCALPHA, rmask, gmask, bmask, amask);
                AG_FillRect(pFDRead[i], &rec, r);
                AG_FillRect(pFDWrite[i], &rec, b);
                AG_FillRect(pFDNorm[i], &rec, n);
        }

        /*
         * RECT Tape
         */
        pCMTRead = AG_SurfaceRGBA(rec.w, rec.h, 32, AG_SRCALPHA, rmask, gmask, bmask, amask);
        pCMTWrite = AG_SurfaceRGBA(rec.w, rec.h, 32, AG_SRCALPHA, rmask, gmask, bmask, amask);
        pCMTNorm = AG_SurfaceRGBA(rec.w, rec.h, 32, AG_SRCALPHA, rmask, gmask, bmask, amask);

        AG_FillRect(pCMTRead, &rec, r);
        AG_FillRect(pCMTWrite, &rec, b);
        AG_FillRect(pCMTNorm, &rec, n);


//        pCaption = SDL_CreateRGBSurface(SDL_SWSURFACE, 300, 20,
//                                       32, rmask, gmask, bmask, amask);
//        rec.x = 0;
//        rec.y = 0;
//        rec.h = 300;
//        rec.w = 20;
//        SDL_FillRect(pCaption, &rec, COL_BLACKMASK);

        /*
         * Draw
         */
        OsdArea = AG_BoxNewHoriz(AGWIDGET(BarWin), AG_BOX_EXPAND);
        barrect.x = 0;
        barrect.y = 0;
        barrect.h = 40;
        barrect.w = 260;
        box = AG_BoxNewVert(AGWIDGET(OsdArea), 0);
        AG_WidgetSizeAlloc(AGWIDGET(box), &barrect);
        barrect.x = 0;
        barrect.y = 0;
        barrect.h = 20;
        barrect.w = 240;
        StatusBox = AG_BoxNewHoriz(AGWIDGET(box), 0);
        AG_WidgetSizeAlloc(AGWIDGET(StatusBox), &barrect);

        barrect.x = 0;
        barrect.y = 20;
        barrect.h = 20;
        barrect.w = 260;
        hbox = AG_BoxNewVert(AGWIDGET(box), 0);
        AG_WidgetSizeAlloc(AGWIDGET(hbox), &barrect);

        barrect.x = 0;
        barrect.y = 20;
        barrect.h = 20;
        barrect.w = 127;
        FD1Box = AG_BoxNewHoriz(AGWIDGET(hbox), 0);
        AG_WidgetSizeAlloc(AGWIDGET(FD1Box), &barrect);

        barrect.x = 130;
        barrect.y = 20;
        barrect.h = 20;
        barrect.w = 127;
        FD0Box = AG_BoxNewHoriz(AGWIDGET(hbox), 0);
        AG_WidgetSizeAlloc(AGWIDGET(FD0Box), &barrect);

        hbox = AG_BoxNewVert(AGWIDGET(box), 0);
        AG_WidgetSizeAlloc(AGWIDGET(hbox), &barrect);

        box = AG_BoxNewVert(AGWIDGET(OsdArea), AG_BOX_HFILL);
        hbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_HFILL);
        barrect.x = 0;
        barrect.y = 0;
        barrect.h = 20;
        barrect.w = 60;
        hbox = AG_BoxNewVert(AGWIDGET(box), 0);
        AG_WidgetSizeAlloc(AGWIDGET(hbox), &barrect);

        barrect.x = 0;
        barrect.y = 0;
        barrect.h = 20;
        barrect.w = 20;
        INSBox = AG_BoxNewHoriz(AGWIDGET(hbox), 0);
        AG_WidgetSizeAlloc(AGWIDGET(INSBox), &barrect);

        barrect.x = 20;
        barrect.y = 0;
        barrect.h = 20;
        barrect.w = 20;
        CAPSBox = AG_BoxNewHoriz(AGWIDGET(hbox), 0);
        AG_WidgetSizeAlloc(AGWIDGET(CAPSBox), &barrect);

        barrect.x = 40;
        barrect.y = 0;
        barrect.h = 20;
        barrect.w = 20;
        KANABox = AG_BoxNewHoriz(AGWIDGET(hbox), 0);
        AG_WidgetSizeAlloc(AGWIDGET(CAPSBox), &barrect);

        hbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_HFILL);
        CMTBox = AG_BoxNewHoriz(AGWIDGET(hbox), 0);
        AG_WidgetSetSize(AGWIDGET(CMTBox), 60, 20);
#endif
    	AG_WindowSetMinSize(BarWin, 320, 40);
//        AG_WindowSetPosition(BarWin, AG_WINDOW_BL, 0);
        AG_WindowShow(BarWin);
}

void
DestroyDraw(void)
{
        int i;
        if(pInsOn !=NULL ) {
                AG_SurfaceFree(pInsOn);
                pInsOn = NULL;
        }
        if(pInsOff != NULL) {
        		AG_SurfaceFree(pInsOff);
                pInsOff = NULL;
        }
        if(pCapsOn !=NULL ) {
            AG_SurfaceFree(pCapsOn);
                pCapsOn = NULL;
        }
        if(pCapsOff != NULL) {
            AG_SurfaceFree(pCapsOff);
                pCapsOff = NULL;
        }
        if(pKanaOn !=NULL ) {
            AG_SurfaceFree(pKanaOn);
                pKanaOn = NULL;
        }
        if(pKanaOff != NULL) {
            AG_SurfaceFree(pKanaOff);
                pKanaOff = NULL;
        }
        for(i = 0 ; i < 2 ; i++ ) {
                if(pFDRead[i] != NULL) {
                    AG_SurfaceFree(pFDRead[i]);
                       pFDRead[i] = NULL;
                }
                if(pFDWrite[i] != NULL) {
                    AG_SurfaceFree(pFDWrite[i]);
                        pFDWrite[i] = NULL;
                }
                if(pFDNorm[i] != NULL) {
                    AG_SurfaceFree(pFDNorm[i]);
                        pFDNorm[i] = NULL;
                }
        }
        if(pCMTRead !=NULL ) {
            AG_SurfaceFree(pCMTRead);
                pCMTRead = NULL;
        }
        if(pCMTWrite !=NULL ) {
            AG_SurfaceFree(pCMTWrite);
                pCMTWrite = NULL;
        }
        if(pCMTNorm !=NULL ) {
        	AG_SurfaceFree(pCMTNorm);
                pCMTNorm = NULL;
        }
        if(pVFDFont != NULL) {
        	AG_DestroyFont(pVFDFont);
                pVFDFont = NULL;
        }
        if(pStatusFont != NULL) {
        	AG_DestroyFont(pStatusFont);
                pStatusFont = NULL;
        }

        AG_WindowHide(BarWin);
        AG_ObjectDetach(BarWin);



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
    AG_Surface   *pS, *tmpSurface;
    AG_Color     n;
    AG_Rect      rec,drec;


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
    }


}


    /*
     *  CAPキー描画
     */
static void DrawCAP(void)
{
        int            num;
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
        if (nCAP) {
        	AG_WidgetBlit(CAPSBox, pCapsOn, 0, 0);
        } else {
        	AG_WidgetBlit(CAPSBox, pCapsOff, 0, 0);
        }
        AG_WidgetUpdate(CAPSBox);
        //SDL_Flip(p);
}


    /*
     *  かなキー描画
     */
static void DrawKANA(void)
{
        int            num;

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
        if (nKANA) {
        	AG_WidgetBlit(KANABox, pKanaOn, 0, 0);
        } else {
        	AG_WidgetBlit(KANABox, pKanaOff, 0, 0);
        }
        AG_WidgetUpdate(KANABox);
}


    /*
     *  INSキー描画
     */
static void
DrawINS(void)
{
        int            num;
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
        if (nINS) {
        	AG_WidgetBlit(INSBox, pInsOn, 0, 0);
        } else {
        	AG_WidgetBlit(INSBox, pInsOff, 0, 0);
        }
        AG_WidgetUpdate(INSBox);

}

#if 0
    /*
     *  ドライブ描画
     */
static void DrawDrive(int drive)
{
    int            num;
    char          *name;
    char          string[64];
    char          utf8[256];
    char          *pIn, *pOut;
    iconv_t       hd;
    size_t        in, out;


    ASSERT((drive >= 0) && (drive <= 1));


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
                    	//レンダリング
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
    memset(szDrive[drive], 0, 16);
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
            if(f != NULL) {
                    tmp = TTF_RenderUTF8_Blended(f, utf8, n );
                    SDL_BlitSurface(tmp, &rec, pFDRead[drive], &drec);
                    SDL_FreeSurface(tmp);
            }
            SDL_FillRect(pFDWrite[drive], &rec, COL_BLUE);
            if(f != NULL) {
                    tmp = TTF_RenderUTF8_Blended(f, utf8, n );
                    SDL_BlitSurface(tmp, &rec, pFDWrite[drive], &drec);
                    SDL_FreeSurface(tmp);
            }

            SDL_FillRect(pFDNorm[drive], &rec, COL_NORM);
            if(f != NULL) {
                    tmp = TTF_RenderUTF8_Blended(f, utf8, black );
                    SDL_BlitSurface(tmp, &rec, pFDNorm[drive], &drec);
                    SDL_FreeSurface(tmp);
            }
            memset(szOldDrive[drive], 0, 16);
            strncpy(szOldDrive[drive], szDrive[drive], 16);
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
    SDL_Color n , black;
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
            if(f != NULL) {
                    tmp = TTF_RenderUTF8_Blended(f, string, n );
                    SDL_BlitSurface(tmp, &rec, pCMTRead, &drec);
                    SDL_FreeSurface(tmp);
            }
            SDL_FillRect(pCMTWrite, &rec, COL_BLUE);
            if(f != NULL) {
                    tmp = TTF_RenderUTF8_Blended(f, string, n );
                    SDL_BlitSurface(tmp, &rec, pCMTWrite, &drec);
                    SDL_FreeSurface(tmp);
            }

            SDL_FillRect(pCMTNorm, &rec, COL_NORM);
            if(f != NULL) {
                    tmp = TTF_RenderUTF8_Blended(f, string, black );
                    SDL_BlitSurface(tmp, &rec, pCMTNorm, &drec);
                    SDL_FreeSurface(tmp);
            }
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

#endif
/*
 *  描画
 */
void
DrawStatus(void)
{
#ifndef USE_OPENGL
        DrawMainCaption();
        DrawCAP();
        DrawKANA();
        DrawINS();
        DrawDrive(0);
        DrawDrive(1);
        DrawTape();
        nInitialDrawFlag = FALSE;
#else
        if(BarWin == NULL) return;
//        DrawCAP();
//        DrawKANA();
//        DrawINS();
//        nInitialDrawFlag = FALSE;

#endif
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
        AG_Surface *p;
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
        DrawStatus();
}
