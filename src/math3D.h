
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

#ifndef MATH3D_H
#define MATH3D_H

//#include <math.h>

short SIN(u8 angle);
short COS(u8 angle);

//  word version
typedef struct
{
    short x;
    short y;
    short z;
} Vec3;

//  Compact version for storing models
typedef struct
{
    signed char x;
    signed char y;
    signed char z;
} Vec3Char;

#define FPONE		65536
#define FPP			16
#define FPI(x)		((x)<<FPP)

class Matrix
{
public:
	Matrix () { Identity (); }
	void Identity();
	void Transform(Vec3& v);
	void RotateX(int angle);
	void RotateY(int angle);
	void RotateZ(int angle);
	void Translate(long x, long y, long z);
	void Concatenate( Matrix& m2 );
private:
	long _m[16];
};

#endif
