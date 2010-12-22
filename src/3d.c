
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

#include "RBox.h"

#ifndef _WIN32
extern "C"
void memset(void* d, int v, int len);
#endif

int _hacky;
int _hackyColor;

void Fill(int x0, int x1, int y, int color);
void DrawBG(int y0, int y1);

#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 320

#if 1
#include "math3D.h"

#define DRAWFG
#define DRAWBG

inline short CEIL(int _x) { return ((_x + 16) >> 4); }
inline short FLOOR(int _x) { return (_x >> 4); }

int _colorMap[] = {
    0x0000,0xFC00,0x001F,0x03F0,
    0x8888,0xAAAA,0x4444,0x789A,
    0x1234,0x5678,0x9ABC,0xDEF0,
    0xABCD,0xDEAD,0xC0ED,0xBABE,
};

void ReadAccelerometer(char* dst);

short _gradientPoints[] =
{
    0,
    128,
    129,
    133,
    240-1,
};

byte _gradientColors[] =
{
    0xC0,0xC0,0xFF,
    0xFF,0xFF,0xFF,
    0x40,0x40,0x40,
    0x80,0x80,0x80,
    0xFF,0xFF,0xFF 
};

byte Lerp(byte* rgb, short f)
{
    short c = (rgb[0]*(127-f) + rgb[3]*f) >> 7;
    if (c < 0) return 0;
    if (c > 255) return 0xFF;
    return c;
}

//  Lerp
short GetBGColor(short y)
{
    short* p = _gradientPoints;
    int i = 0;
    while (p[i+1] <= y)
        i++;
    short d = p[i+1]-p[i];
    y -= p[i];
    short f = (y*127)/d;
   
    byte* c = _gradientColors + i*3;
    byte r = Lerp(c,f);
    byte g = Lerp(c+1,f);
    byte b = Lerp(c+2,f);
    return ((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r >> 3);
}

//  Triangles
typedef struct
{
    byte p0;
    byte p1;
    byte p2;
    byte fill;
} Triangle;

//  Active Edges, sorted by x
//  ~50 visible edges per line with 1k (seems about right)
class Edge
{
public:
    Edge*   Prev;
    Edge*   Next;
    short   YEnd;
    short   Z;          // TODO avg Z? Recalculate from points on intersection?
    
    long X;
    long XStep;
   // short ZStep;    // 1/Z actually
    
    byte    p0;
    byte    p1;
    byte    leftTriangle;
    byte    rightTriangle;
    
    void InsertAfter(Edge* a)
    {
	    Next = a->Next;
	    Prev = a;
	    a->Next->Prev = this;
	    a->Next = this;
    }

    void Setup(Vec3& v1, Vec3& v2)    // from v0 at top to v1 at bottom
    {
        short dy = v2.y - v1.y;
        short dx = v2.x - v1.x;    
        long gradient = dx;
        gradient = (gradient << 16) / dy;               // Nasty long division gradient is 16:16 (N.R. instead?)
        char ey = ((v1.y + 16) & 0xFFF0) - v1.y;        //+-15
        X = ((long)v1.x << 12) + ((ey * gradient) >> 4);
        XStep = gradient;
    }
    
    inline void Step()
    {
	    X += XStep;
	}
	
	inline int GetX()
	{
	    return ((short*)&X)[1];
	}
};

//  XForm points
//  List tri, sort by min y

static void Line(int x, int y, int x2, int y2)
{
    //Graphics::AALine(x,y,x2,y2);
}

void DrawT(Triangle& t, Vec3* p)
{
    Line(p[t.p0].x,p[t.p0].y,p[t.p1].x,p[t.p1].y);
    Line(p[t.p1].x,p[t.p1].y,p[t.p2].x,p[t.p2].y);
    Line(p[t.p2].x,p[t.p2].y,p[t.p0].x,p[t.p0].y);
}

void SWAP(byte& a, byte& b)
{
    byte tmp = a;
    a = b;
    b = tmp;
}

class Renderer
{
public:
    #define MAXVERTEXES 12
    #define MAXTRIANGLES 20
    #define MAXEDGES 16         // 16 per line
        
    Edge    _head;
    Edge    _tail;
    Edge    _edges[MAXEDGES];
    Edge*   _freeList;
    Edge*   _newEdgeList;   // New edges activated this round
    short   _x;
    short   _bgColor;
    
    Matrix  _matrix;
    
    byte    _vertexCount;
    byte    _triangleCount;
    Vec3    _vertexes[MAXVERTEXES];    
    Triangle _triangles[MAXTRIANGLES];
        
    void Init()
    {
        _vertexCount = _triangleCount = 0;
        memset(_triangles,0x69,sizeof(_triangles));
        memset(_edges,0x0,sizeof(_edges));
        _head.X = -0x7FFFFFFF;
        _tail.X = 0x7FFFFFFF;
    }
    
    void SetMatrix(const Matrix& m)
    {
		_matrix = m;
      //  memcpy(&_matrix,&m,sizeof(_matrix));
    }
    
    void Project(Vec3Char* src, Vec3* dst)
    {
        Vec3 v;
        v.x = (signed char)src->x;
        v.y = (signed char)src->y;
        v.z = (signed char)src->z;
        _matrix.Transform(v);   // comes out 12:4
        
        int x = v.x;
        int y = -v.y;
        int inv = 0x8000/v.z;    // short div
        dst->x = (x*inv >> 4) + ((SCREEN_WIDTH>>1) << 4);
        dst->y = (y*inv >> 4) + ((SCREEN_HEIGHT>>1) << 4);
        dst->z = v.z;
    }

    short TriangleMinY(byte ti)
    {
        return _vertexes[_triangles[ti].p0].y;
    }
    
    void TriangleSwap(byte i, byte j)
    {
        Triangle* t = _triangles;
        Triangle tmp = t[i];
        t[i] = t[j];
        t[j] = tmp;
    }
    
    //  Sort triangles by Y
    //  Could use a more expanded data structure at beginning of frame to speed this up
    void SortY(int left, int right)
    {
        int i = left;
        int j = right;
        short pivot = TriangleMinY((left + right) >> 1);

        while (i <= j)
        {
            while (TriangleMinY(i) < pivot)
                i++;
            while (TriangleMinY(j) > pivot)
                j--;
            if (i <= j)
            {
                TriangleSwap(i,j);
                i++;
                j--;
            }
        }

        if (left < j)
            SortY(left, j);
        if (i < right)
            SortY(i, right);
    }
        
    bool AddTriangles(Triangle* tris, byte tcount, Vec3Char* p, byte pcount)
    {
        if (tcount + _triangleCount > MAXTRIANGLES || pcount + _vertexCount > MAXVERTEXES)
            return false;
        
        //  Copy and transform points
        byte pointMark = _vertexCount;
        for (byte i = 0; i < pcount; i++)
            Project(p + i,_vertexes + _vertexCount++);

        //  Copy triangles to buffer, map point indexes
        byte triangleMark = _triangleCount;
        for (byte i = 0; i < tcount; i++)
        {
            Triangle& t = _triangles[triangleMark];
            t = *tris++;
            t.p0 += pointMark;  // do this later
            t.p1 += pointMark;
            t.p2 += pointMark;
            
            // Cull backfaces
            Vec3& v0 = _vertexes[t.p0];
		    Vec3& v1 = _vertexes[t.p1];
		    Vec3& v2 = _vertexes[t.p2];
		    bool d = ((long)(v0.x-v2.x))*(v1.y - v2.y) > ((long)(v1.x-v2.x))*(v0.y - v2.y);
            if (!d)
                continue;
                
            //  TODO garbage collect unused vertexes from culled triangles
                              
            //  order point v0,v1,v2 by y
            if (_vertexes[t.p0].y > _vertexes[t.p1].y) { SWAP(t.p0,t.p1); d = !d;  }
            if (_vertexes[t.p0].y > _vertexes[t.p2].y) { SWAP(t.p0,t.p2); d = !d;  }
            if (_vertexes[t.p1].y > _vertexes[t.p2].y) { SWAP(t.p1,t.p2); d = !d;  }
            if (d)
                t.fill |= 0x80; // dem in top bit of fill, 4 byte tris
            triangleMark++;     // Keep this one
        }
        
        if (triangleMark == _triangleCount)
            _vertexCount = pointMark;    // All triangles were clipped, ditch points
        _triangleCount = triangleMark;
        return true;
    }
    
    void InitEdgeList()
    {
        for (int i = 0; i < MAXEDGES; i++)
            _edges[i].Next = _edges + i + 1;
        _edges[MAXEDGES-1].Next = 0;
        _freeList = _edges;
        _newEdgeList = 0;
        _head.Prev = _tail.Next = 0;
        _head.Next = &_tail;
        _tail.Prev = &_head;
    }
    
    Edge* NewEdge()
    {
        if (_freeList == 0)
            return 0;
        Edge* e = _freeList;
        _freeList = e->Next;
        return e;
    }
    
    Edge* Release(Edge* e)
    {
        Edge* next = e->Next;
        e->Next->Prev = e->Prev;
        e->Prev->Next = e->Next;
        e->Next = _freeList;
        _freeList = e;  // Free list is singly linked
        return next;
    }
    
    //	Step and resort (good candidate for asm)
    void StepActive(int y)
    {
	    Edge* e,*e2,*next;
	    for (e = _head.Next; e != &_tail;)
	    {
		    if (y >= e->YEnd)
		    {
                //  Activate 2nd part of part of triangle if required
                if (e->rightTriangle != 0xFF)
                    Activte2ndPart(e,e->rightTriangle);
                if (e->leftTriangle != 0xFF)
                    Activte2ndPart(e,e->leftTriangle);
                //  Release this edge
                e = Release(e);
			    continue;
		    }
    		e->Step();
		    next = e->Next;
		    while (e->X < e->Prev->X)  // Reorder in x if required
		    {
			    e2 = e->Prev;
			    e2->Next = e->Next;		// Move it earlier, does not happen very often
			    e->Next->Prev = e2;
			    e2->Prev->Next = e;
			    e->Prev = e2->Prev;
			    e->Next = e2;
			    e2->Prev = e;
		    }
		    e = next;
	    }
    }
    
    //  Activate the bottom edge of a triangle if required
    void Activte2ndPart(Edge* e, byte triangleIndex)
    {
        Triangle& t = _triangles[triangleIndex];
        if (e->p0 == t.p1)
            return; // Was 2nd part already
            
        //  Now setup bottom bit, add to list
        Setup(t.p1,t.p2,triangleIndex,!(t.fill & 0x80));
    }
    
    //  Activate edge
    bool Setup(byte p0, byte p1, byte ti, bool left)
    {
        //  Does the edge already exist?
        //  Only need to search the new Edge list, can only match newly created edges
        Edge* e = _newEdgeList;
        while (e)
        {
            if (e->p0 == p0 && e->p1 == p1) // Share existing edge
            {
                if (left)
                    e->rightTriangle = ti;
                else
                    e->leftTriangle = ti;
                return true;
            }
            e = e->Next;
        }
        
        //  New edge
        Vec3& v0 = _vertexes[p0];
        Vec3& v1 = _vertexes[p1];
        if (v0.y == v1.y)
           return false;     // Horizontal edge, no pixels here
               
        short Y = CEIL(v0.y);
        short YEnd = FLOOR(v1.y);
        if (Y > YEnd)
            return false;   // Does happen
            
        e = NewEdge();
        if (!e)
            return false;     // out of edges.
        
        e->Setup(v0,v1);
        e->p0 = p0;
        e->p1 = p1;
        e->YEnd = YEnd;
        if (left)
        {
            e->rightTriangle = ti;
            e->leftTriangle = 0xFF;
        }
        else
        {
            e->leftTriangle = ti;
            e->rightTriangle = 0xFF;
        }
        
        //  Insert in new Edge list
        e->Next = _newEdgeList;
        _newEdgeList = e;
        return true;
    }
    
    void DrawBGSpan(short x0, short x1, short y)
    {
    	Fill(x0,x1-x0,y,-1);
    }

    void DrawSpan(short x0, short x1, short y, byte triangleIndex)
    {
#if 0
        if (_x < x0)
        {
            DrawBGSpan(_x,x0,y);
            _x = x0;
        }
        else if (x0 < _x)
        {
            //LCD::SetGRAM(x0,y); // THIS IS A BUG TODO
        }
        byte color = _triangles[triangleIndex].fill;
        //Graphics::SetColor(_colorMap[color & 0xF]);
        	Fill(x0,x1,y,color & 0xF);
        _x = x1;
#endif
    }
    
    //  This is not right but works well enough
    //  Needs more work to extend to Z
    u8* DrawActive(int y, u8* dst)
    {
        Edge* _stack[16];
        int _depth = 0;
        u8* start = dst;
        Edge* curr = _head.Next;
	    while (curr != &_tail)
	    {
	        if (y <= curr->YEnd)
	        {
                // Right edge
                if (curr->leftTriangle != 0xFF)
                {
                    byte triangleIndex = curr->leftTriangle;
                    for (byte i = 0; i < _depth; i++)
                    {
                        if (_stack[i]->rightTriangle == triangleIndex)
                        {
                            short x0 = _stack[i]->GetX();
                            short x1 = curr->GetX();
                            if (x1 > x0)
							{
                                //DrawSpan(x0,x1,y,triangleIndex);
								if (start == dst)
								{
									dst++;			// count
									*dst++ = x0;	// left
									_x = x0;
								}
								int run = x1-x0;
								if (run > 0)
								{
									 byte color = _triangles[triangleIndex].fill & 0x0f;
									 *dst++ = run;
									 *dst++ = (color << 4) | _hackyColor;
									 if (_hacky == -1)
										 _hacky = y;
								}
							}
                             
                            // Remove
                            _depth--;
                            for (;i < _depth;i++)
                                _stack[i] = _stack[i+1];   // remove
                            break;
                        }
                    }
                }
                
                // Left edge
                if (curr->rightTriangle != 0xFF)
                {
                    //ASSERT(_depth < 16);
                    _stack[_depth++] = curr;
                }
	        }
            curr = curr->Next;
	    }
		if (start == dst)
			start[0] = 0;
		else
			start[0] = (dst - (start+2))>>1;	// # of runs
		return dst;
    }

#if 0
    void Draw(short& top, short& bottom)
    {
        if (_triangleCount == 0)
            return;
        SortY(0,_triangleCount-1);
            
        InitEdgeList();        
        
        // slight hack to only draw dirty lines
        int yy = _vertexes[_triangles[0].p0].y;
        short firstY = (int) CEIL(yy);  //
        short nextY = min(firstY,top);
        
        //  Setup BG drawing
        DrawBG(nextY,firstY);
        
        //  Top of dirty area for this frame
        top = firstY;
        
        short y;
        short triangleIndex = 0;
        for (y = top; y < SCREEN_HEIGHT; y++)
        {
            if (_head.Next == &_tail && _newEdgeList == 0)
            {
                if (triangleIndex < _triangleCount)
                {
                    y = nextY;
                  //  LCD::SetGRAM(0,y);
                }
                else
                    break;
            }
            
            // Activate triangles, insert edges into new edge list
            if (nextY <= y)
            {
                while (triangleIndex < _triangleCount)
                {
                    Triangle& t = _triangles[triangleIndex];
                    nextY = CEIL(_vertexes[t.p0].y);
                    if (nextY > y)
                        break;
                        
                    //  Activate first 2 triangle edges
                    bool dem = t.fill & 0x80;
                    Setup(t.p0,t.p2,triangleIndex,dem);
                    if (!Setup(t.p0,t.p1,triangleIndex,!dem)) // if p0.y == p1.y edge won't be inserted so ...
                        Setup(t.p1,t.p2,triangleIndex,!dem); //  ... setup thrid immediately
                    triangleIndex++;
                }
            }
            
            //  Merge new edge list into edge list
            while (_newEdgeList)
            {
                Edge* e = _newEdgeList;
                _newEdgeList = _newEdgeList->Next;
                Edge* a = &_head;
                long x = e->X;
		        while (a->Next->X < x)
			        a = a->Next;
		        e->InsertAfter(a);  //  Insert into sorted active edgelist
            }
            
            _x = 0;
            _bgColor = GetBGColor(y);
            if (_head.Next != &_tail)
            {
                DrawActive(y);
                StepActive(y);
            }
            if (_x < SCREEN_WIDTH)
                DrawBGSpan(_x,SCREEN_WIDTH,y);
        }

        DrawBG(y,bottom);
        bottom = y;
            
      //  ASSERT(triangleIndex == _triangleCount);
    }

#endif
	int _y;
	int _nextY;
	int _triangleIndex;

    void DrawBegin()
    {
        SortY(0,_triangleCount-1);
        InitEdgeList();        
        
        // slight hack to only draw dirty lines
        int yy = _vertexes[_triangles[0].p0].y;
        _nextY = (int) CEIL(yy);  //
		_y = _nextY;
		_triangleIndex = 0;
		//_hacky = _y;
	}

	u8* DrawLoop(u8* runs)
	{
		if (_head.Next == &_tail && _newEdgeList == 0)
        {
            if (_triangleIndex < _triangleCount)
            {
                _y = _nextY;
              //  LCD::SetGRAM(0,y);
            }
            else
                return 0;
        }
        
        // Activate triangles, insert edges into new edge list
        if (_nextY <= _y)
        {
            while (_triangleIndex < _triangleCount)
            {
				int ti = _triangleIndex;
                Triangle& t = _triangles[ti];
                _nextY = CEIL(_vertexes[t.p0].y);
                if (_nextY > _y)
                    break;
                    
                //  Activate first 2 triangle edges
                bool dem = t.fill & 0x80;
                Setup(t.p0,t.p2,ti,dem);
                if (!Setup(t.p0,t.p1,ti,!dem))	// if p0.y == p1.y edge won't be inserted so ...
                    Setup(t.p1,t.p2,ti,!dem);	//  ... setup thrid immediately
                _triangleIndex++;
            }
        }
        
        //  Merge new edge list into edge list
        while (_newEdgeList)
        {
            Edge* e = _newEdgeList;
            _newEdgeList = _newEdgeList->Next;
            Edge* a = &_head;
            long x = e->X;
	        while (a->Next->X < x)
		        a = a->Next;
	        e->InsertAfter(a);  //  Insert into sorted active edgelist
        }
        
        _x = 0;
        if (_head.Next != &_tail)
        {
            runs = DrawActive(_y,runs);
            StepActive(_y);
        }
		_y++;
		return runs;
    }
    
    void Wireframe()
    {
        //LCD::OpenWrap();
        for (int i = 0; i < _triangleCount; i++)
            DrawT(*(_triangles + i),_vertexes);
    }  
};

#define FARZ (160L << FPP)
#define NEARZ (16L << FPP)

inline int DELTA(int x)
{
	x = (x - 512) >> 5;
	bool sign = x < 0;
	if (sign)
		x = -x;
	x -= 3;	// deadzone
	if (x <= 0)
		return 0;
	return sign ? -x : x;
}
int _model = 0;

class ModelState
{
    public:
    long _z;
    long _zsrc;
    long _zdst;
    byte _phase;
    byte _rotX;
    byte _rotY;
    byte _rotZ;
    
    byte _samp;    
    char _xyz[3*8];

    short _top; // Dirty region
    short _bottom;
    Renderer _r;
   
    void Key(short key)
    {
     	if ((_phase++ & 0xFF) == 0)
     		_zdst = FARZ;	// Trigger zoom away

        if (key == 1 || key == 0x80)
        {
            if (key == 0x80)
            {
            	_zdst = _zsrc = _z = NEARZ;
                _rotX = _rotY = _rotZ = 0;
                _phase = 8;
                //_samp = 0;
            } else {
             //   Graphics::SetAAColor(0x0000,0xFFFF);
             //   _phase++;
              //  if (_phase == 3)
              //      _phase = 0;
            }
            _top = 0;
            _bottom = SCREEN_HEIGHT;
        }
    }
    
    static char Filter(char *c)
    {
        int n = 0;
        byte i = 8;
        while (i--)
        {
            n += *c;
            c += 3;
        }
        return n >> 3;
    }
    
    void Rotate()
    {
        #if 0
    	int x,y;
    	if (ReadXY(&y,&x) & 1)
    		_model = (_model + 0xFF) & ~0xFF;
    	_rotX += DELTA(x);
    	_rotY += DELTA(y);
        #else
        _rotX += 1;
        _rotY += 2;
        _rotZ += 3;
        #endif
    }
    
    void Draw(short key, Triangle* triangles, byte tcount, Vec3Char* vertexes, byte vcount)
    {
        Key(key);
        
        Rotate();
        
        Matrix matrix;
        _r.Init();
        {
            Matrix m;
            matrix.Translate(0,  FPI(0L), _z);
            m.RotateX(_rotX);
            matrix.Concatenate(m);
            m.RotateY(_rotY);
            matrix.Concatenate(m);
            m.RotateZ(_rotZ);
            matrix.Concatenate(m);
            _r.SetMatrix(matrix);
       }
#if 0
        //  Shrink over time
       if (_z < (150L << FPP))
           _z += (1L<<FPP)/256;
#endif
            
        _r.AddTriangles(triangles,tcount,vertexes,vcount);
		_r.DrawBegin();
#if 0
        _top = 0;
        _bottom = SCREEN_HEIGHT;
        _r.Draw(_top,_bottom);
#endif
    }
};

#include "Models3D.h"

#if 1
const Model* _models[] =
{
	&_tetrahedron,
	&_octahedron,
	&_cube,
	&_icosohedron
};

#endif

void Sampler(short countdown);
void DrawTriangles(int keys, byte* appBuffer, int len)
{    
    ModelState* m = (ModelState*)appBuffer;
	int i = (_model++ >> 8) & 3;
	_hackyColor = i+3;
	const Model* model = _models[i];
	m->Draw(keys,(Triangle*)model->Triangles,model->TriangleCount,(Vec3Char*)model->Vertexes,model->VertexCount);
}

u8* DrawTrianglesLoop(u8* appBuffer, u8* runs)
{    
    ModelState* m = (ModelState*)appBuffer;
	return m->_r.DrawLoop(runs);
}

#endif
