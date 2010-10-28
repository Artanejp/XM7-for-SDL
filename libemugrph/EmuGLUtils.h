/*
 * EmuGLUtils.h
 *
 *  Created on: 2010/10/27
 *      Author: whatisthis
 */

#ifndef EMUGLUTILS_H_
#define EMUGLUTILS_H_

#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <stdio.h>
#include <stdlib.h>
#include "EmuGrphScale1x2.h"

class EmuGLUtils : public EmuGrphScale1x2 {
public:
	EmuGLUtils();
	virtual ~EmuGLUtils();
	int InitGL(int w, int h);
	void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
	void SetViewPort(int x, int y, int w, int h);
	void SetViewPort(void);
	void DiscardTextures(int n, GLuint *tid);
	void DiscardTextures(void);
	void Flip(void);
protected:
	int viewport_x;
	int viewport_y;
	int viewport_w;
	int viewport_h;
	int gl_bpp;
	GLuint textureid;
	float minX;
	float minY;
	float maxX;
	float maxY;
	GLuint CreateTexture(int w, int h, Uint8 *bitmap);
	void Enter2DMode();
	void Leave2DMode();
	void DrawTexture(void);
private:
	int power_of_two(int input);
};

#endif /* EMUGLUTILS_H_ */
