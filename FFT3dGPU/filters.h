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

#include <stack>
#include <vector>
//#include "texture.h"
#include "filtersps.h"
#include "windows.h"


class ImgStream {
public:
  ImgStream(unsigned int x, unsigned int xnum, unsigned int y, unsigned int ynum, int mode, unsigned int width, unsigned int height, LPDIRECT3DDEVICE9 _pDevice, GPUTYPES* _gtype, bool _useHalf, int bits_per_pixel, HRESULT &hr, int border = 0);


  ~ImgStream();
  void ImgToStream(Texture* src, pTextureRTpair *dst);
  void ImgToStream(Texture* src, TextureRT* dst);

  void StreamToImg(Texture* src, TextureRT* dst);
  void StreamToImg(pTextureRTpair *src, TextureRT* dst);

  static void CreateFactorMap(float* Map, unsigned int x, unsigned int xnum, unsigned int y, unsigned int ynum, bool full);
protected:
  void CreateFactorMapBorder(float* Map, int x, int y, int border);
  TextureM *FactorLUT;
  //TextureM *Img;
  TextureRT *Imgd;
  psImg2toFloat4 *Img2toFloat4;
  psFloat4toImg2 *Float4toImg2;
  psImg2toFloat4_2 *Img2toFloat4_2;
  psFloat4toImg2_2 *Float4toImg2_2;

  psImg2toImg4* conv;

  unsigned int lastpitch;
  unsigned int bw, bh;

  LPDIRECT3DDEVICE9 pDevice;
  GPUTYPES* gtype;


  //psFFTtoFixed* FFTtoFixed;
};

class ImgStream2
{
public:
  ImgStream2(unsigned int bw, unsigned int bh, int ow, int oh, Texture* src, TextureRT* fdst, TextureRT* dst, bool chroma, int wintype, bool interlaced, LPDIRECT3DDEVICE9 _pDevice, GPUTYPES* _gtype, bool _useHalf, int bits_per_pixel, HRESULT &hr);
  ~ImgStream2();
  void ImgToTexture(Texture* src, pTextureRTpair *dst);
  void TextureToImg(pTextureRTpair *src, TextureRT* dst);
  void TextureToSrc(pTextureRTpair *src, UCHAR* dst, int pitch);
protected:
  void CreateFactorMap(float* MapAna, float* MapSyn, unsigned int bw, unsigned int bh, int ow, int oh, int wintype);

  TextureM *FactorLUTana;
  TextureM *FactorLUTsyn;
  psImg2ToFloat4* i2f;
  psFloat4ToImg2* f2i;
  TextureRT *Imgd;

  psImg2toImg4* conv;
  LPDIRECT3DDEVICE9 pDevice;
  GPUTYPES* gtype;
};

class KalmanFilter {
public:
  KalmanFilter(TexturePool *_StreamPoolPointer, float sigmaSquaredNoiseNormed2D, float kratio, bool useTexturepair, LPDIRECT3DDEVICE9 _pDevice);
  KalmanFilter(TexturePool *_StreamPoolPointer, float kratio, bool useTexturepair, LPDIRECT3DDEVICE9 _pDevice, TextureM *pattern);
  ~KalmanFilter();
  void Filter(Texture* src, TextureRT* dst);
  void Filter(pTextureRTpair* src, pTextureRTpair* dst);
  void Restore();
protected:
  void Init(bool useTexturepair);
  psKalman* ps;
  LPDIRECT3DDEVICE9 pDevice;
  TexturePool *StreamPoolPointer;
  TextureRT* covar1;
  pTextureRTpair* covar1d;
  TextureRT* covar2;
  pTextureRTpair* covar2d;
  TextureRT* covarprocess1;
  pTextureRTpair* covarprocess1d;
  TextureRT* covarprocess2;
  pTextureRTpair* covarprocess2d;
  Texture* last;
  pTextureRTpair* lastd;
  TextureM* _pattern;
  float _sigmaSquaredNoiseNormed2D;
};

class Sharpen {
public:
  Sharpen(float strength, float svr, float scutoff, float sigmaSquaredSharpenMin, float sigmaSquaredSharpenMax,
    unsigned int _x, unsigned int _xnum, unsigned int _y, unsigned int _ynum, bool degrid, TexturePool *StreamPoolPointer, HRESULT &hr,
    LPDIRECT3DDEVICE9 pDevice, GPUTYPES* gtype);
  Sharpen::~Sharpen();
  void Filter(TextureRT* src, TextureRT* (&dst), TextureRT* degrid = 0);
  void Filter(pTextureRTpair* src, pTextureRTpair* dst, pTextureRTpair* degrid = 0);
protected:
  //float Strength;
  TextureM* FilterLUT;
  psSharpen* Sharp;
  TexturePool *FreeStreamPool;
};

