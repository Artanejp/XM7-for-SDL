/*
 * EmuGLUtils.cpp
 *
 *  Created on: 2010/10/27
 *      Author: whatisthis
 */

#include <math.h>
#include "EmuGLUtils.h"

EmuGLUtils::EmuGLUtils() {
	// TODO Auto-generated constructor stub
	minX = 0.0f;
	minY = 0.0f;
	maxX = 1.0f;
	maxY = 1.0f;
}

EmuGLUtils::~EmuGLUtils() {
	// TODO Auto-generated destructor stub
}


/*
 * OpenGL/SDLの初期化。
 * 何らかの理由で初期化出来ないとき -1
 * 初期化できたらbpp値を返す
 */
int EmuGLUtils::InitGL(int w, int h)
{
	const SDL_VideoInfo *info = NULL;
	Uint32 flags;
	SDL_Surface *p;
	float aspect;
	int rgb_size[4];
	int fsaa = 1;
	int sync = 1;
	int accel = 1;

	info = SDL_GetVideoInfo();

	if(info == NULL) return -1;
	p = SDL_GetVideoSurface();
	if(p == NULL) return -1;
	flags = p->flags;
	flags |= SDL_OPENGL;
	if(SDL_SetVideoMode(w, h, info->vfmt->BitsPerPixel, flags) == 0)
	{
		return -1;
	}

	aspect = (float)w / (float)h;
//	glClearColor(0, 0, 0, 0);

    switch (p->format->BitsPerPixel) {
         case 8:
             rgb_size[0] = 3;
             rgb_size[1] = 3;
             rgb_size[2] = 2;
             break;
         case 15:
         case 16:
             rgb_size[0] = 5;
             rgb_size[1] = 5;
             rgb_size[2] = 5;
             break;
         default:
             rgb_size[0] = 8;
             rgb_size[1] = 8;
             rgb_size[2] = 8;
             break;
     }
     SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rgb_size[0] );
     SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, rgb_size[1] );
     SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, rgb_size[2] );
     SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
     SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
     if ( fsaa ) {
              SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
              SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, fsaa );
      }
      if ( accel ) {
              SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
      }
      if ( sync ) {
              SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 );
      } else {
              SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 0 );
      }

      printf("Screen BPP: %d\n", SDL_GetVideoSurface()->format->BitsPerPixel);
      printf("\n");
      printf( "Vendor     : %s\n", glGetString( GL_VENDOR ) );
      printf( "Renderer   : %s\n", glGetString( GL_RENDERER ) );
      printf( "Version    : %s\n", glGetString( GL_VERSION ) );
      printf( "Extensions : %s\n", glGetString( GL_EXTENSIONS ) );
      printf("\n");

}

/*
 * OpenGLのテクスチャ作る。
 * OpenGL 2.0以下の場合不具合あるかも
 */
GLuint EmuGLUtils::CreateTexture(int w, int h, Uint8 *bitmap)
{

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
                 bitmap);

	return textureid;
}

static inline void putdot(GLubyte *addr, Uint32 c)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[0] = 0xff;		/* A */
    addr[1] = c & 0xff;	/* B */
    addr[2] = (c >> 8) & 0xff;	/* R */
    addr[3] = (c >> 16) & 0xff; /* G */
#else
    addr[3] = 0xff;		/* A */
    addr[2] = c & 0xff;	/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff; /* G */
#endif

}

void EmuGLUtils::SetViewPort(int x, int y, int w, int h)
{
	viewport_x = x;
	viewport_y = y;
	viewport_h = h;
	viewport_w = w;
	minX = 0.0f;
	minY = 0.0f;
	maxX =  ((float)vramwidth * 8.0f) / (float)viewport_w ;
	maxY = (float)vramheight / (float)viewport_h;
#if 1
	printf("VIEWPORT %d,%d %d,%d %f,%f - %f,%f\n",
			viewport_x, viewport_y, viewport_h, viewport_w,
			minX, minY, maxX, maxY);
#endif
}

/*
 * ビューポートのリセット
 */
void EmuGLUtils::SetViewPort(void)
{
	SDL_Surface *p;
	p = SDL_GetVideoSurface();
	if(p == NULL) return;
	SetViewPort(0, 0, p->w, p->h);
}


void EmuGLUtils::PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	int xx, yy;
	int hh, ww;
	int addr;
	int size;
	int ofset;
	int i;
	Uint32 c[8];
	GLubyte *bitmap;

	size = vramwidth * vramheight * 8 * 4;
	glClearColor(0, 0, 0, 0);

	bitmap = (GLubyte *)malloc(size); // この関数内で全て処理する
	if(bitmap == NULL) return;
	ofset = 0;
	for(yy = 0; yy < vramheight; yy++) {
		for(xx = 0; xx < vramwidth; xx++) {
			addr = yy * vramwidth + xx;
			getvram(addr, c, mpage);
			for(i = 7; i >= 0; i--){
				putdot((GLubyte *)&bitmap[ofset], c[i]);
				ofset+=4;
			}
		}
	}
	textureid = CreateTexture(vramwidth * 8 , vramheight, bitmap);
	if(textureid <= 0) {
		free(bitmap);
		return;
	}
	DrawTexture();
	free(bitmap);
}


void EmuGLUtils::Enter2DMode()
{
     int w = viewport_w;
     int h = viewport_h;


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
        glViewport(0, 0 , w,  h);
        /*
         * 座標系は(0,0)-(0,1)
         */
        glOrtho(0.0, 1.0 ,
        		1.0, 0.0,
        		0.0,  1.0);
//        glOrtho(0.0, (GLdouble)w, (GLdouble)h, 0.0, 0.0, 2.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

void EmuGLUtils::Leave2DMode()
{
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glPopAttrib();
}

void EmuGLUtils::DrawTexture(void)
{
        static GLfloat texMinX, texMinY;
        static GLfloat texMaxX, texMaxY;
        static int x = 0;
        static int y = 0;
        static int w, h;

        SDL_Surface *screen = SDL_GetVideoSurface();
        /* Make texture coordinates easy to understand */
        /* Make sure that the texture conversion is okay */
        /* Move the image around */
                Enter2DMode();
                glBindTexture(GL_TEXTURE_2D, textureid);
                glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0, 0.0  );
                glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0, 1.0);
                glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0, 0.0 );
                glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0, 1.0);
             //   glViewport(0, 0 , viewport_w, viewport_h);
                glEnd();
                Leave2DMode();

//                SDL_GL_SwapBuffers();
}


void EmuGLUtils::DiscardTextures(void)
{
	DiscardTextures(1, &textureid);
}

void EmuGLUtils::DiscardTextures(int n, GLuint *tid)
{
	glDeleteTextures(n, tid);
}

void EmuGLUtils::Flip(void)
{
	glFlush();
	SDL_GL_SwapBuffers();
	DiscardTextures();
}
/* Quick utility function for texture creation */
int EmuGLUtils::power_of_two(int input)
{
        int value = 1;

        while ( value < input ) {
                value <<= 1;
        }
        return value;
}
