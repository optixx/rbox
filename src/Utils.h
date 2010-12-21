
/* Copyright (c) 2009, Peter Barrett
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

#define PROGMEM

#ifndef byte
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned long ulong;
#endif

#ifndef uchar
typedef unsigned char uchar;
#endif

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

extern "C"
{
#include <string.h>
}

#ifndef abs
#define abs(_x) ((_x) < 0 ? -(_x) : (_x))
#endif

#define pgm_read_byte(_x) (*(_x))
#define pgm_read_word(_x) (*((short*)(_x)))

#ifndef max
#define max(_a,_b) (((_a) > (_b)) ? (_a) : (_b))
#define min(_a,_b) (((_a) < (_b)) ? (_a) : (_b))
#endif

void delay(ushort ms);
void print(const char* s);
void printhex(int d);
void printdec(int d);
void assertFailed(const char* str, int len);

#define ASSERT(_x) if (!(_x)) assertFailed(0,0);
//#define ASSERT(_x) if (!(_x)) assertFailed(NULL,__LINE__);

ulong ReadPerfCounter();    // 1.5 mhz counter

void Console(const char* s);
void Console(const char* s, long n);
void ConsoleReset();


class Perf
{
    ulong _t;
public:
    Perf();
    ulong Elapsed();
    ulong ElapsedMS();
};

typedef struct
{
    ushort x;
    ushort y;
} TouchEvent;

byte TouchRead(TouchEvent& e);

typedef struct
{
    enum EventType
    {
        None,
        Up,
        Down,
        Move
    };
    EventType Type;
    ushort X;
    ushort Y;
    ushort LastX;
    ushort LastY;
} TouchState;

class Application
{
    public:
    static void    Init();
    static void    Loop(byte phase);
    static byte    Command(byte len, const byte* cmd);

    static void    TouchDown(TouchEvent& e);
    static void    TouchUp(TouchEvent& e);
    static void    TouchMove(TouchEvent& e);
};

void printHex4(uchar h);
void printHex8(uchar h);
void printHex(int h);
void printChar(char c);
void print(long d);
void print(const char* str);
void sendFlush();       // Wait until send buffer is empty

class RingBuffer
{
    byte* _buffer;
    byte _len;
    volatile byte _head;
    volatile byte _tail;
public:
    RingBuffer(byte* buffer, byte len) : _buffer(buffer),_len(len),_head(0),_tail(0){};
    byte Count()
    {
        return _head - _tail;
    }

    byte Read()
    {
        //while (_head == _tail);   // don't read empty buffers please
        byte c = _buffer[_tail & (_len-1)];
        _tail++;
        return c;
    }

    void Write(byte c)
    {
        if ((_head - _tail) == _len)
            return; // overrun
        _buffer[_head & (_len-1)] = c;
        _head++;
    }
};

void Fatal(int count);
void CheckStack(void* t, int len);

class Smashed
{
    short _sentinel;
    public:
    Smashed() : _sentinel(0x1234)
    {
        CheckStack(this,sizeof(*this));
    }
    ~Smashed()
    {
        if (_sentinel != 0x1234)
            Fatal(1);
    }
};
