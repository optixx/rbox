
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

#ifdef TILE_DEMO

#include "tiles_scrap.h"

static int Range(int x, int m)
{
     while (x < 0)
         x += m;
     while (x >= m)
         x -= m;
     return x;
}

void Get256x256(int index, int y, u8* dst, u8* end)
{
    const u8* tm256 = tiles256x256;
    const u8* tm32 = tiles32x32;
	tm256 += (index << 6) + ((y >> 2) << 3);	// 8x8 blocks
	tm32 += ((y & 3) << 2);						// 4x4 blocks

	if (end > (dst+32))
		end = dst+32;
    u32* d32 = (u32*)dst;
    while (d32 < (u32*)end)
    {
		int index = *tm256++;
		u32* s32 = (u32*)(tm32 + (index << 4));
		*d32++ = *s32;
    }
}

//  buffer must be 96 bytes long (256xblocks)
//	Will return 1,2 or 3 256x256 pixel chunks
void GetCellLine(int x, int y, u8* dst, int count)
{
    const u8* tm = tilesmap;
    int width = TILESET_WIDTH >> 3;		// in blocks
    int height = TILESET_HEIGHT >> 3;
    y = Range(y, height);
	width >>= 5;
	x >>= 5;
	tm += (y >> 5) * width;				// vertical block index
	y &= 31;

	u8* end = dst+count;	// may exceed count by 31
    while (dst < end)
    {
        x = Range(x, width);			// read up to 32 cells from 256x256 block
        Get256x256(tm[x++], y, dst, end);
        dst += 32;
    }
}

u32 _counter = 0;
RenderCellsProc _renderCells;
u8 _procBuf[256];	// Guess!

#define HREZ (320-16)	// 1 block either side for smooth scrolling

void TileInit()
{
	VideoOutInit(HREZ);
	LineBufferInit();
	_renderCells = (RenderCellsProc)CopyProcToRam((void*)RenderCells,_procBuf,sizeof(_procBuf));
}

int _x = 0;
int _dx = 0;
int _y = 0;
int _dy = 0;

#define abs(_x) (_x) < 0 ? -(_x) : (_x)

int Accelerate(int n, int dir)
{
	int ad = abs(dir);
	if (ad < 100)
	{
		if (n < 0)
			n++;
		else if (n > 0)
			n--;
	}
	else
	{
		if (dir < 0)
		{
			if (n > -256)
				n--;
		}
		else
		{
			if (n < 256)
				n++;
		}
	}
	return n;
}

int TileLoop(int n)
{
	int px,py;
	ReadXY(&px,&py);

	_dx = Accelerate(_dx,512-px);
	_dy = Accelerate(_dy,py-512);
	_x += (_dx>>3);
	_y += (_dy>>3);
	px = _x;
	py = _y;

	//printf("%dx%d\n",px,py);

	u8 cells[32*3];
	int cellx = px>>3;
	int lastCellLine = -1;
	int cellWidth = (HREZ+16)>>3;
	const u8* base = tiles8x8;
	VideoOutWaitVBL();
	for (int y = 0; y < 240; y++)
	{
		int cellLine = (py+y)>>3;
		if (cellLine != lastCellLine)
		{
			GetCellLine(cellx,(py+y)>>3,cells,cellWidth + (cellx & 31));	// Get line of cells
			base = tiles8x8;
			if (lastCellLine == -1)
				base += (py & 7) << 3;
			lastCellLine = cellLine;
		}

		u8* dst = LineBufferGet();
		_renderCells(dst,cells + (cellx & 31),base,cellWidth);
		LineBufferEnqueue(dst+(px&7));
		base += 8;
	}
	_counter++;
	return 0;
}
#endif
