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


#include <d3d9.h>
#include "vertexbuffer.h"
#include <dxerr.h>
#include "Debug class.h"


static int log2(int n)
{
  int logn = 0;
  for (int i = n; i > 1; i /= 2, logn++);
  return logn;
}

int  NQuad::fvf = 0;
int  NQuad::vertexsize = 0;
int  NQuad::nvertex = 0;
vertex*  NQuad::V = 0;
LPDIRECT3DVERTEXBUFFER9  NQuad::pVertexBuffer = 0;
LPDIRECT3DDEVICE9  NQuad::_pDevice = 0;
const float pi(acos(-1.0));

HRESULT  NQuad::CreateVertexBuffer() {
  if (nvertex == 0)
    return -1;
  if (pVertexBuffer)
    pVertexBuffer->Release();
#ifdef _DEBUG
  OutputDebugString("Creating vertexbuffer\n");
#endif
  _pDevice->CreateVertexBuffer(nvertex*vertexsize, D3DUSAGE_WRITEONLY, fvf, D3DPOOL_MANAGED/*D3DPOOL_DEFAULT*/, &pVertexBuffer, NULL);
  HRESULT hr;
  VOID* pVertices;
  if (FAILED(hr = pVertexBuffer->Lock(0, vertexsize*nvertex, (void**)&pVertices, 0)))
    return DXTrace("vertexbuffer.cpp", __LINE__, hr, "Quad::CreateVertexBuffer", true); ;
  for (int i = 0; i < nvertex; i++)
    memcpy((unsigned char*)pVertices + i * vertexsize, &V[i], vertexsize);
  return pVertexBuffer->Unlock();
}

NQuad::NQuad(LPDIRECT3DDEVICE9 pDevice, RECT* Rect, int size, int RectOffset, bool IndependentOffset) :
  vertexoffset(nvertex)
{
  if (!_pDevice)
    _pDevice = pDevice;
  if (!nvertex)
    V = NEW vertex[1000];
  vertex* v = V + nvertex;
  fvf = fvf | D3DFVF_XYZRHW | D3DFVF_DIFFUSE | ((size >= 1)*D3DFVF_TEX1) | ((size >= 2)*D3DFVF_TEX2) | ((size >= 3)*D3DFVF_TEX3) | ((size >= 4)*D3DFVF_TEX4)
    | ((size >= 5)*D3DFVF_TEX5) | ((size >= 6)*D3DFVF_TEX6) | ((size >= 7)*D3DFVF_TEX7) | ((size >= 8)*D3DFVF_TEX8);

  for (int i = 0; i < 4; i++) {
    v[i].rhw = 1.0f;
    v[i].z = 0.5f;
    v[i].color = 0x00808000;
  }
  int W0 = Rect[0].right - Rect[0].left;
  int H0 = Rect[0].bottom - Rect[0].top;
  v[0].x = 0; v[0].y = 0;
  v[1].x = W0; v[1].y = 0;
  v[2].x = 0; v[2].y = H0;
  v[3].x = W0; v[3].y = H0;
  for (int i = 0; i < size; i++) {
    float W = Rect[i + RectOffset].right - Rect[i + RectOffset].left;
    float H = Rect[i + RectOffset].bottom - Rect[i + RectOffset].top;
    float etaW = 0.5 / W + (float)Rect[i + RectOffset].left / W;
    float etaH = 0.5 / H + (float)Rect[i + RectOffset].top / H;
    if (IndependentOffset) {
      W0 = W;
      H0 = H;
    }
    v[0].tex[i].u = etaW; v[0].tex[i].v = etaH;
    v[1].tex[i].u = etaW + W0 / W; v[1].tex[i].v = etaH;
    v[2].tex[i].u = etaW; v[2].tex[i].v = etaH + H0 / H;
    v[3].tex[i].u = etaW + W0 / W; v[3].tex[i].v = etaH + H0 / H;
  }
  int vs = sizeof(float)*(4 + 2 * size) + sizeof(int);
  vertexsize = vertexsize > vs ? vertexsize : vs;
  nvertex += 4;
  /*_pDevice->CreateVertexBuffer( 4*vertexsize,D3DUSAGE_WRITEONLY, fvf,D3DPOOL_DEFAULT, &pVertexBuffer, NULL );
  VOID* pVertices;
  pVertexBuffer->Lock(0,vertexsize*4,(void**)&pVertices,0);
  for(int i=0;i<4;i++)
    memcpy((unsigned char*)pVertices+i*vertexsize,&v[i],vertexsize);
  pVertexBuffer->Unlock();*/
}

NQuad::NQuad(LPDIRECT3DDEVICE9 pDevice, RECT* Rect, int size, int RectOffset, RECT &QuadRect, bool IndependentOffset) :
  vertexoffset(nvertex)
{
  if (!_pDevice)
    _pDevice = pDevice;
  if (!nvertex)
    V = NEW vertex[1000];
  vertex* v = V + nvertex;
  fvf = fvf | D3DFVF_XYZRHW | D3DFVF_DIFFUSE | ((size >= 1)*D3DFVF_TEX1) | ((size >= 2)*D3DFVF_TEX2) | ((size >= 3)*D3DFVF_TEX3) | ((size >= 4)*D3DFVF_TEX4)
    | ((size >= 5)*D3DFVF_TEX5) | ((size >= 6)*D3DFVF_TEX6) | ((size >= 7)*D3DFVF_TEX7) | ((size >= 8)*D3DFVF_TEX8);

  for (int i = 0; i < 4; i++) {
    v[i].rhw = 1.0f;
    v[i].z = 0.5f;
    v[i].color = 0x00808000;
  }
  int W0 = Rect[0].right - Rect[0].left;
  int H0 = Rect[0].bottom - Rect[0].top;
  v[0].x = QuadRect.left; v[0].y = QuadRect.top;
  v[1].x = QuadRect.right; v[1].y = QuadRect.top;
  v[2].x = QuadRect.left; v[2].y = QuadRect.bottom;
  v[3].x = QuadRect.right; v[3].y = QuadRect.bottom;
  for (int i = 0; i < size; i++) {
    float W = Rect[i + RectOffset].right - Rect[i + RectOffset].left;
    float H = Rect[i + RectOffset].bottom - Rect[i + RectOffset].top;
    float etaW = 0.5 / W + (float)Rect[i + RectOffset].left / W;
    float etaH = 0.5 / H + (float)Rect[i + RectOffset].top / H;
    if (IndependentOffset) {
      W0 = W;
      H0 = H;
    }
    v[0].tex[i].u = etaW; v[0].tex[i].v = etaH;
    v[1].tex[i].u = etaW + W0 / W; v[1].tex[i].v = etaH;
    v[2].tex[i].u = etaW; v[2].tex[i].v = etaH + H0 / H;
    v[3].tex[i].u = etaW + W0 / W; v[3].tex[i].v = etaH + H0 / H;
  }
  int vs = sizeof(float)*(4 + 2 * size) + sizeof(int);
  vertexsize = vertexsize > vs ? vertexsize : vs;
  nvertex += 4;
  /*_pDevice->CreateVertexBuffer( 4*vertexsize,D3DUSAGE_WRITEONLY, fvf,D3DPOOL_DEFAULT, &pVertexBuffer, NULL );
  VOID* pVertices;
  pVertexBuffer->Lock(0,vertexsize*4,(void**)&pVertices,0);
  for(int i=0;i<4;i++)
    memcpy((unsigned char*)pVertices+i*vertexsize,&v[i],vertexsize);
  pVertexBuffer->Unlock();*/
}

NQuad::~NQuad() {
  nvertex -= 4;
  if (nvertex <= 0) {
    delete[] V;
    if (pVertexBuffer)
#ifdef _DEBUG
      cerrwin << std::endl << "pVertexBuffer RefCount: " << pVertexBuffer->Release() << std::endl;
    OutputDebugString("Releasing vertexbuffer\n");
#else
      pVertexBuffer->Release();
#endif
    pVertexBuffer = 0;
    _pDevice = 0;
  }
}

HRESULT  NQuad::SetActive() {
  _pDevice->SetStreamSource(0, pVertexBuffer, 0, vertexsize);
  return _pDevice->SetFVF(fvf);
}

HRESULT  NQuad::Draw() {
  HRESULT hr = _pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, vertexoffset, 2);
  return hr;
}

/*
NIQuad::NIQuad(LPDIRECT3DDEVICE9 pDevice,int dstwidth,int dstheight,int bw,int bh,int width,int height):
_pDevice(pDevice)
{
  vertex v[4*3*2];//4 vertex per quad *3 quads*2 fields
  fvf=D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_TEX2|D3DFVF_TEX3;
  short *Indexbuffer=NEW short[6*3*2];//6 index per quad *3 quads*2 fields
  unsigned int index=0;
  unsigned int vindex=0;

  RECT rec[4];
  rec[0].left=0;
  rec[0].right=dstwidth;
  rec[0].top=0;
  rec[0].bottom=dstheight;
  rec[1].left=-bw/2;//xy
  rec[1].right=width-bw/2;
  rec[1].top=-bh/2;
  rec[1].bottom=height-bh/2;
  rec[2].left=0;//zw
  rec[2].top=-bh/2;
  rec[2].right=width;
  rec[2].bottom=height-bh/2;
  rec[3].left=0;	//factor
  rec[3].right=bw;
  rec[3].top=0;
  rec[3].bottom=bh;

  rec[2].top=rec[1].top=-bh/2-1;
  rec[2].bottom=rec[1].bottom=-1;
  rec[3].bottom=bh/2;
  RECT qrec={0,0,dstwidth,bh/4};
  CreateQuad(v,Indexbuffer,rec,3,1,qrec,false);
  index+=6;
  vindex+=4;
  rec[2].top=rec[1].top=-bh/2-1;
  rec[2].bottom=rec[1].bottom=-1;
  rec[3].bottom=bh/2;



  vertexsize=sizeof(v);
  _pDevice->CreateVertexBuffer( 4*vertexsize,D3DUSAGE_WRITEONLY, fvf,D3DPOOL_MANAGED, &pVertexBuffer, NULL );
  VOID* pVertices;
  pVertexBuffer->Lock(0,vertexsize*4,(void**)&pVertices,0);
  for(int i=0;i<4;i++)
    memcpy((unsigned char*)pVertices+i*vertexsize,&v[i],vertexsize);
  pVertexBuffer->Unlock();
}

void NIQuad::CreateQuad(vertex3 *v,short *Indexbuffer,RECT* Rect,int size,int RectOffset,RECT &QuadRect,bool IndependentOffset)
{
  for(int i=0;i<4;i++){
    v[i].rhw=1.0f;
    v[i].z=0.5f;
    v[i].color=0x00808000;
  }
  int W0=Rect[0].right-Rect[0].left;
  int H0=Rect[0].bottom-Rect[0].top;
  v[0].x=QuadRect.left;v[0].y=QuadRect.top;
  v[1].x=QuadRect.right;v[1].y=QuadRect.top;
  v[2].x=QuadRect.left;v[2].y=QuadRect.bottom;
  v[3].x=QuadRect.right;v[3].y=QuadRect.bottom;
  Indexbuffer[0]=0;
  Indexbuffer[1]=1;
  Indexbuffer[2]=2;
  Indexbuffer[3]=1;
  Indexbuffer[4]=3;
  Indexbuffer[5]=2;
  for(int i=0;i<size;i++){
    float W=Rect[i+RectOffset].right-Rect[i+RectOffset].left;
    float H=Rect[i+RectOffset].bottom-Rect[i+RectOffset].top;
    float etaW=0.5/W+(float)Rect[i+RectOffset].left/W;
    float etaH=0.5/H+(float)Rect[i+RectOffset].top/H;
    if(IndependentOffset){
      W0=W;
      H0=H;
    }
    v[0].tex[i].u=etaW;v[0].tex[i].v=etaH;
    v[1].tex[i].u=etaW+W0/W;v[1].tex[i].v=etaH;
    v[2].tex[i].u=etaW;v[2].tex[i].v=etaH+H0/H;
    v[3].tex[i].u=etaW+W0/W;v[3].tex[i].v=etaH+H0/H;
  }
}


NIQuad::~NIQuad(){
  if(pVertexBuffer)
  {
#ifdef _DEBUG
    cerrwin<<std::endl<<"pVertexBuffer RefCount: "<<pVertexBuffer->Release()<<std::endl;
    OutputDebugString("Releasing vertexbuffer\n");
#else
    pVertexBuffer->Release();
#endif
    pVertexBuffer=0;
  }
}

HRESULT  NIQuad::SetActive(){
  _pDevice->SetStreamSource( 0, pVertexBuffer, 0, vertexsize );
  return _pDevice->SetFVF( fvf );
}

HRESULT  NIQuad::Draw(){
  HRESULT hr=_pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
  return hr;
}*/

NLQuad::NLQuad(LPDIRECT3DDEVICE9 pDevice, RECT* Rect, int size, int RectOffset, bool IndependentOffset) :
  _pDevice(pDevice)
{
  vertex v[4];
  fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | ((size >= 1)*D3DFVF_TEX1) | ((size >= 2)*D3DFVF_TEX2) | ((size >= 3)*D3DFVF_TEX3) | ((size >= 4)*D3DFVF_TEX4)
    | ((size >= 5)*D3DFVF_TEX5) | ((size >= 6)*D3DFVF_TEX6) | ((size >= 7)*D3DFVF_TEX7) | ((size >= 8)*D3DFVF_TEX8);

  for (int i = 0; i < 4; i++) {
    v[i].rhw = 1.0f;
    v[i].z = 0.5f;
    v[i].color = 0x00808000;
  }
  int W0 = Rect[0].right - Rect[0].left;
  int H0 = Rect[0].bottom - Rect[0].top;
  v[0].x = 0; v[0].y = 0;
  v[1].x = W0; v[1].y = 0;
  v[2].x = 0; v[2].y = H0;
  v[3].x = W0; v[3].y = H0;
  for (int i = 0; i < size; i++) {
    float W = Rect[i + RectOffset].right - Rect[i + RectOffset].left;
    float H = Rect[i + RectOffset].bottom - Rect[i + RectOffset].top;
    float etaW = 0.5 / W + (float)Rect[i + RectOffset].left / W;
    float etaH = 0.5 / H + (float)Rect[i + RectOffset].top / H;
    if (IndependentOffset) {
      W0 = W;
      H0 = H;
    }
    v[0].tex[i].u = etaW; v[0].tex[i].v = etaH;
    v[1].tex[i].u = etaW + W0 / W; v[1].tex[i].v = etaH;
    v[2].tex[i].u = etaW; v[2].tex[i].v = etaH + H0 / H;
    v[3].tex[i].u = etaW + W0 / W; v[3].tex[i].v = etaH + H0 / H;
  }
  vertexsize = sizeof(float)*(4 + 2 * size) + sizeof(int);
  _pDevice->CreateVertexBuffer(4 * vertexsize, D3DUSAGE_WRITEONLY, fvf, D3DPOOL_MANAGED, &pVertexBuffer, NULL);
  VOID* pVertices;
  pVertexBuffer->Lock(0, vertexsize * 4, (void**)&pVertices, 0);
  for (int i = 0; i < 4; i++)
    memcpy((unsigned char*)pVertices + i * vertexsize, &v[i], vertexsize);
  pVertexBuffer->Unlock();
}

NLQuad::~NLQuad() {
  if (pVertexBuffer)
  {
#ifdef _DEBUG
    cerrwin << std::endl << "pVertexBuffer RefCount: " << pVertexBuffer->Release() << std::endl;
    OutputDebugString("Releasing vertexbuffer\n");
#else
    pVertexBuffer->Release();
#endif
    pVertexBuffer = 0;
  }
}

HRESULT  NLQuad::SetActive() {
  _pDevice->SetStreamSource(0, pVertexBuffer, 0, vertexsize);
  return _pDevice->SetFVF(fvf);
}

HRESULT  NLQuad::Draw() {
  HRESULT hr = _pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
  return hr;
}


MQuad::MQuad(LPDIRECT3DDEVICE9 pDevice) :_pDevice(pDevice), pVertexBuffer(0), pIndexBuffer(0), vertexsize(0), fvf(0), trianglecount(0) {}

MQuad::~MQuad() {
  if (pVertexBuffer)
    pVertexBuffer->Release();
  if (pIndexBuffer)
    pIndexBuffer->Release();
}

void MQuad::CreateVB(int width, int height, int bw, int bh, int repx, int repy, int border) {
  //repx++;repy++;
  int borderx = border;
  int bordery = border * 2;
  fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2 | D3DFVF_TEX3;
  int widthRT = bw * repx;
  int heightRT = bh * repy;
  float etaW = 0.5f*(1) / width;
  float etaH = 0.5f*(1) / height;
  float etaWf = 0.5f / bw;
  float etaHf = 0.5f / bh;
  float xoff = (0.5*bw - borderx) / width;
  float yoff = (0.5*bh - bordery) / height;
  vertex3 *v = NEW vertex3[4 * repx*repy];
  short *Indexbuffer = NEW short[6 * repx*repy];
  for (int j = 0, posy = 0, offset = 0, srcoffsety = -0.5*bh, iboffset = 0; j < repy; j++, posy += bh, srcoffsety += bh - 2 * bordery)
    for (int i = 0, posx = 0, srcoffsetx = -0.5*bw; i < repx; i++, offset += 4, posx += bw, srcoffsetx += bw - 2 * borderx) {
      trianglecount++; trianglecount++;
      Indexbuffer[iboffset++] = offset;
      Indexbuffer[iboffset++] = offset + 1;
      Indexbuffer[iboffset++] = offset + 2;
      Indexbuffer[iboffset++] = offset;
      Indexbuffer[iboffset++] = offset + 2;
      Indexbuffer[iboffset++] = offset + 3;

      v[offset].x = posx;
      v[offset].y = posy;
      v[offset].z = 0.5f;
      v[offset].rhw = 1.0f;
      v[offset].color = 0x00808080;
      v[offset].tex[0].u = etaWf;
      v[offset].tex[0].v = etaHf;
      v[offset].tex[1].u = etaW + (float)srcoffsetx / width;
      v[offset].tex[1].v = etaH + (float)srcoffsety / height;
      v[offset].tex[2].u = etaW + (float)srcoffsetx / width + xoff;
      v[offset].tex[2].v = etaH + (float)srcoffsety / height + yoff;

      v[offset + 1].x = (posx + bw);
      v[offset + 1].y = posy;
      v[offset + 1].z = 0.5f;
      v[offset + 1].rhw = 1.0f;
      v[offset + 1].color = 0x00404040;
      v[offset + 1].tex[0].u = etaWf + 1.0;
      v[offset + 1].tex[0].v = etaHf;
      v[offset + 1].tex[1].u = etaW + (float)(srcoffsetx + bw) / width;
      v[offset + 1].tex[1].v = etaH + (float)srcoffsety / height;
      v[offset + 1].tex[2].u = etaW + (float)(srcoffsetx + bw) / width + xoff;
      v[offset + 1].tex[2].v = etaH + (float)srcoffsety / height + yoff;

      v[offset + 2].x = (posx + bw);
      v[offset + 2].y = posy + bh;
      v[offset + 2].z = 0.5f;
      v[offset + 2].rhw = 1.0f;
      v[offset + 2].color = 0x00808080;
      v[offset + 2].tex[0].u = etaWf + 1.0;
      v[offset + 2].tex[0].v = etaHf + 1.0;
      v[offset + 2].tex[1].u = etaW + (float)(srcoffsetx + bw) / width;
      v[offset + 2].tex[1].v = etaH + (float)(srcoffsety + bh) / height;
      v[offset + 2].tex[2].u = etaW + (float)(srcoffsetx + bw) / width + xoff;
      v[offset + 2].tex[2].v = etaH + (float)(srcoffsety + bh) / height + yoff;

      v[offset + 3].x = posx;
      v[offset + 3].y = posy + bh;
      v[offset + 3].z = 0.5f;
      v[offset + 3].rhw = 1.0f;
      v[offset + 3].color = 0x00808080;
      v[offset + 3].tex[0].u = etaWf;
      v[offset + 3].tex[0].v = etaHf + 1.0;
      v[offset + 3].tex[1].u = etaW + (float)srcoffsetx / width;
      v[offset + 3].tex[1].v = etaH + (float)(srcoffsety + bh) / height;
      v[offset + 3].tex[2].u = etaW + (float)srcoffsetx / width + xoff;
      v[offset + 3].tex[2].v = etaH + (float)(srcoffsety + bh) / height + yoff;
    }
  _pDevice->CreateIndexBuffer(6 * repx*repy * sizeof(short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED/*D3DPOOL_DEFAULT*/, &pIndexBuffer, NULL);
  VOID *pIndices, *pVertexes;

  pIndexBuffer->Lock(0, 6 * repx*repy * sizeof(short), &pIndices, 0);
  memcpy(pIndices, Indexbuffer, 6 * repx*repy * sizeof(short));
  pIndexBuffer->Unlock();
  _pDevice->CreateVertexBuffer(4 * repx*repy * sizeof(vertex3), D3DUSAGE_WRITEONLY, fvf, D3DPOOL_MANAGED/*D3DPOOL_DEFAULT*/, &pVertexBuffer, NULL);
  pVertexBuffer->Lock(0, 4 * repx*repy * sizeof(vertex3), &pVertexes, 0);
  memcpy(pVertexes, v, 4 * repx*repy * sizeof(vertex3));
  pVertexBuffer->Unlock();
  vertexsize = sizeof(vertex3);
  delete[] v;
  delete[] Indexbuffer;
}

void MQuad::CreateVBi(int width, int height, int bw, int bh, int repx, int repy, int border) {
  //repx++;repy++;
  int borderx = border;
  int bordery = border * 2;
  fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4;
  int widthSrc = bw * repx;
  int heightSrc = bh * repy;
  float etaW = 0.5f / widthSrc;
  float etaH = 0.5f / heightSrc;
  float etaWf = (0.5f) / bw;
  float etaHf = (0.5f) / bh;
  float xoff = (0.5*bw - borderx) / widthSrc;
  float yoff = (0.5*bh - bordery) / heightSrc;
  int nblockx = (width + (bw - 2 * borderx) - 1) / (bw - 2 * borderx);
  int nblocky = (height + (bh - 2 * bordery) - 1) / (bh - 2 * bordery);
  int nblock = nblockx * nblocky;
  vertex4 *v = NEW vertex4[4 * 4 * nblock];
  short *Indexbuffer = NEW short[4 * 6 * nblock];
  for (int j = 0, posy = 0, offset = 0, srcoffsety = bordery, iboffset = 0; j < nblocky; j++, posy += bh - 2 * bordery, srcoffsety += bh)
    for (int i = 0, posx = 0, srcoffsetx = borderx; i < nblockx; i++, posx += bw - 2 * borderx, srcoffsetx += bw) {
      for (int j1 = 0; j1 < 2; j1++)
        for (int i1 = 0; i1 < 2; i1++, offset += 4) {
          trianglecount++; trianglecount++;
          Indexbuffer[iboffset++] = offset;
          Indexbuffer[iboffset++] = offset + 1;
          Indexbuffer[iboffset++] = offset + 2;
          Indexbuffer[iboffset++] = offset;
          Indexbuffer[iboffset++] = offset + 2;
          Indexbuffer[iboffset++] = offset + 3;
          v[offset].x = posx + i1 * (0.5*bw - borderx);
          v[offset].y = posy + j1 * (0.5*bh - bordery);
          v[offset].z = 0.5f;
          v[offset].rhw = 1.0f;
          v[offset].color = 0x00808080;
          v[offset].tex[2].u = etaWf + (borderx + i1 * (0.5*bw - borderx)) / bw;
          v[offset].tex[2].v = etaHf + (bordery + j1 * (0.5*bh - bordery)) / bh;
          v[offset].tex[3].u = etaWf + 0.5*(1 - i1) + i1 * (float)borderx / bw;
          v[offset].tex[3].v = etaHf + 0.5*(1 - j1) + j1 * (float)bordery / bh;
          v[offset].tex[0].u = etaW + (float)(srcoffsetx + i1 * (0.5*bw - borderx)) / widthSrc;
          v[offset].tex[0].v = etaH + (float)(srcoffsety + j1 * (0.5*bh - bordery)) / heightSrc;
          v[offset].tex[1].u = etaW + (float)(srcoffsetx + i1 * (0.5*bw + borderx)) / widthSrc + xoff;
          v[offset].tex[1].v = etaH + (float)(srcoffsety + j1 * (0.5*bh + bordery)) / heightSrc + yoff;

          v[offset + 1].x = posx + 0.5*(1 + i1)*(bw - 2 * borderx);
          v[offset + 1].y = posy + j1 * (0.5*bh - bordery);
          v[offset + 1].z = 0.5f;
          v[offset + 1].rhw = 1.0f;
          v[offset + 1].color = 0x00404040;
          v[offset + 1].tex[2].u = etaWf + (borderx + 0.5*(1 + i1)*(bw - 2 * borderx)) / bw;
          v[offset + 1].tex[2].v = etaHf + (bordery + j1 * (0.5*bh - bordery)) / bh;
          v[offset + 1].tex[3].u = etaWf + (float)(i1*0.5*bw + (1 - i1)*(bw - borderx)) / bw;
          v[offset + 1].tex[3].v = etaHf + 0.5*(1 - j1) + j1 * (float)bordery / bh;
          v[offset + 1].tex[0].u = etaW + (float)(srcoffsetx + (1 + i1)*(0.5*bw - borderx)) / widthSrc;
          v[offset + 1].tex[0].v = etaH + (float)(srcoffsety + j1 * (0.5*bh - bordery)) / heightSrc;
          v[offset + 1].tex[1].u = etaW + (float)(srcoffsetx + (0.5*bw - borderx) + i1 * (0.5*bw + borderx)) / widthSrc + xoff;
          v[offset + 1].tex[1].v = etaH + (float)(srcoffsety + j1 * (0.5*bh + bordery)) / heightSrc + yoff;

          v[offset + 2].x = posx + 0.5*(1 + i1)*(bw - 2 * borderx);
          v[offset + 2].y = posy + 0.5*(1 + j1)*(bh - 2 * bordery);
          v[offset + 2].z = 0.5f;
          v[offset + 2].rhw = 1.0f;
          v[offset + 2].color = 0x00808080;
          v[offset + 2].tex[2].u = etaWf + (borderx + 0.5*(1 + i1)*(bw - 2 * borderx)) / bw;
          v[offset + 2].tex[2].v = etaHf + (bordery + 0.5*(1 + j1)*(bh - 2 * bordery)) / bh;
          v[offset + 2].tex[3].u = etaWf + (float)(i1*0.5*bw + (1 - i1)*(bw - borderx)) / bw;
          v[offset + 2].tex[3].v = etaHf + (float)(j1*0.5*bh + (1 - j1)*(bh - bordery)) / bh;
          v[offset + 2].tex[0].u = etaW + (float)(srcoffsetx + (1 + i1)*(0.5*bw - borderx)) / widthSrc;
          v[offset + 2].tex[0].v = etaH + (float)(srcoffsety + (1 + j1)*(0.5*bh - bordery)) / heightSrc;
          v[offset + 2].tex[1].u = etaW + (float)(srcoffsetx + (0.5*bw - borderx) + i1 * (0.5*bw + borderx)) / widthSrc + xoff;
          v[offset + 2].tex[1].v = etaH + (float)(srcoffsety + (0.5*bh - bordery) + j1 * (0.5*bh + bordery)) / heightSrc + yoff;

          v[offset + 3].x = posx + i1 * (0.5*bw - borderx);
          v[offset + 3].y = posy + 0.5*(1 + j1)*(bh - 2 * bordery);
          v[offset + 3].z = 0.5f;
          v[offset + 3].rhw = 1.0f;
          v[offset + 3].color = 0x00808080;
          v[offset + 3].tex[2].u = etaWf + (borderx + i1 * (0.5*bw - borderx)) / bw;
          v[offset + 3].tex[2].v = etaHf + (bordery + 0.5*(1 + j1)*(bh - 2 * bordery)) / bh;
          v[offset + 3].tex[3].u = etaWf + 0.5*(1 - i1) + i1 * (float)borderx / bw;
          v[offset + 3].tex[3].v = etaHf + (float)(j1*0.5*bh + (1 - j1)*(bh - bordery)) / bh;
          v[offset + 3].tex[0].u = etaW + (float)(srcoffsetx + i1 * (0.5*bw - borderx)) / widthSrc;
          v[offset + 3].tex[0].v = etaH + (float)(srcoffsety + (1 + j1)*(0.5*bh - bordery)) / heightSrc;
          v[offset + 3].tex[1].u = etaW + (float)(srcoffsetx + i1 * (0.5*bw + borderx)) / widthSrc + xoff;
          v[offset + 3].tex[1].v = etaH + (float)(srcoffsety + (0.5*bh - bordery) + j1 * (0.5*bh + bordery)) / heightSrc + yoff;
        }
    }
  _pDevice->CreateIndexBuffer(4 * 6 * nblock * sizeof(short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED/*D3DPOOL_DEFAULT*/, &pIndexBuffer, NULL);
  VOID *pIndices, *pVertexes;

  pIndexBuffer->Lock(0, 4 * 6 * nblock * sizeof(short), &pIndices, 0);
  memcpy(pIndices, Indexbuffer, 4 * 6 * nblock * sizeof(short));
  pIndexBuffer->Unlock();
  _pDevice->CreateVertexBuffer(4 * 4 * nblock * sizeof(vertex4), D3DUSAGE_WRITEONLY, fvf, D3DPOOL_MANAGED/*D3DPOOL_DEFAULT*/, &pVertexBuffer, NULL);
  pVertexBuffer->Lock(0, 4 * 4 * nblock * sizeof(vertex4), &pVertexes, 0);
  memcpy(pVertexes, v, 4 * 4 * nblock * sizeof(vertex4));
  pVertexBuffer->Unlock();
  vertexsize = sizeof(vertex4);
  delete[] v;
  delete[] Indexbuffer;
}


HRESULT MQuad::SetActive() {
  _pDevice->SetStreamSource(0, pVertexBuffer, 0, vertexsize);
  _pDevice->SetIndices(pIndexBuffer);
  return _pDevice->SetFVF(fvf);
}

HRESULT MQuad::Draw() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, trianglecount * 2, 0, trianglecount);
}

OQuad::OQuad(LPDIRECT3DDEVICE9 pDevice) :_pDevice(pDevice), pVertexBuffer(0), pIndexBuffer(0), vertexsize(0), fvf(0) {}

OQuad::~OQuad() {
  if (pVertexBuffer)
    pVertexBuffer->Release();
  if (pIndexBuffer)
    pIndexBuffer->Release();
}

//  vertex0 x----x vertex1
//			|	/|
//			|  / |
//			| /  |
//			|/   |
//  vertex2 x----x vertex3
void OQuad::CreateVB(int width, int height, int bw, int bh, int ow, int oh) {
  int nx = ((width + ow - 1) / (bw - ow) + 1 + 1) / 2;
  int ny = ((height + oh - 1) / (bh - oh) + 1 + 1) / 2;
  vertex3 *v = NEW vertex3[4 * nx*ny];
  fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2 | D3DFVF_TEX3;
  short *Indexbuffer = NEW short[6 * nx*ny];
  int index = 0;
  int voffset = 0;
  float etaSourceW = 0.5 / width;
  float etaSourceH = 0.5 / height;
  float etaFactorW = 0.5 / bw;
  float etaFactorH = 0.5 / bh;
  for (int y = 0, yoffset = 0, srctexoffsety = -oh; y < ny; y++, yoffset += bh, srctexoffsety += 2 * (bh - oh))
    for (int x = 0, xoffset = 0, srctexoffsetx = -ow; x < nx; x++, xoffset += bw, srctexoffsetx += 2 * (bw - ow))
    {
      //clockwise
      Indexbuffer[index++] = voffset;//vertex0
      Indexbuffer[index++] = voffset + 1;//vertex1
      Indexbuffer[index++] = voffset + 2;//vertex2
      Indexbuffer[index++] = voffset + 1;//vertex1 
      Indexbuffer[index++] = voffset + 3;//vertex2
      Indexbuffer[index++] = voffset + 2;//vertex3
      //vertex0
      v[voffset].x = xoffset;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00FFFF00;
      v[voffset].tex[0].u = etaSourceW + srctexoffsetx / (float)width;
      v[voffset].tex[0].v = etaSourceH + srctexoffsety / (float)height;
      v[voffset].tex[1].u = etaSourceW + (srctexoffsetx + bw - ow) / (float)width;
      v[voffset].tex[1].v = etaSourceH + (srctexoffsety) / (float)height;
      v[voffset].tex[2].u = etaFactorW;
      v[voffset].tex[2].v = etaFactorH;
      voffset++;
      //vertex1
      v[voffset].x = xoffset + bw;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00FFFF00;
      v[voffset].tex[0].u = etaSourceW + (srctexoffsetx + bw) / (float)width;
      v[voffset].tex[0].v = etaSourceH + srctexoffsety / (float)height;
      v[voffset].tex[1].u = etaSourceW + (srctexoffsetx + 2 * bw - ow) / (float)width;
      v[voffset].tex[1].v = etaSourceH + (srctexoffsety) / (float)height;
      v[voffset].tex[2].u = etaFactorW + 1.0;
      v[voffset].tex[2].v = etaFactorH;
      voffset++;
      //vertex2
      v[voffset].x = xoffset;
      v[voffset].y = yoffset + bh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00FFFF00;
      v[voffset].tex[0].u = etaSourceW + srctexoffsetx / (float)width;
      v[voffset].tex[0].v = etaSourceH + (srctexoffsety + bh) / (float)height;
      v[voffset].tex[1].u = etaSourceW + (srctexoffsetx + bw - ow) / (float)width;
      v[voffset].tex[1].v = etaSourceH + (srctexoffsety + bh) / (float)height;
      v[voffset].tex[2].u = etaFactorW;
      v[voffset].tex[2].v = etaFactorH + 1.0;
      voffset++;
      //vertex3
      v[voffset].x = xoffset + bw;
      v[voffset].y = yoffset + bh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00FFFF00;
      v[voffset].tex[0].u = etaSourceW + (srctexoffsetx + bw) / (float)width;
      v[voffset].tex[0].v = etaSourceH + (srctexoffsety + bh) / (float)height;
      v[voffset].tex[1].u = etaSourceW + (srctexoffsetx + 2 * bw - ow) / (float)width;
      v[voffset].tex[1].v = etaSourceH + (srctexoffsety + bh) / (float)height;
      v[voffset].tex[2].u = etaFactorW + 1.0;
      v[voffset].tex[2].v = etaFactorH + 1.0;
      voffset++;
    }
  nblocks = nx * ny;


  _pDevice->CreateIndexBuffer(index * sizeof(short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pIndexBuffer, NULL);
  VOID *pIndices, *pVertexes;

  pIndexBuffer->Lock(0, index * sizeof(short), &pIndices, 0);
  memcpy(pIndices, Indexbuffer, index * sizeof(short));
  pIndexBuffer->Unlock();
  _pDevice->CreateVertexBuffer(voffset * sizeof(vertex3), D3DUSAGE_WRITEONLY, fvf, D3DPOOL_MANAGED, &pVertexBuffer, NULL);
  pVertexBuffer->Lock(0, voffset * sizeof(vertex3), &pVertexes, 0);
  memcpy(pVertexes, v, voffset * sizeof(vertex3));
  pVertexBuffer->Unlock();
  vertexsize = sizeof(vertex3);
  delete[] v;
  delete[] Indexbuffer;
}

//vertical block=# horizontalblock=* nooverlap block=0 cornerblock=%
//					
//Grid:	/-------width----------\
//	/	0000#0000#0000#0000#0000
//	|	0000#0000#0000#0000#0000
//	|	0000#0000#0000#0000#0000
//	|	0000#0000#0000#0000#0000
//	h	****%****%****%****%****
//	e	0000#0000#0000#0000#0000
//	i	0000#0000#0000#0000#0000
//	g	0000#0000#0000#0000#0000
//	h	0000#0000#0000#0000#0000
//	t	****%****%****%****%****
//	|	0000#0000#0000#0000#0000
//	|	0000#0000#0000#0000#0000
//	|	0000#0000#0000#0000#0000
//	|	0000#0000#0000#0000#0000
//	\	****%****%****%****%****

//Factor texture: xy=xy factor,zw=zw factor

void OQuad::CreateVBi(int width, int height, int bw, int bh, int ow, int oh, int srcwidth, int srcheight) {
  //two pixels per texture pixel

  int nverticaloverlapblocksx = (width - (bw - 2 * ow) - 1) / (bw - ow) + 1;
  int nverticaloverlapblocksy = (height - 1) / (bh - oh) + 1;
  int nhorizontaloverlapblocksx = (width - 1) / (bw - ow) + 1;
  int nhorizontaloverlapblocksy = (height - (bh - 2 * oh) - 1) / (bh - oh) + 1;
  int nnonoverlapblocksx = (width - 1) / (bw - ow) + 1;
  int nnonoverlapblocksy = (height - 1) / (bh - oh) + 1;
  float etaFactorW = 0.5 / bw;
  float etaFactorH = 0.5 / bh;
  float etaSrcW = 0.5 / srcwidth;
  float etaSrcH = 0.5 / srcheight;
  ncornerblock = (nverticaloverlapblocksx*nhorizontaloverlapblocksy);
  int totvertex = 4 * (
    (nverticaloverlapblocksx*nverticaloverlapblocksy) +
    (nhorizontaloverlapblocksx*nhorizontaloverlapblocksy) +
    (nnonoverlapblocksx*nnonoverlapblocksy) +
    ncornerblock
    //corner blocks
    )
    ;
  int totindex = 6 * totvertex / 4;
  fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4;
  vertex4 *v = NEW vertex4[totvertex];
  memset(v, 0, totvertex * sizeof(v));

  short *Indexbuffer = NEW short[totindex];
  memset(Indexbuffer, 0, totindex * sizeof(short));
  int index = 0;
  int voffset = 0;
  //  vertex0 x----x vertex1
  //			|	/|
  //			|  / |
  //			| /  |
  //			|/   |
  //  vertex2 x----x vertex3


  //Setup corner blocks
  startindexcornerblock = 0;
  startvertexcornerblock = 0;
  for (int y = 0, yoffset = bh - 2 * oh, ytex0offset = bh - oh, ytex1offset = 0, ytex0factoroffset = bh - oh, ytex1factoroffset = 0; y < nhorizontaloverlapblocksy;
    y++, yoffset += bh - oh, ytex0offset += (y & 1 ? oh : bh - oh), ytex1offset += (y & 1 ? bh - oh : oh), ytex0factoroffset = (y & 1 ? 0 : bh - oh), ytex1factoroffset = (y & 1 ? bh - oh : 0))
    for (int x = 0, xoffset = bw - 2 * ow, xtex0offset = bw - ow, xtex1offset = 0, xtex0factoroffset = bw - ow, xtex1factoroffset = 0; x < nverticaloverlapblocksx;
      x++, xoffset += bw - ow, xtex0offset += (x & 1 ? ow : bw - ow), xtex1offset += (x & 1 ? bw - ow : ow), xtex0factoroffset = (x & 1 ? 0 : bw - ow), xtex1factoroffset = (x & 1 ? bw - ow : 0))
  {
    //clockwise
    Indexbuffer[index++] = voffset;//vertex0
    Indexbuffer[index++] = voffset + 1;//vertex1
    Indexbuffer[index++] = voffset + 2;//vertex2
    Indexbuffer[index++] = voffset + 1;//vertex1 
    Indexbuffer[index++] = voffset + 3;//vertex2
    Indexbuffer[index++] = voffset + 2;//vertex3
    //vertex0
    v[voffset].x = xoffset;
    v[voffset].y = yoffset;
    v[voffset].z = 0.5f;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00FFFF00;
    v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//tex0
    v[voffset].tex[0].v = etaSrcH + ytex0offset / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + xtex1offset / (float)srcwidth;//tex1
    v[voffset].tex[1].v = etaSrcH + ytex1offset / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + ytex0factoroffset / (float)bh;
    v[voffset].tex[3].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture1
    v[voffset].tex[3].v = etaFactorH + ytex1factoroffset / (float)bh;
    voffset++;
    //vertex1
    v[voffset].x = xoffset + ow;
    v[voffset].y = yoffset;
    v[voffset].z = 0.5f;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00FFFF00;
    v[voffset].tex[0].u = etaSrcW + (xtex0offset + ow) / (float)srcwidth;
    v[voffset].tex[0].v = etaSrcH + ytex0offset / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + (xtex1offset + ow) / (float)srcwidth;
    v[voffset].tex[1].v = etaSrcH + ytex1offset / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + ytex0factoroffset / (float)bh;
    v[voffset].tex[3].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture1
    v[voffset].tex[3].v = etaFactorH + ytex1factoroffset / (float)bh;
    voffset++;
    //vertex2
    v[voffset].x = xoffset;
    v[voffset].y = yoffset + oh;
    v[voffset].z = 0.5f;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00FFFF00;
    v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;
    v[voffset].tex[0].v = etaSrcH + (ytex0offset + oh) / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + xtex1offset / (float)srcwidth;
    v[voffset].tex[1].v = etaSrcH + (ytex1offset + oh) / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + (ytex0factoroffset + oh) / (float)bh;
    v[voffset].tex[3].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture1
    v[voffset].tex[3].v = etaFactorH + (ytex1factoroffset + oh) / (float)bh;
    voffset++;
    //vertex3
    v[voffset].x = xoffset + ow;
    v[voffset].y = yoffset + oh;
    v[voffset].z = 0.5f;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00FFFF00;
    v[voffset].tex[0].u = etaSrcW + (xtex0offset + ow) / (float)srcwidth;
    v[voffset].tex[0].v = etaSrcH + (ytex0offset + oh) / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + (xtex1offset + ow) / (float)srcwidth;
    v[voffset].tex[1].v = etaSrcH + (ytex1offset + oh) / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + (ytex0factoroffset + oh) / (float)bh;
    v[voffset].tex[3].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture1
    v[voffset].tex[3].v = etaFactorH + (ytex1factoroffset + oh) / (float)bh;
    voffset++;
  }

  //Setup nonoverlap blocks
  //first texture1 xy nonoverlap
  nnonoverlaptex1xyblock = ((nnonoverlapblocksy + 1) / 2)*((nnonoverlapblocksx + 1) / 2);
  startindexnonoverlaptex1xyblock = index;
  startvertexnonoverlaptex1xyblock = voffset;
  for (int y = 0, yoffset = 0, ytexoffset = oh; y < (nnonoverlapblocksy + 1) / 2; y++, yoffset += 2 * (bh - oh), ytexoffset += bh)
    for (int x = 0, xoffset = 0, xtexoffset = ow; x < (nnonoverlapblocksx + 1) / 2; x++, xoffset += 2 * (bw - ow), xtexoffset += bw)
    {
      //clockwise
      Indexbuffer[index++] = voffset;//vertex0
      Indexbuffer[index++] = voffset + 1;//vertex1
      Indexbuffer[index++] = voffset + 2;//vertex2
      Indexbuffer[index++] = voffset + 1;//vertex1 
      Indexbuffer[index++] = voffset + 3;//vertex2
      Indexbuffer[index++] = voffset + 2;//vertex3
      //vertex0
      v[voffset].x = xoffset;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFFF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex1
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFFF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex2
      v[voffset].x = xoffset;
      v[voffset].y = yoffset + bh - 2 * oh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFFF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex3
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset + bh - 2 * oh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFFF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
    }
  //texture1 zw nonoverlap
  nnonoverlaptex1zwblock = ((nnonoverlapblocksy + 1) / 2)*((nnonoverlapblocksx) / 2);
  startindexnonoverlaptex1zwblock = index;
  startvertexnonoverlaptex1zwblock = voffset;
  for (int y = 0, yoffset = 0, ytexoffset = oh; y < (nnonoverlapblocksy + 1) / 2; y++, yoffset += 2 * (bh - oh), ytexoffset += bh)
    for (int x = 0, xoffset = bw - ow, xtexoffset = ow; x < (nnonoverlapblocksx) / 2; x++, xoffset += 2 * (bw - ow), xtexoffset += bw)
    {
      //clockwise
      Indexbuffer[index++] = voffset;//vertex0
      Indexbuffer[index++] = voffset + 1;//vertex1
      Indexbuffer[index++] = voffset + 2;//vertex2
      Indexbuffer[index++] = voffset + 1;//vertex1 
      Indexbuffer[index++] = voffset + 3;//vertex2
      Indexbuffer[index++] = voffset + 2;//vertex3
      //vertex0
      v[voffset].x = xoffset;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFDF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex1
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFDF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex2
      v[voffset].x = xoffset;
      v[voffset].y = yoffset + bh - 2 * oh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFDF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex3
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset + bh - 2 * oh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFDF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
    }
  //texture2 xy nonoverlap
  nnonoverlaptex2xyblock = ((nnonoverlapblocksy) / 2)*((nnonoverlapblocksx + 1) / 2);
  startindexnonoverlaptex2xyblock = index;
  startvertexnonoverlaptex2xyblock = voffset;
  for (int y = 0, yoffset = bh - oh, ytexoffset = oh; y < (nnonoverlapblocksy) / 2; y++, yoffset += 2 * (bh - oh), ytexoffset += bh)
    for (int x = 0, xoffset = 0, xtexoffset = ow; x < (nnonoverlapblocksx + 1) / 2; x++, xoffset += 2 * (bw - ow), xtexoffset += bw)
    {
      //clockwise
      Indexbuffer[index++] = voffset;//vertex0
      Indexbuffer[index++] = voffset + 1;//vertex1
      Indexbuffer[index++] = voffset + 2;//vertex2
      Indexbuffer[index++] = voffset + 1;//vertex1 
      Indexbuffer[index++] = voffset + 3;//vertex2
      Indexbuffer[index++] = voffset + 2;//vertex3
      //vertex0
      v[voffset].x = xoffset;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFFF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex1
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFFF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex2
      v[voffset].x = xoffset;
      v[voffset].y = yoffset + bh - 2 * oh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFFF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex3
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset + bh - 2 * oh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFFF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
    }
  //texture2 zw nonoverlap
  nnonoverlaptex2zwblock = ((nnonoverlapblocksy) / 2)*((nnonoverlapblocksx) / 2);
  startindexnonoverlaptex2zwblock = index;
  startvertexnonoverlaptex2zwblock = voffset;
  for (int y = 0, yoffset = bh - oh, ytexoffset = oh; y < (nnonoverlapblocksy) / 2; y++, yoffset += 2 * (bh - oh), ytexoffset += bh)
    for (int x = 0, xoffset = bw - ow, xtexoffset = ow; x < (nnonoverlapblocksx) / 2; x++, xoffset += 2 * (bw - ow), xtexoffset += bw)
    {
      //clockwise
      Indexbuffer[index++] = voffset;//vertex0
      Indexbuffer[index++] = voffset + 1;//vertex1
      Indexbuffer[index++] = voffset + 2;//vertex2
      Indexbuffer[index++] = voffset + 1;//vertex1 
      Indexbuffer[index++] = voffset + 3;//vertex2
      Indexbuffer[index++] = voffset + 2;//vertex3
      //vertex0
      v[voffset].x = xoffset;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFDF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex1
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFDF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex2
      v[voffset].x = xoffset;
      v[voffset].y = yoffset + bh - 2 * oh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFDF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex3
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset + bh - 2 * oh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFDF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
    }

  //xy horizontal blocks
  nhorizontalxyblock = ((nhorizontaloverlapblocksx + 1) / 2)*(nhorizontaloverlapblocksy);
  startindexhorizontalxyblock = index;
  startvertexhorizontalxyblock = voffset;
  for (int y = 0, yoffset = bh - 2 * oh, ytex0offset = bh - oh, ytex1offset = 0, ytex0factoroffset = bh - oh, ytex1factoroffset = 0; y < nhorizontaloverlapblocksy;
    y++, yoffset += (bh - oh), ytex0offset += (y & 1 ? oh : bh - oh), ytex1offset += (y & 1 ? bh - oh : oh), ytex0factoroffset = (y & 1 ? 0 : bh - oh), ytex1factoroffset = (y & 1 ? bh - oh : 0))
    for (int x = 0, xoffset = 0, xtex0offset = ow; x < (nhorizontaloverlapblocksx + 1) / 2; x++, xoffset += 2 * (bw - ow), xtex0offset += bw)
    {
      //clockwise
      Indexbuffer[index++] = voffset;//vertex0
      Indexbuffer[index++] = voffset + 1;//vertex1
      Indexbuffer[index++] = voffset + 2;//vertex2
      Indexbuffer[index++] = voffset + 1;//vertex1 
      Indexbuffer[index++] = voffset + 3;//vertex2
      Indexbuffer[index++] = voffset + 2;//vertex3
      //vertex0
      v[voffset].x = xoffset;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00FF0000;
      v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + ytex0offset / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + xtex0offset / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + ytex1offset / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + ow / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + ytex0factoroffset / (float)bh;
      v[voffset].tex[3].u = etaFactorW + ow / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + ytex1factoroffset / (float)bh;
      voffset++;
      //TODO: texcoord
      //vertex1
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00FF0000;
      v[voffset].tex[0].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + ytex0offset / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + ytex1offset / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + ytex0factoroffset / (float)bh;
      v[voffset].tex[3].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + ytex1factoroffset / (float)bh;
      voffset++;
      //TODO: texcoord
      //vertex2
      v[voffset].x = xoffset;
      v[voffset].y = yoffset + oh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00FF0000;
      v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + (ytex0offset + oh) / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + xtex0offset / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + (ytex1offset + oh) / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + ow / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + (ytex0factoroffset + oh) / (float)bh;
      v[voffset].tex[3].u = etaFactorW + ow / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + (ytex1factoroffset + oh) / (float)bh;
      voffset++;
      //TODO: texcoord
      //vertex3
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset + oh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00FF0000;
      v[voffset].tex[0].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + (ytex0offset + oh) / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + (ytex1offset + oh) / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + (ytex0factoroffset + oh) / (float)bh;
      v[voffset].tex[3].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + (ytex1factoroffset + oh) / (float)bh;
      voffset++;
      //TODO: texcoord
    }
  //zw horizontal blocks
  nhorizontalzwblock = ((nhorizontaloverlapblocksx) / 2)*(nhorizontaloverlapblocksy);
  startindexhorizontalzwblock = index;
  startvertexhorizontalzwblock = voffset;
  for (int y = 0, yoffset = bh - 2 * oh, ytex0offset = bh - oh, ytex1offset = 0, ytex0factoroffset = bh - oh, ytex1factoroffset = 0; y < nhorizontaloverlapblocksy;
    y++, yoffset += (bh - oh), ytex0offset += (y & 1 ? oh : bh - oh), ytex1offset += (y & 1 ? bh - oh : oh), ytex0factoroffset = (y & 1 ? 0 : bh - oh), ytex1factoroffset = (y & 1 ? bh - oh : 0))
    for (int x = 0, xoffset = bw - ow, xtex0offset = ow; x < (nhorizontaloverlapblocksx) / 2; x++, xoffset += 2 * (bw - ow), xtex0offset += bw)
    {
      //clockwise
      Indexbuffer[index++] = voffset;//vertex0
      Indexbuffer[index++] = voffset + 1;//vertex1
      Indexbuffer[index++] = voffset + 2;//vertex2
      Indexbuffer[index++] = voffset + 1;//vertex1 
      Indexbuffer[index++] = voffset + 3;//vertex2
      Indexbuffer[index++] = voffset + 2;//vertex3
      //vertex0
      v[voffset].x = xoffset;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00DF0000;
      v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + ytex0offset / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + xtex0offset / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + ytex1offset / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + ow / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + ytex0factoroffset / (float)bh;
      v[voffset].tex[3].u = etaFactorW + ow / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + ytex1factoroffset / (float)bh;
      voffset++;
      //TODO: texcoord
      //vertex1
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00DF0000;
      v[voffset].tex[0].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + ytex0offset / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + ytex1offset / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + ytex0factoroffset / (float)bh;
      v[voffset].tex[3].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + ytex1factoroffset / (float)bh;
      voffset++;
      //TODO: texcoord
      //vertex2
      v[voffset].x = xoffset;
      v[voffset].y = yoffset + oh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00DF0000;
      v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + (ytex0offset + oh) / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + xtex0offset / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + (ytex1offset + oh) / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + ow / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + (ytex0factoroffset + oh) / (float)bh;
      v[voffset].tex[3].u = etaFactorW + ow / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + (ytex1factoroffset + oh) / (float)bh;
      voffset++;
      //TODO: texcoord
      //vertex3
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset + oh;
      v[voffset].z = 0.5f;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00DF0000;
      v[voffset].tex[0].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + (ytex0offset + oh) / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + (ytex1offset + oh) / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + (ytex0factoroffset + oh) / (float)bh;
      v[voffset].tex[3].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + (ytex1factoroffset + oh) / (float)bh;
      voffset++;
      //TODO: texcoord
    }
  //vertical blocks tex1
  nverticaltex1block = (nverticaloverlapblocksx)*((nverticaloverlapblocksy + 1) / 2);
  startindexverticaltex1block = index;
  startvertexverticaltex1block = voffset;
  for (int y = 0, yoffset = 0, ytexoffset = oh; y < (nverticaloverlapblocksy + 1) / 2; y++, yoffset += 2 * (bh - oh), ytexoffset += bh)
    for (int x = 0, xoffset = bw - 2 * ow, xtex0offset = bw - ow, xtex1offset = 0, xtex0factoroffset = bw - ow; x < (nverticaloverlapblocksx);
      x++, xoffset += (bw - ow), xtex0offset += (x & 1 ? ow : bw - ow), xtex1offset += (x & 1 ? bw - ow : ow), xtex0factoroffset = (x & 1 ? 0 : bw - ow))
  {
    //clockwise
    Indexbuffer[index++] = voffset;//vertex0
    Indexbuffer[index++] = voffset + 1;//vertex1
    Indexbuffer[index++] = voffset + 2;//vertex2
    Indexbuffer[index++] = voffset + 1;//vertex1 
    Indexbuffer[index++] = voffset + 3;//vertex2
    Indexbuffer[index++] = voffset + 2;//vertex3
    //vertex0
    v[voffset].x = xoffset;
    v[voffset].y = yoffset;
    v[voffset].z = 0.5f;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + xtex1offset / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + oh / (float)bh;
    voffset++;
    //vertex1
    v[voffset].x = xoffset + ow;
    v[voffset].y = yoffset;
    v[voffset].z = 0.5f;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + (xtex0offset + ow) / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + (xtex1offset + ow) / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + oh / (float)bh;
    voffset++;
    //vertex2
    v[voffset].x = xoffset;
    v[voffset].y = yoffset + bh - 2 * oh;
    v[voffset].z = 0.5f;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + xtex1offset / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + (bh - oh) / (float)bh;
    voffset++;
    //vertex3
    v[voffset].x = xoffset + ow;
    v[voffset].y = yoffset + bh - 2 * oh;
    v[voffset].z = 0.5f;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + (xtex0offset + ow) / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + (xtex1offset + ow) / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + (bh - oh) / (float)bh;
    voffset++;
  }
  //vertical blocks tex2
  nverticaltex2block = (nverticaloverlapblocksx)*((nverticaloverlapblocksy) / 2);
  startindexverticaltex2block = index;
  startvertexverticaltex2block = voffset;
  for (int y = 0, yoffset = (bh - oh), ytexoffset = oh; y < (nverticaloverlapblocksy) / 2; y++, yoffset += 2 * (bh - oh), ytexoffset += bh)
    for (int x = 0, xoffset = bw - 2 * ow, xtex0offset = bw - ow, xtex1offset = 0, xtex0factoroffset = bw - ow; x < (nverticaloverlapblocksx);
      x++, xoffset += (bw - ow), xtex0offset += (x & 1 ? ow : bw - ow), xtex1offset += (x & 1 ? bw - ow : ow), xtex0factoroffset = (x & 1 ? 0 : bw - ow))
  {
    //clockwise
    Indexbuffer[index++] = voffset;//vertex0
    Indexbuffer[index++] = voffset + 1;//vertex1
    Indexbuffer[index++] = voffset + 2;//vertex2
    Indexbuffer[index++] = voffset + 1;//vertex1 
    Indexbuffer[index++] = voffset + 3;//vertex2
    Indexbuffer[index++] = voffset + 2;//vertex3
    //vertex0
    v[voffset].x = xoffset;
    v[voffset].y = yoffset;
    v[voffset].z = 0.5f;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + xtex1offset / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + oh / (float)bh;
    voffset++;
    //vertex1
    v[voffset].x = xoffset + ow;
    v[voffset].y = yoffset;
    v[voffset].z = 0.5f;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + (xtex0offset + ow) / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + (xtex1offset + ow) / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + oh / (float)bh;
    voffset++;
    //vertex2
    v[voffset].x = xoffset;
    v[voffset].y = yoffset + bh - 2 * oh;
    v[voffset].z = 0.5f;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + xtex1offset / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + (bh - oh) / (float)bh;
    voffset++;
    //vertex3
    v[voffset].x = xoffset + ow;
    v[voffset].y = yoffset + bh - 2 * oh;
    v[voffset].z = 0.5f;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + (xtex0offset + ow) / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + (xtex1offset + ow) / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + (bh - oh) / (float)bh;
    voffset++;
    //TODO: texcoord
  }
  _pDevice->CreateIndexBuffer(totindex * sizeof(short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED/*D3DPOOL_DEFAULT*/, &pIndexBuffer, NULL);
  VOID *pIndices, *pVertexes;

  pIndexBuffer->Lock(0, totindex * sizeof(short), &pIndices, 0);
  memcpy(pIndices, Indexbuffer, totindex * sizeof(short));
  pIndexBuffer->Unlock();
  _pDevice->CreateVertexBuffer(totvertex * sizeof(vertex4), D3DUSAGE_WRITEONLY, fvf, D3DPOOL_MANAGED/*D3DPOOL_DEFAULT*/, &pVertexBuffer, NULL);
  pVertexBuffer->Lock(0, totvertex * sizeof(vertex4), &pVertexes, 0);
  memcpy(pVertexes, v, totvertex * sizeof(vertex4));
  pVertexBuffer->Unlock();
  vertexsize = sizeof(vertex4);
  delete[] v;
  delete[] Indexbuffer;
}


HRESULT OQuad::SetActive() {
  _pDevice->SetStreamSource(0, pVertexBuffer, 0, vertexsize);
  _pDevice->SetIndices(pIndexBuffer);
  return _pDevice->SetFVF(fvf);
}

HRESULT OQuad::DrawCorners() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexcornerblock, ncornerblock * 4, startindexcornerblock, ncornerblock * 2);
}

HRESULT OQuad::DrawHorizontalxy() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexhorizontalxyblock, nhorizontalxyblock * 4, startindexhorizontalxyblock, nhorizontalxyblock * 2);
}
HRESULT OQuad::DrawHorizontalzw() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexhorizontalzwblock, nhorizontalzwblock * 4, startindexhorizontalzwblock, nhorizontalzwblock * 2);
}

HRESULT OQuad::DrawVerticaltex1() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexverticaltex1block, nverticaltex1block * 4, startindexverticaltex1block, nverticaltex1block * 2);
}
HRESULT OQuad::DrawVerticaltex2() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexverticaltex2block, nverticaltex2block * 4, startindexverticaltex2block, nverticaltex2block * 2);
}

HRESULT OQuad::DrawNonoverlaptex1xy() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexnonoverlaptex1xyblock, nnonoverlaptex1xyblock * 4, startindexnonoverlaptex1xyblock, nnonoverlaptex1xyblock * 2);
}
HRESULT OQuad::DrawNonoverlaptex2xy() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexnonoverlaptex2xyblock, nnonoverlaptex2xyblock * 4, startindexnonoverlaptex2xyblock, nnonoverlaptex2xyblock * 2);
}
HRESULT OQuad::DrawNonoverlaptex1zw() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexnonoverlaptex1zwblock, nnonoverlaptex1zwblock * 4, startindexnonoverlaptex1zwblock, nnonoverlaptex1zwblock * 2);
}
HRESULT OQuad::DrawNonoverlaptex2zw() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexnonoverlaptex2zwblock, nnonoverlaptex2zwblock * 4, startindexnonoverlaptex2zwblock, nnonoverlaptex2zwblock * 2);
}

HRESULT OQuad::Draw()
{
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, nblocks * 4, 0, nblocks * 2);;
}

//*******************
// OIQuad
//*******************

OIQuad::OIQuad(LPDIRECT3DDEVICE9 pDevice) :_pDevice(pDevice), pVertexBuffer(0), pIndexBuffer(0), vertexsize(0), fvf(0) {}

OIQuad::~OIQuad() {
  if (pVertexBuffer)
    pVertexBuffer->Release();
  if (pIndexBuffer)
    pIndexBuffer->Release();
}

//  vertex0 x----x vertex1
//			|	/|
//			|  / |
//			| /  |
//			|/   |
//  vertex2 x----x vertex3
void OIQuad::CreateVB(int width, int height, int bw, int bh, int ow, int oh) {
  int nx = ((width + ow - 1) / (bw - ow) + 1 + 1) / 2;
  int ny = (((height / 2 + oh - 1) / (bh - oh) + 1 + 1) / 2) * 2;
  vertex3 *v = NEW vertex3[2 * 4 * nx*ny];
  fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2 | D3DFVF_TEX3;
  short *Indexbuffer = NEW short[2 * 6 * nx*ny];
  int index = 0;
  int voffset = 0;
  float etaSourceW = 0.5 / width;
  float etaSourceH = 0.5 / height;
  float etaFactorW = 0.5 / bw;
  float etaFactorH = 0.5 / bh;
  for (int i = 0, ystartoffset = 0; i < 2; i++, etaSourceH = 1.5 / height, ystartoffset = ny * bh / 2)
    for (int y = 0, yoffset = ystartoffset, srctexoffsety = -2 * oh; y < ny / 2; y++, yoffset += bh, srctexoffsety += 2 * 2 * (bh - oh))
      for (int x = 0, xoffset = 0, srctexoffsetx = -ow; x < nx; x++, xoffset += bw, srctexoffsetx += 2 * (bw - ow))
      {
        //clockwise
        Indexbuffer[index++] = voffset;//vertex0
        Indexbuffer[index++] = voffset + 1;//vertex1
        Indexbuffer[index++] = voffset + 2;//vertex2
        Indexbuffer[index++] = voffset + 1;//vertex1 
        Indexbuffer[index++] = voffset + 3;//vertex2
        Indexbuffer[index++] = voffset + 2;//vertex3
        //vertex0
        v[voffset].x = xoffset;
        v[voffset].y = yoffset;
        v[voffset].z = 0.5;
        v[voffset].rhw = 1.0;
        v[voffset].color = 0x00FFFF00;
        v[voffset].tex[0].u = etaSourceW + srctexoffsetx / (float)width;//tex[0]=xy
        v[voffset].tex[0].v = etaSourceH + srctexoffsety / (float)height;
        v[voffset].tex[1].u = etaSourceW + (srctexoffsetx + bw - ow) / (float)width;//tex[1]=zw
        v[voffset].tex[1].v = etaSourceH + (srctexoffsety) / (float)height;
        v[voffset].tex[2].u = etaFactorW;//tex[2] factor
        v[voffset].tex[2].v = etaFactorH;
        voffset++;
        //vertex1
        v[voffset].x = xoffset + bw;
        v[voffset].y = yoffset;
        v[voffset].z = 0.5;
        v[voffset].rhw = 1.0;
        v[voffset].color = 0x00FFFF00;
        v[voffset].tex[0].u = etaSourceW + (srctexoffsetx + bw) / (float)width;
        v[voffset].tex[0].v = etaSourceH + srctexoffsety / (float)height;
        v[voffset].tex[1].u = etaSourceW + (srctexoffsetx + 2 * bw - ow) / (float)width;
        v[voffset].tex[1].v = etaSourceH + (srctexoffsety) / (float)height;
        v[voffset].tex[2].u = etaFactorW + 1.0;
        v[voffset].tex[2].v = etaFactorH;
        voffset++;
        //vertex2
        v[voffset].x = xoffset;
        v[voffset].y = yoffset + bh;
        v[voffset].z = 0.5;
        v[voffset].rhw = 1.0;
        v[voffset].color = 0x00FFFF00;
        v[voffset].tex[0].u = etaSourceW + srctexoffsetx / (float)width;
        v[voffset].tex[0].v = etaSourceH + (srctexoffsety + 2 * bh) / (float)height;
        v[voffset].tex[1].u = etaSourceW + (srctexoffsetx + bw - ow) / (float)width;
        v[voffset].tex[1].v = etaSourceH + (srctexoffsety + 2 * bh) / (float)height;
        v[voffset].tex[2].u = etaFactorW;
        v[voffset].tex[2].v = etaFactorH + 1.0;
        voffset++;
        //vertex3
        v[voffset].x = xoffset + bw;
        v[voffset].y = yoffset + bh;
        v[voffset].z = 0.5;
        v[voffset].rhw = 1.0;
        v[voffset].color = 0x00FFFF00;
        v[voffset].tex[0].u = etaSourceW + (srctexoffsetx + bw) / (float)width;
        v[voffset].tex[0].v = etaSourceH + (srctexoffsety + 2 * bh) / (float)height;
        v[voffset].tex[1].u = etaSourceW + (srctexoffsetx + 2 * bw - ow) / (float)width;
        v[voffset].tex[1].v = etaSourceH + (srctexoffsety + 2 * bh) / (float)height;
        v[voffset].tex[2].u = etaFactorW + 1.0;
        v[voffset].tex[2].v = etaFactorH + 1.0;
        voffset++;
      }
  nblocks = nx * ny;


  _pDevice->CreateIndexBuffer(index * sizeof(short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pIndexBuffer, NULL);
  VOID *pIndices, *pVertexes;

  pIndexBuffer->Lock(0, index * sizeof(short), &pIndices, 0);
  memcpy(pIndices, Indexbuffer, index * sizeof(short));
  pIndexBuffer->Unlock();
  _pDevice->CreateVertexBuffer(voffset * sizeof(vertex3), D3DUSAGE_WRITEONLY, fvf, D3DPOOL_MANAGED, &pVertexBuffer, NULL);
  pVertexBuffer->Lock(0, voffset * sizeof(vertex3), &pVertexes, 0);
  memcpy(pVertexes, v, voffset * sizeof(vertex3));
  pVertexBuffer->Unlock();
  vertexsize = sizeof(vertex3);
  delete[] v;
  delete[] Indexbuffer;
}

//vertical block=# horizontalblock=* nooverlap block=0 cornerblock=%
//					
//Grid:	/-------width----------\
//	/	0000#0000#0000#0000#0000
//	|	0000#0000#0000#0000#0000
//	|	0000#0000#0000#0000#0000
//	|	0000#0000#0000#0000#0000
//	h	****%****%****%****%****
//	e	0000#0000#0000#0000#0000
//	i	0000#0000#0000#0000#0000
//	g	0000#0000#0000#0000#0000
//	h	0000#0000#0000#0000#0000
//	t	****%****%****%****%****
//	|	0000#0000#0000#0000#0000
//	|	0000#0000#0000#0000#0000
//	|	0000#0000#0000#0000#0000
//	|	0000#0000#0000#0000#0000
//	\	****%****%****%****%****

//Factor texture: xy=xy factor,zw=zw factor

void OIQuad::CreateVBi(int width, int height, int bw, int bh, int ow, int oh, int srcwidth, int srcheight, bool even) {
  //two pixels per texture pixel

  int nverticaloverlapblocksx = (width - (bw - 2 * ow) - 1) / (bw - ow) + 1;
  int nverticaloverlapblocksy = (height / 2 - 1) / (bh - oh) + 1;
  int nhorizontaloverlapblocksx = (width - 1) / (bw - ow) + 1;
  int nhorizontaloverlapblocksy = (height / 2 - (bh - 2 * oh) - 1) / (bh - oh) + 1;
  int nnonoverlapblocksx = (width - 1) / (bw - ow) + 1;
  int nnonoverlapblocksy = (height / 2 - 1) / (bh - oh) + 1;
  float etaFactorW = 0.5 / bw;
  float etaFactorH = 0.5 / bh;
  float etaSrcW = 0.5 / srcwidth;
  float etaSrcH = 0.5 / srcheight + even * 0.5;
  ncornerblock = (nverticaloverlapblocksx*nhorizontaloverlapblocksy);
  int totvertex = 4 * (
    (nverticaloverlapblocksx*nverticaloverlapblocksy) +
    (nhorizontaloverlapblocksx*nhorizontaloverlapblocksy) +
    (nnonoverlapblocksx*nnonoverlapblocksy) +
    ncornerblock
    //corner blocks
    )
    ;
  int totindex = 6 * totvertex / 4;
  fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4;
  vertex4 *v = NEW vertex4[totvertex];
  memset(v, 0, totvertex * sizeof(v));

  short *Indexbuffer = NEW short[totindex];
  memset(Indexbuffer, 0, totindex * sizeof(short));
  int index = 0;
  int voffset = 0;
  //  vertex0 x----x vertex1
  //			|	/|
  //			|  / |
  //			| /  |
  //			|/   |
  //  vertex2 x----x vertex3


  //Setup corner blocks
  startindexcornerblock = 0;
  startvertexcornerblock = 0;
  for (int y = 0, yoffset = even + 2 * (bh - 2 * oh), ytex0offset = bh - oh, ytex1offset = 0, ytex0factoroffset = bh - oh, ytex1factoroffset = 0; y < nhorizontaloverlapblocksy;
    y++, yoffset += 2 * (bh - oh), ytex0offset += (y & 1 ? oh : bh - oh), ytex1offset += (y & 1 ? bh - oh : oh), ytex0factoroffset = (y & 1 ? 0 : bh - oh), ytex1factoroffset = (y & 1 ? bh - oh : 0))
    for (int x = 0, xoffset = bw - 2 * ow, xtex0offset = bw - ow, xtex1offset = 0, xtex0factoroffset = bw - ow, xtex1factoroffset = 0; x < nverticaloverlapblocksx;
      x++, xoffset += bw - ow, xtex0offset += (x & 1 ? ow : bw - ow), xtex1offset += (x & 1 ? bw - ow : ow), xtex0factoroffset = (x & 1 ? 0 : bw - ow), xtex1factoroffset = (x & 1 ? bw - ow : 0))
  {
    //clockwise
    Indexbuffer[index++] = voffset;//vertex0
    Indexbuffer[index++] = voffset + 1;//vertex1
    Indexbuffer[index++] = voffset + 2;//vertex2
    Indexbuffer[index++] = voffset + 1;//vertex1 
    Indexbuffer[index++] = voffset + 3;//vertex2
    Indexbuffer[index++] = voffset + 2;//vertex3
    //vertex0
    v[voffset].x = xoffset;
    v[voffset].y = yoffset;
    v[voffset].z = 0.5;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00FFFF00;
    v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//tex0
    v[voffset].tex[0].v = etaSrcH + ytex0offset / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + xtex1offset / (float)srcwidth;//tex1
    v[voffset].tex[1].v = etaSrcH + ytex1offset / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + ytex0factoroffset / (float)bh;
    v[voffset].tex[3].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture1
    v[voffset].tex[3].v = etaFactorH + ytex1factoroffset / (float)bh;
    voffset++;
    //vertex1
    v[voffset].x = xoffset + ow;
    v[voffset].y = yoffset;
    v[voffset].z = 0.5;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00FFFF00;
    v[voffset].tex[0].u = etaSrcW + (xtex0offset + ow) / (float)srcwidth;
    v[voffset].tex[0].v = etaSrcH + ytex0offset / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + (xtex1offset + ow) / (float)srcwidth;
    v[voffset].tex[1].v = etaSrcH + ytex1offset / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + ytex0factoroffset / (float)bh;
    v[voffset].tex[3].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture1
    v[voffset].tex[3].v = etaFactorH + ytex1factoroffset / (float)bh;
    voffset++;
    //vertex2
    v[voffset].x = xoffset;
    v[voffset].y = yoffset + 2 * oh;
    v[voffset].z = 0.5;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00FFFF00;
    v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;
    v[voffset].tex[0].v = etaSrcH + (ytex0offset + oh) / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + xtex1offset / (float)srcwidth;
    v[voffset].tex[1].v = etaSrcH + (ytex1offset + oh) / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + (ytex0factoroffset + oh) / (float)bh;
    v[voffset].tex[3].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture1
    v[voffset].tex[3].v = etaFactorH + (ytex1factoroffset + oh) / (float)bh;
    voffset++;
    //vertex3
    v[voffset].x = xoffset + ow;
    v[voffset].y = yoffset + 2 * oh;
    v[voffset].z = 0.5;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00FFFF00;
    v[voffset].tex[0].u = etaSrcW + (xtex0offset + ow) / (float)srcwidth;
    v[voffset].tex[0].v = etaSrcH + (ytex0offset + oh) / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + (xtex1offset + ow) / (float)srcwidth;
    v[voffset].tex[1].v = etaSrcH + (ytex1offset + oh) / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + (ytex0factoroffset + oh) / (float)bh;
    v[voffset].tex[3].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture1
    v[voffset].tex[3].v = etaFactorH + (ytex1factoroffset + oh) / (float)bh;
    voffset++;
  }

  //Setup nonoverlap blocks
  //first texture1 xy nonoverlap
  nnonoverlaptex1xyblock = ((nnonoverlapblocksy + 1) / 2)*((nnonoverlapblocksx + 1) / 2);
  startindexnonoverlaptex1xyblock = index;
  startvertexnonoverlaptex1xyblock = voffset;
  for (int y = 0, yoffset = even, ytexoffset = oh; y < (nnonoverlapblocksy + 1) / 2; y++, yoffset += 2 * 2 * (bh - oh), ytexoffset += bh)
    for (int x = 0, xoffset = 0, xtexoffset = ow; x < (nnonoverlapblocksx + 1) / 2; x++, xoffset += 2 * (bw - ow), xtexoffset += bw)
    {
      //clockwise
      Indexbuffer[index++] = voffset;//vertex0
      Indexbuffer[index++] = voffset + 1;//vertex1
      Indexbuffer[index++] = voffset + 2;//vertex2
      Indexbuffer[index++] = voffset + 1;//vertex1 
      Indexbuffer[index++] = voffset + 3;//vertex2
      Indexbuffer[index++] = voffset + 2;//vertex3
      //vertex0
      v[voffset].x = xoffset;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFFF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex1
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFFF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex2
      v[voffset].x = xoffset;
      v[voffset].y = yoffset + 2 * (bh - 2 * oh);
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFFF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex3
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset + 2 * (bh - 2 * oh);
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFFF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
    }
  //texture1 zw nonoverlap
  nnonoverlaptex1zwblock = ((nnonoverlapblocksy + 1) / 2)*((nnonoverlapblocksx) / 2);
  startindexnonoverlaptex1zwblock = index;
  startvertexnonoverlaptex1zwblock = voffset;
  for (int y = 0, yoffset = even, ytexoffset = oh; y < (nnonoverlapblocksy + 1) / 2; y++, yoffset += 2 * 2 * (bh - oh), ytexoffset += bh)
    for (int x = 0, xoffset = bw - ow, xtexoffset = ow; x < (nnonoverlapblocksx) / 2; x++, xoffset += 2 * (bw - ow), xtexoffset += bw)
    {
      //clockwise
      Indexbuffer[index++] = voffset;//vertex0
      Indexbuffer[index++] = voffset + 1;//vertex1
      Indexbuffer[index++] = voffset + 2;//vertex2
      Indexbuffer[index++] = voffset + 1;//vertex1 
      Indexbuffer[index++] = voffset + 3;//vertex2
      Indexbuffer[index++] = voffset + 2;//vertex3
      //vertex0
      v[voffset].x = xoffset;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFDF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex1
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFDF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex2
      v[voffset].x = xoffset;
      v[voffset].y = yoffset + 2 * (bh - 2 * oh);
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFDF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex3
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset + 2 * (bh - 2 * oh);
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000FFDF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
    }
  //texture2 xy nonoverlap
  nnonoverlaptex2xyblock = ((nnonoverlapblocksy) / 2)*((nnonoverlapblocksx + 1) / 2);
  startindexnonoverlaptex2xyblock = index;
  startvertexnonoverlaptex2xyblock = voffset;
  for (int y = 0, yoffset = even + 2 * (bh - oh), ytexoffset = oh; y < (nnonoverlapblocksy) / 2; y++, yoffset += 2 * 2 * (bh - oh), ytexoffset += bh)
    for (int x = 0, xoffset = 0, xtexoffset = ow; x < (nnonoverlapblocksx + 1) / 2; x++, xoffset += 2 * (bw - ow), xtexoffset += bw)
    {
      //clockwise
      Indexbuffer[index++] = voffset;//vertex0
      Indexbuffer[index++] = voffset + 1;//vertex1
      Indexbuffer[index++] = voffset + 2;//vertex2
      Indexbuffer[index++] = voffset + 1;//vertex1 
      Indexbuffer[index++] = voffset + 3;//vertex2
      Indexbuffer[index++] = voffset + 2;//vertex3
      //vertex0
      v[voffset].x = xoffset;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFFF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex1
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFFF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex2
      v[voffset].x = xoffset;
      v[voffset].y = yoffset + 2 * (bh - 2 * oh);
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFFF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex3
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset + 2 * (bh - 2 * oh);
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFFF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
    }
  //texture2 zw nonoverlap
  nnonoverlaptex2zwblock = ((nnonoverlapblocksy) / 2)*((nnonoverlapblocksx) / 2);
  startindexnonoverlaptex2zwblock = index;
  startvertexnonoverlaptex2zwblock = voffset;
  for (int y = 0, yoffset = even + 2 * (bh - oh), ytexoffset = oh; y < (nnonoverlapblocksy) / 2; y++, yoffset += 2 * 2 * (bh - oh), ytexoffset += bh)
    for (int x = 0, xoffset = bw - ow, xtexoffset = ow; x < (nnonoverlapblocksx) / 2; x++, xoffset += 2 * (bw - ow), xtexoffset += bw)
    {
      //clockwise
      Indexbuffer[index++] = voffset;//vertex0
      Indexbuffer[index++] = voffset + 1;//vertex1
      Indexbuffer[index++] = voffset + 2;//vertex2
      Indexbuffer[index++] = voffset + 1;//vertex1 
      Indexbuffer[index++] = voffset + 3;//vertex2
      Indexbuffer[index++] = voffset + 2;//vertex3
      //vertex0
      v[voffset].x = xoffset;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFDF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex1
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFDF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex2
      v[voffset].x = xoffset;
      v[voffset].y = yoffset + 2 * (bh - 2 * oh);
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFDF;
      v[voffset].tex[0].u = etaSrcW + xtexoffset / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
      //vertex3
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset + 2 * (bh - 2 * oh);
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x0000DFDF;
      v[voffset].tex[0].u = etaSrcW + (xtexoffset + bw - 2 * ow) / (float)srcwidth;
      v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
      voffset++;
      //TODO: texcoord
    }

  //xy horizontal blocks
  nhorizontalxyblock = ((nhorizontaloverlapblocksx + 1) / 2)*(nhorizontaloverlapblocksy);
  startindexhorizontalxyblock = index;
  startvertexhorizontalxyblock = voffset;
  for (int y = 0, yoffset = even + 2 * (bh - 2 * oh), ytex0offset = bh - oh, ytex1offset = 0, ytex0factoroffset = bh - oh, ytex1factoroffset = 0; y < nhorizontaloverlapblocksy;
    y++, yoffset += 2 * (bh - oh), ytex0offset += (y & 1 ? oh : bh - oh), ytex1offset += (y & 1 ? bh - oh : oh), ytex0factoroffset = (y & 1 ? 0 : bh - oh), ytex1factoroffset = (y & 1 ? bh - oh : 0))
    for (int x = 0, xoffset = 0, xtex0offset = ow; x < (nhorizontaloverlapblocksx + 1) / 2; x++, xoffset += 2 * (bw - ow), xtex0offset += bw)
    {
      //clockwise
      Indexbuffer[index++] = voffset;//vertex0
      Indexbuffer[index++] = voffset + 1;//vertex1
      Indexbuffer[index++] = voffset + 2;//vertex2
      Indexbuffer[index++] = voffset + 1;//vertex1 
      Indexbuffer[index++] = voffset + 3;//vertex2
      Indexbuffer[index++] = voffset + 2;//vertex3
      //vertex0
      v[voffset].x = xoffset;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00FF0000;
      v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + ytex0offset / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + xtex0offset / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + ytex1offset / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + ow / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + ytex0factoroffset / (float)bh;
      v[voffset].tex[3].u = etaFactorW + ow / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + ytex1factoroffset / (float)bh;
      voffset++;
      //TODO: texcoord
      //vertex1
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00FF0000;
      v[voffset].tex[0].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + ytex0offset / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + ytex1offset / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + ytex0factoroffset / (float)bh;
      v[voffset].tex[3].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + ytex1factoroffset / (float)bh;
      voffset++;
      //TODO: texcoord
      //vertex2
      v[voffset].x = xoffset;
      v[voffset].y = yoffset + 2 * oh;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00FF0000;
      v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + (ytex0offset + oh) / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + xtex0offset / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + (ytex1offset + oh) / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + ow / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + (ytex0factoroffset + oh) / (float)bh;
      v[voffset].tex[3].u = etaFactorW + ow / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + (ytex1factoroffset + oh) / (float)bh;
      voffset++;
      //TODO: texcoord
      //vertex3
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset + 2 * oh;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00FF0000;
      v[voffset].tex[0].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + (ytex0offset + oh) / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + (ytex1offset + oh) / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + (ytex0factoroffset + oh) / (float)bh;
      v[voffset].tex[3].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + (ytex1factoroffset + oh) / (float)bh;
      voffset++;
      //TODO: texcoord
    }
  //zw horizontal blocks
  nhorizontalzwblock = ((nhorizontaloverlapblocksx) / 2)*(nhorizontaloverlapblocksy);
  startindexhorizontalzwblock = index;
  startvertexhorizontalzwblock = voffset;
  for (int y = 0, yoffset = even + 2 * (bh - 2 * oh), ytex0offset = bh - oh, ytex1offset = 0, ytex0factoroffset = bh - oh, ytex1factoroffset = 0; y < nhorizontaloverlapblocksy;
    y++, yoffset += 2 * (bh - oh), ytex0offset += (y & 1 ? oh : bh - oh), ytex1offset += (y & 1 ? bh - oh : oh), ytex0factoroffset = (y & 1 ? 0 : bh - oh), ytex1factoroffset = (y & 1 ? bh - oh : 0))
    for (int x = 0, xoffset = bw - ow, xtex0offset = ow; x < (nhorizontaloverlapblocksx) / 2; x++, xoffset += 2 * (bw - ow), xtex0offset += bw)
    {
      //clockwise
      Indexbuffer[index++] = voffset;//vertex0
      Indexbuffer[index++] = voffset + 1;//vertex1
      Indexbuffer[index++] = voffset + 2;//vertex2
      Indexbuffer[index++] = voffset + 1;//vertex1 
      Indexbuffer[index++] = voffset + 3;//vertex2
      Indexbuffer[index++] = voffset + 2;//vertex3
      //vertex0
      v[voffset].x = xoffset;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00DF0000;
      v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + ytex0offset / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + xtex0offset / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + ytex1offset / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + ow / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + ytex0factoroffset / (float)bh;
      v[voffset].tex[3].u = etaFactorW + ow / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + ytex1factoroffset / (float)bh;
      voffset++;
      //TODO: texcoord
      //vertex1
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00DF0000;
      v[voffset].tex[0].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + ytex0offset / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + ytex1offset / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + ytex0factoroffset / (float)bh;
      v[voffset].tex[3].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + ytex1factoroffset / (float)bh;
      voffset++;
      //TODO: texcoord
      //vertex2
      v[voffset].x = xoffset;
      v[voffset].y = yoffset + 2 * oh;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00DF0000;
      v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + (ytex0offset + oh) / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + xtex0offset / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + (ytex1offset + oh) / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + ow / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + (ytex0factoroffset + oh) / (float)bh;
      v[voffset].tex[3].u = etaFactorW + ow / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + (ytex1factoroffset + oh) / (float)bh;
      voffset++;
      //TODO: texcoord
      //vertex3
      v[voffset].x = xoffset + bw - 2 * ow;
      v[voffset].y = yoffset + 2 * oh;
      v[voffset].z = 0.5;
      v[voffset].rhw = 1.0;
      v[voffset].color = 0x00DF0000;
      v[voffset].tex[0].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex0
      v[voffset].tex[0].v = etaSrcH + (ytex0offset + oh) / (float)srcheight;
      v[voffset].tex[1].u = etaSrcW + (xtex0offset + bw - 2 * ow) / (float)srcwidth;//tex1
      v[voffset].tex[1].v = etaSrcH + (ytex1offset + oh) / (float)srcheight;
      v[voffset].tex[2].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture0
      v[voffset].tex[2].v = etaFactorH + (ytex0factoroffset + oh) / (float)bh;
      v[voffset].tex[3].u = etaFactorW + (bw - ow) / (float)bw;//factorLUT for texture1
      v[voffset].tex[3].v = etaFactorH + (ytex1factoroffset + oh) / (float)bh;
      voffset++;
      //TODO: texcoord
    }
  //vertical blocks tex1
  nverticaltex1block = (nverticaloverlapblocksx)*((nverticaloverlapblocksy + 1) / 2);
  startindexverticaltex1block = index;
  startvertexverticaltex1block = voffset;
  for (int y = 0, yoffset = even, ytexoffset = oh; y < (nverticaloverlapblocksy + 1) / 2; y++, yoffset += 2 * 2 * (bh - oh), ytexoffset += bh)
    for (int x = 0, xoffset = bw - 2 * ow, xtex0offset = bw - ow, xtex1offset = 0, xtex0factoroffset = bw - ow; x < (nverticaloverlapblocksx);
      x++, xoffset += (bw - ow), xtex0offset += (x & 1 ? ow : bw - ow), xtex1offset += (x & 1 ? bw - ow : ow), xtex0factoroffset = (x & 1 ? 0 : bw - ow))
  {
    //clockwise
    Indexbuffer[index++] = voffset;//vertex0
    Indexbuffer[index++] = voffset + 1;//vertex1
    Indexbuffer[index++] = voffset + 2;//vertex2
    Indexbuffer[index++] = voffset + 1;//vertex1 
    Indexbuffer[index++] = voffset + 3;//vertex2
    Indexbuffer[index++] = voffset + 2;//vertex3
    //vertex0
    v[voffset].x = xoffset;
    v[voffset].y = yoffset;
    v[voffset].z = 0.5;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + xtex1offset / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + oh / (float)bh;
    voffset++;
    //vertex1
    v[voffset].x = xoffset + ow;
    v[voffset].y = yoffset;
    v[voffset].z = 0.5;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + (xtex0offset + ow) / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + (xtex1offset + ow) / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + oh / (float)bh;
    voffset++;
    //vertex2
    v[voffset].x = xoffset;
    v[voffset].y = yoffset + 2 * (bh - 2 * oh);
    v[voffset].z = 0.5;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + xtex1offset / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + (bh - oh) / (float)bh;
    voffset++;
    //vertex3
    v[voffset].x = xoffset + ow;
    v[voffset].y = yoffset + 2 * (bh - 2 * oh);
    v[voffset].z = 0.5;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + (xtex0offset + ow) / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + (xtex1offset + ow) / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + (bh - oh) / (float)bh;
    voffset++;
  }
  //vertical blocks tex2
  nverticaltex2block = (nverticaloverlapblocksx)*((nverticaloverlapblocksy) / 2);
  startindexverticaltex2block = index;
  startvertexverticaltex2block = voffset;
  for (int y = 0, yoffset = even + 2 * (bh - oh), ytexoffset = oh; y < (nverticaloverlapblocksy) / 2; y++, yoffset += 2 * 2 * (bh - oh), ytexoffset += bh)
    for (int x = 0, xoffset = bw - 2 * ow, xtex0offset = bw - ow, xtex1offset = 0, xtex0factoroffset = bw - ow; x < (nverticaloverlapblocksx);
      x++, xoffset += (bw - ow), xtex0offset += (x & 1 ? ow : bw - ow), xtex1offset += (x & 1 ? bw - ow : ow), xtex0factoroffset = (x & 1 ? 0 : bw - ow))
  {
    //clockwise
    Indexbuffer[index++] = voffset;//vertex0
    Indexbuffer[index++] = voffset + 1;//vertex1
    Indexbuffer[index++] = voffset + 2;//vertex2
    Indexbuffer[index++] = voffset + 1;//vertex1 
    Indexbuffer[index++] = voffset + 3;//vertex2
    Indexbuffer[index++] = voffset + 2;//vertex3
    //vertex0
    v[voffset].x = xoffset;
    v[voffset].y = yoffset;
    v[voffset].z = 0.5;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + xtex1offset / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + oh / (float)bh;
    voffset++;
    //vertex1
    v[voffset].x = xoffset + ow;
    v[voffset].y = yoffset;
    v[voffset].z = 0.5;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + (xtex0offset + ow) / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + (xtex1offset + ow) / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + ytexoffset / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + oh / (float)bh;
    voffset++;
    //vertex2
    v[voffset].x = xoffset;
    v[voffset].y = yoffset + 2 * (bh - 2 * oh);
    v[voffset].z = 0.5;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + xtex0offset / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + xtex1offset / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + xtex0factoroffset / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + (bh - oh) / (float)bh;
    voffset++;
    //vertex3
    v[voffset].x = xoffset + ow;
    v[voffset].y = yoffset + 2 * (bh - 2 * oh);
    v[voffset].z = 0.5;
    v[voffset].rhw = 1.0;
    v[voffset].color = 0x00DF0000;
    v[voffset].tex[0].u = etaSrcW + (xtex0offset + ow) / (float)srcwidth;//xy
    v[voffset].tex[0].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[1].u = etaSrcW + (xtex1offset + ow) / (float)srcwidth;//zw
    v[voffset].tex[1].v = etaSrcH + (ytexoffset + bh - 2 * oh) / (float)srcheight;
    v[voffset].tex[2].u = etaFactorW + (xtex0factoroffset + ow) / (float)bw;//factorLUT for texture0
    v[voffset].tex[2].v = etaFactorH + (bh - oh) / (float)bh;
    voffset++;
    //TODO: texcoord
  }
  _pDevice->CreateIndexBuffer(totindex * sizeof(short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED/*D3DPOOL_DEFAULT*/, &pIndexBuffer, NULL);
  VOID *pIndices, *pVertexes;

  pIndexBuffer->Lock(0, totindex * sizeof(short), &pIndices, 0);
  memcpy(pIndices, Indexbuffer, totindex * sizeof(short));
  pIndexBuffer->Unlock();
  _pDevice->CreateVertexBuffer(totvertex * sizeof(vertex4), D3DUSAGE_WRITEONLY, fvf, D3DPOOL_MANAGED/*D3DPOOL_DEFAULT*/, &pVertexBuffer, NULL);
  pVertexBuffer->Lock(0, totvertex * sizeof(vertex4), &pVertexes, 0);
  memcpy(pVertexes, v, totvertex * sizeof(vertex4));
  pVertexBuffer->Unlock();
  vertexsize = sizeof(vertex4);
  delete[] v;
  delete[] Indexbuffer;
}


HRESULT OIQuad::SetActive() {
  _pDevice->SetStreamSource(0, pVertexBuffer, 0, vertexsize);
  _pDevice->SetIndices(pIndexBuffer);
  return _pDevice->SetFVF(fvf);
}

HRESULT OIQuad::DrawCorners() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexcornerblock, ncornerblock * 4, startindexcornerblock, ncornerblock * 2);
}

HRESULT OIQuad::DrawHorizontalxy() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexhorizontalxyblock, nhorizontalxyblock * 4, startindexhorizontalxyblock, nhorizontalxyblock * 2);
}
HRESULT OIQuad::DrawHorizontalzw() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexhorizontalzwblock, nhorizontalzwblock * 4, startindexhorizontalzwblock, nhorizontalzwblock * 2);
}

HRESULT OIQuad::DrawVerticaltex1() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexverticaltex1block, nverticaltex1block * 4, startindexverticaltex1block, nverticaltex1block * 2);
}
HRESULT OIQuad::DrawVerticaltex2() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexverticaltex2block, nverticaltex2block * 4, startindexverticaltex2block, nverticaltex2block * 2);
}

HRESULT OIQuad::DrawNonoverlaptex1xy() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexnonoverlaptex1xyblock, nnonoverlaptex1xyblock * 4, startindexnonoverlaptex1xyblock, nnonoverlaptex1xyblock * 2);
}
HRESULT OIQuad::DrawNonoverlaptex2xy() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexnonoverlaptex2xyblock, nnonoverlaptex2xyblock * 4, startindexnonoverlaptex2xyblock, nnonoverlaptex2xyblock * 2);
}
HRESULT OIQuad::DrawNonoverlaptex1zw() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexnonoverlaptex1zwblock, nnonoverlaptex1zwblock * 4, startindexnonoverlaptex1zwblock, nnonoverlaptex1zwblock * 2);
}
HRESULT OIQuad::DrawNonoverlaptex2zw() {

  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertexnonoverlaptex2zwblock, nnonoverlaptex2zwblock * 4, startindexnonoverlaptex2zwblock, nnonoverlaptex2zwblock * 2);
}

HRESULT OIQuad::Draw()
{
  //return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,nblocks,nblocks/2*3,nblocks/2);;
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, nblocks * 4, 0, nblocks * 2);;
}


//x: width of complex 1d horizontal fft
//nx: number of fft in width
//y: height of fft
//nx: number of fft in height
C2RQuad::C2RQuad(LPDIRECT3DDEVICE9 pDevice, int x, int nx, int y, int ny) :
  _pDevice(pDevice)
{
  int totquad = 5 * nx;
  //5 quads per fft organized like this:
  //quad 1: from 0 to 1
  //quad 2: from 1 to x/2-1
  //quad 3: from x/2-1 to x/2
  //quad 4: from x/2 to x-1
  //quad 5:from x-1 to x
  //total size=x+1
  // quad 1 and 5 belongs to 00 as they only read from texture 0
  // quad 2 to 01 as it read from texture 0 and 1
  // quad 4 to 10 as it read from texture 1 and 0
  // quad 5 to 11 as it only read from texture 1

  vertexut3 *v = NEW vertexut3[4 * totquad];
  vertexut3 *vi = v;

  short *ib = NEW short[6 * totquad];

  int nvertex = 0;
  int nindex = 0;
  int width = nx * (x + 1);
  int height = y * ny;
  int srcwidth = nx * x / 2;

  float etax = 0.5 / srcwidth;
  float etay = 0.5 / height;

  //Setup 00
  nvertex00 = 8 * nx;
  nindex00 = 0;
  startvertex00 = nvertex;
  for (int i = 0, offset = 0, srcoffset = 0; i < nx; i++, offset += x + 1, srcoffset += x / 2)
    for (int j = 0, o = offset; j < 2; j++, o += x)
    {
      v[nvertex].x = (float)(o) / width * 2 - 1; v[nvertex].y = 1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
      v[nvertex].tex[0].u = (float)srcoffset / srcwidth + etax; v[nvertex].tex[0].v = etay;
      v[nvertex].tex[1].u = 0; v[nvertex].tex[1].v = 0;
      v[nvertex].tex[2].u = (j == 0 ? 1 : -1); v[nvertex].tex[2].v = (j == 1 ? 1 : 0);
      ib[nindex++] = nvertex;
      nvertex++;
      //v1
      v[nvertex].x = (float)(o + 1) / width * 2 - 1; v[nvertex].y = 1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
      v[nvertex].tex[0].u = (float)(srcoffset + 1) / srcwidth + etax; v[nvertex].tex[0].v = etay;
      v[nvertex].tex[1].u = 0; v[nvertex].tex[1].v = 0;
      v[nvertex].tex[2].u = (j == 0 ? 1 : -1); v[nvertex].tex[2].v = 0;
      ib[nindex++] = nvertex;
      nvertex++;
      //v2
      v[nvertex].x = (float)(o) / width * 2 - 1; v[nvertex].y = -1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
      v[nvertex].tex[0].u = (float)srcoffset / srcwidth + etax; v[nvertex].tex[0].v = 1 + etay;
      v[nvertex].tex[1].u = 0; v[nvertex].tex[1].v = 0;
      v[nvertex].tex[2].u = (j == 0 ? 1 : -1); v[nvertex].tex[2].v = 0;
      ib[nindex++] = nvertex;
      nvertex++;
      //v3
      v[nvertex].x = (float)(o + 1) / width * 2 - 1; v[nvertex].y = -1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
      v[nvertex].tex[0].u = (float)(srcoffset + 1) / srcwidth + etax; v[nvertex].tex[0].v = 1 + etay;
      v[nvertex].tex[1].u = 0; v[nvertex].tex[1].v = 0;
      v[nvertex].tex[2].u = (j == 0 ? 1 : -1); v[nvertex].tex[2].v = 0;
      ib[nindex++] = nvertex;
      ib[nindex++] = nvertex - 1;
      ib[nindex++] = nvertex - 2;
      nvertex++;
    }

  nvertex01 = 4 * nx;
  nindex01 = nindex;
  startvertex01 = nvertex;
  for (int i = 0, offset = 0, srcoffset = 0; i < nx; i++, offset += x + 1, srcoffset += x / 2)
  {
    v[nvertex].x = (float)(offset + 1) / width * 2 - 1; v[nvertex].y = 1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = (float)(srcoffset + 1) / srcwidth + etax; v[nvertex].tex[0].v = etay;
    v[nvertex].tex[1].u = (float)(srcoffset + x / 2 - 1) / srcwidth + etax; v[nvertex].tex[1].v = etay;
    v[nvertex].tex[2].u = 2 * pi * 1 / (2 * x); v[nvertex].tex[2].v = 1;
    ib[nindex++] = nvertex;
    nvertex++;
    //v1
    v[nvertex].x = (float)(offset + x / 2) / width * 2 - 1; v[nvertex].y = 1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = (float)(srcoffset + x / 2) / srcwidth + etax; v[nvertex].tex[0].v = etay;
    v[nvertex].tex[1].u = (float)(srcoffset) / srcwidth + etax; v[nvertex].tex[1].v = etay;
    v[nvertex].tex[2].u = 2 * pi*(x / 2) / (2 * x); v[nvertex].tex[2].v = 1;
    ib[nindex++] = nvertex;
    nvertex++;
    //v2
    v[nvertex].x = (float)(offset + 1) / width * 2 - 1; v[nvertex].y = -1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = (float)(srcoffset + 1) / srcwidth + etax; v[nvertex].tex[0].v = 1 + etay;
    v[nvertex].tex[1].u = (float)(srcoffset + x / 2 - 1) / srcwidth + etax; v[nvertex].tex[1].v = 1 + etay;
    v[nvertex].tex[2].u = 2 * pi * 1 / (2 * x); v[nvertex].tex[2].v = 1;
    ib[nindex++] = nvertex;
    nvertex++;
    //v3
    v[nvertex].x = (float)(offset + x / 2) / width * 2 - 1; v[nvertex].y = -1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = (float)(srcoffset + x / 2) / srcwidth + etax; v[nvertex].tex[0].v = 1 + etay;
    v[nvertex].tex[1].u = (float)(srcoffset) / srcwidth + etax; v[nvertex].tex[1].v = 1 + etay;
    v[nvertex].tex[2].u = 2 * pi*(x / 2) / (2 * x); v[nvertex].tex[2].v = 1;
    ib[nindex++] = nvertex;
    ib[nindex++] = nvertex - 1;
    ib[nindex++] = nvertex - 2;
    nvertex++;
  }
  nvertex11 = 4 * nx;
  nindex11 = nindex;
  startvertex11 = nvertex;
  for (int i = 0, offset = 0, srcoffset = 0; i < nx; i++, offset += x + 1, srcoffset += x / 2)
  {
    v[nvertex].x = (float)(offset + x / 2) / width * 2 - 1; v[nvertex].y = 1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = (float)(srcoffset) / srcwidth + etax; v[nvertex].tex[0].v = etay;
    v[nvertex].tex[1].u = 0; v[nvertex].tex[1].v = 0;
    v[nvertex].tex[2].u = 0; v[nvertex].tex[2].v = 0;
    ib[nindex++] = nvertex;
    nvertex++;
    //v1
    v[nvertex].x = (float)(offset + x / 2 + 1) / width * 2 - 1; v[nvertex].y = 1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = (float)(srcoffset + 1) / srcwidth + etax; v[nvertex].tex[0].v = etay;
    v[nvertex].tex[1].u = 0; v[nvertex].tex[1].v = 0;
    v[nvertex].tex[2].u = 0; v[nvertex].tex[2].v = 0;
    ib[nindex++] = nvertex;
    nvertex++;
    //v2
    v[nvertex].x = (float)(offset + x / 2) / width * 2 - 1; v[nvertex].y = -1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = (float)srcoffset / srcwidth + etax; v[nvertex].tex[0].v = 1 + etay;
    v[nvertex].tex[1].u = 0; v[nvertex].tex[1].v = 0;
    v[nvertex].tex[2].u = 0; v[nvertex].tex[2].v = 0;
    ib[nindex++] = nvertex;
    nvertex++;
    //v3
    v[nvertex].x = (float)(offset + x / 2 + 1) / width * 2 - 1; v[nvertex].y = -1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = (float)(srcoffset + 1) / srcwidth + etax; v[nvertex].tex[0].v = 1 + etay;
    v[nvertex].tex[1].u = 0; v[nvertex].tex[1].v = 0;
    v[nvertex].tex[2].u = 0; v[nvertex].tex[2].v = 0;
    ib[nindex++] = nvertex;
    ib[nindex++] = nvertex - 1;
    ib[nindex++] = nvertex - 2;
    nvertex++;
  }

  nvertex10 = 4 * nx;
  nindex10 = nindex;
  startvertex10 = nvertex;
  for (int i = 0, offset = 0, srcoffset = 0; i < nx; i++, offset += x + 1, srcoffset += x / 2)
  {
    v[nvertex].x = (float)(offset + x / 2 + 1) / width * 2 - 1; v[nvertex].y = 1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = (float)(srcoffset + 1) / srcwidth + etax; v[nvertex].tex[0].v = etay;
    v[nvertex].tex[1].u = (float)(srcoffset + x / 2 - 1) / srcwidth + etax; v[nvertex].tex[1].v = etay;
    v[nvertex].tex[2].u = 2 * pi*(1 + x / 2) / (2 * x); v[nvertex].tex[2].v = 1;
    ib[nindex++] = nvertex;
    nvertex++;
    //v1
    v[nvertex].x = (float)(offset + x) / width * 2 - 1; v[nvertex].y = 1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = (float)(srcoffset + x / 2) / srcwidth + etax; v[nvertex].tex[0].v = etay;
    v[nvertex].tex[1].u = (float)(srcoffset) / srcwidth + etax; v[nvertex].tex[1].v = etay;
    v[nvertex].tex[2].u = 2 * pi*(x) / (2 * x); v[nvertex].tex[2].v = 1;
    ib[nindex++] = nvertex;
    nvertex++;
    //v2
    v[nvertex].x = (float)(offset + x / 2 + 1) / width * 2 - 1; v[nvertex].y = -1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = (float)(srcoffset + 1) / srcwidth + etax; v[nvertex].tex[0].v = 1 + etay;
    v[nvertex].tex[1].u = (float)(srcoffset + x / 2 - 1) / srcwidth + etax; v[nvertex].tex[1].v = 1 + etay;
    v[nvertex].tex[2].u = 2 * pi*(1 + x / 2) / (2 * x); v[nvertex].tex[2].v = 1;
    ib[nindex++] = nvertex;
    nvertex++;
    //v3
    v[nvertex].x = (float)(offset + x) / width * 2 - 1; v[nvertex].y = -1; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = (float)(srcoffset + x / 2) / srcwidth + etax; v[nvertex].tex[0].v = 1 + etay;
    v[nvertex].tex[1].u = (float)(srcoffset) / srcwidth + etax; v[nvertex].tex[1].v = 1 + etay;
    v[nvertex].tex[2].u = 2 * pi*(x) / (2 * x); v[nvertex].tex[2].v = 1;
    ib[nindex++] = nvertex;
    ib[nindex++] = nvertex - 1;
    ib[nindex++] = nvertex - 2;
    nvertex++;
  }
  _pDevice->CreateIndexBuffer(sizeof(short)*nindex, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIndexBuffer, NULL);
  VOID *pIndices, *pVertexes;

  _pIndexBuffer->Lock(0, nindex * sizeof(short), &pIndices, 0);
  memcpy(pIndices, ib, nindex * sizeof(short));
  _pIndexBuffer->Unlock();
  _pDevice->CreateVertexBuffer(nvertex * sizeof(vertexut3), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &_pVertexBuffer, NULL);
  _pVertexBuffer->Lock(0, nvertex * sizeof(vertexut3), &pVertexes, 0);
  memcpy(pVertexes, v, nvertex * sizeof(vertexut3));
  _pVertexBuffer->Unlock();

  D3DVERTEXELEMENT9 kDX9VertexElements[] =
  {
    { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 4 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
  { 0, 6 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
  { 0, 8 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
    D3DDECL_END()
  };

  _pDevice->CreateVertexDeclaration(kDX9VertexElements, &_pVertexDeclaration);

  delete v;
  delete ib;
}

HRESULT C2RQuad::SetActive()
{
  _pDevice->SetVertexDeclaration(_pVertexDeclaration);
  _pDevice->SetStreamSource(0, _pVertexBuffer, 0, sizeof(vertexut3));
  return _pDevice->SetIndices(_pIndexBuffer);
}

HRESULT C2RQuad::Draw()
{
  return 0;
}

HRESULT C2RQuad::Draw00()
{
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertex00, nvertex00, nindex00, nvertex00 / 2);
}
HRESULT C2RQuad::Draw11()
{
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertex11, nvertex11, nindex11, nvertex11 / 2);
}
HRESULT C2RQuad::Draw01()
{
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertex01, nvertex01, nindex01, nvertex01 / 2);
}
HRESULT C2RQuad::Draw10()
{
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, startvertex10, nvertex10, nindex10, nvertex10 / 2);
}

CollectQuad::CollectQuad(LPDIRECT3DDEVICE9 pDevice, int x, int nx, int y, int ny, bool horizontal) :_pDevice(pDevice), nvertex(0), nindex(0)
{
  vertexut1 *v = NEW vertexut1[2 * 4 * nx*ny];
  vertexut1 *vi = v;




  short *ib = NEW short[2 * 6 * nx*ny];


  int width = nx * x;
  int height = ny * y;
  int xstartoffset = 0, ystartoffset = 0;
  if (horizontal) {
    x >>= 1; xstartoffset = x;
  }
  else
  {
    y >>= 1; ystartoffset = y;
  }

  int srcheight = horizontal ? height : height / 2;
  int srcwidth = horizontal ? width / 2 : width;
  float etax = horizontal ? 1.0 / width : 0.5 / width;
  float etay = horizontal ? 0.5 / height : 1.0 / height;

  for (int k = 0; k < 2; k++)
    for (int i = 0, yoffset = k * ystartoffset, srcyoffset = 0; i < ny; i++, yoffset += y + ystartoffset, srcyoffset += y)
      for (int j = 0, xoffset = k * xstartoffset, srcxoffset = 0; j < nx; j++, xoffset += x + xstartoffset, srcxoffset += x)
      {
        //    v0-v1
        //    | / |
        //    v2-v3
        //v0
        v[nvertex].x = (float)(xoffset) / width * 2 - 1; v[nvertex].y = 1 - (float)(yoffset) / height * 2; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
        v[nvertex].tex.u = (float)srcxoffset / srcwidth + etax; v[nvertex].tex.v = (float)(srcyoffset) / srcheight + etay;
        //v[nvertex].tex[1].u=0;v[nvertex].tex[1].v=0;
        ib[nindex++] = nvertex;
        nvertex++;
        //v1
        v[nvertex].x = (float)(xoffset + x) / width * 2 - 1; v[nvertex].y = 1 - (float)(yoffset) / height * 2; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
        v[nvertex].tex.u = (float)(srcxoffset + x) / srcwidth + etax; v[nvertex].tex.v = (float)(srcyoffset) / srcheight + etay;
        //v[nvertex].tex[1].u=x/(x-1.0);v[nvertex].tex[1].v=0;
        ib[nindex++] = nvertex;
        nvertex++;
        //v2
        v[nvertex].x = (float)(xoffset) / width * 2 - 1; v[nvertex].y = 1 - (float)(yoffset + y) / height * 2; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
        v[nvertex].tex.u = (float)srcxoffset / srcwidth + etax; v[nvertex].tex.v = (float)(srcyoffset + y) / srcheight + etay;
        //v[nvertex].tex[1].u=0;v[nvertex].tex[1].v=1;
        ib[nindex++] = nvertex;
        nvertex++;
        //v3
        v[nvertex].x = (float)(xoffset + x) / width * 2 - 1; v[nvertex].y = 1 - (float)(yoffset + y) / height * 2; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
        v[nvertex].tex.u = (float)(srcxoffset + x) / srcwidth + etax; v[nvertex].tex.v = (float)(srcyoffset + y) / srcheight + etay;
        //v[nvertex].tex[1].u=x/(x-1.0);v[nvertex].tex[1].v=1;
        ib[nindex++] = nvertex;
        ib[nindex++] = nvertex - 1;
        ib[nindex++] = nvertex - 2;
        nvertex++;
      }
  _pDevice->CreateIndexBuffer(sizeof(short)*nindex, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIndexBuffer, NULL);
  VOID *pIndices, *pVertexes;

  _pIndexBuffer->Lock(0, nindex * sizeof(short), &pIndices, 0);
  memcpy(pIndices, ib, nindex * sizeof(short));
  _pIndexBuffer->Unlock();
  _pDevice->CreateVertexBuffer(nvertex * sizeof(vertexut1), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &_pVertexBuffer, NULL);
  _pVertexBuffer->Lock(0, nvertex * sizeof(vertexut1), &pVertexes, 0);
  memcpy(pVertexes, v, nvertex * sizeof(vertexut1));
  _pVertexBuffer->Unlock();

  D3DVERTEXELEMENT9 kDX9VertexElements[] =
  {
    { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 4 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
    D3DDECL_END()
  };

  _pDevice->CreateVertexDeclaration(kDX9VertexElements, &_pVertexDeclaration);

  delete v;
  delete ib;
}

HRESULT CollectQuad::SetActive()
{
  _pDevice->SetVertexDeclaration(_pVertexDeclaration);
  _pDevice->SetStreamSource(0, _pVertexBuffer, 0, sizeof(vertexut1));
  return _pDevice->SetIndices(_pIndexBuffer);
}

HRESULT CollectQuad::Draw()
{
  DrawEven();
  return DrawOdd();
}

HRESULT CollectQuad::DrawEven()
{
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, nvertex / 2, 0, nvertex / 2, 0, nvertex / 4);
}

HRESULT CollectQuad::DrawOdd()
{
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, nvertex / 2, 0, nvertex / 4);
}

CollectQuad::~CollectQuad()
{
  if (_pVertexBuffer)
    _pVertexBuffer->Release();
  if (_pIndexBuffer)
    _pIndexBuffer->Release();
  if (_pVertexDeclaration)
    _pVertexDeclaration->Release();
}



R2CFFTQuad::R2CFFTQuad(LPDIRECT3DDEVICE9 pDevice, int x, int nx, int y, int ny) :_pDevice(pDevice), nvertex(0), nindex(0)
{
  vertexut3 *v = NEW vertexut3[2 * 4 * nx*ny];
  vertexut3 *vi = v;




  short *ib = NEW short[2 * 6 * nx*ny];


  int width = nx * x;
  int height = ny * y;
  int xstartoffset = 0, ystartoffset = 0;
  y >>= 1; ystartoffset = y;


  int srcheight = height / 2;
  int srcwidth = (1 + x)*nx;
  float etax = 0.5 / srcwidth;
  float etay = 0.5 / srcheight;

  for (int k = 0; k < 2; k++)
    for (int i = 0, yoffset = k * ystartoffset, srcyoffset = 0; i < ny; i++, yoffset += 2 * y, srcyoffset += y)
      for (int j = 0, xoffset = 0, srcxoffset = 0; j < nx; j++, xoffset += x, srcxoffset += x + 1)
      {
        //    v0-v1
        //    | / |
        //    v2-v3
        //v0
        v[nvertex].x = (float)(xoffset) / width * 2 - 1; v[nvertex].y = 1 - (float)(yoffset) / height * 2; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
        v[nvertex].tex[0].u = (float)srcxoffset / srcwidth + etax; v[nvertex].tex[0].v = (float)(srcyoffset) / srcheight + etay;
        v[nvertex].tex[1].u = (float)(srcxoffset + x) / srcwidth + etax; v[nvertex].tex[1].v = (float)(srcyoffset) / srcheight + etay;
        v[nvertex].tex[2].u = 0; v[nvertex].tex[2].v = 0;
        ib[nindex++] = nvertex;
        nvertex++;
        //v1
        v[nvertex].x = (float)(xoffset + x) / width * 2 - 1; v[nvertex].y = 1 - (float)(yoffset) / height * 2; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
        v[nvertex].tex[0].u = (float)(srcxoffset + x) / srcwidth + etax; v[nvertex].tex[0].v = (float)(srcyoffset) / srcheight + etay;
        v[nvertex].tex[1].u = (float)(srcxoffset) / srcwidth + etax; v[nvertex].tex[1].v = (float)(srcyoffset) / srcheight + etay;
        v[nvertex].tex[2].u = pi; v[nvertex].tex[2].v = 0;
        ib[nindex++] = nvertex;
        nvertex++;
        //v2
        v[nvertex].x = (float)(xoffset) / width * 2 - 1; v[nvertex].y = 1 - (float)(yoffset + y) / height * 2; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
        v[nvertex].tex[0].u = (float)srcxoffset / srcwidth + etax; v[nvertex].tex[0].v = (float)(srcyoffset + y) / srcheight + etay;
        v[nvertex].tex[1].u = (float)(srcxoffset + x) / srcwidth + etax; v[nvertex].tex[1].v = (float)(srcyoffset + y) / srcheight + etay;
        v[nvertex].tex[2].u = 0; v[nvertex].tex[2].v = 1;
        ib[nindex++] = nvertex;
        nvertex++;
        //v3
        v[nvertex].x = (float)(xoffset + x) / width * 2 - 1; v[nvertex].y = 1 - (float)(yoffset + y) / height * 2; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
        v[nvertex].tex[0].u = (float)(srcxoffset + x) / srcwidth + etax; v[nvertex].tex[0].v = (float)(srcyoffset + y) / srcheight + etay;
        v[nvertex].tex[1].u = (float)(srcxoffset) / srcwidth + etax; v[nvertex].tex[1].v = (float)(srcyoffset + y) / srcheight + etay;
        v[nvertex].tex[2].u = pi; v[nvertex].tex[2].v = 1;
        ib[nindex++] = nvertex;
        ib[nindex++] = nvertex - 1;
        ib[nindex++] = nvertex - 2;
        nvertex++;
      }
  _pDevice->CreateIndexBuffer(sizeof(short)*nindex, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIndexBuffer, NULL);
  VOID *pIndices, *pVertexes;

  _pIndexBuffer->Lock(0, nindex * sizeof(short), &pIndices, 0);
  memcpy(pIndices, ib, nindex * sizeof(short));
  _pIndexBuffer->Unlock();
  _pDevice->CreateVertexBuffer(nvertex * sizeof(vertexut3), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &_pVertexBuffer, NULL);
  _pVertexBuffer->Lock(0, nvertex * sizeof(vertexut3), &pVertexes, 0);
  memcpy(pVertexes, v, nvertex * sizeof(vertexut3));
  _pVertexBuffer->Unlock();

  D3DVERTEXELEMENT9 kDX9VertexElements[] =
  {
    { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 4 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
  { 0, 6 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
  { 0, 8 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
    D3DDECL_END()
  };

  _pDevice->CreateVertexDeclaration(kDX9VertexElements, &_pVertexDeclaration);

  delete v;
  delete ib;
}

HRESULT R2CFFTQuad::SetActive()
{
  _pDevice->SetVertexDeclaration(_pVertexDeclaration);
  _pDevice->SetStreamSource(0, _pVertexBuffer, 0, sizeof(vertexut3));
  return _pDevice->SetIndices(_pIndexBuffer);
}

HRESULT R2CFFTQuad::Draw()
{
  DrawEven();
  return DrawOdd();
}

HRESULT R2CFFTQuad::DrawEven()
{
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, nvertex / 2, 0, nvertex / 2, 0, nvertex / 4);
}

HRESULT R2CFFTQuad::DrawOdd()
{
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, nvertex / 2, 0, nvertex / 4);
}

R2CFFTQuad::~R2CFFTQuad()
{
  if (_pVertexBuffer)
    _pVertexBuffer->Release();
  if (_pIndexBuffer)
    _pIndexBuffer->Release();
  if (_pVertexDeclaration)
    _pVertexDeclaration->Release();
}



//x: width of fft
//nx: number of fft in width
//y: height of fft
//nx: number of fft in height
FFTQuad2::FFTQuad2(LPDIRECT3DDEVICE9 pDevice, int x, int nx, int y, int ny, bool horizontal, int startstage) :_pDevice(pDevice), nvertex(0), nindex(0), startstage(startstage)
{

  int log2n = horizontal ? log2(x) : log2(y);
  stageLUT = NEW viLUT[log2n - startstage];
  int nrep = horizontal ? nx : ny;

  endstage = log2n - 1;

  int totquad = 0;
  for (int i = 0; i < log2n - startstage; i++)
    totquad += 4 * nrep*(2 << i);

  vertexut2 *v = NEW vertexut2[4 * totquad];
  vertexut2 *vi = v;

  if (horizontal) {
    x /= 2;
  }
  else {
    y /= 2;
  }

  short *ib = NEW short[6 * totquad];


  int width = nx * x;
  int height = ny * y;
  float etax = 0.5 / width;
  float etay = 0.5 / height;
  int n = horizontal ? x : y;


  for (int i = 0, parts = 1, size = n >> 1; i < log2n - startstage; i++, parts <<= 1, size >>= 1) {
    stageLUT[i].nvertex = 4 * nrep*parts;
    stageLUT[i].startindexodd = nindex;
    stageLUT[i].minvertexodd = nvertex;
    for (int j = 0; j < 2; j++) {
      stageLUT[i].startindexeven = nindex;
      stageLUT[i].minvertexeven = nvertex;
      for (int k = 0, offset = j * size, srcoffset = 0; k < nrep; k++, srcoffset += parts * size)
        for (int l = 0, m = 0; l < parts; l++, offset += 2 * size, srcoffset += size, m += 2 * size)
        {
          //    v0-v1
          //    | / |
          //    v2-v3
          //v0
          v[nvertex].x = horizontal ? ((float)(offset) / width * 2 - 1) : -1;
          v[nvertex].y = horizontal ? 1 : (1 - (float)(offset) / height * 2); v[nvertex].z = 0.5; v[nvertex].rhw = 1;
          v[nvertex].tex[0].u = horizontal ? (float)srcoffset / width + etax : etax;
          v[nvertex].tex[0].v = horizontal ? etay : (float)(srcoffset) / height + etay;
          v[nvertex].tex[1].u = cos(-pi * m / n); v[nvertex].tex[1].v = sin(-pi * m / n);
          ib[nindex++] = nvertex;
          nvertex++;
          //v1
          v[nvertex].x = horizontal ? ((float)(offset + size) / width * 2 - 1) : 1;
          v[nvertex].y = horizontal ? 1 : (1 - (float)(offset) / height * 2); v[nvertex].z = 0.5; v[nvertex].rhw = 1;
          v[nvertex].tex[0].u = horizontal ? (float)(srcoffset + size) / width + etax : 1.0 + etax;
          v[nvertex].tex[0].v = horizontal ? etay : (float)(srcoffset) / height + etay;
          v[nvertex].tex[1].u = cos(-pi * m / n); v[nvertex].tex[1].v = sin(-pi * m / n);
          ib[nindex++] = nvertex;
          nvertex++;
          //v2
          v[nvertex].x = horizontal ? ((float)(offset) / width * 2 - 1) : -1;
          v[nvertex].y = horizontal ? -1 : (1 - (float)(offset + size) / height * 2); v[nvertex].z = 0.5; v[nvertex].rhw = 1;
          v[nvertex].tex[0].u = horizontal ? (float)srcoffset / width + etax : etax;
          v[nvertex].tex[0].v = horizontal ? 1 + etay : (float)(srcoffset + size) / height + etay;
          v[nvertex].tex[1].u = cos(-pi * m / n); v[nvertex].tex[1].v = sin(-pi * m / n);
          ib[nindex++] = nvertex;
          nvertex++;
          //v3
          v[nvertex].x = horizontal ? ((float)(offset + size) / width * 2 - 1) : 1;
          v[nvertex].y = horizontal ? -1 : (1 - (float)(offset + size) / height * 2); v[nvertex].z = 0.5; v[nvertex].rhw = 1;
          v[nvertex].tex[0].u = horizontal ? (float)(srcoffset + size) / width + etax : 1.0 + etax;
          v[nvertex].tex[0].v = horizontal ? 1 + etay : (float)(srcoffset + size) / height + etay;
          v[nvertex].tex[1].u = cos(-pi * m / n); v[nvertex].tex[1].v = sin(-pi * m / n);
          ib[nindex++] = nvertex;
          ib[nindex++] = nvertex - 1;
          ib[nindex++] = nvertex - 2;
          nvertex++;
        }
    }
  }
  _pDevice->CreateIndexBuffer(sizeof(short)*nindex, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIndexBuffer, NULL);
  VOID *pIndices, *pVertexes;

  _pIndexBuffer->Lock(0, nindex * sizeof(short), &pIndices, 0);
  memcpy(pIndices, ib, nindex * sizeof(short));
  _pIndexBuffer->Unlock();
  _pDevice->CreateVertexBuffer(nvertex * sizeof(vertexut2), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &_pVertexBuffer, NULL);
  _pVertexBuffer->Lock(0, nvertex * sizeof(vertexut2), &pVertexes, 0);
  memcpy(pVertexes, v, nvertex * sizeof(vertexut2));
  _pVertexBuffer->Unlock();

  D3DVERTEXELEMENT9 kDX9VertexElements[] =
  {
    { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 4 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
    { 0, 6 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
    D3DDECL_END()
  };

  _pDevice->CreateVertexDeclaration(kDX9VertexElements, &_pVertexDeclaration);

  delete v;
  delete ib;
}




HRESULT FFTQuad2::SetActive()
{
  _pDevice->SetVertexDeclaration(_pVertexDeclaration);
  _pDevice->SetStreamSource(0, _pVertexBuffer, 0, sizeof(vertexut2));
  return _pDevice->SetIndices(_pIndexBuffer);
}

HRESULT FFTQuad2::Draw()
{
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, nvertex, 0, nvertex / 2);
}

HRESULT FFTQuad2::DrawOdd(int stage)
{
  int i = endstage - stage;
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, stageLUT[i].minvertexodd, stageLUT[i].nvertex, stageLUT[i].startindexodd, stageLUT[i].nvertex / 2);
}

HRESULT FFTQuad2::DrawEven(int stage)
{
  int i = endstage - stage;
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, stageLUT[i].minvertexeven, stageLUT[i].nvertex, stageLUT[i].startindexeven, stageLUT[i].nvertex / 2);
}

FFTQuad2::~FFTQuad2()
{
  if (_pVertexBuffer)
    _pVertexBuffer->Release();
  if (_pIndexBuffer)
    _pIndexBuffer->Release();
  if (_pVertexDeclaration)
    _pVertexDeclaration->Release();
  delete stageLUT;
}


//*******
FFTQuad0::FFTQuad0(LPDIRECT3DDEVICE9 pDevice, int x, int nx, int y, int ny, bool horizontal) :_pDevice(pDevice), nvertex(0), nindex(0)
{
  int totquad = (horizontal ? nx : ny);
  vertexut2 *v = NEW vertexut2[4 * totquad];
  vertexut2 *vi = v;
  bool h = horizontal;


  short *ib = NEW short[6 * totquad];


  int width = nx * x / (h ? 2 : 1);
  int height = ny * y / (h ? 1 : 2);
  int srcwidth = nx * x;
  int srcheight = ny * y;
  float etax = 0.5 / srcwidth;
  float etay = 0.5 / srcheight;
  int n = h ? x : y;
  int nrep = h ? nx : ny;

  for (int j = 0, srcoffset = 0, offset = 0; j < nrep; j++, srcoffset += n, offset += n / 2)
  {
    //    v0-v1
    //    | / |
    //    v2-v3
    //v0
    //v[nvertex].x=xoffset;v[nvertex].y=yoffset;v[nvertex].z=1;v[nvertex].rhw=1;
    v[nvertex].x = h ? (float)(offset) / width * 2 - 1 : -1;
    v[nvertex].y = h ? 1 : 1 - (float)(offset) / height * 2; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = h ? (float)srcoffset / srcwidth + etax : etax;
    v[nvertex].tex[0].v = h ? etay : (float)(srcoffset) / srcheight + etay;
    v[nvertex].tex[1].u = 0; v[nvertex].tex[1].v = 0;
    ib[nindex++] = nvertex;
    nvertex++;
    //v1
    //v[nvertex].x=xoffset+x;v[nvertex].y=yoffset;v[nvertex].z=1;v[nvertex].rhw=1;
    v[nvertex].x = h ? (float)(offset + n / 2) / width * 2 - 1 : 1;
    v[nvertex].y = h ? 1 : 1 - (float)(offset) / height * 2; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = h ? (float)(srcoffset + n / 2) / srcwidth + etax : 1 + etax;
    v[nvertex].tex[0].v = h ? etay : (float)(srcoffset) / srcheight + etay;
    v[nvertex].tex[1].u = h ? -pi : 0; v[nvertex].tex[1].v = 0;
    ib[nindex++] = nvertex;
    nvertex++;
    //v2
    //v[nvertex].x=xoffset;v[nvertex].y=yoffset+y;v[nvertex].z=1;v[nvertex].rhw=1;
    v[nvertex].x = h ? (float)(offset) / width * 2 - 1 : -1;
    v[nvertex].y = h ? -1 : 1 - (float)(offset + n / 2) / height * 2; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = h ? (float)srcoffset / srcwidth + etax : etax;
    v[nvertex].tex[0].v = h ? 1 + etay : (float)(srcoffset + n / 2) / srcheight + etay;
    v[nvertex].tex[1].u = h ? 0 : -pi; v[nvertex].tex[1].v = 1;
    ib[nindex++] = nvertex;
    nvertex++;
    //v3
    //v[nvertex].x=xoffset+x;v[nvertex].y=yoffset+y;v[nvertex].z=1;v[nvertex].rhw=1;
    v[nvertex].x = h ? (float)(offset + n / 2) / width * 2 - 1 : 1;
    v[nvertex].y = h ? -1 : 1 - (float)(offset + n / 2) / height * 2; v[nvertex].z = 0.5; v[nvertex].rhw = 1;
    v[nvertex].tex[0].u = h ? (float)(srcoffset + n / 2) / srcwidth + etax : 1 + etax;
    v[nvertex].tex[0].v = h ? 1 + etay : (float)(srcoffset + n / 2) / srcheight + etay;
    v[nvertex].tex[1].u = -pi; v[nvertex].tex[1].v = 1;
    ib[nindex++] = nvertex;
    ib[nindex++] = nvertex - 1;
    ib[nindex++] = nvertex - 2;
    nvertex++;
  }
  _pDevice->CreateIndexBuffer(sizeof(short)*nindex, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &_pIndexBuffer, NULL);
  VOID *pIndices, *pVertexes;

  _pIndexBuffer->Lock(0, nindex * sizeof(short), &pIndices, 0);
  memcpy(pIndices, ib, nindex * sizeof(short));
  _pIndexBuffer->Unlock();
  _pDevice->CreateVertexBuffer(nvertex * sizeof(vertexut2), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &_pVertexBuffer, NULL);
  _pVertexBuffer->Lock(0, nvertex * sizeof(vertexut2), &pVertexes, 0);
  memcpy(pVertexes, v, nvertex * sizeof(vertexut2));
  _pVertexBuffer->Unlock();

  D3DVERTEXELEMENT9 kDX9VertexElements[] =
  {
    { 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 4 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
    { 0, 6 * sizeof(float), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
    D3DDECL_END()
  };

  _pDevice->CreateVertexDeclaration(kDX9VertexElements, &_pVertexDeclaration);

  delete v;
  delete ib;
}




HRESULT FFTQuad0::SetActive()
{
  _pDevice->SetVertexDeclaration(_pVertexDeclaration);
  _pDevice->SetStreamSource(0, _pVertexBuffer, 0, sizeof(vertexut2));
  return _pDevice->SetIndices(_pIndexBuffer);
}

HRESULT FFTQuad0::Draw()
{
  return _pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, nvertex, 0, nvertex / 2);
}

FFTQuad0::~FFTQuad0()
{
  if (_pVertexBuffer)
    _pVertexBuffer->Release();
  if (_pIndexBuffer)
    _pIndexBuffer->Release();
  if (_pVertexDeclaration)
    _pVertexDeclaration->Release();
}

