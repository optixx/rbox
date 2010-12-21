
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


#include "RBox.h"

#ifdef D3D_DEMO

int _counter = 0;
int	_vcounter = 0;
FillRunsProc _fillRuns;
FillLineProc _fillLine;
u8 _fillRunsBuffer[256];	// Guess!
u8 _fillLineBuffer[80];	// Guess!
u8 _appBuffer[800];

void DrawTriangles(int keys, byte* appBuffer, int len);

#if 0
#include "math.h"
void Set()
{
	for (int i = 0; i < 64; i++)
	{
		double oh = i+0.5;
		oh = oh/sqrt(oh*oh+64*64);
		double len = 364.0/oh;	// 0.007 TO .7 =>
		int p = (int)len;
		p >>= 4;
		if (p > 255)
			p = 255;
		p = 255 - p;
		char dst[128];
		sprintf(dst,"0x%02X,",p);
		::OutputDebugStringA(dst);
	}
}
#endif

 u8 voronoidark[] = { // 24x24
0x43,0x42,0x45,0x5E,0x4D,0x48,0x47,0x3F,0x31,0x50,0x4A,0x48,0x48,0x41,0x44,0x5C,0x4D,0x48,0x48,0x45,0x40,0x49,0x49,0x44,
0x3F,0x3D,0x5D,0x67,0x4E,0x47,0x3F,0x2B,0x3C,0x5C,0x4E,0x4A,0x45,0x3D,0x5D,0x67,0x4D,0x48,0x47,0x4B,0x4A,0x4E,0x48,0x42,
0x44,0x60,0x70,0x4C,0x2B,0x27,0x28,0x3B,0x5C,0x4D,0x3D,0x42,0x48,0x60,0x70,0x4B,0x2B,0x28,0x2C,0x38,0x49,0x59,0x58,0x44,
0x69,0x6F,0x4F,0x28,0x3D,0x54,0x5D,0x60,0x50,0x42,0x62,0x72,0x73,0x6E,0x4F,0x29,0x3D,0x54,0x54,0x44,0x41,0x54,0x62,0x63,
0x4F,0x4F,0x33,0x3F,0x61,0x59,0x54,0x4F,0x41,0x5F,0x6F,0x5E,0x57,0x50,0x33,0x3F,0x61,0x59,0x56,0x54,0x4C,0x4F,0x51,0x4E,
0x47,0x4A,0x36,0x52,0x56,0x4F,0x4F,0x4B,0x45,0x66,0x5A,0x4F,0x4F,0x4A,0x37,0x51,0x55,0x4F,0x4F,0x4C,0x4C,0x59,0x51,0x48,
0x48,0x43,0x34,0x59,0x52,0x4F,0x4E,0x47,0x4A,0x64,0x53,0x4F,0x4F,0x4C,0x41,0x51,0x50,0x4E,0x4F,0x47,0x4B,0x62,0x54,0x48,
0x3A,0x2C,0x3D,0x5C,0x4E,0x49,0x44,0x3D,0x5C,0x67,0x4E,0x47,0x47,0x4C,0x4B,0x4F,0x48,0x47,0x44,0x3D,0x5C,0x67,0x4D,0x41,
0x24,0x39,0x5D,0x4D,0x3D,0x41,0x48,0x5F,0x72,0x4D,0x2C,0x28,0x2E,0x38,0x49,0x59,0x56,0x49,0x49,0x60,0x71,0x4D,0x2C,0x23,
0x55,0x61,0x50,0x41,0x61,0x72,0x72,0x6F,0x50,0x29,0x3B,0x53,0x52,0x44,0x41,0x54,0x63,0x6D,0x72,0x6F,0x50,0x29,0x3C,0x4D,
0x4D,0x4F,0x41,0x60,0x6E,0x5E,0x56,0x50,0x34,0x3E,0x60,0x58,0x55,0x54,0x4D,0x4F,0x52,0x56,0x56,0x50,0x34,0x3E,0x60,0x50,
0x47,0x4C,0x45,0x63,0x59,0x4F,0x4F,0x4A,0x36,0x52,0x56,0x4F,0x4F,0x4E,0x4D,0x57,0x51,0x4F,0x4E,0x49,0x34,0x52,0x55,0x48,
0x48,0x4D,0x49,0x59,0x51,0x4E,0x4F,0x4C,0x42,0x4F,0x50,0x4F,0x4F,0x4D,0x4A,0x56,0x50,0x4F,0x4F,0x45,0x35,0x57,0x52,0x48,
0x44,0x4D,0x4E,0x4C,0x42,0x44,0x47,0x4B,0x4A,0x4E,0x48,0x48,0x4B,0x4D,0x4D,0x4C,0x41,0x44,0x3F,0x2B,0x3C,0x5C,0x4E,0x44,
0x3C,0x44,0x4C,0x54,0x44,0x31,0x2E,0x37,0x48,0x58,0x58,0x4A,0x43,0x44,0x4C,0x54,0x44,0x30,0x29,0x3B,0x5D,0x4D,0x3C,0x3B,
0x62,0x58,0x4A,0x51,0x59,0x59,0x53,0x44,0x41,0x53,0x62,0x6B,0x6A,0x59,0x4B,0x52,0x59,0x5A,0x5C,0x61,0x50,0x41,0x62,0x67,
0x52,0x56,0x4F,0x4F,0x50,0x53,0x56,0x54,0x4C,0x50,0x52,0x55,0x59,0x56,0x4F,0x4F,0x50,0x54,0x54,0x4F,0x41,0x5F,0x6F,0x56,
0x48,0x4C,0x43,0x52,0x50,0x4F,0x4F,0x4C,0x4C,0x59,0x51,0x4E,0x4F,0x4D,0x45,0x51,0x50,0x4E,0x4F,0x4B,0x44,0x64,0x59,0x48,
0x48,0x44,0x35,0x58,0x52,0x4F,0x4F,0x47,0x4B,0x63,0x53,0x4F,0x4F,0x4C,0x43,0x50,0x51,0x4F,0x4F,0x4D,0x4B,0x56,0x50,0x48,
0x39,0x2C,0x3E,0x5D,0x4E,0x49,0x44,0x3C,0x5C,0x67,0x4E,0x46,0x47,0x4C,0x4A,0x4F,0x48,0x47,0x4A,0x4D,0x4D,0x4C,0x43,0x3E,
0x25,0x39,0x5D,0x4D,0x3D,0x41,0x48,0x5F,0x72,0x4D,0x2C,0x28,0x2E,0x38,0x49,0x58,0x56,0x49,0x44,0x44,0x4B,0x52,0x43,0x2A,
0x56,0x60,0x50,0x42,0x62,0x72,0x72,0x6F,0x50,0x29,0x3C,0x53,0x53,0x44,0x41,0x54,0x63,0x6C,0x68,0x58,0x49,0x51,0x59,0x53,
0x4D,0x50,0x41,0x60,0x6E,0x5E,0x56,0x51,0x34,0x3F,0x60,0x58,0x56,0x54,0x4D,0x4F,0x52,0x56,0x59,0x59,0x4F,0x4F,0x51,0x4D,
0x44,0x45,0x40,0x5D,0x52,0x48,0x48,0x44,0x2F,0x4B,0x4E,0x48,0x48,0x47,0x47,0x51,0x4A,0x47,0x48,0x46,0x3E,0x4C,0x4A,0x44};

u8 ramp[]  =
{
	0x00,0x1E,0x3C,0x53,0x65,0x73,0x7F,0x89,0x91,0x99,0x9F,0xA4,0xA9,0xAE,0xB1,0xB5,
	0xB8,0xBB,0xBD,0xC0,0xC2,0xC4,0xC6,0xC8,0xC9,0xCB,0xCC,0xCD,0xCE,0xD0,0xD1,0xD2,
	0xD2,0xD3,0xD4,0xD5,0xD6,0xD6,0xD7,0xD8,0xD8,0xD9,0xD9,0xDA,0xDA,0xDB,0xDB,0xDC,
	0xDC,0xDC,0xDD,0xDD,0xDD,0xDE,0xDE,0xDE,0xDF,0xDF,0xDF,0xDF,0xE0,0xE0,0xE0,0xE0,
};

void D3DInit()
{
	//Set();
	VideoOutInit(40*8);	//
	LineBufferInit();
	_fillRuns = (FillRunsProc)CopyProcToRam((void*)FillRuns,_fillRunsBuffer,sizeof(_fillRunsBuffer));
	_fillLine = (FillLineProc)CopyProcToRam((void*)FillLine24,_fillLineBuffer,sizeof(_fillLineBuffer));
	DrawTriangles(0x80,_appBuffer,sizeof(_appBuffer));
}

int _lasty;
int _firstY;
int _left;
u8 _runs[2048];
int _read = 0;
int _write = 0;
int _lastWrite = 0;
u8* DrawTrianglesLoop(u8* appBuffer, u8* runs);


extern int _hacky;

u8 _patternBlack[24] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

int D3DLoop(int n)
{
	_lasty = -1;
	_read = _write = _lastWrite = 0;
	_counter++;
	_vcounter += 6;
	if (_vcounter == 6*24)
		_vcounter = 0;

	_hacky = -1;
	DrawTriangles(0,_appBuffer,sizeof(_appBuffer));		// Start a new frame
	u8* write = _runs;
	u8* w = write;

	while (VideoOutLine() >= 22)		// finishing last frame
	{
		w = DrawTrianglesLoop(_appBuffer,write);	// Get ahead
		if (w)
			write = w;
	}

	//	Start a new frame
	//
	int line;
	int geoTop = _hacky;		// First y of geometric figure
	int bgTop = (240-128)>>1;	// First y of bg
	int bgBottom = 240-bgTop;	// Last y of bg
	int y = 255;				// current y

	for (;;)
	{
		line = VideoOutLine();
		if (y == 255)
		{
			if (line > (bgTop+22))			// Hit bg first
			{
				y = bgTop;
				break;
			}
			else if (line > (geoTop+22))	// Hit geometry first
			{
				while (line == VideoOutLine())
					__WFI();
				y = geoTop;
				break;
			}
		}
		w = DrawTrianglesLoop(_appBuffer,write);	// Get ahead
		if (w)
			write = w;
	}

	u8* read = _runs;	// count,left,runs
	u32 v = _vcounter;
	for (;;)
	{
		u8* dst = LineBufferGet();
		bool bg = y >= bgTop && y < bgBottom;
		if (bg)
		{
			int c;
			line = y-bgTop;
			u32 p[6];
			u8* pattern;
			if (line < 64)
			{
				// sky
				u32 color = ((_counter + line) >> 7) & 0x7;
				color |= color << 8;
				color |= color << 16;
				u32 add = (63-line)<<1;
				add |= add << 8;
				add |= add << 16;
				u32* src = (u32*)voronoidark + v;
				//src += v;
				u32 mask = 0xF8F8F8F8;
				p[0] = ((src[0] + add) & mask) | color;
				p[1] = ((src[1] + add) & mask) | color;
				p[2] = ((src[2] + add) & mask) | color;
				p[3] = ((src[3] + add) & mask) | color;
				p[4] = ((src[4] + add) & mask) | color;
				p[5] = ((src[5] + add) & mask) | color;
				pattern = (u8*)p;
				v += 6;
				if (v == 6*24)
					v = 0;
			}
			else
			{
				//	Ground animation
				line &= 0x3F;
				c = ramp[line];
				int r = _counter - c;
				u8 cr = 0;
				if ((r & 0x30) == 0x30)
				{
					if (line > 4)
						cr = (_counter >> 8) & 7;
					c += c>>1;
					if (c > 255)
						c = 255;
				}
				c &= 0xF8;
				c |= cr;
				pattern = (u8*)p;
				c |= c<<8;
				c |= c<<16;
				p[0] = p[1] = p[2] = p[3]  = p[4] = p[5]= c;
			}
			_fillLine(dst,pattern,12);
		} else
			_fillLine(dst,_patternBlack,12);

		if (y > geoTop)
		{
			if (read < write)
			{
				u8 left = read[1];
				u8 count = read[0];
				if (count)
					_fillRuns(dst+left,read+2,count);
				read += 2 + (count<<1);
			}
		}
		LineBufferEnqueue(dst);
		y++;

		if (w)
		{
			w = DrawTrianglesLoop(_appBuffer,write);	// read more
			if (!w)
				break;
			write = w;
		}

		if (!w && y >= bgBottom && read == write)
			break;
	}
	return 0;
}

#endif