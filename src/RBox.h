
/* Copyright (c) 2010, Peter Barrett
**
** Permission to use, copy, modify, and/or distribute this software for
** any purpose with or without fee is hereby granted, provided that the
** above copyright notice and this permission notice appear in all copies.
**
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
** BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
** OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
** ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
** SOFTWARE.
*/


#ifndef _WIN32
#include "LPC11xx.h"                        /* LPC11xx definitions */
#else
#include "stdafx.h"
#define __WFI()
#endif

typedef unsigned char u8;
typedef unsigned char byte;
typedef unsigned long u32;

extern "C" void VideoOutInit(int width);
extern "C" void VideoOutWaitVBL();
extern "C" int VideoOutLine();
extern "C" void VideoOutEnqueueLine(u8** lineHandle);
extern "C"	u8 ReadXY(int* x, int* y);

void	LineBufferInit();
u8*		LineBufferGet();
void	LineBufferEnqueue(u8* dst);
bool	LineBufferBusy();

extern "C" void RenderCells(u8* dst, const u8* cells, const u8* base, int count);
extern "C" void FillRuns(u8* dst, u8* runs, int count);
extern "C" void FillLine(u8* dst, u8* pattern, int count);		// 16x
extern "C" void FillLine24(u8* dst, u8* pattern, int count);	// 24x
extern "C" void DrawSpriteLine(u8* dst,const u8** mark);
extern "C" 	void CopyPattern(u8* dst, const u8* src, int dlum, int color);

//	For running some procs from ram if desired
typedef void (*RenderCellsProc)(u8* dst, const u8* cells, const u8* base, int count);
typedef void (*FillLineProc)(u8* dst, u8* pattern, int count);
typedef void (*FillRunsProc)(u8* dst, u8* runs, int count);
void* CopyProcToRam(void* proc, u8* buffer, int len);


#define TILE_DEMO
//#define PALETTE_DEMO
//#define SPRITE_DEMO
//#define CURVE_DEMO
//#define D3D_DEMO
//#define PARTICLE_DEMO

#ifdef PALETTE_DEMO
void	PaletteInit();
int		PaletteLoop(int n);
#define AppInit PaletteInit
#define AppLoop PaletteLoop
#endif

#ifdef TILE_DEMO
void	TileInit();
int		TileLoop(int n);
#define AppInit TileInit
#define AppLoop TileLoop
#endif

#ifdef SPRITE_DEMO
void	SpriteInit();
int		SpriteLoop(int n);
#define AppInit SpriteInit
#define AppLoop SpriteLoop
#endif

#ifdef PARTICLE_DEMO
void	ParticleInit();
int		ParticleLoop(int n);
#define AppInit ParticleInit
#define AppLoop ParticleLoop
#endif

#ifdef D3D_DEMO
void D3DInit();
int D3DLoop(int n);
#define AppInit D3DInit
#define AppLoop D3DLoop
#endif

#ifdef CURVE_DEMO
#define AppInit CurveInit
#define AppLoop CurveLoop
#endif

