
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
#include "string.h"

//=============================================================================
//=============================================================================
//	Linebuffer helpers

#ifdef D3D_DEMO
#define LINE_BUFFER_COUNT 4
#else
#define LINE_BUFFER_COUNT 4
#endif

#ifndef LINE_BUFFER_WIDTH
#define LINE_BUFFER_WIDTH 320
#endif

typedef struct
{
	int LineIndex;
	volatile u8* volatile Lines[LINE_BUFFER_COUNT];	// bitmap would do here
	u8	Buffer[LINE_BUFFER_COUNT*LINE_BUFFER_WIDTH];
} LineBuffer;

LineBuffer gLineBuffer;

void LineBufferInit()
{
	gLineBuffer.LineIndex = 0;
	for (int i = 0; i < LINE_BUFFER_COUNT; i++)
		gLineBuffer.Lines[i] = 0;
}

bool LineBufferBusy()
{
	return gLineBuffer.Lines[gLineBuffer.LineIndex & (LINE_BUFFER_COUNT-1)] != 0;
}

u8*	LineBufferGet()
{
	int i = gLineBuffer.LineIndex & (LINE_BUFFER_COUNT-1);
	volatile u8* volatile* line = gLineBuffer.Lines + i;
	while (*line)
		__WFI();
	u8* dst = gLineBuffer.Buffer + i*LINE_BUFFER_WIDTH;
	gLineBuffer.Lines[i] = dst;
	return dst;
}

void LineBufferEnqueue(u8* dst)
{
	int i = gLineBuffer.LineIndex & (LINE_BUFFER_COUNT-1);
	gLineBuffer.Lines[i] = dst;
	VideoOutEnqueueLine((u8**)&gLineBuffer.Lines[i]);
	gLineBuffer.LineIndex++;
}

//=============================================================================
//=============================================================================
//	ASM blits

extern volatile int _vbl;

#ifndef _WIN32

extern "C"
{
void __aeabi_unwind_cpp_pr0(){};
void __aeabi_unwind_cpp_pr1(){};
}


extern "C"
void CopyPattern(u8* dst, const u8* src, int dlum, int color)
{
	asm volatile (
		".syntax unified\n"
		"	mov 	r0,%[dst]\n"
		"	mov 	r1,%[src]\n"
		//"	push	{r6,r7}\n"		// Save

		"	mov 	r6,%[dlum]\n"
		"	mov 	r7,%[color]\n"
		"	movs	r5,#24\n"
		"	adds	r5,r5,r1\n"

		"2:	ldrb	r3,[r1,#0]\n"
		"	add		r3,r6\n"
		"	cmp		r3,#0\n"
		"	bge		0f\n"
		"	movs	r3,#0\n"
		"0:	cmp		r3,#255\n"
		"	ble		1f\n"
		"	movs	r3,#255\n"
		"1:	movs	r4,#0xF8\n"
		"	ands	r4,r4,r3\n"
		"	adds	r4,r4,r7\n"

		"	strb	r4,[r0,#0]\n"
		"	adds	r0,r0,#1\n"
		"	adds	r1,r1,#1\n"
		"	cmp		r1,r5\n"
		"	bne		2b\n"

		//"9:	pop		{r6,r7}\n"
		".syntax divided\n"
		:
		: [dst] "r" (dst),[src] "r" (src),[dlum] "r" (dlum),[color] "r" (color)
		: "r0","r1","r2","r3","r4","r5","r6","r7"
	);

};

extern "C"
void DrawSpriteLines(u8* dst,void* lines,int count)
{
	asm volatile (
		".syntax unified\n"
		"	mov 	r0,%[dst]\n"
		"	mov 	r1,%[lines]\n"
		"	mov 	r2,%[count]\n"
		"	push	{r6,r7}\n"		// Save

		"	lsls	r7,r2,#3\n"		// 8 bytes per
		"	adds	r7,r7,r1\n"		// end ptr in r7
		"	movs	r6,r0\n"		// dst in r6

		"10:\n"						// loop
		"	ldm		r1!,{r0,r2}\n"	// HPOS, DATA
		"	adds	r0,r0,r6\n"		// dst in r0, data in r2
		"	b		0f\n"

		"4:	subs	r0,r0,r3\n"		// skip

		"0:	ldrb	r3,[r2,#0]\n"
		"1:	adds	r2,r2,#1\n"
		"	SXTB	r3,r3\n"
		"	cmp		r3,#0\n"
		"	blt		4b\n"			// -ve skip

		"	beq		8f\n"			// 0 = End of line

		"6:	movs	r5,#8\n"		// +ve Pixel run count
		"	cmp		r3,#8\n"
		"	bge		5f\n"			// 8 at once

		"	movs	r4,#8\n"
		"	mov		r5,r3\n"
		"	subs	r4,r4,r3\n"
		"	lsls	r4,r4,#2\n"
		"	add		PC,PC,r4\n"
		"	nop\n"

		"5:	ldrb	r4,[r2,#7]\n"	// Copy pixels
		"	strb	r4,[r0,#7]\n"
		"	ldrb	r4,[r2,#6]\n"
		"	strb	r4,[r0,#6]\n"
		"	ldrb	r4,[r2,#5]\n"
		"	strb	r4,[r0,#5]\n"
		"	ldrb	r4,[r2,#4]\n"
		"	strb	r4,[r0,#4]\n"
		"	ldrb	r4,[r2,#3]\n"
		"	strb	r4,[r0,#3]\n"
		"	ldrb	r4,[r2,#2]\n"
		"	strb	r4,[r0,#2]\n"
		"	ldrb	r4,[r2,#1]\n"
		"	strb	r4,[r0,#1]\n"
		"	ldrb	r4,[r2,#0]\n"
		"	strb	r4,[r0,#0]\n"

		"	adds	r0,r0,r5\n"
		"	adds	r2,r2,r5\n"
		"	subs	r3,r3,r5\n"
		"	bne		6b\n"
		"	b		0b\n"

		"8:	cmp		r1,r7\n"		// end of line, do another
		"	bne		10b\n"

		"9:	pop		{r6,r7}\n"
		".syntax divided\n"
		:
		: [dst] "r" (dst),[lines] "r" (lines),[count] "r" (count)
		: "r0","r1","r2","r3","r4","r5"
	);
}
extern "C"
void FillLine(u8* dst, u8* pattern, int count)
{
	asm volatile (
		".syntax unified\n"
		"	mov 	r0,%[dst]\n"
		"	mov 	r1,%[pattern]\n"

		"	mov		r4,r8\n"		// save r8
		"	mov		r5,r9\n"		// save r9
		"	push	{r4,r5}\n"

		"	ldm		r1!,{r4,r5,r6,r3}\n"	// 16(24) pixels per write

		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"

		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
#if 1
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
		"	stm		r0!,{r4,r5,r6,r3}\n"
#endif

		"	pop		{r3,r4}\n"
		"	mov		r8,r3\n"
		"	mov		r9,r4\n"
			".syntax divided\n"
			:
			: [dst] "r" (dst),[pattern] "r" (pattern)
			: "r0","r1","r2","r3","r4","r5","r6"
		);
}


extern "C"
void FillLine24(u8* dst, u8* pattern, int count)
{
	asm volatile (
		".syntax unified\n"
		"	mov 	r0,%[dst]\n"
		"	mov 	r1,%[pattern]\n"

		"	push	{r7}\n"

		"	ldm		r1!,{r2,r3,r4,r5,r6,r7}\n"	// 24 pixels per write

		"	stm		r0!,{r2,r3,r4,r5,r6,r7}\n"	// 288 pix
		"	stm		r0!,{r2,r3,r4,r5,r6,r7}\n"
		"	stm		r0!,{r2,r3,r4,r5,r6,r7}\n"
		"	stm		r0!,{r2,r3,r4,r5,r6,r7}\n"

		"	stm		r0!,{r2,r3,r4,r5,r6,r7}\n"
		"	stm		r0!,{r2,r3,r4,r5,r6,r7}\n"
		"	stm		r0!,{r2,r3,r4,r5,r6,r7}\n"
		"	stm		r0!,{r2,r3,r4,r5,r6,r7}\n"

		"	stm		r0!,{r2,r3,r4,r5,r6,r7}\n"
		"	stm		r0!,{r2,r3,r4,r5,r6,r7}\n"
		"	stm		r0!,{r2,r3,r4,r5,r6,r7}\n"
		"	stm		r0!,{r2,r3,r4,r5,r6,r7}\n"

		"	pop		{r7}\n"
			".syntax divided\n"
			:
			: [dst] "r" (dst),[pattern] "r" (pattern)
			: "r0","r1","r2","r3","r4","r5","r6"
		);
}

extern "C"
void FillRuns(u8* dst, u8* runs, int count)
{
	asm volatile (
		".syntax unified\n"
		"	mov		r3,r8\n"
		"	push	{r3,r4,r5,r6,r7}\n"
		"	mov 	r0,%[dst]\n"
		"	mov 	r7,%[runs]\n"
		"	mov		r8,%[count]\n"
		"	add		r8,r8,r8\n"
		"	add		r8,r7\n"		// 2 bytes per run

		"0:\n"
		"	ldrb	r1,[r7,#1]\n"	// color
		"	ldrb	r3,[r7,#0]\n"	// run
		"	adds	r2,r0,r3\n"		// end
		"	cmp		r3,#4\n"
		"	blt		9f\n"			// short blit

		//	align to word boundary
		"1:\n"
		"	movs	r3,#3\n"
		"	ands	r3,r3,r0\n"
		"	beq		5f\n"
		"	movs	r5,r0\n"
		"	adds	r0,r0,#4\n"
		"	subs	r0,r0,r3\n"
		"	lsls	r3,r3,#1\n"
		"	add		PC,PC,r3\n"
		"	nop\n"
		"	nop\n"
		"	strb	r1,[r5,#2]\n"
		"	strb	r1,[r5,#1]\n"
		"	strb	r1,[r5,#0]\n"

		//	16 at a time
		"5:\n"
		"	lsls	r4,r1,#8\n"	// make color 4 wide
		"	adds	r1,r4,r1\n"
		"	lsls	r4,r1,#16\n"
		"	adds	r1,r1,r4\n"

		"	subs	r4,r2,r0\n"
		"	lsrs	r4,r4,#4\n"	// # of 16 pixel blits
		"	beq		4f\n"
		"	movs	r3,#20\n"
		"	subs	r3,r3,r4\n"
		"	movs	r4,r1\n"
		"	movs	r5,r1\n"
		"	movs	r6,r1\n"
		"	lsls	r3,r3,#1\n"
		"	add		PC,PC,r3\n"
		"	nop\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"

			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"
			"	stm		r0!,{r1,r4,r5,r6}\n"

		//	todo 0..15 as 4 up?
		"4:\n"
		"	subs	r3,r2,r0\n"
		"	lsrs	r3,r3,#2\n"
		"	beq		9f\n"

		"	movs	r4,#4\n"
		"	subs	r4,r4,r3\n"
		"	lsls	r4,r4,#1\n"
			"	add		PC,PC,r4\n"
			"	nop\n"
			"	stm		r0!,{r1}\n"
			"	stm		r0!,{r1}\n"
			"	stm		r0!,{r1}\n"
			"	stm		r0!,{r1}\n"

		//	short loop
		"9:\n"
		"	subs	r3,r2,r0\n"
		"	movs	r4,#16\n"
		"	subs	r4,r4,r3\n"
		"	lsls	r4,r4,#1\n"
			"	add		PC,PC,r4\n"
			"	nop\n"
			"	strb	r1,[r0,#15]\n"
			"	strb	r1,[r0,#14]\n"
			"	strb	r1,[r0,#13]\n"
			"	strb	r1,[r0,#12]\n"
			"	strb	r1,[r0,#11]\n"
			"	strb	r1,[r0,#10]\n"
			"	strb	r1,[r0,#9]\n"
			"	strb	r1,[r0,#8]\n"
			"	strb	r1,[r0,#7]\n"
			"	strb	r1,[r0,#6]\n"
			"	strb	r1,[r0,#5]\n"
			"	strb	r1,[r0,#4]\n"
			"	strb	r1,[r0,#3]\n"
			"	strb	r1,[r0,#2]\n"
			"	strb	r1,[r0,#1]\n"
			"	strb	r1,[r0,#0]\n"

			"	movs	r0,r2\n"
			"	adds	r7,#2\n"
			"	cmp		r7,r8\n"
			"	bne		0b\n"

		"99:"
			"	pop	{r3,r4,r5,r6,r7}\n"
			"	mov	r8,r3\n"
		".syntax divided\n"
		:
		: [dst] "r" (dst),[runs] "r" (runs),[count] "r" (count)
		: "r0","r1","r2","r3"
	);
}

void RenderCells(u8* dst, const u8* cells, const u8* base, int count)
{
	asm volatile (
		".syntax unified\n"
		"	mov 	r0,%[dst]\n"
		"	mov 	r1,%[cells]\n"
		"	mov 	r2,%[base]\n"
		"	mov		r6,%[count]\n"

		"	mov		r3,r8\n"		// save r8
		"	mov		r4,r9\n"		// save r9
		"	mov		r5,r7\n"		// save r7
		"	push	{r3,r4,r5}\n"
		"	mov		r8,r2\n"		// base in r8
		"	adds	r7,r6,r1\n"		// End ptr

		"0:\n"

			"	ldrb	r2,[r1,#0]\n"	//b0
			"	lsls	r2,r2,#6\n"		// *64
			"	add		r2,r2,r8\n"		// base
			"	ldm		r2!,{r3,r4}\n"	//

			"	ldrb	r2,[r1,#1]\n"	//b1
			"	lsls	r2,r2,#6\n"		// *64
			"	add		r2,r2,r8\n"		// base
			"	ldm		r2!,{r5,r6}\n"	//
			"	stm		r0!,{r3,r4,r5,r6}\n"	//

			"	ldrb	r2,[r1,#2]\n"	//b2
			"	lsls	r2,r2,#6\n"		// *64
			"	add		r2,r2,r8\n"		// base
			"	ldm		r2!,{r3,r4}\n"	//

			"	ldrb	r2,[r1,#3]\n"	//b3
			"	lsls	r2,r2,#6\n"		// *64
			"	add		r2,r2,r8\n"		// base
			"	ldm		r2!,{r5,r6}\n"	//
			"	stm		r0!,{r3,r4,r5,r6}\n"	//

			"	adds	r1,r1,#4\n"
			"	cmp		r1,r7\n"
			"	blt		0b\n"

		"	pop		{r3,r4,r5}\n"
		"	mov		r7,r5\n"
		"	mov		r8,r3\n"
		"	mov		r9,r4\n"
			".syntax divided\n"
			:
			: [dst] "r" (dst),[cells] "r" (cells),[base] "r" (base),[count] "r" (count)
			: "r0","r1","r2","r3","r4","r5","r6"
		);
}


extern "C"
void memcpy(void* dst, void* src, int len);
void* CopyProcToRam(void* proc, u8* buffer, int len)
{
	int addr = (int)proc & 0xFFFFFFFE;
	memcpy(buffer,(void*)addr,len);
	return (void*)(((int)buffer) + 1);
}

#else

void* CopyProcToRam(void* proc, u8* buffer, int len)
{
	return proc;
}

#endif

extern "C" void JamPalette(int c);

#define STATE LPC_GPIO3->MASKED_ACCESS[1<<1]

#if 0
void RenderOCOS(int count)
{
	VideoOutWaitVBL();

	const u8* d = _renderme;
	u8 pattern[16];
	for (int i = 0; i < 16; i++)
		pattern[i] = 0x80;
	for (int i = 0; i < 240; i++)
	{
		if (d[0] == 0)
			break;
		u8* dst = gLineBuffer.GetBuffer();
		FillLine(dst,pattern);
		int h = *d++;
		u8 runs[32];
		u8* b = runs;
		//*b++ = h;
		//*b++ = 0;
		u8 color;
		do
		{
			*b++ = *d++;
			color = *d++;
			*b++ = ((color <<  5) | color);
		} while (!(color & 0x80));
		*b = 0;
		_fillAsm(dst+h,runs);

		//_fillAsm(dst+h,240-h,0);
		gLineBuffer.CommitBuffer(dst);
	}
}
#endif

#if TRIANGLES
u8 appBuffer[1024];
int _lasty = -1;
int _lastx = 0;
int _m = 0;
u8 _runs[2048];

void DrawBG(int y0, int y1)
{
}

void Fill(int x0, int x1, int y, int color)
{
	if (x1 <= x0)
		return;
	if (_lasty != y)
	{
		if (_m)
			_runs[_m++] = 0;	// finish old y
	}
	_runs[_m++] = x1-x0;
	_runs[_m++] = (color << 4) | color;
	_lasty = y;
}


void DrawAll()
{
	VideoOutWaitVBL();
	int m = 0;
	u8 pattern[16];
	pattern[0] = 0xF0;
	pattern[1] = 0xE0;
	pattern[2] = 0xD0;
	pattern[3] = 0xC0;
	pattern[4] = 0xB0;
	pattern[5] = 0xA0;
	pattern[6] = 0xA0;
	pattern[7] = 0xA0;
	pattern[8] = 0xA0;
	pattern[9] = 0xA0;
	pattern[10] = 0xB0;
	pattern[11] = 0xC0;
	pattern[12] = 0xD0;
	pattern[13] = 0xE0;
	pattern[14] = 0xF0;
	pattern[15] = 0xF0;
	for (int y = 0; y < 160; y++)
	{
		u8* dst = gLineBuffer.GetBuffer();
		FillLine(dst,pattern);
		if (m < _m)
		{
			int h = _runs[m];
			if (h > 0 && h < 160)
				_fillAsm(dst+h,_runs+m+2);
			while (_runs[m])
				m += 2;
			m++;
		}
		gLineBuffer.CommitBuffer(dst);
	}
}

void DrawTriangles(int keys, byte* appBuffer, int len);
void test3D()
{
	if (_fillAsm == 0)
		_fillAsm = (FillAsmProc)BufferProc((void*)&FillAsm,_procBuf,sizeof(_procBuf));
	DrawTriangles(0x80,appBuffer,sizeof(appBuffer));
	for (;;)
	{
		_m = 0;
		DrawTriangles(0,appBuffer,sizeof(appBuffer));
		_runs[_m++] = 0;
		_runs[_m++] = 0;
		_runs[_m++] = 0;
		DrawAll();
	}
}
#endif

extern "C"
void CheckSleep();

extern "C"
void CPPShell()
{
	AppInit();
	for (;;)
	{
		CheckSleep();
		AppLoop(0);
	}
}
