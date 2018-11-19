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

#include "./core/pixelshader.h"
#include <vector>

class psMinimize : public Pixelshader {
public:
  psMinimize(LPDIRECT3DDEVICE9 pDevice, TextureRT* _Rendertarget, Texture* _src);
  HRESULT Apply(Texture* src, TextureRT *dst);
protected:
  D3DXHANDLE sSrc;
};

class psGridCorrection : public Pixelshader {
public:
  psGridCorrection(LPDIRECT3DDEVICE9 pDevice, Texture* target);
  psGridCorrection(LPDIRECT3DDEVICE9 pDevice, Texture* target, float degrid);
  HRESULT Apply(Texture* src, TextureRT *dst, Texture* degridsample);
protected:
  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;
};

//******************************************************************************************************
class psWiennerFilter : public Pixelshader {
public:
  psWiennerFilter(LPDIRECT3DDEVICE9 pDevice, TextureRT* _Rendertarget, D3DXVECTOR2 &beta, D3DXVECTOR2 &sigma, int n, bool degrid, bool pattern);
  HRESULT Apply(Texture* src, TextureRT *dst, Texture* degrid = 0, Texture* sigma = 0);
  HRESULT Apply(std::vector<TextureRT*> src, TextureRT* dst, Texture* degrid = 0, Texture* sigma = 0);
  HRESULT Apply(std::vector<pTextureRTpair*> src, pTextureRTpair* dst, pTextureRTpair* degrid = 0, Texture* sigma = 0);
  HRESULT Apply(pTextureRTpair* src, pTextureRTpair* dst, pTextureRTpair* degrid = 0, Texture* sigma = 0);
protected:
  D3DXHANDLE sSrc[4];
  D3DXHANDLE sFactor;
  D3DXHANDLE sSigma;
  bool _degrid;
};

//*******************************************************************************************************
class psKalman : public Pixelshader {
public:
  psKalman(LPDIRECT3DDEVICE9 pDevice, LPCSTR pSrcFile, LPCSTR pFunctionName, LPCSTR pProfile, D3DXMACRO* defs = 0) :
    Pixelshader(pDevice, pSrcFile, pFunctionName, pProfile, defs) {}
  virtual HRESULT Apply(Texture* src, Texture* last, Texture* covarprocesslast, Texture* covarlast, TextureRT* covarprocess, TextureRT* covar, TextureRT *dst, Texture* pattern) = 0;
};

class psKalmanSP : public psKalman {
public:
  psKalmanSP(LPDIRECT3DDEVICE9 pDevice, TextureRT* _Rendertarget, float covarNoise, float kratiosquared, bool pattern);
  HRESULT Apply(Texture* src, Texture* last, Texture* covarprocesslast, Texture* covarlast, TextureRT* covarprocess, TextureRT* covar, TextureRT *dst, Texture* pattern);
protected:
  D3DXHANDLE sSrc;
  D3DXHANDLE sCovarNoise;
  D3DXHANDLE sLast;
  D3DXHANDLE sCovar;
  D3DXHANDLE sCovarProcess;
};

class psKalmanMP : public psKalman {
public:
  psKalmanMP(LPDIRECT3DDEVICE9 pDevice, TextureRT* _Rendertarget, float covarNoise, float kratiosquared, bool pattern);
  HRESULT Apply(Texture* src, Texture* last, Texture* covarprocesslast, Texture* covarlast, TextureRT* covarprocess, TextureRT* covar, TextureRT *dst, Texture* pattern);
protected:
  D3DXHANDLE sSrc;
  D3DXHANDLE sCovarNoise;
  D3DXHANDLE sLast;
  D3DXHANDLE sCovar;
  D3DXHANDLE sCovarProcess;
  Pixelshader pass2;
  Pixelshader pass3;
};

//******************************************************************************************************



class psImg2toFloat4 : public Pixelshader {
public:
  psImg2toFloat4(LPDIRECT3DDEVICE9 pDevice, LPCSTR pSrcFile, LPCSTR pFunctionName, LPCSTR pProfile, D3DXMACRO* defs = 0) :
    Pixelshader(pDevice, pSrcFile, pFunctionName, pProfile, defs) {}
  virtual ~psImg2toFloat4() {};
  virtual HRESULT Apply(Texture* src, Texture* FactorLUT, TextureRT* dst) = 0;
};

class psImg2toFloat4m0 : public psImg2toFloat4 {
public:
  //psImg2toFloat4m0(LPDIRECT3DDEVICE9 pDevice,Texture* _Rendertarget,D3DXVECTOR2 &offset,TextureM* src);
  psImg2toFloat4m0(LPDIRECT3DDEVICE9 pDevice, RECT _Rendertarget, RECT Factormap, D3DXVECTOR2 &offset, Texture* src);
  HRESULT Apply(Texture* src, Texture* FactorLUT, TextureRT* dst);
protected:

  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;

};

class psImg2toFloat4m2 : public psImg2toFloat4 {
public:
  psImg2toFloat4m2(LPDIRECT3DDEVICE9 pDevice, int bw, int bh, int repx, int repy, int border, Texture* src);
  HRESULT Apply(Texture* src, Texture* FactorLUT, TextureRT* dst);
protected:
  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;

};


class psFloat4toImg2 : public Pixelshader {
public:
  psFloat4toImg2(LPDIRECT3DDEVICE9 pDevice, LPCSTR pSrcFile, LPCSTR pFunctionName, LPCSTR pProfile, D3DXMACRO* defs = 0) :
    Pixelshader(pDevice, pSrcFile, pFunctionName, pProfile, defs) {}
  virtual ~psFloat4toImg2() {};
  virtual HRESULT Apply(Texture *src, Texture *FactorLUT, TextureRT *dst) = 0;
};

class psFloat4toImg2m0 : public psFloat4toImg2 {
public:
  psFloat4toImg2m0(LPDIRECT3DDEVICE9 pDevice, TextureRT* _Rendertarget, Texture* _FactorMap, D3DXVECTOR2 &offset);
  ~psFloat4toImg2m0();
  HRESULT Apply(Texture *src, Texture *FactorLUT, TextureRT *dst);
protected:
  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;

  HRESULT ResetShader(LPDIRECT3DDEVICE9 pDevice, bool firstpass);
  IDirect3DSurface9* pback;
};

class psFloat4toImg2m2 : public psFloat4toImg2 {
public:
  psFloat4toImg2m2(LPDIRECT3DDEVICE9 pDevice, int bw, int bh, int repx, int repy, int border, TextureRT* dst);
  ~psFloat4toImg2m2();
  HRESULT Apply(Texture *src, Texture *FactorLUT, TextureRT *dst);
protected:
  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;

  HRESULT ResetShader(LPDIRECT3DDEVICE9 pDevice, bool firstpass);
  IDirect3DSurface9* pback;
};


//*******************************************************************************************
class psImg2toFloat4_2 : public Pixelshader {
public:
  psImg2toFloat4_2(LPDIRECT3DDEVICE9 pDevice, RECT _Rendertarget, RECT Src, D3DXVECTOR2 &offset, Texture* src);
  HRESULT Apply(Texture* src, Texture* FactorLUT, TextureRT* dst);
protected:

  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;

};

class psFloat4toImg2_2 : public Pixelshader {
public:
  psFloat4toImg2_2(LPDIRECT3DDEVICE9 pDevice, TextureRT* _Rendertarget, Texture* _FactorMap, D3DXVECTOR2 &offset);
  ~psFloat4toImg2_2();
  HRESULT Apply(TextureRT *src1, TextureRT *src2, Texture *FactorLUT, TextureRT *dst);
protected:
  D3DXHANDLE sSrc1;
  D3DXHANDLE sSrc2;
  D3DXHANDLE sFactor;

  HRESULT ResetShader(LPDIRECT3DDEVICE9 pDevice, bool firstpass);
  IDirect3DSurface9* pback;
};

//*****************************************************************************************************
class psImg2toImg4 : public Pixelshader {
public:
  psImg2toImg4(LPDIRECT3DDEVICE9 pDevice, Texture* Src, Texture* Dst);
  HRESULT Apply(Texture* src, TextureRT* dst);
protected:
  D3DXHANDLE sSrc;
};
//******************************************************************************************************
//mode 1

//convert 8 bit src to float
class psImg2ToFloat4 : public Pixelshader {
public:
  psImg2ToFloat4(LPDIRECT3DDEVICE9 pDevice, LPCSTR pSrcFile, LPCSTR pFunctionName, LPCSTR pProfile, D3DXMACRO* defs = 0) :
    Pixelshader(pDevice, pSrcFile, pFunctionName, pProfile, defs) {}
  virtual ~psImg2ToFloat4() {};
  virtual HRESULT Apply(Texture* src, Texture* factor, TextureRT *dst1, TextureRT *dst2) = 0;
};

//single pass non interlaced ow==bw/2 and oh==bh/2
class psImg2ToFloat4ohalfSP : public psImg2ToFloat4 {
public:
  psImg2ToFloat4ohalfSP(LPDIRECT3DDEVICE9 pDevice, Texture* src, int bw, int bh, bool chroma);
  HRESULT Apply(Texture* src, Texture* factor, TextureRT *dst1, TextureRT *dst2);
protected:
  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;
};

//single pass interlaced ow==bw/2 and oh==bh/2
class psImg2ToFloat4ohalfInterlacedSP : public psImg2ToFloat4 {
public:
  psImg2ToFloat4ohalfInterlacedSP(LPDIRECT3DDEVICE9 pDevice, Texture* src, TextureRT* dst, int bw, int bh, bool chroma);
  HRESULT Apply(Texture* src, Texture* factor, TextureRT *dst1, TextureRT *dst2);
  ~psImg2ToFloat4ohalfInterlacedSP();
protected:
  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;
  NQuad* bottomquad;
  TextureRT* temptex;
  RECT r[12];
};

//multi pass non interlaced ow==bw/2 and oh==bh/2
class psImg2ToFloat4ohalfMP : public psImg2ToFloat4 {
public:
  psImg2ToFloat4ohalfMP(LPDIRECT3DDEVICE9 pDevice, Texture* src, int bw, int bh, bool chroma);
  HRESULT Apply(Texture* src, Texture* factor, TextureRT *dst1, TextureRT *dst2);
protected:
  Pixelshader pass2;
  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;
};

//multi pass interlaced ow==bw/2 and oh==bh/2
class psImg2ToFloat4ohalfInterlacedMP : public psImg2ToFloat4 {
public:
  psImg2ToFloat4ohalfInterlacedMP(LPDIRECT3DDEVICE9 pDevice, Texture* src, TextureRT* dst, int bw, int bh, bool chroma);
  HRESULT Apply(Texture* src, Texture* factor, TextureRT *dst1, TextureRT *dst2);
  ~psImg2ToFloat4ohalfInterlacedMP();
protected:
  Pixelshader pass2;
  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;
  NQuad* bottomquad;
  TextureRT* temptex;
  RECT r[12];
};

//single pass non interlaced ow!=bw/2 or oh!=bh/2
class psImg2ToFloat4oSP : public psImg2ToFloat4 {
public:
  psImg2ToFloat4oSP(LPDIRECT3DDEVICE9 pDevice, Texture* src, int bw, int bh, int ow, int oh, bool chroma);
  HRESULT Apply(Texture* src, Texture *factor, TextureRT *dst1, TextureRT *dst2);
  ~psImg2ToFloat4oSP();
protected:
  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;
  OQuad* oquad;
};

//single pass interlaced ow!=bw/2 or oh!=bh/2
class psImg2ToFloat4oInterlacedSP : public psImg2ToFloat4 {
public:
  psImg2ToFloat4oInterlacedSP(LPDIRECT3DDEVICE9 pDevice, Texture* src, TextureRT* dst, int bw, int bh, int ow, int oh, bool chroma);
  HRESULT Apply(Texture* src, Texture *factor, TextureRT *dst1, TextureRT *dst2);
  ~psImg2ToFloat4oInterlacedSP();
protected:
  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;
  OIQuad* oiquad;
  RECT r[12];
  TextureRT* temptex;
};

//multi pass non interlaced ow!=bw/2 or oh!=bh/2
class psImg2ToFloat4oMP : public psImg2ToFloat4 {
public:
  psImg2ToFloat4oMP(LPDIRECT3DDEVICE9 pDevice, Texture* src, int bw, int bh, int ow, int oh, bool chroma);
  HRESULT Apply(Texture* src, Texture *factor, TextureRT *dst1, TextureRT *dst2);
  ~psImg2ToFloat4oMP();
protected:
  Pixelshader pass2;
  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;
  OQuad* oquad;
};

//multi pass interlaced ow!=bw/2 or oh!=bh/2
class psImg2ToFloat4oInterlacedMP : public psImg2ToFloat4 {
public:
  psImg2ToFloat4oInterlacedMP(LPDIRECT3DDEVICE9 pDevice, Texture* src, TextureRT* dst, int bw, int bh, int ow, int oh, bool chroma);
  HRESULT Apply(Texture* src, Texture *factor, TextureRT *dst1, TextureRT *dst2);
  ~psImg2ToFloat4oInterlacedMP();
protected:
  Pixelshader pass2;
  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;
  OIQuad* oiquad;
  RECT r[12];
  TextureRT* temptex;
};

//converts float src to 8 bit dst
class psFloat4ToImg2 : public Pixelshader {
public:
  psFloat4ToImg2(LPDIRECT3DDEVICE9 pDevice, LPCSTR pSrcFile, LPCSTR pFunctionName, LPCSTR pProfile, D3DXMACRO* defs = 0) :
    Pixelshader(pDevice, pSrcFile, pFunctionName, pProfile, defs) {}
  virtual ~psFloat4ToImg2() {};
  virtual HRESULT Apply(Texture* src1, Texture* src2, Texture *factor, TextureRT *dst) = 0;
};

//non interlaced ow==bw/2 and oh==bh/2
class psFloat4ToImg2ohalf : public psFloat4ToImg2 {
public:
  psFloat4ToImg2ohalf(LPDIRECT3DDEVICE9 pDevice, int width, int height, int bw, int bh, int srcwidth, int srcheight, bool chroma, bool fullsizefactor);
  HRESULT Apply(Texture* src1, Texture* src2, Texture *factor, TextureRT *dst);
protected:
  D3DXHANDLE sSrc1;
  D3DXHANDLE sSrc2;
  D3DXHANDLE sFactor;
};

//interlaced ow==bw/2 and oh==bh/2
class psFloat4ToImg2ohalfInterlaced : public psFloat4ToImg2 {
public:
  psFloat4ToImg2ohalfInterlaced(LPDIRECT3DDEVICE9 pDevice, TextureRT* src, TextureRT* dst, int bw, int bh, GPUTYPES* _gtype, bool chroma, bool fullsizefactor);
  HRESULT Apply(Texture* src1, Texture* src2, Texture *factor, TextureRT *dst);
  ~psFloat4ToImg2ohalfInterlaced();
protected:
  D3DXHANDLE sSrc1;
  D3DXHANDLE sSrc2;
  D3DXHANDLE sFactor;
  IDirect3DSurface9* zbuffer;
  TextureRT* ztex;
  NQuad* evenfieldquad;
  HRESULT ResetShader(LPDIRECT3DDEVICE9 pDevice, bool firstpass);
};

//non interlaced ow!=bw/2 or oh!=bh/2
class psFloat4ToImg2o : public psFloat4ToImg2 {
public:
  psFloat4ToImg2o(LPDIRECT3DDEVICE9 pDevice, int width, int height, int bw, int bh, int ow, int oh, int srcwidth, int srcheight, bool chroma);
  HRESULT Apply(Texture* src1, Texture* src2, Texture *factor, TextureRT *dst);
  ~psFloat4ToImg2o();
protected:
  D3DXHANDLE sSrc1;
  D3DXHANDLE sSrc2;
  D3DXHANDLE sFactor;
  Pixelshader nooverlapxy;
  Pixelshader nooverlapzw;
  Pixelshader horizontalxy;
  Pixelshader horizontalzw;
  Pixelshader vertical;
  OQuad* oquad;
};

//interlaced ow!=bw/2 or oh!=bh/2
class psFloat4ToImg2oInterlaced : public psFloat4ToImg2 {
public:
  psFloat4ToImg2oInterlaced(LPDIRECT3DDEVICE9 pDevice, Texture* src, TextureRT* dst, int bw, int bh, GPUTYPES* _gtype, int ow, int oh, bool chroma);
  HRESULT Apply(Texture* src1, Texture* src2, Texture *factor, TextureRT *dst);
  ~psFloat4ToImg2oInterlaced();
protected:
  D3DXHANDLE sSrc1;
  D3DXHANDLE sSrc2;
  D3DXHANDLE sFactor;
  Pixelshader nooverlapxy;
  Pixelshader nooverlapzw;
  Pixelshader horizontalxy;
  Pixelshader horizontalzw;
  Pixelshader vertical;
  IDirect3DSurface9* zbuffer;
  TextureRT* ztex;

  OIQuad* oevenquad;
  OIQuad* ooddquad;
  HRESULT ResetShader(LPDIRECT3DDEVICE9 pDevice, bool firstpass);
};

//******************************************************************************************************
class psSharpen : public Pixelshader {
public:
  psSharpen(LPDIRECT3DDEVICE9 pDevice, Texture* _Rendertarget, float sigmaSquaredSharpenMax, float sigmaSquaredSharpenMin, bool degrid);
  HRESULT Apply(Texture* src, Texture* Factor, TextureRT *dst, Texture* degrid = 0);
protected:
  D3DXHANDLE sSrc;
  D3DXHANDLE sFactor;
  D3DXHANDLE sDegrid;
};


class psFFTtoFixed : public Pixelshader {
public:
  psFFTtoFixed(LPDIRECT3DDEVICE9 pDevice, RECT src, RECT dst, bool FFT);
  ~psFFTtoFixed();
  HRESULT Apply(Texture* src, TextureRT *dst, bool backbuffer, bool endscene);
protected:
  D3DXHANDLE sSrc;
  IDirect3DSurface9* pback;
  HRESULT ResetShader(LPDIRECT3DDEVICE9 pDevice, bool firstpass);
};
/*
class psMeanSD : public Pixelshader{
  public:
    psMeanSD(LPDIRECT3DDEVICE9 pDevice,RECT src);
    HRESULT Apply(Texture* src,TextureRT* dst);
  protected:
    D3DXHANDLE sSrc;
};
*/