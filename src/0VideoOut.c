
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

//	This file is named 0VideoOut.c so it will link first and not move about.
//	Editing code in interrupt handlers or building with different optimizations (or with different compilers)
//	or by moving the code in flash will shift chroma phases around.

#include "LPC11xx.h"                        /* LPC11xx definitions */

typedef unsigned char u8;
typedef unsigned long u32;
typedef unsigned char byte;

#define SYNC &LPC_GPIO1->MASKED_ACCESS[1<<1]
#define SYNC1 2

#define STATE LPC_GPIO3->MASKED_ACCESS[7]

typedef void (*blit_ptr)(volatile uint32_t* lum,volatile uint32_t* spi, volatile uint8_t* pixels, volatile uint8_t* pal, int count);

#define LINE_QUEUE_SIZE 8

typedef struct
{
	const u8* Track;
	const u8* TrackMark;
	int WaveMark;
	int WaveStep;
	int	Duration16th;
	int Duration;
} MusicState;

typedef struct
{
	blit_ptr blit;
	volatile int Line;
	volatile int Width;
	volatile int Pulse;
	volatile int Write;
	volatile int Read;
	volatile u8** LineQueue[LINE_QUEUE_SIZE];

	volatile uint8_t pal[256];		// might be able to shrink this with a mask
	u8	blitprocbuf[256];			// Copy of blit code: execute from memory

	MusicState	Music;
} ScreenState;
ScreenState gScreen;

volatile int _vbl = 0;			// vbl counter
const short _pulses[] = {
		2756,256,2756,0,		// equalization
		128,2872,128,0			// post/pre
};

/*
	“Front Porch”(1.4us)
	Horizontal Sync Pulse (4.7us)
	“Back Porch” (5.9us)
C	Visible scan region (51.5us)

Total scan time for one line is 63.5us
*/


//===================================================================================================
//===================================================================================================
//	Timing Critical region 1
//	3648 cpu clocks per scanline
//	Change anything in this region and you will need to fix chroma alignment

void blit_proc(volatile uint32_t* lum,volatile uint32_t* spi, volatile uint8_t* pixels, volatile uint8_t* pal, int count)
{
	asm volatile (
		".syntax unified\n"
		"	mov 	r0,%[lum]\n"
		"	mov 	r1,%[spi]\n"
		"	mov 	r2,%[pal]\n"
		"	movs	r4,#0xAA\n"
		"	str		r4,[r1,#0]\n"	// start chroma pipe - now we are on the clock

		"	mov 	r6,%[pixels]\n"

		"	mov		r5,r7\n"		// save r7
		"	mov		r3,r8\n"		// save r8
		"	mov		r4,r9\n"		// save r9
		"	push	{r3,r4,r5}\n"

		"	mov		r7,r6\n"		// pixels

		"	mov		r6,%[count]\n"	//
		"	lsrs	r6,r6,#3\n"		// 8 per
		"	lsls	r6,r6,#3\n"		// 8 per
		"	adds	r6,r6,r7\n"
		"	mov		r8,r6\n"

		"	ldrb	r3,[r7,#0]\n"	// y0
		"	ldrb	r5,[r2,r3]\n"	// c0
		"	ldrb	r4,[r7,#2]\n"	// y2
		"	ldrb	r6,[r2,r3]\n"	// c2
		"	lsls	r5,r5,#8\n"
		"	adds	r6,r6,r5\n"		// c2c0
		"	b		2f\n"

		//	from now on we are on the clock
		//"	b	2f\n"				// GO
		"0:\n"
			"	strb	r3,[r0,#0]\n"	//>y5 8

			"	ldrb	r3,[r7,#8]\n"	// y0 next
			"	ldrb	r6,[r2,r3]\n"	// c0 next
			"	lsls	r6,r6,#8\n"
			"	adds	r7,r7,#8\n"
			"	strb	r4,[r0,#0]\n"	//>y6 8

			"	ldrb	r4,[r7,#2]\n"	// y2
			"	mov		r9,r6\n"		// c0 in r9
			"	ldrb	r6,[r2,r4]\n"	// c2
			"	add		r6,r6,r9\n"		//
			"	strb	r5,[r0,#0]\n"	//>y7	8

		"2:\n"	// Enter here
			"	nop\n"
			"	str		r6,[r1,#0]\n"	//>c0c2 5
			"	strb	r3,[r0,#0]\n"	//>y0	8

			"	ldrb	r3,[r7,#4]\n"	// y4
			"	ldrb	r5,[r2,r3]\n"	// c4
			"	ldrb	r6,[r7,#1]\n"	// y1
			"	strb	r6,[r0,#0]\n"	//>y1	8

			"	lsls	r5,r5,#8\n"		// c4
			"	mov		r9,r5\n"
			"	ldrb	r5,[r7,#3]\n"	// y3
			"	ldrb	r6,[r2,r3]\n"	// c6
			"	strb	r4,[r0,#0]\n"	//>y2	8

			"	add		r6,r6,r9\n"		// c4c6
			"	str		r6,[r1,#0]\n"	//>c4c6
			"	strb	r5,[r0,#0]\n"	//>y3	8

			//	4-8
			"	ldrb	r6,[r7,#5]\n"	// y5
			"	ldrb	r4,[r7,#6]\n"	// y6
			"	ldrb	r5,[r7,#7]\n"	// y7
			"	strb	r3,[r0,#0]\n"	//>y4	8

			"	movs	r3,r6\n"		// y5
			"	mov		r6,r8\n"
			"	cmp		r6,r7\n"
			"	bne		0b\n"
		"1:\n"
			"	pop		{r3,r4,r5}\n"
			"	mov		r7,r5\n"
			"	mov		r8,r3\n"
			"	mov		r9,r4\n"
				".syntax divided\n"
				:
				: [spi] "r" (spi),[pal] "r" (pal),[pixels] "r" (pixels),[lum] "r" (lum),[count] "r" (count)
				: "r0","r1","r2","r3","r4","r5","r6"
			);
	*lum = 0x0;
}

int blit_proc_len()
{
	uint8_t* a = (uint8_t*)blit_proc;
	uint8_t* b = (uint8_t*)blit_proc_len;
	return b-a;
}

void ComeBackIn(int count)
{
	LPC_TMR16B0->TC = 0;
	LPC_TMR16B0->MR0 = count>>1;
	LPC_TMR16B0->TCR = 1;
}

void ActiveVideo()
{
	ScreenState* s = &gScreen;
	if (*SYNC == 0)
	{
		*SYNC = SYNC1;	// End of sync, send colorburst
		volatile uint32_t* spi = &LPC_SSP0->DR;
		*spi = 0x0000;	// breezeway
		*spi = 0x0000;
		*spi = 0xF0F0;	// colorburst 10 cycles into fifo
		*spi = 0xF0F0;
		*spi = 0xF0F0;
		*spi = 0xF0F0;
		*spi = 0xF0F0;

		int b = (360-s->Width)>>3;	// center active video
		b = 256 + (b << 5);
		ComeBackIn(b-6);			// -6 fudge is to align colorbusrt with first color clock in active video
	}
	else
	{
		if (s->Read == s->Write || s->Line < 22)
			return;
		int mask = LINE_QUEUE_SIZE-1;
		u8** handle = s->LineQueue[s->Read++ & mask];
		s->blit(&LPC_GPIO2->MASKED_ACCESS[0xF8],&LPC_SSP0->DR,*handle,s->pal,s->Width);
		*handle = 0;
	}
}

//	Called at end of HSYNC, at start of active video
void TIMER16_0_IRQHandler()
{
	//STATE |= 2;
	int ir = LPC_TMR16B0->IR;
	LPC_TMR16B0->IR = ir;
	if (gScreen.Pulse < 0)
		ActiveVideo();
	else
	{
		int t = _pulses[gScreen.Pulse++];
		*SYNC = (gScreen.Pulse & 1) ? 0 : SYNC1;
		if (t == 0)
			gScreen.Pulse = -1;
		else
		{
			LPC_TMR16B0->MR0 = t>>1;
			LPC_TMR16B0->TC = 0;
			LPC_TMR16B0->TCR = 1;
		}
	}
	//STATE &= ~2;
}


//===================================================================================================
//===================================================================================================
//	Timing Critical region 2
//	Less critical than active video, should not alter chroma phases

void StartLine(ScreenState* s)
{
	*SYNC = 0;							// Start sync
	s->Line += 1;
	int line = s->Line;
	if (line == 262)
	{
		line = s->Line = 0;
		_vbl++;
	}
	else if (line >= 9)
	{
		ComeBackIn(256);	// HSYNC
		return;
	}
	//	vertical blanking
	s->Pulse = (line >= 3 && line < 6) ? 0 : 4;
	LPC_TMR16B0->TC = 0;
	ComeBackIn(_pulses[s->Pulse++]);
}

u8 MusicSample();	// In Music.h

//	Called at start of HSYNC
void SysTick_Handler(void)
{
	//STATE |= 4;
	StartLine(&gScreen);
#if 1
	int n = MusicSample(&gScreen.Music);
	LPC_TMR16B1->MR0 = (n*57)>>3;	// 8 bit pcm
	LPC_TMR16B1->MR1 = (n*57)>>3;
	LPC_TMR16B1->TC = 0;
#endif
}

//===================================================================================================
//===================================================================================================
//	Non critical timing

#include "Music.h"

int chroma_phase(int b)
{

	//	luma only
	//	green
	//	yellow/green
	//	yellow/brown
	//	red
	//	pink
	//	purple
	//	blue

	uint8_t _map[8] = { 0x00,0xC1,0xE0,0x70,0x38,0x1C,0x0E,0x07 };	// 3 bit default 0x83->0x00 for BW
	return _map[b];
}

int chroma_palette(int b)
{
	return chroma_phase(b&7);
}

void InitTimer()
{
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<7);	// enable TMR16B0
	LPC_TMR16B0->MCR = 7;	//
	NVIC_EnableIRQ(TIMER_16_0_IRQn);
}

/* SSP CR1 register */
#define SSPCR1_LBM		(1 << 0)
#define SSPCR1_SSE		(1 << 1)
#define SSPCR1_MS		(1 << 2)
#define SSPCR1_SOD		(1 << 3)

#define SSP_BUFSIZE		16
#define FIFOSIZE		8

void SSPInit( void )
{
  uint8_t i, Dummy;

  LPC_SYSCON->PRESETCTRL |= (0x1<<0);
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<11);
  LPC_SYSCON->SYSTICKCLKDIV  = 0x02;			/* Divided by 2 */
  LPC_IOCON->PIO0_8 &= ~0x07;	/*  SSP I/O config */
  LPC_IOCON->PIO0_8 |= 0x01;	/* SSP MISO */
  LPC_IOCON->PIO0_9  &= ~0x07;
  LPC_IOCON->PIO0_9 |= 0x01;	/* SSP MOSI */

  LPC_IOCON->SCK_LOC = 0x02;
  LPC_IOCON->PIO0_6 = 0x02;		/* P0.6 function 2 is SSP clock, need to combined with IOCONSCKLOC register setting */

  LPC_IOCON->PIO0_2 &= ~0x07;		/* SSP SSEL is a GPIO pin */

  LPC_SSP0->CR0 = 0x008F;	// 16-bit, 24Mhz CPOL = 0, CPHA = 1

  /* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
  LPC_SSP0->CPSR = 0x2;

  for ( i = 0; i < FIFOSIZE; i++ )
	Dummy = LPC_SSP0->DR;		/* clear the RxFIFO */

  /* Master mode */
  LPC_SSP0->CR1 = SSPCR1_SSE;
}

void InitScreen(ScreenState* s)
{
	int b;
	s->Write = s->Read = 0;
	s->Width = 256;

	for (b = 0; b < 256; b++)
		s->pal[b] = chroma_palette(b);
	SSPInit();

	memcpy(s->blitprocbuf,(void*)((int)&blit_proc & 0xFFFFFFFE),256);
	s->blit = (blit_ptr)(((int)s->blitprocbuf) + 1);

	s->Line = 0;
	s->Pulse = -1;
	s->Music.Duration = 0;
}

//===============================================================================================
//===============================================================================================
//	API

int VideoOutLine()
{
	return gScreen.Line;
}

void VideoOutWaitVBL()
{
	ScreenState* s = &gScreen;
	while (s->Line > 20)
		__WFI();
}

void VideoOutEnqueueLine(volatile u8** lineHandle)
{
	ScreenState* s = &gScreen;
	while ((s->Write - s->Read) == LINE_QUEUE_SIZE)
		__WFI();
	int mask = LINE_QUEUE_SIZE-1;
	s->LineQueue[gScreen.Write++ & mask] = lineHandle;
}

void ADCInit()
{
	int ADC_Clk = 4500000;
	LPC_SYSCON->PDRUNCFG &= ~(0x1<<4);		// Power on
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<13);	// Enable AHB clock
	LPC_ADC->CR = ((SystemCoreClock/LPC_SYSCON->SYSAHBCLKDIV)/ADC_Clk-1)<<8;
}

#define ADC_BASE 0x4001C000

static int ADCRead(int channelNum )
{
	long regVal;
	LPC_ADC->CR &= 0xFFFFFF00; // clear channel selection
	LPC_ADC->CR |= (1 << 24) | (1 << channelNum);
	volatile long* a = (volatile long *)(ADC_BASE + 0x10 +  channelNum*4);
	do {
		regVal = *a;
	} while (regVal >= 0);	// wait for high bit
	LPC_ADC->CR &= 0xF8FFFFFF;
	return (regVal >> 6) & 0x3FF;
}

#define JOY_Y LPC_IOCON->R_PIO0_11
#define JOY_X LPC_IOCON->R_PIO1_0

u8 _keyState = 1;
u8 _keyDown = 0;
void Debounce(u8 kin)
{
	static u8 ct0, ct1;
	uint8_t i = _keyState ^ kin;
	ct0 = ~(ct0 & i);
	ct1 = ct0 ^ (ct1 & i);
	i &= ct0 & ct1;
	_keyState ^= i;
	_keyDown = _keyState & i;
}

volatile int _lastx;
volatile int _lasty;
u8 ReadXY(int* x, int* y)
{
	LPC_GPIO0->DIR |= 4;	//	Power Joystick
	LPC_GPIO0->DATA |= 0x04;	// JOY_VCC power
	JOY_Y = 2;
	JOY_X = 2;
	*x = ADCRead(0);
	*y = ADCRead(1);
	_lastx = *x;
	_lasty = *y;
	LPC_GPIO0->DIR &= ~4;	//	Release Joystick
	Debounce(LPC_GPIO1->DATA & (1<<4));
	return _keyState | (_keyDown ? 1 : 0);
}

int _lastKey = 0;
void CheckSleep()
{
	if (!_keyState)
		_lastKey = _vbl;

	if (_vbl - _lastKey > 3*60*60)	// power off after 3 minutes
	{
		 SCB->SCR |= 4;
		 LPC_PMU->PCON = 2;
		__asm volatile ("WFI");	// power off
	}
}

void VideoOutInit(int width)
{
	// port 2 d4-d7 luminance out
	LPC_GPIO0->DIR = 0x2;
	LPC_GPIO0->DIR = 0x2;	// NOP! for color phase!
	LPC_IOCON->PIO0_1 = 1;	// clock out
	LPC_GPIO2->DIR = 0xFF;	// ouy
	LPC_GPIO3->DIR = 0x7;
	STATE = 0;

	LPC_GPIO1->DIR |= 0x2;	// sync
	LPC_IOCON->R_PIO1_1= 1;

	LPC_SYSCON->CLKOUTCLKSEL = 3;
	LPC_SYSCON->CLKOUTUEN = 0;
	LPC_SYSCON->CLKOUTUEN = 1;
	LPC_SYSCON->CLKOUTDIV = 1;

	gScreen.Width = width;
	InitScreen(&gScreen);

	SysTick_Config(3648);	// Clock per scanline
	if ( !(SysTick->CTRL & (1<<SysTick_CTRL_CLKSOURCE_Msk)) )
		LPC_SYSCON->SYSTICKCLKDIV = 0x08;

	InitTimer();

	//	PIO1_9,PIO_10 PWM audio out
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<8);	// enable TMR16B1
	//LPC_GPIO1->DIR |= (1<<9)|(1<<10);
	LPC_IOCON->PIO1_9 = 1;	//	CT16B1_MAT0
	LPC_IOCON->PIO1_10 = 2;	//	CT16B1_MAT1
	LPC_TMR16B1->PWMC = 3;	// Enable PWM
	LPC_TMR16B1->TCR = 1;

	ADCInit();
	MusicPlay(&gScreen.Music,_clocks);	// play someth'n
}
