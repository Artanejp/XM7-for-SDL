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
#include "agar_gldraw.h"
#else
#include "sdl.h"
#endif



#include "sdl_bar.h"
#include "sdl_sch.h"
#include "api_draw.h"

#include <iconv.h>

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
static int     nDriveOld[2];	/* フロッピードライブ */
static char    szDrive[2][16 + 1];	/* フロッピードライブ */
static char    szOldDrive[2][16+1];       /* フロッピードライブ(過去) */
static BOOL     old_writep[4];  /* 過去のライトプロテクトフラグ */
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
static AG_Font         *pStatusFont;
static AG_Color			r;
static AG_Color 		b;
static AG_Color			n;
static AG_Color 		black;
static AG_Color			alpha;


extern "C" {
extern AG_GLView *DrawArea;
extern AG_Window *MainWindow;
extern AG_Menu  *MenuBar;
AG_GLView *OsdArea;
}

GLuint tid_ins_on;
GLuint tid_kana_on;
GLuint tid_caps_on;
GLuint tid_ins_off;
GLuint tid_kana_off;
GLuint tid_caps_off;

GLuint tid_fd[4][3];
GLuint tid_cmt[3];
GLuint tid_caption;
GLfloat LedAlpha;
GLfloat FDDAlpha;


static GLuint CreateTexture(AG_Surface *p)
{
	Uint8 *pix;
	GLuint textureid;
	int w;
	int h;

	if(agDriverOps == NULL) return 0;
	if(p == NULL) return 0;
	w = p->w;
	h = p->h;
	pix = (Uint8 *)p->pixels;

    glGenTextures(1, &textureid);
    glBindTexture(GL_TEXTURE_2D, textureid);
    /*
     * 文字はスムージングしようよ(´・ω・｀)
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 w, h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pix);

	return textureid;
}

static void DiscardTextures(int n, GLuint *id)
{
	if(agDriverOps == NULL) return;
	glDeleteTextures(n, id);

}

static void DiscardTexture(GLuint tid)
{
	DiscardTextures(1, &tid);
}

static void DrawTexture(GLuint tid, int offset_x, int offset_y, int w, int h)
{
	float xbegin = 0.0f;
	float xend = 1.0f;
	float ybegin = 0.0f;
	float yend = 1.0f;
	AG_Surface *pixvram = GetVramSurface_AG_GL();
	if(pixvram == NULL) return;
	/*
	 * 開始座標系はVRAMに合わせる。する
	 */
	xbegin = (float)offset_x * 2.0f/ 1280.0f - 1.0f;
	xend = (float)(offset_x + w) * 2.0f / 1280.0f - 1.0f;
	ybegin = (float)offset_y * 2.0f / 800.0f - 1.0f;
	yend = (float)(offset_y + h) * 2.0f / 800.0f - 1.0f;

	/*
	 * GL初期設定はagar_gldraw.cppの物を使う(と言うかオブジェクトならべる感覚で)
	 */

    glBindTexture(GL_TEXTURE_2D, tid);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(xbegin, ybegin, -0.5);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(xbegin, yend, -0.5);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(xend, ybegin, -0.5);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(xend, yend, -0.5);
    glEnd();

}

static void DrawCAP(void);
static void DrawINS(void);
static void DrawKANA(void);
static void DrawMainCaption(void);
static void DrawDrive(AG_Widget *w, int drive);
static void DrawTape(void);

void DrawOSDGL(AG_GLView *w)
{

    DrawCAP();
    DrawKANA();
    DrawINS();
    DrawDrive(AGWIDGET(w), 1);
    DrawDrive(AGWIDGET(w), 0);
    DrawTape();
    DrawMainCaption();

}

void DrawOSDEv(AG_Event *event)
{
	AG_GLView *glv = (AG_GLView *)AG_SELF();

//	DrawMainCaption();
//    DrawCAP();
//    DrawKANA();
//    DrawINS();
    DrawDrive(AGWIDGET(glv), 1);
    DrawDrive(AGWIDGET(glv), 0);
//    DrawTape();
}


/*-[ ステータスバー ]-------------------------------------------------------*/
/*
 *  ステータスバーの生成
 */
void CreateStatus(void)
{
        AG_Rect rec;
        AG_Surface *tmps;
        AG_PixelFormat fmt;
        int i;


        r.r = 255; // r->g
        r.g = 0; // g->b
        r.b = 0;  // b->r
        r.a = 255; // a->a

        b.r = 0;
        b.g = 0;
        b.b = 255;
        b.a = 255;

        n.r = 255;
        n.g = 255;
        n.b = 255;
        n.a = 255;

        black.r = 0;
        black.g = 0;
        black.b = 0;
        black.a = 255; //
        // アルファチャンネル
        alpha.r = 1;
        alpha.g = 1;
        alpha.b = 1;
        alpha.a = 180; //

    	// Surfaceつくる
    	fmt.BitsPerPixel = 32;
    	fmt.BytesPerPixel = 4;
#ifdef AG_BIG_ENDIAN
    	fmt.Rmask = 0x000000ff; // R
    	fmt.Gmask = 0x0000ff00; // G
    	fmt.Bmask = 0x00ff0000; // B
    	fmt.Amask = 0xff000000; // A
#else
    	fmt.Rmask = 0x00ff0000; // R
    	fmt.Gmask = 0x0000ff00; // G
    	fmt.Bmask = 0xff000000; // B
    	fmt.Amask = 0x000000ff; // A
    #endif
    	fmt.Rshift = 0;
    	fmt.Gshift = 8;
    	fmt.Bshift = 16;
    	fmt.Ashift = 24;
    	fmt.Rloss = 0;
    	fmt.Gloss = 0;
    	fmt.Bloss = 0;
    	fmt.Aloss = 0;
    	fmt.palette = NULL;

    	pStatusFont =  AG_FetchFont (STAT_FONT,STAT_PT, 0);
        AG_PushTextState();
        AG_TextFont(pStatusFont);
        //AG_TextFontPts(11);
#if 1
        AG_TextColor(black);
        AG_TextBGColor(r);
        pInsOn = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH * 2, LED_HEIGHT * 2, &fmt, AG_SRCALPHA);
        AG_FillRect(pInsOn, NULL, r);
        tmps = AG_TextRender("Ins");
        AG_SurfaceBlit(tmps, NULL, pInsOn, 4, 0);
        AG_SurfaceFree(tmps);
    	tid_ins_on = CreateTexture(pInsOn);
    	AG_SurfaceFree(pInsOn);
    	pInsOn = NULL;

        pCapsOn = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH * 2, LED_HEIGHT * 2, &fmt, AG_SRCALPHA);
        AG_FillRect(pCapsOn, NULL, r);
        tmps = AG_TextRender("CAP");
        AG_SurfaceBlit(tmps, NULL, pCapsOn, 1, 0);
        AG_SurfaceFree(tmps);
        tid_caps_on = CreateTexture(pCapsOn);
        AG_SurfaceFree(pCapsOn);
        pCapsOn = NULL;

        pKanaOn = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH * 2, LED_HEIGHT * 2, &fmt, AG_SRCALPHA);
        AG_FillRect(pKanaOn, NULL, r);
        tmps = AG_TextRender("カナ");
        AG_SurfaceBlit(tmps, NULL, pKanaOn, 8, 0);
        AG_SurfaceFree(tmps);
    	tid_kana_on = CreateTexture(pKanaOn);
    	AG_SurfaceFree(pKanaOn);
    	pKanaOn = NULL;

        AG_TextColor(n);
        AG_TextBGColor(alpha);

        pInsOff = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH * 2, LED_HEIGHT * 2, &fmt, AG_SRCALPHA);
        AG_FillRect(pInsOff, NULL, alpha);
        tmps = AG_TextRender("Ins");
        AG_SurfaceBlit(tmps, NULL, pInsOff, 4, 0);
        AG_SurfaceFree(tmps);
    	tid_ins_off = CreateTexture(pInsOff);
    	AG_SurfaceFree(pInsOff);
    	pInsOff = NULL;

        pCapsOff = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH * 2, LED_HEIGHT * 2,  &fmt, AG_SRCALPHA);
        AG_FillRect(pCapsOff, NULL, alpha);
        tmps = AG_TextRender("CAP");
        AG_SurfaceBlit(tmps, NULL, pCapsOff, 1, 0);
        AG_SurfaceFree(tmps);
        tid_caps_off = CreateTexture(pCapsOff);
        AG_SurfaceFree(pCapsOff);
        pCapsOff = NULL;


        pKanaOff = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH *2, LED_HEIGHT * 2, &fmt, AG_SRCALPHA);
        AG_FillRect(pKanaOff, NULL, alpha);
        tmps = AG_TextRender("カナ");
        AG_SurfaceBlit(tmps, NULL, pKanaOff, 8, 0);
        AG_SurfaceFree(tmps);
    	tid_kana_off = CreateTexture(pKanaOff);
    	AG_SurfaceFree(pKanaOff);
    	pKanaOff = NULL;
    	AG_PopTextState();

        AG_PushTextState();
        AG_TextFont(pStatusFont);

        /*
         * RECT Drive1
         */
        rec.x = 0;
        rec.y = 0;
        rec.w = VFD_WIDTH * 2;
        rec.h = VFD_HEIGHT * 2;
        for(i = 0 ; i < 2 ; i++) {
                pFDRead[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
                pFDWrite[i] =AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
                pFDNorm[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
                AG_FillRect(pFDRead[i], &rec, r);
                AG_FillRect(pFDWrite[i], &rec, b);
                AG_FillRect(pFDNorm[i], &rec, alpha);
        }

        /*
         * RECT Tape
         */
        rec.x = 0;
        rec.y = 0;
        rec.w = CMT_WIDTH * 2;
        rec.h = CMT_HEIGHT * 2;
        pCMTRead = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
        pCMTWrite = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
        pCMTNorm = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);

        AG_FillRect(pCMTRead, &rec, r);
        AG_FillRect(pCMTWrite, &rec, b);
        AG_FillRect(pCMTNorm, &rec, alpha);

        rec.x = 0;
        rec.y = 0;
        rec.w = STAT_WIDTH * 2;
        rec.h = STAT_HEIGHT * 2;

        pCaption = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
        AG_FillRect(pCaption, &rec,  alpha);

#endif
        AG_PopTextState();
}

void DestroyStatus(void)
{
        int i, j;
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
        if(pKanaOff !=NULL ) {
            AG_SurfaceFree(pKanaOff);
                pKanaOff = NULL;
        }
        if(pCaption !=NULL ) {
            AG_SurfaceFree(pCaption);
                pCaption = NULL;
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
        if(pStatusFont != NULL) {
        	AG_DestroyFont(pStatusFont);
                pStatusFont = NULL;
        }
        DiscardTexture(tid_ins_on);
        DiscardTexture(tid_caps_on);
        DiscardTexture(tid_kana_on);
        DiscardTexture(tid_ins_off);
        DiscardTexture(tid_caps_off);
        DiscardTexture(tid_kana_off);
        for(i = 0; i < 4; i++) {
        	for(j = 0; j < 3; j++){
                DiscardTexture(tid_fd[i][j]);
        	}
        }
        for(i = 0; i < 3 ; i++) {
        	DiscardTexture(tid_cmt[i]);
        }
        DiscardTexture(tid_caption);

}
    /*
     *  キャプション描画
     */
static void DrawMainCaption(void)
{
    char           string[1024];
    char           tmp[128];
    char          *p;
    AG_Surface   *tmps;
    AG_Rect rect;
    AG_Font *fp;

    if(pCaption == NULL) return;
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
        AG_PushTextState();
        AG_TextFont(pStatusFont);
    	AG_TextColor(n);
    	AG_TextBGColor(alpha);
    	AG_FillRect(pCaption, NULL, alpha);
        tmps = AG_TextRender(szCaption);
        rect.x = 0;
        rect.y = 0;
        rect.h = STAT_HEIGHT * 2;
        rect.w = STAT_WIDTH * 2;
        AG_SurfaceBlit(tmps, &rect, pCaption, 2, 0);
        AG_SurfaceFree(tmps);
        AG_PopTextState();
        strcpy(szOldCaption, szCaption);
//        DiscardTexture(tid_caption);
       	tid_caption =  CreateTexture(pCaption);
        DrawTexture(tid_caption, 8 ,  800 - STAT_HEIGHT *2 - 4, STAT_WIDTH*2, STAT_HEIGHT*2);
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
//        if((pCapsOff == NULL) || (pCapsOn == NULL)) return;
        if (caps_flag) {
                num = 1;
        } else {
                num = 0;
        }
/*
 * 描画、ワーク更新
 */
        nCAP = num;
        if (nCAP) {
            DrawTexture(tid_caps_on, 1280 - 4 - LED_WIDTH * 4,  800 - LED_HEIGHT*2 - 4, LED_WIDTH * 2, LED_HEIGHT * 2);
        } else {
            DrawTexture(tid_caps_off, 1280 - 4 - LED_WIDTH * 4,  800 - LED_HEIGHT*2 - 4, LED_WIDTH * 2, LED_HEIGHT * 2);
        }
}


    /*
     *  かなキー描画
     */
static void DrawKANA(void)
{
        int            num;
//        if((pKanaOff == NULL) || (pKanaOn == NULL)) return;

        /*
         * 番号決定
         */
        if (kana_flag) {
                num = 1;
        } else {
                num = 0;
        }

/*
 * 描画、ワーク更新
 */

        nKANA = num;
        if (nKANA) {
            DrawTexture(tid_kana_on, 1280 - LED_WIDTH*2 - 4,  800 - LED_HEIGHT*2 - 4 , LED_WIDTH*2, LED_HEIGHT*2);
        } else {
            DrawTexture(tid_kana_off, 1280 - LED_WIDTH*2 - 4,  800 - LED_HEIGHT*2 - 4, LED_WIDTH*2, LED_HEIGHT*2);
        }
}


    /*
     *  INSキー描画
     */
static void DrawINS(void)
{
        int            num;
//        if((pInsOff == NULL) || (pInsOn == NULL)) return;

        /*
         * 番号決定
         */
        if (ins_flag) {
                num = 1;
        }  else {
                num = 0;
        }

/*
 * 描画、ワーク更新
 */
        nINS = num;
        if (nINS) {
            DrawTexture(tid_ins_on, 1280 - LED_WIDTH * 6 - 4,  800 - LED_HEIGHT*2- 4, LED_WIDTH*2, LED_HEIGHT*2);
        } else {
            DrawTexture(tid_ins_off, 1280 - LED_WIDTH * 6 - 4,  800 - LED_HEIGHT*2 - 4, LED_WIDTH*2, LED_HEIGHT*2);
        }
}

    /*
     *  ドライブ描画
     */
static void DrawDrive(AG_Widget *w, int drive)
{
    int            num;
    int i;
    char          *name;
    char          string[64];
    char          utf8[256];
    char		outstr[300];
    char          *pIn, *pOut;
    iconv_t       hd;
    size_t        in, out;
    AG_Surface *tmp;
    AG_Rect		rect;

    memset(string, 0x00, sizeof(string));
    memset(utf8, 0x00, sizeof(utf8));
    memset(outstr, 0x00, sizeof(outstr));


    ASSERT((drive >= 0) && (drive <= 1));

    if((pFDRead[drive] == NULL) || (pFDWrite[drive] == NULL) || (pFDNorm[drive] == NULL)) return;

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
    if (nDrive[drive] == num) {
            if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
                    if(strlen(szDrive[drive]) > 0) {
                    	//レンダリング
                    }
                szDrive[drive][0] = '\0';
                szOldDrive[drive][0] = '\0';
                /*
                 * テクスチャ破棄必要か？
                 */
            	return;
            }
            if (strcmp(szDrive[drive], name) == 0) {
//            	for(i = 0; i < 3 ; i++) {
//            		DiscardTexture(tid_fd[drive][i]);
//            	}
//            	return;
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


    if((strcmp(szDrive[drive], szOldDrive[drive]) != 0) || (old_writep[drive] != fdc_writep[drive])) {
            /*
             * 過去のファイルネームと違うのでフォントレンダリングする
             */
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
            if(strlen(utf8) >0) {
            	if(fdc_writep[drive]) {
            		sprintf(outstr, "■ %s", utf8); // 書込み禁止
            	} else {
            		sprintf(outstr, "　 %s", utf8); // 書込み許可
            	}
            	old_writep[drive] = fdc_writep[drive];
            }
            /*
             * 旧いテクスチャを破棄
             */
            for(i = 0; i < 3 ; i++) {
        		DiscardTexture(tid_fd[drive][i]);
        	}
            	AG_PushTextState();
                AG_TextFont(pStatusFont);
            	AG_TextColor(alpha);
            	AG_TextBGColor(r);
                rect.x = 0;
                rect.y = 0;
                rect.h = VFD_HEIGHT * 2;
                rect.w = VFD_WIDTH * 2;
            	AG_FillRect(pFDRead[drive], &rect, r);
                tmp = AG_TextRender(outstr);
                AG_SurfaceBlit(tmp, &rect, pFDRead[drive], 0, 0);
                AG_SurfaceFree(tmp);
        		tid_fd[drive][1] =  CreateTexture(pFDRead[drive]);

               	AG_TextBGColor(b);
                AG_FillRect(pFDWrite[drive], &rect, b);
                tmp = AG_TextRender(outstr);
                AG_SurfaceBlit(tmp, &rect, pFDWrite[drive], 0, 0);
                AG_SurfaceFree(tmp);
        		tid_fd[drive][2] =  CreateTexture(pFDWrite[drive]);

               	AG_TextColor(n);
               	AG_TextBGColor(alpha);
                AG_FillRect(pFDNorm[drive], &rect, alpha);
                tmp = AG_TextRender(outstr);
                AG_SurfaceBlit(tmp, &rect, pFDNorm[drive], 0, 0);
                AG_SurfaceFree(tmp);
        		tid_fd[drive][0] =  CreateTexture(pFDNorm[drive]);

        		AG_PopTextState();

            memset(szOldDrive[drive], 0, 16);
            strncpy(szOldDrive[drive], szDrive[drive], 16);
    }

    	if (nDrive[drive] == FDC_ACCESS_READ) {
    	    DrawTexture(tid_fd[drive][1], STAT_WIDTH*2 + 16 + (VFD_WIDTH + 8) * (1 - drive) * 2, 800 - VFD_HEIGHT*2 - 4, VFD_WIDTH*2, VFD_HEIGHT*2);
    	} else if (nDrive[drive] == FDC_ACCESS_WRITE) {
    	    DrawTexture(tid_fd[drive][2], STAT_WIDTH*2 + 16 + (VFD_WIDTH + 8) * (1 - drive) * 2, 800 - VFD_HEIGHT*2 - 4, VFD_WIDTH*2, VFD_HEIGHT*2);
    	} else {
    	    DrawTexture(tid_fd[drive][0], STAT_WIDTH*2 + 16 + (VFD_WIDTH + 8) * (1 - drive) * 2, 800 - VFD_HEIGHT*2 - 4, VFD_WIDTH*2, VFD_HEIGHT*2);
    	}
}


/*
 *  テープ描画
 */
static void DrawTape(void)
{
	int 		i;
    int             num;
    char            string[128];
    char     protect[16];
    AG_Rect rect;
    AG_Surface *tmp;


    if(tape_writep){
    	strcpy(protect, "■");
    } else {
    	strcpy(protect, "　");
    }
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
    	sprintf(string, "%s%04d", protect, nTape % 10000);
    }
    if(nOldTape != nTape) {
            /*
             * カウンタ番号レンダリング(仮)
             */
    		for(i = 0; i < 3 ; i++) {
    			DiscardTexture(tid_cmt[i]);
    		}
            if(pStatusFont != NULL) {
           	AG_PushTextState();
            AG_FillRect(pCMTRead, &rect, r);
            rect.x = 0;
            rect.y = 0;
            rect.h = CMT_HEIGHT * 2;
            rect.w = CMT_WIDTH * 2;
            AG_TextFont(pStatusFont);
            AG_TextColor(black);
            AG_TextBGColor(r);
           	tmp = AG_TextRender(string);
           	AG_SurfaceBlit(tmp, &rect, pCMTRead, 0, 0);
           	AG_SurfaceFree(tmp);
        	tid_cmt[1] =  CreateTexture(pCMTRead);


            AG_FillRect(pCMTWrite, &rect, b);
            AG_TextFont(pStatusFont);
            AG_TextColor(black);
            AG_TextBGColor(b);
           	tmp = AG_TextRender(string);
           	AG_SurfaceBlit(tmp, &rect, pCMTWrite, 0, 0);
           	AG_SurfaceFree(tmp);
        	tid_cmt[2] =  CreateTexture(pCMTWrite);


            AG_FillRect(pCMTNorm, &rect, alpha);
            AG_TextFont(pStatusFont);
            AG_TextColor(n);
            AG_TextBGColor(alpha);
           	tmp = AG_TextRender(string);
           	AG_SurfaceBlit(tmp, &rect, pCMTNorm, 0, 0);
           	AG_SurfaceFree(tmp);
        	tid_cmt[0] =  CreateTexture(pCMTNorm);

           	AG_PopTextState();
            }
            if ((nTape >= 10000) && (nTape < 30000)) {
                    if (nTape >= 20000) {
                    	DrawTexture(tid_cmt[2], 1280 - LED_WIDTH * 6 - CMT_WIDTH * 2 - 8 , 800 - CMT_HEIGHT*2 - 4,  CMT_WIDTH * 2, CMT_HEIGHT * 2);
                    }   else {
                    	DrawTexture(tid_cmt[1], 1280 - LED_WIDTH * 6 - CMT_WIDTH * 2 - 8 , 800 - CMT_HEIGHT*2 - 4,  CMT_WIDTH * 2, CMT_HEIGHT * 2);
                    }
            } else {
            	DrawTexture(tid_cmt[0], 1280 - LED_WIDTH * 6 - CMT_WIDTH * 2 - 8 , 800 - CMT_HEIGHT*2 - 4,  CMT_WIDTH * 2, CMT_HEIGHT * 2);
            }
            nOldTape = nTape;
    }


}

/*
 *  描画
 */

void DrawStatus(void)
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
        nInitialDrawFlag = FALSE;
#endif
}


/*
 *  強制描画
 */
void DrawStatusForce(void)
{
        nInitialDrawFlag = TRUE;
        DrawStatus();
}


/*
 *  再描画
 */
void PaintStatus(void)
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
        nDriveOld[0] = -1;
        nDriveOld[1] = -1;
        szDrive[0][0] = '\0';
        szDrive[1][0] = '\0';
        szOldDrive[0][0] = '\0';
        szOldDrive[1][0] = '\0';
        nTape = -1;
        nOldTape = 0;

        /*
         * 描画
         */
  //      DrawStatus();
}
