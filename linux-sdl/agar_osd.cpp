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
static AG_Surface      *pStatusBar; /* ステータス表示バー */
static AG_Font         *pStatusFont;

extern "C" {
extern AG_GLView *DrawArea;
extern AG_Window *MainWindow;
extern AG_Menu  *MenuBar;
}

GLuint tid_ins_on;
GLuint tid_kana_on;
GLuint tid_caps_on;
GLuint tid_ins_off;
GLuint tid_kana_off;
GLuint tid_caps_off;

GLuint tid_fd[4];
GLuint tid_cmt;
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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

void Enter2DMode(int x, int y, int w, int h)
{
    /* Note, there may be other things you need to change,
       depending on how you have your OpenGL state set up.
    */
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    /* This allows alpha blending of 2D textures with the scene */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    /*
     * ビューポートは表示する画面の大きさ
     */
    glViewport(x, y , w + x,  h + y );
    /*
     * 座標系は(0,0)-(0,1)
     */
    glOrtho(0.0, 1.0 ,
    		1.0, 0.0,
    		0.0,  1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

static void Leave2DMode()
{
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glPopAttrib();
}


static void DrawTexture(GLuint tid, int offset_x, int offset_y, int w, int h, int viewport_w, int viewport_h)
{
	float xbegin = 0.0f;
	float xend = 1.0f;
	float ybegin = 0.0f;
	float yend = 1.0f;

#if 1
	if(viewport_w != 0) {
		xbegin = (float)offset_x / (float)viewport_w;
		xend = ((float)offset_x + (float)w)/ (float)viewport_w;
	} else {
		xbegin = 0.0;
		xend = 1.0;
	}
	if(viewport_h != 0) {
		ybegin = (float)offset_y / (float)viewport_h;
		yend =   ((float)offset_y + (float) h) / (float)viewport_h;
	} else {
		xbegin = 0.0;
		xend = 1.0;
	}
#endif

	Enter2DMode(0, 0, viewport_w, viewport_h);
    glBindTexture(GL_TEXTURE_2D, tid);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(xbegin, ybegin, -0.5);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(xbegin, yend, -0.5);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(xend, ybegin, -0.5);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(xend, yend, -0.5);
    glEnd();
    Leave2DMode();
}

static void DrawCAP(void);
static void DrawINS(void);
static void DrawKANA(void);
static void DrawMainCaption(void);
static void DrawDrive(int drive);
static void DrawTape(void);

void DrawOSDGL(AG_GLView *w)
{
    DrawMainCaption();
    DrawCAP();
    DrawKANA();
    DrawINS();
    DrawDrive(1);
    DrawDrive(0);
    DrawTape();
}


/*-[ ステータスバー ]-------------------------------------------------------*/
/*
 *  ステータスバーの生成
 */
void CreateStatus(void)
{
        AG_Rect rec;
        AG_Color r, b, n , black;
        AG_Surface *tmps;
        AG_PixelFormat fmt;
        int i;


        r.r = 0; // r->g
        r.g = 0; // g->b
        r.b = 255;  // b->r
        r.a = 255; // a->a

        b.r = 0;
        b.g = 255;
        b.b = 0;
        b.a = 255;

        n.r = 255;
        n.g = 255;
        n.b = 255;
        n.a = 255;

        black.r = 0;
        black.g = 0;
        black.b = 0;
        black.a = 255;
    	// Surfaceつくる
    	fmt.BitsPerPixel = 32;
    	fmt.BytesPerPixel = 4;
#ifdef AG_BIG_ENDIAN
    	fmt.Rmask = 0x0000ff00; // R
    	fmt.Gmask = 0x00ff0000; // G
    	fmt.Bmask = 0x000000ff; // B
    	fmt.Amask = 0xff000000; // A
#else
    	fmt.Rmask = 0x00ff0000; // R
    	fmt.Gmask = 0x0000ff00; // G
    	fmt.Bmask = 0xff000000; // B
    	fmt.Amask = 0x000000ff; // A
    #endif
    	fmt.Rshift = 8;
    	fmt.Gshift = 16;
    	fmt.Bshift = 0;
    	fmt.Ashift = 24;
    	fmt.Rloss = 0;
    	fmt.Gloss = 0;
    	fmt.Bloss = 0;
    	fmt.Aloss = 0;
    	fmt.palette = NULL;

    	pStatusFont =  AG_FetchFont (STAT_FONT,STAT_PT , 0);

        AG_PushTextState();
        AG_TextFont(pStatusFont);
        //AG_TextFontPts(11);
#if 1
        AG_TextColor(black);
        AG_TextBGColor(r);
        pInsOn = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH, LED_HEIGHT, &fmt, AG_SRCALPHA);
        AG_FillRect(pInsOn, NULL, r);
        tmps = AG_TextRender("Ins");
        AG_SurfaceBlit(tmps, NULL, pInsOn, 4, 0);
        AG_SurfaceFree(tmps);
    	tid_ins_on = CreateTexture(pInsOn);
    	AG_SurfaceFree(pInsOn);
    	pInsOn = NULL;

        pCapsOn = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH, LED_HEIGHT, &fmt, AG_SRCALPHA);
        AG_FillRect(pCapsOn, NULL, r);
        tmps = AG_TextRender("CAP");
        AG_SurfaceBlit(tmps, NULL, pCapsOn, 2, 0);
        AG_SurfaceFree(tmps);
        tid_caps_on = CreateTexture(pCapsOn);
        AG_SurfaceFree(pCapsOn);
        pCapsOn = NULL;

        pKanaOn = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH, LED_HEIGHT, &fmt, AG_SRCALPHA);
        AG_FillRect(pKanaOn, NULL, r);
        tmps = AG_TextRender("カナ");
        AG_SurfaceBlit(tmps, NULL, pKanaOn, 2, 0);
        AG_SurfaceFree(tmps);
    	tid_kana_on = CreateTexture(pKanaOn);
    	AG_SurfaceFree(pKanaOn);
    	pKanaOn = NULL;

        AG_TextColor(n);
        AG_TextBGColor(black);

        pInsOff = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH, LED_HEIGHT, &fmt, AG_SRCALPHA);
        AG_FillRect(pInsOff, NULL, black);
        tmps = AG_TextRender("Ins");
        AG_SurfaceBlit(tmps, NULL, pInsOff, 4, 0);
        AG_SurfaceFree(tmps);
    	tid_ins_off = CreateTexture(pInsOff);
    	AG_SurfaceFree(pInsOff);
    	pInsOff = NULL;

        pCapsOff = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH, LED_HEIGHT,  &fmt, AG_SRCALPHA);
        AG_FillRect(pCapsOff, NULL, black);
        tmps = AG_TextRender("CAP");
        AG_SurfaceBlit(tmps, NULL, pCapsOff, 2, 0);
        AG_SurfaceFree(tmps);
        tid_caps_off = CreateTexture(pCapsOff);
        AG_SurfaceFree(pCapsOff);
        pCapsOff = NULL;


        pKanaOff = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH, LED_HEIGHT, &fmt, AG_SRCALPHA);
        AG_FillRect(pKanaOff, NULL, black);
        tmps = AG_TextRender("カナ");
        AG_SurfaceBlit(tmps, NULL, pKanaOff, 2, 0);
        AG_SurfaceFree(tmps);
    	tid_kana_off = CreateTexture(pKanaOff);
    	AG_SurfaceFree(pKanaOff);
    	pKanaOff = NULL;

        /*
         * RECT Drive1
         */
        rec.x = 0;
        rec.y = 0;
        rec.w = VFD_WIDTH;
        rec.h = VFD_HEIGHT;
        for(i = 0 ; i < 2 ; i++) {
                pFDRead[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
                pFDWrite[i] =AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
                pFDNorm[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
                AG_FillRect(pFDRead[i], &rec, r);
                AG_FillRect(pFDWrite[i], &rec, b);
                AG_FillRect(pFDNorm[i], &rec, black);
        }

        /*
         * RECT Tape
         */
        rec.x = 0;
        rec.y = 0;
        rec.w = CMT_WIDTH;
        rec.h = CMT_HEIGHT;
        pCMTRead = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
        pCMTWrite = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
        pCMTNorm = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);

        AG_FillRect(pCMTRead, &rec, r);
        AG_FillRect(pCMTWrite, &rec, b);
        AG_FillRect(pCMTNorm, &rec, black);

        rec.x = 0;
        rec.y = 0;
        rec.w = STAT_WIDTH;
        rec.h = STAT_HEIGHT;

        pCaption = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH, LED_HEIGHT, &fmt, AG_SRCALPHA);
        AG_FillRect(pCaption, NULL, black);

#endif
        AG_PopTextState();
}

void DestroyStatus(void)
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
        DiscardTexture(tid_fd[0]);
        DiscardTexture(tid_fd[1]);
        DiscardTexture(tid_fd[2]);
        DiscardTexture(tid_fd[3]);
        DiscardTexture(tid_cmt);
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
    AG_Color     white, black;
//    AG_Rect      rec,drec;

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
        black.r = 0;
        black.g = 0;
        black.b = 0;
        black.a = 255;

        white.r = 255;
        white.g = 255;
        white.b = 255;
        white.a = 255;

        AG_PushTextState();
        AG_TextFont(pStatusFont);
    	AG_TextColor(white);
    	AG_TextBGColor(black);
    	AG_FillRect(pCaption, NULL, black);
        tmps = AG_TextRender(szCaption);
        AG_SurfaceBlit(tmps, NULL, pCaption, 2, 0);
        AG_SurfaceFree(tmps);
        AG_PopTextState();
        strcpy(szOldCaption, szCaption);
    }

   	tid_caption =  CreateTexture(pCaption);
    DrawTexture(tid_caption, 0,  nDrawHeight, pCaption->w, pCaption->h , nDrawWidth , nDrawHeight + OSD_HEIGHT);
    DiscardTexture(tid_caption);
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
            DrawTexture(tid_caps_on, nDrawWidth - LED_WIDTH * 2,  nDrawHeight, LED_WIDTH, LED_HEIGHT, nDrawWidth, nDrawHeight + OSD_HEIGHT);
        } else {
            DrawTexture(tid_caps_off, nDrawWidth - LED_WIDTH * 2,  nDrawHeight, LED_WIDTH, LED_HEIGHT, nDrawWidth, nDrawHeight + OSD_HEIGHT);
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
            DrawTexture(tid_kana_on, nDrawWidth - LED_WIDTH,  nDrawHeight, LED_WIDTH, LED_HEIGHT, nDrawWidth, nDrawHeight + OSD_HEIGHT);
        } else {
            DrawTexture(tid_kana_off, nDrawWidth - LED_WIDTH,  nDrawHeight, LED_WIDTH, LED_HEIGHT, nDrawWidth, nDrawHeight + OSD_HEIGHT);
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
            DrawTexture(tid_ins_on, nDrawWidth - LED_WIDTH * 3,  nDrawHeight, LED_WIDTH, LED_HEIGHT, nDrawWidth, nDrawHeight + OSD_HEIGHT);
        } else {
            DrawTexture(tid_ins_off, nDrawWidth - LED_WIDTH * 3,  nDrawHeight, LED_WIDTH, LED_HEIGHT, nDrawWidth, nDrawHeight + OSD_HEIGHT);
        }
}

    /*
     *  ドライブ描画
     */
static void DrawDrive(int drive)
{
    int            num;
    char          *name;
    char          string[64];
    char          utf8[256];
    char		outstr[300];
    char          *pIn, *pOut;
    iconv_t       hd;
    size_t        in, out;
    AG_Surface *tmp;
    AG_Color r, b, white, black;
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
    if ((nDrive[drive] == num) && (nInitialDrawFlag != TRUE)) {
            if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
                    if(strlen(szDrive[drive]) > 0) {
                    	//レンダリング
                    }
                szDrive[drive][0] = '\0';
                szOldDrive[drive][0] = '\0';
//            	return;
            }
            if (strcmp(szDrive[drive], name) == 0) {
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

            white.r = 255;
            white.g = 255;
            white.b = 255;
            white.a = 255;

            black.r = 0;
            black.g = 0;
            black.b = 0;
            black.a = 255;

            r.r = 0; // r->g
            r.g = 0; // g->b
            r.b = 255;  // b->r
            r.a = 255; // a->a

            b.r = 0;
            b.g = 255;
            b.b = 0;
            b.a = 255;

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
            	AG_PushTextState();
                AG_TextFont(pStatusFont);
            	AG_TextColor(black);
            	AG_TextBGColor(r);
            	AG_FillRect(pFDRead[drive], NULL, r);
                tmp = AG_TextRender(outstr);
                AG_SurfaceBlit(tmp, NULL, pFDRead[drive], 2, 0);
                AG_SurfaceFree(tmp);

               	AG_TextBGColor(b);
                AG_FillRect(pFDWrite[drive], NULL, b);
                tmp = AG_TextRender(outstr);
                AG_SurfaceBlit(tmp, NULL, pFDWrite[drive], 2, 0);
                AG_SurfaceFree(tmp);

               	AG_TextColor(white);
               	AG_TextBGColor(black);
                AG_FillRect(pFDNorm[drive], NULL, black);
                tmp = AG_TextRender(outstr);
                AG_SurfaceBlit(tmp, NULL, pFDNorm[drive], 2, 0);
                AG_SurfaceFree(tmp);
            	AG_PopTextState();
            memset(szOldDrive[drive], 0, 16);
            strncpy(szOldDrive[drive], szDrive[drive], 16);
    }

    if (nDrive[drive] == FDC_ACCESS_READ) {
    	tid_fd[drive] =  CreateTexture(pFDRead[drive]);
    } else if (nDrive[drive] == FDC_ACCESS_WRITE) {
    	tid_fd[drive] =  CreateTexture(pFDWrite[drive]);
    } else {
    	tid_fd[drive] =  CreateTexture(pFDNorm[drive]);
    }
    DrawTexture(tid_fd[drive], nDrawWidth - pCMTNorm->h - pFDNorm[0]->w * (drive + 2) - LED_WIDTH * 3,  nDrawHeight, pFDNorm[0]->w, pFDNorm[0]->h , nDrawWidth , nDrawHeight + OSD_HEIGHT);
    DiscardTexture(tid_fd[drive]);
}


/*
 *  テープ描画
 */
static void DrawTape(void)
{
    int             num;
    char            string[128];
    char     protect[16];
    AG_Color n, black, r, b;
    AG_Rect rec;
    AG_Surface *tmp;


    rec.x = 0;
    rec.y = 0;
    rec.w = CMT_WIDTH;
    rec.h = CMT_HEIGHT;
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

            n.r = 255;
            n.g = 255;
            n.b = 255;
            n.a = 255;

            black.r = 0;
            black.g = 0;
            black.b = 0;
            black.a = 255;

            r.r = 0; // r->g
            r.g = 0; // g->b
            r.b = 255;  // b->r
            r.a = 255; // a->a

            b.r = 0;
            b.g = 255;
            b.b = 0;
            b.a = 255;

            if(pStatusFont != NULL) {
           	AG_PushTextState();
            AG_FillRect(pCMTRead, NULL, r);
            AG_TextFont(pStatusFont);
            AG_TextColor(black);
            AG_TextBGColor(r);
           	tmp = AG_TextRender(string);
           	AG_SurfaceBlit(tmp, NULL, pCMTRead, 4, 2);
           	AG_SurfaceFree(tmp);

            AG_FillRect(pCMTWrite, NULL, b);
            AG_TextFont(pStatusFont);
            AG_TextColor(black);
            AG_TextBGColor(b);
           	tmp = AG_TextRender(string);
           	AG_SurfaceBlit(tmp, NULL, pCMTWrite, 4, 2);
           	AG_SurfaceFree(tmp);


            AG_FillRect(pCMTNorm, NULL, black);
            AG_TextFont(pStatusFont);
            AG_TextColor(n);
            AG_TextBGColor(black);
           	tmp = AG_TextRender(string);
           	AG_SurfaceBlit(tmp, NULL, pCMTNorm, 4, 2);
           	AG_SurfaceFree(tmp);
           	AG_PopTextState();
            }
            nOldTape = nTape;
    }

    if(DrawArea== NULL) return;


    if ((nTape >= 10000) && (nTape < 30000)) {
            if (nTape >= 20000) {
            	tid_cmt =  CreateTexture(pCMTWrite);
            }   else {
            	tid_cmt =  CreateTexture(pCMTRead);
            }
    } else {
    	tid_cmt =  CreateTexture(pCMTNorm);
    }
    DrawTexture(tid_cmt, nDrawWidth - pCMTNorm->w - LED_WIDTH * 3,  nDrawHeight, pCMTNorm->w, pCMTNorm->h , nDrawWidth , nDrawHeight + OSD_HEIGHT);
    DiscardTexture(tid_cmt);
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
