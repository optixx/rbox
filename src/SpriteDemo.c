
/*
 * SpriteDemo.c
 *
 *  Created on: May 27, 2010
 *      Author: Peter
 */

#include "RBox.h"

typedef struct
{
	u8	Width;
	u8	Height;
	u8	FrameCount;
	u8	R0;
	const u8* Frames[8];
} Sprite;

typedef struct
{
	short	X;
	short	Y;
	short	DX;
	short	DY;

	short	Frame;
	Sprite*	SpritePtr;
	const u8* Mark;
} SpriteRef;

#ifdef SPRITE_DEMO

#include "sprites.h"


int _counter = 0;
u8 _pattern[] = 
{
	0x80,0x90,0xA0,0xB0,
	0xA0,0x90,0x80,0x90,
	0xA0,0x90,0x80,0x90,
	0xA0,0x90,0x80,0x90
};

#define MAX_SPRITES 4
SpriteRef _spriteRefs[MAX_SPRITES];

#define MAX_SPRITE_LINES 256
typedef struct
{
	u32	HPos;	// could be 16
	const u8*	Data;	// could be an 16 offset from spritebase
} SpriteLine;
SpriteLine _spriteLines[MAX_SPRITE_LINES];	// 2k could be a 
u8	_ylines[256];

void MoveSprites()
{
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		SpriteRef& r = _spriteRefs[i];
	//	r.Frame++;
		if (r.Frame == r.SpritePtr->FrameCount)
			r.Frame = 0;
	}
}

void MixSprites()
{
	int lines = 0;
	int last = 0;
	for (int y = 0; y < 240; y++)
	{
		for (int i = 0; i < MAX_SPRITES; i++)
		{
			SpriteRef& r = _spriteRefs[i];
			if (r.Frame < 0)
				continue;

			if (y == r.Y)
				r.Mark = r.SpritePtr->Frames[r.Frame];	// Activate sprite

			const u8* m = r.Mark;
			if (m)
			{
				int x = r.X;
				signed char c = m[0];
				if (c == 0)
				{
					m++;	// empty line
				}
				else
				{
					if (((u8)c) == 0x80)
						m = 0;
					else {
						if (c < 0)
						{
							x -= c;
							m++;
						}
						_spriteLines[lines].HPos = x;
						_spriteLines[lines].Data = m;
						lines++;
						if (lines == MAX_SPRITE_LINES)
							return;

						//	skip to next line
						while ((c = *m++))
						{
							if (c > 0)
								m += c;
						}
					}
				}
				r.Mark = m;
			}
		}
		_ylines[y] = lines-last;
		last = lines;
	}
}

u8 _ramSprite[1024];
Sprite yshi_parakoopaflyRam = {
	40,
	57,
	1,
	0x00,
	_ramSprite
};

u8 _procBuf[128];
FillLineProc _fillLine;

extern "C"
void memcpy(void* dst, const void* src, int count);
void SpriteInit()
{
	VideoOutInit(16*16);
	LineBufferInit();
	_fillLine = (FillLineProc)CopyProcToRam((void*)FillLine,_procBuf,sizeof(_procBuf));

	int i = sizeof(yshi_parakoopafly_6);
	memcpy(_ramSprite,yshi_parakoopafly_6,i);

	SpriteRef f0 = {	100,200,0,0,0,&yshi_koopawalk,0 };
	SpriteRef f1 = {	120,100,0,0,0,&smwaniboo2,0 };
	SpriteRef f2 = {	40,30,0,0,0,&yshi_gooberwalk,0 };
	SpriteRef f3 = {	40,130,0,0,0,&yshi_parakoopafly,0 };

	SpriteRef f4 = {	40,130,0,0,0,&yshi_parakoopafly,0 };
	SpriteRef f5 = {	80,130,0,0,0,&yshi_parakoopafly,0 };
	SpriteRef f6 = {	120,130,0,0,0,&yshi_parakoopafly,0 };
	SpriteRef f7 = {	160,130,0,0,0,&yshi_parakoopafly,0 };

	_spriteRefs[0] = f0;
	_spriteRefs[1] = f1;
	_spriteRefs[2] = f2;
	_spriteRefs[3] = f3;

	_spriteRefs[0] = f4;
	_spriteRefs[1] = f5;
	_spriteRefs[2] = f6;
	_spriteRefs[3] = f7;
}

void DrawSprites(u8* dst, int y)
{
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		SpriteRef& r = _spriteRefs[i];
		if (r.Frame < 0)
			continue;
		if (y == r.Y)
			r.Mark = r.SpritePtr->Frames[r.Frame];	// Activate sprite
		if (r.Mark)
			DrawSpriteLine(dst+r.X,&r.Mark);
	}
}

extern "C"
void DrawSpriteLines(u8* dst, SpriteLine* s, int count);

#ifdef _WIN32
extern "C"
void DrawSpriteLines(u8* dst, SpriteLine* s, int count)
{
	SpriteLine* end = s + count;
	while (s<end)
	{
		u8* d = dst + s->HPos;
		const u8* src = s->Data;
		signed char c;
		while ((c = *src++))
		{
			if (c < 0)
				d -= c;
			else
			{
				while (c--)
					*d++ = *src++;
			}
		}
		s++;
	}
}
#endif

int SpriteLoop(int n)
{
	_counter++;
	MoveSprites();
	MixSprites();
	VideoOutWaitVBL();
	SpriteLine* s = _spriteLines;
	u8* yy = _ylines;
	int c;
	for (int y = 0; y < 200; y++)
	{
		u8* dst = LineBufferGet();
		_fillLine(dst,_pattern,18);
		if (c = *yy++)
		{
			DrawSpriteLines(dst,s,c);
			s += c;
		}
		LineBufferEnqueue(dst);
	}
	return 0;
}

#endif
