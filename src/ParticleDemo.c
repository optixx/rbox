
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

#define FIXBITS 4
#ifdef PARTICLE_DEMO

typedef signed char s8;
typedef struct
{
	short y;
	short x;
	s8	dy;
	s8	dx;
	u8	age;
	u8	color;
} Particle;

int _counter = 0;
FillLineProc _fillLine;
u8 _fillLineBuffer[80];

#define MAX_PARTICLES 500
Particle _particles[MAX_PARTICLES];
short _particleIndex[MAX_PARTICLES];

void CopyPatternM(u8* dst, const u8* src, int dlum, int color)
{
	u32* s = (u32*)src;
	int c;
	int i = 6;
	int mask = 0xF8;
	while (i--)
	{
		u32 p = *s++;
		c = (u8)p;
		p >>= 8;
		c = (c*dlum) >> 8;
		*dst++ = (c & mask) + color;
		c = (u8)p;
		p >>= 8;
		c = (c*dlum) >> 8;
		*dst++ = (c & mask) + color;
		c = (u8)p;
		p >>= 8;
		c = (c*dlum) >> 8;
		*dst++ = (c & mask) + color;
		c = (u8)p;
		p >>= 8;
		c = (c*dlum) >> 8;
		*dst++ = (c & mask) + color;
	}
}

int _lfsr = 0x8BADF00D;
u32 RAND()
{
	int t;
	int i = 4;
	while (i--)
	{
		t = _lfsr << 1;
		if (t <= 0)
			t ^= 0x20AA95B5;
		_lfsr = t;
	}
	return t;
}

const u8 ramp[]  = 
{
	0x00,0x1E,0x3C,0x53,0x65,0x73,0x7F,0x89,0x91,0x99,0x9F,0xA4,0xA9,0xAE,0xB1,0xB5,
	0xB8,0xBB,0xBD,0xC0,0xC2,0xC4,0xC6,0xC8,0xC9,0xCB,0xCC,0xCD,0xCE,0xD0,0xD1,0xD2,
	0xD2,0xD3,0xD4,0xD5,0xD6,0xD6,0xD7,0xD8,0xD8,0xD9,0xD9,0xDA,0xDA,0xDB,0xDB,0xDC,
	0xDC,0xDC,0xDD,0xDD,0xDD,0xDE,0xDE,0xDE,0xDF,0xDF,0xDF,0xDF,0xE0,0xE0,0xE0,0xE0,
};
const u8 lumpsDark[] = { // 24x24
0x9E,0x98,0x8F,0x86,0x7B,0x6F,0x61,0x52,0x41,0x2C,0x14,0x00,0x00,0x14,0x2C,0x41,0x52,0x61,0x6F,0x7B,0x86,0x8F,0x98,0x9E,
0x98,0x93,0x8C,0x83,0x79,0x6D,0x5F,0x50,0x3E,0x29,0x17,0x14,0x14,0x17,0x29,0x3E,0x50,0x5F,0x6D,0x79,0x83,0x8C,0x93,0x98,
0x8F,0x8C,0x86,0x7D,0x74,0x68,0x5A,0x4B,0x3A,0x29,0x29,0x2C,0x2C,0x29,0x29,0x3A,0x4B,0x5A,0x68,0x74,0x7E,0x86,0x8C,0x8F,
0x86,0x83,0x7D,0x76,0x6D,0x61,0x54,0x44,0x38,0x3A,0x3E,0x41,0x41,0x3E,0x3A,0x38,0x44,0x54,0x61,0x6D,0x76,0x7D,0x83,0x86,
0x7B,0x79,0x74,0x6D,0x63,0x58,0x4B,0x41,0x44,0x4B,0x50,0x52,0x52,0x50,0x4B,0x44,0x41,0x4B,0x58,0x63,0x6D,0x74,0x79,0x7B,
0x6F,0x6D,0x68,0x61,0x58,0x4D,0x44,0x4B,0x54,0x5A,0x5F,0x61,0x61,0x5F,0x5A,0x53,0x4B,0x44,0x4D,0x58,0x61,0x68,0x6D,0x6F,
0x61,0x5F,0x5A,0x54,0x4B,0x44,0x4D,0x59,0x61,0x68,0x6D,0x6F,0x6F,0x6D,0x68,0x61,0x59,0x4D,0x44,0x4B,0x54,0x5A,0x5F,0x61,
0x51,0x50,0x4B,0x44,0x41,0x4B,0x58,0x63,0x6D,0x74,0x79,0x7B,0x7B,0x79,0x74,0x6D,0x63,0x59,0x4B,0x41,0x44,0x4B,0x50,0x52,
0x41,0x3E,0x3A,0x38,0x44,0x53,0x61,0x6D,0x76,0x7D,0x83,0x86,0x86,0x83,0x7D,0x76,0x6D,0x61,0x54,0x44,0x38,0x3A,0x3E,0x41,
0x2C,0x29,0x29,0x3A,0x4B,0x5A,0x68,0x74,0x7D,0x86,0x8C,0x8F,0x8F,0x8C,0x86,0x7D,0x74,0x68,0x5A,0x4B,0x3A,0x29,0x29,0x2C,
0x14,0x17,0x29,0x3E,0x50,0x5F,0x6C,0x79,0x83,0x8C,0x93,0x98,0x98,0x93,0x8C,0x83,0x79,0x6D,0x5F,0x50,0x3E,0x29,0x17,0x14,
0x00,0x14,0x2C,0x41,0x52,0x61,0x6F,0x7B,0x86,0x8F,0x98,0x9E,0x9E,0x98,0x8F,0x86,0x7B,0x6F,0x61,0x52,0x41,0x2C,0x14,0x00,
0x00,0x14,0x2C,0x41,0x51,0x61,0x6F,0x7B,0x86,0x8F,0x98,0x9E,0x9E,0x98,0x8F,0x86,0x7B,0x6F,0x61,0x52,0x41,0x2C,0x14,0x00,
0x14,0x17,0x29,0x3E,0x50,0x5F,0x6D,0x79,0x83,0x8C,0x93,0x98,0x98,0x93,0x8C,0x83,0x79,0x6D,0x5F,0x50,0x3E,0x29,0x17,0x14,
0x2C,0x29,0x29,0x3A,0x4B,0x5A,0x68,0x74,0x7D,0x86,0x8C,0x8F,0x8F,0x8C,0x86,0x7D,0x74,0x68,0x5A,0x4B,0x3A,0x2A,0x29,0x2C,
0x41,0x3E,0x3A,0x38,0x44,0x53,0x61,0x6D,0x76,0x7E,0x83,0x86,0x86,0x83,0x7D,0x76,0x6D,0x61,0x53,0x44,0x38,0x3A,0x3E,0x41,
0x52,0x50,0x4B,0x44,0x41,0x4B,0x58,0x63,0x6C,0x74,0x79,0x7B,0x7B,0x79,0x74,0x6C,0x63,0x58,0x4B,0x41,0x44,0x4B,0x50,0x52,
0x61,0x5F,0x5A,0x54,0x4B,0x44,0x4D,0x58,0x61,0x68,0x6D,0x6F,0x6F,0x6D,0x68,0x61,0x59,0x4D,0x44,0x4B,0x53,0x5A,0x5F,0x61,
0x6F,0x6D,0x68,0x61,0x58,0x4D,0x44,0x4B,0x53,0x5A,0x5F,0x61,0x61,0x5F,0x5A,0x54,0x4B,0x44,0x4D,0x59,0x61,0x68,0x6D,0x6F,
0x7B,0x79,0x74,0x6C,0x63,0x58,0x4B,0x41,0x44,0x4B,0x50,0x52,0x52,0x50,0x4B,0x44,0x41,0x4B,0x58,0x63,0x6C,0x74,0x79,0x7B,
0x86,0x83,0x7D,0x76,0x6D,0x61,0x53,0x44,0x38,0x3A,0x3E,0x41,0x41,0x3E,0x3A,0x38,0x44,0x53,0x61,0x6D,0x76,0x7D,0x83,0x86,
0x8F,0x8C,0x86,0x7D,0x74,0x68,0x5A,0x4B,0x3A,0x29,0x29,0x2C,0x2C,0x29,0x29,0x3A,0x4B,0x5A,0x68,0x74,0x7E,0x86,0x8C,0x8F,
0x98,0x93,0x8C,0x83,0x79,0x6C,0x5F,0x50,0x3E,0x29,0x17,0x14,0x14,0x17,0x29,0x3E,0x50,0x5F,0x6D,0x79,0x83,0x8C,0x93,0x98,
0x9E,0x98,0x8F,0x86,0x7B,0x6F,0x61,0x52,0x41,0x2C,0x14,0x00,0x00,0x14,0x2C,0x41,0x52,0x62,0x6F,0x7B,0x86,0x8F,0x98,0x9E
};


short SIN(u8 angle);
short COS(byte angle);	//+-256


void quicksortp(short arr[], int left, int right)
{
    int i = left, j = right;
    Particle* p = _particles;
    int pivot = p[arr[(left + right) >> 1]].y;

    while (i <= j)
	{
        while (p[arr[i]].y < pivot)
            i++;
        while (p[arr[j]].y > pivot)
            j--;
        if (i <= j)
        {
			short tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            i++;
            j--;
        }
    }

    if (left < j)
        quicksortp(arr, left, j);
    if (i < right)
        quicksortp(arr, i, right);
}


void Spawn(Particle* p)
{
	int x,y;
	int n = _counter;
	x = SIN(n)>>2;
	y = COS(n)>>2;
	x += SIN(n<<3)>>5;
	y += COS(n<<3)>>5;
	if (_counter & 1)
	{
		x = -x;
		y = -y;
	}

	p->x = (128+x)<<FIXBITS;
	p->y = (100+y)<<FIXBITS;
	p->dx = RAND();
	p->dy = RAND();
	p->dx >>= 3;
	p->dy >>= 3;
	p->age = 255;
}

void StepParticles()
{
	//int ax = 0;
	int ay = 1;	// gravity
	int maxSpawn = 32;
	Particle* p = _particles;
	Particle* end = p + MAX_PARTICLES;
	do
	{
		if (p->age == 0)
		{
			if (maxSpawn-- > 0)
				Spawn(p);
			continue;
		}
		p->age--;
		p->x += p->dx;
		if (p->x < 0 || p->x > (280<<FIXBITS))
		{
			if (maxSpawn-- > 0)
				Spawn(p);
			continue;
		}

		p->y += p->dy;
		if (p->y < 0 || p->y > (200<<FIXBITS))
		{
			if (maxSpawn-- > 0)
				Spawn(p);
			continue;
		}
		p->dy += ay;	// accelerate

	} while (++p < end);
	quicksortp(_particleIndex,0,MAX_PARTICLES-1);	// Worst case of qsort
}

void InitParticles()
{
	for (int i = 0; i < MAX_PARTICLES; i++)
		_particles[i].age = 0;
	for (int i = 0; i < MAX_PARTICLES; i++)
		_particleIndex[i] = i;
}

void ParticleInit()
{
	VideoOutInit(38*8);	//
	LineBufferInit();
	InitParticles();
	_fillLine = (FillLineProc)CopyProcToRam((void*)FillLine24,_fillLineBuffer,sizeof(_fillLineBuffer));
}

const u8 _patternBlack[24*2] =
{
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};

void Wu2(u8* dst, int x, int c)
{
	int px = x >> 4;
	if (px > 0 && px < 260)
	{
		int x0 = x & 0xF;	//
		int x1 = 15-x0;
		dst[px] = ((c*x0)>>11)<<3;
		dst[px+1] = ((c*x1)>>11)<<3;
	}
}

#define MAX_LINES 190
int _vcounter = 0;
int ParticleLoop(int n)
{
	if (sizeof(Particle) != 8)
		return 0;

	_counter++;
	int v = _counter % 24;
	if (v < 0)
		v += 24;
	v *= 24;

	int line = VideoOutLine();
	StepParticles();

	while (VideoOutLine() > line)	// Still in last frame, should never happen
		;
	while (VideoOutLine() < 42)		// Wait till first line
		;

	int pi = 0;
	Particle* p = _particles + _particleIndex[0];
	for (int y = 0; y < MAX_LINES; y++)
	{
		u8* dst = LineBufferGet();
		u32 pl[6];
		int n = y-(MAX_LINES/2);	// -100..100
		n = 192 + n;
		n = (256 + SIN(n)) >> 1;	// 0..255
		CopyPatternM((u8*)pl,lumpsDark + v,n,((y+_counter)>>8) & 0x07);
		_fillLine(dst,(u8*)pl,10);
		v += 24;
		if (v == 24*24)
			v = 0;

		int py = y << FIXBITS;
		while (p && p->y < py)
		{
			dst[(p->x >> FIXBITS) & 0xFF] = 0xF8;
			if (++pi == MAX_PARTICLES)
				p = 0;
			else
				p = _particles + _particleIndex[pi];
		}

		LineBufferEnqueue(dst);
	}
	return 0;
}

#endif
