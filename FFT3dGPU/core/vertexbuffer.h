// FFT3DGPU plugin for Avisynth 2.6/Avisynth+
// Copyright(C)2005 Tonny Petersen (tsp@person.dk) 
// Based on FFT3DFilter plugin for Avisynth 2.5 - 3D Frequency Domain filter
// Copyright(C)2004-2005 A.G.Balakhnin aka Fizick, email: bag@hotmail.ru, web: http://bag.hotmail.ru
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .


#ifndef __VERTEXBUFFER__
#define __VERTEXBUFFER__

struct textcoord{
	textcoord():u(0),v(0){}
	textcoord(float _u,float _v):u(_u),v(_v){}
	FLOAT u,v;
};

struct vertex {
	vertex(){};
	FLOAT x, y, z, rhw; // The transformed position for the vertex
    DWORD color;        // The vertex color
	textcoord tex[8];
};

struct vertex3{
	FLOAT x,y,z,rhw;
	DWORD color;
	textcoord tex[3];
};

struct vertex4{
	FLOAT x,y,z,rhw;
	DWORD color;
	textcoord tex[4];
};

//untransformed
struct vertexut3{
	FLOAT x,y,z,rhw;
	textcoord tex[3];
};
struct vertexut2{
	FLOAT x,y,z,rhw;
	textcoord tex[2];
};
struct vertexut1{
	FLOAT x,y,z,rhw;
	textcoord tex;
};

class Quad {
	public:
		virtual HRESULT SetActive()=0;
		virtual HRESULT Draw()=0;
		virtual ~Quad(){};
protected:
	Quad(){};
};

struct viLUT{int nvertex;int startindexeven;int startindexodd;int minvertexeven;int minvertexodd;} ;

class FFTQuad2 : public Quad {
public:
FFTQuad2(LPDIRECT3DDEVICE9 pDevice,int x,int nx,int y,int ny,bool horizontal,int startstage);
	~FFTQuad2();
	HRESULT SetActive();
	HRESULT Draw();
	HRESULT DrawEven(int stage);
	HRESULT DrawOdd(int stage);
protected:
	LPDIRECT3DDEVICE9 _pDevice;
	LPDIRECT3DVERTEXDECLARATION9 _pVertexDeclaration;
	LPDIRECT3DVERTEXBUFFER9      _pVertexBuffer;
    LPDIRECT3DINDEXBUFFER9       _pIndexBuffer;
	int nvertex;
	int nindex;
	int startstage;
	int endstage;
	viLUT* stageLUT;
};

class FFTQuad0 : public Quad {
public:
	FFTQuad0(LPDIRECT3DDEVICE9 pDevice,int x,int nx,int y,int ny,bool horizontal);
	~FFTQuad0();
	HRESULT SetActive();
	HRESULT Draw();
protected:
	LPDIRECT3DDEVICE9 _pDevice;
	LPDIRECT3DVERTEXDECLARATION9 _pVertexDeclaration;
	LPDIRECT3DVERTEXBUFFER9      _pVertexBuffer;
    LPDIRECT3DINDEXBUFFER9       _pIndexBuffer;
	int nvertex;
	int nindex;
};


class C2RQuad : public Quad {
public:
	C2RQuad(LPDIRECT3DDEVICE9 pDevice,int x,int nx,int y,int ny);
	HRESULT SetActive();
	HRESULT Draw();
	HRESULT Draw00();
	HRESULT Draw11();
	HRESULT Draw01();
	HRESULT Draw10();
protected:
	LPDIRECT3DDEVICE9 _pDevice;
	LPDIRECT3DVERTEXDECLARATION9 _pVertexDeclaration;
	LPDIRECT3DVERTEXBUFFER9      _pVertexBuffer;
    LPDIRECT3DINDEXBUFFER9       _pIndexBuffer;
	int startvertex00;
	int nvertex00;
	int nindex00;
	int startvertex01;
	int nvertex01;
	int nindex01;
	int startvertex10;
	int nvertex10;
	int nindex10;
	int startvertex11;
	int nvertex11;
	int nindex11;
};

class R2CFFTQuad : public Quad {
public:
	R2CFFTQuad(LPDIRECT3DDEVICE9 pDevice,int x,int nx,int y,int ny);
	~R2CFFTQuad();
	HRESULT SetActive();
	HRESULT Draw();
	HRESULT DrawEven();
	HRESULT DrawOdd();
protected:
	LPDIRECT3DDEVICE9 _pDevice;
	LPDIRECT3DVERTEXDECLARATION9 _pVertexDeclaration;
	LPDIRECT3DVERTEXBUFFER9      _pVertexBuffer;
    LPDIRECT3DINDEXBUFFER9       _pIndexBuffer;
	int nvertex;
	int nindex;
};

class CollectQuad : public Quad {
public:
	CollectQuad(LPDIRECT3DDEVICE9 pDevice,int x,int nx,int y,int ny,bool horizontal);
	~CollectQuad();
	HRESULT SetActive();
	HRESULT Draw();
	HRESULT DrawEven();
	HRESULT DrawOdd();
protected:
	LPDIRECT3DDEVICE9 _pDevice;
	LPDIRECT3DVERTEXDECLARATION9 _pVertexDeclaration;
	LPDIRECT3DVERTEXBUFFER9      _pVertexBuffer;
    LPDIRECT3DINDEXBUFFER9       _pIndexBuffer;
	int nvertex;
	int nindex;
};



class NQuad : public Quad{
	public:
		NQuad(LPDIRECT3DDEVICE9 pDevice,RECT* Rect,int size,int RectOffset=0,bool IndependentOffset=false);
		NQuad(LPDIRECT3DDEVICE9 pDevice,RECT* Rect,int size,int RectOffset,RECT &QuadRect,bool IndependentOffset=false);
		~NQuad();
		HRESULT SetActive();
		HRESULT Draw();
		static HRESULT CreateVertexBuffer();
		
protected:
		static LPDIRECT3DVERTEXBUFFER9 pVertexBuffer;
		static LPDIRECT3DDEVICE9 _pDevice;
		static int vertexsize;
		static int nvertex;
		static int fvf;
		static vertex* V;
		int vertexoffset;

};

//like NQuad just without a shared vertexbuffer
class NLQuad : public Quad{
	public:
		NLQuad(LPDIRECT3DDEVICE9 pDevice,RECT* Rect,int size,int RectOffset=0,bool IndependentOffset=false);
		~NLQuad();
		HRESULT SetActive();
		HRESULT Draw();
		
		
protected:
		HRESULT CreateVertexBuffer();
		LPDIRECT3DVERTEXBUFFER9 pVertexBuffer;
		LPDIRECT3DDEVICE9 _pDevice;
		int vertexsize;
		int nvertex;
		int fvf;
		vertex* V;
};

//used with mode=2
class MQuad : public Quad{
public:
	MQuad(LPDIRECT3DDEVICE9 pDevice);
	~MQuad();
	void CreateVB(int width,int height,int bw,int bh,int repx,int repy,int border);
	void CreateVBi(int width,int height,int bw,int bh,int repx,int repy,int border);
	HRESULT SetActive();
	HRESULT Draw();
protected:
		LPDIRECT3DVERTEXBUFFER9 pVertexBuffer;
		LPDIRECT3DINDEXBUFFER9 pIndexBuffer;
		LPDIRECT3DDEVICE9 _pDevice;
		int trianglecount;
		int vertexsize;
		int fvf;
		

};

//used in mode=1 when ow!=bw/2 or oh!=bh/2
class OQuad {
public:
	OQuad(LPDIRECT3DDEVICE9 pDevice);
	~OQuad();
	void CreateVB(int width,int height,int bw,int bh,int ow,int oh);
	void CreateVBi(int width,int height,int bw,int bh,int ow,int oh,int srcwidth,int srcheight);
	HRESULT SetActive();
	HRESULT DrawCorners();
	HRESULT DrawHorizontalxy();
	HRESULT DrawHorizontalzw();
	HRESULT DrawVerticaltex1();
	HRESULT DrawVerticaltex2();
	HRESULT DrawNonoverlaptex1xy();
	HRESULT DrawNonoverlaptex2xy();
	HRESULT DrawNonoverlaptex1zw();
	HRESULT DrawNonoverlaptex2zw();
	HRESULT Draw();

protected:
		LPDIRECT3DVERTEXBUFFER9 pVertexBuffer;
		LPDIRECT3DINDEXBUFFER9 pIndexBuffer;
		LPDIRECT3DDEVICE9 _pDevice;
		int nblocks;
		int vertexsize;
		int fvf;

		int ncornerblock;
		int startindexcornerblock;
		int startvertexcornerblock;
		
		int nhorizontalxyblock;
		int startindexhorizontalxyblock;
		int startvertexhorizontalxyblock;
		int nhorizontalzwblock;
		int startindexhorizontalzwblock;
		int startvertexhorizontalzwblock;

		int nverticaltex1block;
		int startindexverticaltex1block;
		int startvertexverticaltex1block;
		int nverticaltex2block;
		int startindexverticaltex2block;
		int startvertexverticaltex2block;

		int nnonoverlaptex1xyblock;
		int startindexnonoverlaptex1xyblock;
		int startvertexnonoverlaptex1xyblock;
		int nnonoverlaptex1zwblock;
		int startindexnonoverlaptex1zwblock;
		int startvertexnonoverlaptex1zwblock;
		int nnonoverlaptex2xyblock;
		int startindexnonoverlaptex2xyblock;
		int startvertexnonoverlaptex2xyblock;
		int nnonoverlaptex2zwblock;
		int startindexnonoverlaptex2zwblock;
		int startvertexnonoverlaptex2zwblock;
		
};

//used in mode=1 when ow!=bw/2 or oh!=bh/2 and interlaced==true
class OIQuad {
public:
	OIQuad(LPDIRECT3DDEVICE9 pDevice);
	~OIQuad();
	void CreateVB(int width,int height,int bw,int bh,int ow,int oh);
	void CreateVBi(int width,int height,int bw,int bh,int ow,int oh,int srcwidth,int srcheight,bool even);
	HRESULT SetActive();
	HRESULT DrawCorners();
	HRESULT DrawHorizontalxy();
	HRESULT DrawHorizontalzw();
	HRESULT DrawVerticaltex1();
	HRESULT DrawVerticaltex2();
	HRESULT DrawNonoverlaptex1xy();
	HRESULT DrawNonoverlaptex2xy();
	HRESULT DrawNonoverlaptex1zw();
	HRESULT DrawNonoverlaptex2zw();
	HRESULT Draw();

protected:
		LPDIRECT3DVERTEXBUFFER9 pVertexBuffer;
		LPDIRECT3DINDEXBUFFER9 pIndexBuffer;
		LPDIRECT3DDEVICE9 _pDevice;
		int nblocks;
		int vertexsize;
		int fvf;

		int ncornerblock;
		int startindexcornerblock;
		int startvertexcornerblock;
		
		int nhorizontalxyblock;
		int startindexhorizontalxyblock;
		int startvertexhorizontalxyblock;
		int nhorizontalzwblock;
		int startindexhorizontalzwblock;
		int startvertexhorizontalzwblock;

		int nverticaltex1block;
		int startindexverticaltex1block;
		int startvertexverticaltex1block;
		int nverticaltex2block;
		int startindexverticaltex2block;
		int startvertexverticaltex2block;

		int nnonoverlaptex1xyblock;
		int startindexnonoverlaptex1xyblock;
		int startvertexnonoverlaptex1xyblock;
		int nnonoverlaptex1zwblock;
		int startindexnonoverlaptex1zwblock;
		int startvertexnonoverlaptex1zwblock;
		int nnonoverlaptex2xyblock;
		int startindexnonoverlaptex2xyblock;
		int startvertexnonoverlaptex2xyblock;
		int nnonoverlaptex2zwblock;
		int startindexnonoverlaptex2zwblock;
		int startvertexnonoverlaptex2zwblock;
		

};


#endif