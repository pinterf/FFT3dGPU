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

#include "TexturePool.h"
#include "filters.h"
#include "./core/Debug class.h"
#include <dxerr.h>
#include <d3d9.h>

const double pi(acos(-1.0));

/*  ImgStream
 *  Constructor: Set up the neccesary pixelshaders and textures to convert a unsigned char/uint16_t/float source array to a float texture and back
 *
 * Inputs:
 *		x:[in] block width
 *		xnum:[in] number of blocks in width
 *		y:[in] block height
 *		ynum:[in] number of blocks in height
 *		mode:[in] this class supports 3 mode:
 *			0:   1:1 overlap modified cosine window function used
 *			1:	 2:1 overlap half cosine window used
 *			2:   1:1 overlap border is added
 *		width:[in] width of src
 *		height:[in] height of src
 *		_pDevice:[in] Direct3d device to be used
 *		_gtypes:[in] Containes texture information
 *		_useHalf:[in] use 16bit precision/else 32 bit in textures
 *		hr:[out] return result
 *		border:[in] bordersize in pair of pixels
 *
 * Returns:
 *		None
 *
 * Remarks:
 */


ImgStream::ImgStream(unsigned int x, unsigned int xnum, unsigned int y, unsigned int ynum, int mode, unsigned int width, unsigned int height, LPDIRECT3DDEVICE9 _pDevice, GPUTYPES* _gtype, bool _useHalf, int bits_per_pixel, HRESULT &hr, int border)
  :FactorLUT(0), lastpitch(0), bw(x), bh(y), pDevice(_pDevice), gtype(_gtype), Img2toFloat4_2(0), Float4toImg2_2(0)
{
  // Scale 10-14 bits to 16 bits in Img2toFloat4 Img2toFloat4MP and Img2toFloat4_2, back to original bit depth in Img2toImg4

  unsigned int yn = y * ynum;
  unsigned int xn = x * xnum;
  unsigned int xn2 = xn / 2;
  //Img=texture to upload src to
  TextureM* Img = NEW  TextureM(pDevice, (width + 1) >> 1, height, bits_per_pixel == 8 ? gtype->FIXED2() : bits_per_pixel <= 16 ? gtype->FIXED2_16() : gtype->FLOAT2(), hr);//NEW  TextureM(pDevice,xn2,yn,gtype->FIXED2(),hr);
  if (FAILED(hr)) {
    hr = DXTrace("fft.cpp", __LINE__, hr, "ImgStream::Create Img", true); return;
  }
  //Imgd=texture to render float texture to
  /*TextureRT* */
  Imgd = NEW  TextureRT(pDevice, (width + 1) >> 1, height, bits_per_pixel == 8 ? gtype->FIXED2() : bits_per_pixel <= 16 ? gtype->FIXED2_16() : gtype->FLOAT2(), hr);

  // fixme PF: really + 1 and not +3? (width + 1)>>2?? Yes let it be +3
  TextureRT* Imgr = NEW  TextureRT(pDevice, (width + 3) >> 2, height, bits_per_pixel == 8 ? gtype->FIXED4() : bits_per_pixel <= 16 ? gtype->FIXED4_16() : gtype->FLOAT4(), hr); // debug
  if (FAILED(hr)) {
    hr = DXTrace("fft.cpp", __LINE__, hr, "ImgStream::Create Imgd", true); return;
  }
  conv = NEW psImg2toImg4(_pDevice, Imgd, Imgr, bits_per_pixel);
  //offset= the amount the src is shifted
  D3DXVECTOR2 offset = D3DXVECTOR2(0.25*(bw), 0.5*bh);
  RECT Rendertarget;
  Rendertarget.top = 0;
  Rendertarget.left = 0;
  Rendertarget.bottom = yn;
  Rendertarget.right = xn2;
  RECT src = Img->GetRect();
  src.left -= 0.25*bw;
  src.right -= 0.25*bw;
  src.top -= 0.5*bh;
  src.bottom -= 0.5*bh;

  float* factorMap = 0;
  if (mode != 2) {
    //setup the window coefficents
    if (_useHalf)
      FactorLUT = NEW  TextureM(pDevice, xn2, yn, gtype->HALF2(), hr);
    else
      FactorLUT = NEW  TextureM(pDevice, xn2, yn, gtype->FLOAT2(), hr);
    if (FAILED(hr)) {
      hr = DXTrace("fft.cpp", __LINE__, hr, "ImgStream::Create FactorLUT", true); return;
    }
    factorMap = NEW  float[xn*yn];
    //HACK to disable window function
    //for(int i=0;i<xn*yn;i++)
    //	factorMap[i]=sqrt(0.5);
    CreateFactorMap(factorMap, x, xnum, y, ynum, mode == 1);
    //Pixelshader initialization
    Img2toFloat4 = NEW  psImg2toFloat4m0(_pDevice, Rendertarget, src, offset, Img, bits_per_pixel);
    Float4toImg2 = NEW  psFloat4toImg2m0(_pDevice, Imgd, FactorLUT, offset);
    //when mode=1 an extra texture is needed to be processed.
    if (mode == 1) {
      Img2toFloat4_2 = NEW  psImg2toFloat4_2(_pDevice, Rendertarget, src, offset, Img, bits_per_pixel);
      Float4toImg2_2 = NEW  psFloat4toImg2_2(_pDevice, Imgd, FactorLUT, offset);
    }
  }
  else//mode=2
  {
    if (_useHalf)
      FactorLUT = NEW  TextureM(pDevice, bw / 2, bh, gtype->HALF2(), hr);
    else
      FactorLUT = NEW  TextureM(pDevice, bw / 2, bh, gtype->FLOAT2(), hr);
    if (FAILED(hr)) {
      hr = DXTrace("fft.cpp", __LINE__, hr, "ImgStream::Create FactorLUT", true); return;
    }
    Img2toFloat4 = NEW  psImg2toFloat4m2(_pDevice, bw / 2, bh, xnum, ynum, border, Img, bits_per_pixel);
    Float4toImg2 = NEW  psFloat4toImg2m2(_pDevice, bw / 2, bh, xnum, ynum, border, Imgd);
    factorMap = NEW  float[x*y];
    CreateFactorMapBorder(factorMap, x, y, border);
  }
  UploadToTexture(FactorLUT, factorMap);
  delete[] factorMap;
  delete Imgr;
  delete Img;
}


/*
 * CreateFactorMap
 *		Creates the window function coefficient. When using full overlap (2:1) the window used is a half cosine window
 *      else a modified cosine function is used.
 *
 * Inputs:
 *     Map:[out] Pointer to the float array that will contain the return map. Must have at least size: x*xnum*y*ynum
 *     x:[in] block width
 *     xnum:[in] number of blocks in width
 *     y:[in] block height
 *     ynum:[in] number of blocks in height
 *     full:[in] full overlap?
 *
 * Returns:
 *     None
 *
 * Remarks:
 *
 *
 */
void ImgStream::CreateFactorMap(float* Map, unsigned int x, unsigned int xnum, unsigned int y, unsigned int ynum, bool full)
{
  double cosy, cosx;
  unsigned int offset = 0;
  double x1 = x;
  double y1 = y;
  double* Factormap = NEW  double[x*y];
  for (double n1 = -y1 / 2.0 + 0.5; n1 < y1 / 2.0; n1++) {
    cosy = cos(n1*pi / (y1));
    for (double n2 = -x1 / 2.0 + 0.5; n2 < x1 / 2.0; n2++) {
      cosx = cos(n2*pi / (x1));
      if (full)
        Factormap[offset++] = cosy * cosx;
      else
        Factormap[offset++] = sqrt(0.5*(cosx*cosx + cosy * cosy));
      //Factormap[offset++]=cosy*cosx;
    }
  }

  offset = 0;
  for (unsigned int repy = 0; repy < ynum; repy++)
    for (unsigned int j = 0; j < y; j++)
      for (unsigned int repx = 0; repx < xnum; repx++)
        for (unsigned int i = 0; i < x; i++)
          Map[offset++] = Factormap[i + x * j];
  delete[] Factormap;


}

/*
 * CreateFactorMapBorder
 *
 *		Creates the window function coefficient. The window used is a modified cosine window.
 *
 * Inputs:
 *     Map:[out] Pointer to the float array that will contain the return map. Must have at least size x*y
 *     x:[in] block width
 *     y:[in] block height
 *     border:[in] bordersize in pair of pixels
 *
 * Returns:
 *     None
 *
 * Remarks:
 *
 *
 */
void ImgStream::CreateFactorMapBorder(float* Map, int x, int y, int border)
{
  int borderx = border * 2;
  int bordery = border * 2;
  double cosy, cosx;
  unsigned int offset = 0;
  double x1 = x;
  double y1 = y;
  //double* Factormap=NEW  double[x*y];
  for (double n1 = -y1 / 2.0 + 0.5; n1 < y1 / 2.0; n1++) {
    /*double compy=0.5*(y1-2*bordery);
    if(n1<-compy||n1>compy)
      cosy=0;//cos(n1*pi/(y1-2*bordery+(-2*compy+1+abs(n1*2))));
    else*/
    cosy = cos(n1*pi / (y1 - 2 * bordery));
    int signy = cosy / abs(cosy);
    for (double n2 = -x1 / 2.0 + 0.5; n2 < x1 / 2.0; n2++) {
      /*double compx=0.5*(x1-2*borderx);
      if(n2<-compx||n2>compx)
        cosx=0;//cos(n2*pi/(x1-2*borderx+(-2*compx+1+abs(n2*2))));
      else*/
      cosx = cos(n2*pi / (x1 - 2 * borderx));
      int signx = cosx / abs(cosx);
      double res = (0.5*(signx*cosx*cosx + signy * cosy*cosy));
      Map[offset++] = res < 0 ? 0 : res;
      //Map[offset-1]=sqrt(0.5);
    }
  }

}

ImgStream::~ImgStream() {
  delete FactorLUT;
  //delete Img;
  delete conv;
  delete Imgd;
  delete Img2toFloat4;
  delete Float4toImg2;
  delete Img2toFloat4_2;
  delete Float4toImg2_2;
}

/*
 * ImgToStream
 *
 *		Converts a const char array to a float4 texture what contains the src*window function + shifted src*window function
 *
 * Inputs:
 *     src:[in] Pointer to src array that contains the data to be converted
 *     dst:[out] Texture to upload the src to
 *     pitch:[in] src pitch
 *
 * Returns:
 *     None
 *
 * Remarks:
 *     The difference between the two version of ImgToStream is that one of them is used with mode 1
 */

 //mode 0 and 2
void ImgStream::ImgToStream(Texture* Img, TextureRT* dst) {
  PROFILE_BLOCK

  //WriteMemFixedToFile(src,Img->GetWidth()*2,Img->GetHeight(),1,"Src.txt","src");
  //UploadToTexture(Img,src,pitch);
  Img2toFloat4->Apply(Img, FactorLUT, dst);
#if 0
  {
    float* ftemp = NEW  float[dst->GetWidth() * dst->GetHeight() * 4];
    DownloadFromTexture(dst, ftemp, 0);
    if (Img->GetType()->type == _FIXED)
      WriteMemFloatToFile(ftemp, dst->GetWidth(), dst->GetHeight(), 4/*elemsize*/, "Img2toFloat4_from_8.txt", "", false);
    else // from 10-16 bits or or float
      WriteMemFloatToFile(ftemp, dst->GetWidth(), dst->GetHeight(), 4, "Img2toFloat4_from_16.txt", "", false);
    delete ftemp;
  }
#endif

}

//mode 1
void ImgStream::ImgToStream(Texture* Img, pTextureRTpair *dst) {
  PROFILE_BLOCK

  //WriteMemFixedToFile(src,Img->GetWidth()*2,Img->GetHeight(),1,"Src.txt","src");
  //UploadToTexture(Img,src,pitch);
  Img2toFloat4->Apply(Img, FactorLUT, dst->first);
  Img2toFloat4_2->Apply(Img, FactorLUT, dst->last);
  /*
  float *ftemp=NEW  float[dst->GetWidth()*dst->GetHeight()*4];
  DownloadFromTexture(dst->first,ftemp,0);
  WriteMemFloatToFile(ftemp,dst->first->GetWidth(),dst->first->GetHeight(),4,"Img2toFloat4_first.txt","");
  float *ftemp2=NEW  float[dst->GetWidth()*dst->GetHeight()*4];
  DownloadFromTexture(dst->last,ftemp,0);
  WriteMemFloatToFile(ftemp2,dst->last->GetWidth(),dst->last->GetHeight(),4,"Img2toFloat4_last.txt","");
  */
}

/*
 * StreamToImg
 *
 *		Converts a float4 texture to a unsigned char array(the inverse function of ImgToStream)
 *
 * Inputs:
 *     src:[in] Pointer to src array that contains the data to be converted
 *     dst:[out] Texture to upload the src to
 *     pitch:[in] src pitch
 *
 * Returns:
 *     None
 *
 * Remarks:
 *     The difference between the two version of StreamToImg is that one of them is used with mode 1
 */


 //mode 0,2
void ImgStream::StreamToImg(Texture* src, TextureRT* dst) {
  PROFILE_BLOCK
  LOG("Float4toImg2...");

  Float4toImg2->Apply(src, FactorLUT, Imgd);
  conv->Apply(Imgd, dst);
  LOG("done" << std::endl << "DownloadFromTexture...");
  //pDevice->EndScene();
  //DownloadFromTexture(Imgd,dst,pitch);
  LOG("done")
}

//mode 1
void ImgStream::StreamToImg(pTextureRTpair *src, TextureRT* dst) {
  PROFILE_BLOCK
  Float4toImg2_2->Apply(src->first, src->last, FactorLUT, Imgd);
  conv->Apply(Imgd, dst);
  //
  //DownloadFromTexture(Imgd,dst,pitch);
}

/*		ImgStream2
 *  Constructor: Set up the neccesary pixelshaders and textures to convert a unsigned char/uint16_t/float source array to a float texture pair and back
 *
 * Inputs:
 *		bw:[in] block width
 *		bh:[in] block height
 *		ow:[in] overlap width
 *		oh:[in] overlap height
 *		src:[in] Texture containing the source image.
 *		fdst:[in] Texture containing the float destination to be send to the fft filter.
 *		dst:[in] Texture containing the final image that are converted from the float destination texture
 *		wintype:[in] weighting windows type
 *		_pDevice:[in] Direct3d device to be used
 *		_gtypes:[in] Containes texture information
 *		_useHalf:[in] use 16bit precision/else 32 bit in textures
 *		hr:[out] return result
 *
 * Returns:
 *		None
 *
 * Remarks:
 */

ImgStream2::ImgStream2(unsigned int bw, unsigned int bh, int ow, int oh, Texture* src, TextureRT* fdst, TextureRT* dst, bool chroma, int wintype, bool interlaced, LPDIRECT3DDEVICE9 _pDevice, GPUTYPES* _gtype, bool _useHalf, int bits_per_pixel, HRESULT &hr) :
  FactorLUTana(0), FactorLUTsyn(0), i2f(0), f2i(0), conv(0), pDevice(_pDevice), gtype(_gtype), Imgd(0)
{
  unsigned int width = src->GetWidth();
  unsigned int height = src->GetHeight();
  Imgd = NEW  TextureRT(pDevice, width, height, bits_per_pixel == 8 ? gtype->FIXED2() : bits_per_pixel <= 16 ? gtype->FIXED2_16() : gtype->FLOAT2(), hr);
  conv = NEW psImg2toImg4(pDevice, Imgd, dst, bits_per_pixel);
  D3DCAPS9 Cap;
  _pDevice->GetDeviceCaps(&Cap);
  //Cap.NumSimultaneousRTs=1;
  if (Cap.NumSimultaneousRTs > 1 || ow * 2 != bw || oh * 2 != bh)//NPOW2 instead?
  {
    FactorLUTana = NEW TextureM(pDevice, bw / 2, bh, gtype->FLOAT4(), hr);
    FactorLUTsyn = NEW TextureM(pDevice, bw / 2, bh, gtype->FLOAT4(), hr);
  }
  else
  {
    FactorLUTana = NEW TextureM(pDevice, fdst->GetWidth(), fdst->GetHeight(), gtype->FLOAT4(), hr);
    FactorLUTsyn = NEW TextureM(pDevice, fdst->GetWidth(), fdst->GetHeight(), gtype->FLOAT4(), hr);
  }
  if (ow * 2 == bw && oh * 2 == bh)//half overlap
  {


    if (Cap.NumSimultaneousRTs > 1)
    {
      if (interlaced)
      {
        i2f = NEW psImg2ToFloat4ohalfInterlacedSP(pDevice, src, fdst, bw / 2, bh, chroma, bits_per_pixel);
        f2i = NEW psFloat4ToImg2ohalfInterlaced(pDevice, fdst, Imgd, bw / 2, bh, _gtype, chroma, bits_per_pixel, false);
      }
      else
      {
        i2f = NEW psImg2ToFloat4ohalfSP(pDevice, src, bw / 2, bh, chroma, bits_per_pixel);
        f2i = NEW psFloat4ToImg2ohalf(pDevice, width, height, bw / 2, bh, fdst->GetWidth(), fdst->GetHeight(), chroma, bits_per_pixel, false);
      }

    }
    else
    {
      if (interlaced)
      {
        i2f = NEW psImg2ToFloat4ohalfInterlacedMP(pDevice, src, fdst, bw / 2, bh, chroma, bits_per_pixel);
        f2i = NEW psFloat4ToImg2ohalfInterlaced(pDevice, fdst, Imgd, bw / 2, bh, _gtype, chroma, bits_per_pixel, true);
      }
      else
      {
        i2f = NEW psImg2ToFloat4ohalfMP(pDevice, src, bw / 2, bh, chroma, bits_per_pixel);
        f2i = NEW psFloat4ToImg2ohalf(pDevice, width, height, bw / 2, bh, fdst->GetWidth(), fdst->GetHeight(), chroma, bits_per_pixel, true);
      }
    }

  }
  else
  {
    if (Cap.NumSimultaneousRTs > 1)
    {
      if (interlaced)
      {
        i2f = NEW psImg2ToFloat4oInterlacedSP(pDevice, src, fdst, bw / 2, bh, ow / 2, oh, chroma, bits_per_pixel);
        f2i = NEW psFloat4ToImg2oInterlaced(pDevice, fdst, Imgd, bw / 2, bh, _gtype, ow / 2, oh, chroma, bits_per_pixel);
      }
      else
      {
        i2f = NEW psImg2ToFloat4oSP(pDevice, src, bw / 2, bh, ow / 2, oh, chroma, bits_per_pixel);
        f2i = NEW psFloat4ToImg2o(pDevice, width, height, bw / 2, bh, ow / 2, oh, fdst->GetWidth(), fdst->GetHeight(), chroma, bits_per_pixel);
      }
    }
    else
    {
      if (interlaced)
      {
        i2f = NEW psImg2ToFloat4oInterlacedMP(pDevice, src, fdst, bw / 2, bh, ow / 2, oh, chroma, bits_per_pixel);
        f2i = NEW psFloat4ToImg2oInterlaced(pDevice, fdst, Imgd, bw / 2, bh, _gtype, ow / 2, oh, chroma, bits_per_pixel);
      }
      else
      {
        i2f = NEW psImg2ToFloat4oMP(pDevice, src, bw / 2, bh, ow / 2, oh, chroma, bits_per_pixel);
        f2i = NEW psFloat4ToImg2o(pDevice, width, height, bw / 2, bh, ow / 2, oh, fdst->GetWidth(), fdst->GetHeight(), chroma, bits_per_pixel);
      }
    }
  }
  //setup factor
  float* facana = NEW float[bw * 2 * bh];
  float* facsyn = NEW float[bw * 2 * bh];
  CreateFactorMap(facana, facsyn, bw, bh, ow, oh, wintype);
  if (Cap.NumSimultaneousRTs == 1 && (ow * 2 == bw && oh * 2 == bh))
  {
    int offset = 0;
    int ynum = fdst->GetHeight() / bh;
    int xnum = fdst->GetWidth() / (bw / 2);
    float* facana0 = NEW float[bw * 2 * bh*xnum*ynum];
    float* facsyn0 = NEW float[bw * 2 * bh*xnum*ynum];
    for (unsigned int repy = 0; repy < ynum; repy++)
      for (unsigned int j = 0; j < bh; j++)
        for (unsigned int repx = 0; repx < xnum; repx++)
          for (unsigned int i = 0; i < bw * 2; i++)
          {
            facsyn0[offset] = facsyn[i + 2 * bw*j];
            facana0[offset++] = facana[i + 2 * bw*j];
          }
    delete facana;
    delete facsyn;
    facana = facana0;
    facsyn = facsyn0;
  }

  UploadToTexture(FactorLUTana, facana);
  UploadToTexture(FactorLUTsyn, facsyn);
  delete facana;
  delete facsyn;

}

/*		CreateFactorMap
 *	creates the analysiz and syntesis window Look Up Table
 *
 *	Input:
 *	MapAna:[in,out]places the analysis window coefficient here
 *	MapSyn:[in,out]places the synthesis window coefficient here
 *	bw:[in]block width
 *	bh:[in]block height
 *	ow:[in]overlap width
 *	oh:[in]overlap height
 *	wintype:[in]window function to use
 *	0	half-cosine
 *	1	analysis: sqrt half-cosine syntesis: (half cosine)^1.5
 *	2	analysis: 1	syntesis: raised cosine
 *
 *	Return:
 *		void
 *
 *	Remarks:
 *		based on the window function code from fft3dfilter 1.8.5
 */
void ImgStream2::CreateFactorMap(float* MapAna, float* MapSyn, unsigned int bw, unsigned int bh, int ow, int oh, int wintype)
{
  double *wanx = NEW double[ow];//analysis window
  double *wany = NEW double[oh];
  double *wsynx = NEW double[ow];//syntesis window
  double *wsyny = NEW double[oh];
  switch (wintype)
  {
  case 0:
  {
    //Calc 1d half-cosine window
    for (int i = 0; i < ow; i++)
    {
      wanx[i] = cosf(pi*(i - ow + 0.5f) / (ow * 2));
      wsynx[i] = wanx[i];
    }
    for (int i = 0; i < oh; i++)
    {
      wany[i] = cosf(pi*(i - oh + 0.5f) / (oh * 2));
      wsyny[i] = wany[i];
    }
  }
  break;
  case 1:
  {
    for (int i = 0; i < ow; i++)
    {
      wanx[i] = sqrt(cosf(pi*(i - ow + 0.5f) / (ow * 2)));
      wsynx[i] = wanx[i] * wanx[i] * wanx[i];
    }
    for (int i = 0; i < oh; i++)
    {
      wany[i] = sqrt(cosf(pi*(i - oh + 0.5f) / (oh * 2)));
      wsyny[i] = wany[i] * wany[i] * wany[i];
    }
  }
  break;
  case 2:
  {
    for (int i = 0; i < ow; i++)
    {
      wanx[i] = 1.0;
      wsynx[i] = cosf(pi*(i - ow + 0.5f) / (ow * 2));
      wsynx[i] *= wsynx[i];
    }
    for (int i = 0; i < oh; i++)
    {
      wany[i] = 1.0;
      wsyny[i] = cosf(pi*(i - oh + 0.5f) / (oh * 2));
      wsyny[i] *= wsyny[i];
    }
  }
  break;
  }
  //Setup the 2D window;
  //analysis
  int offset = 0;
  for (int y = 0, x = 0, i = 0; y < oh; y++, offset += bw * 2, x = 0, i = 0) {
    for (; x < ow; x += 2, i += 4)
    {
      MapAna[offset + i] = wanx[x] * wany[y];
      MapAna[offset + i + 1] = wanx[x + 1] * wany[y];
      MapAna[offset + i + 2] = wanx[(ow - 1) - x] * wany[y];
      MapAna[offset + i + 3] = wanx[ow - x - 2] * wany[y];
      MapSyn[offset + i] = wsynx[x] * wsyny[y];
      MapSyn[offset + i + 1] = wsynx[x + 1] * wsyny[y];
      MapSyn[offset + i + 2] = wsynx[(ow - 1) - x] * wsyny[y];
      MapSyn[offset + i + 3] = wsynx[ow - x - 2] * wsyny[y];
    }
    for (; x < bw - ow; x += 2, i += 4)
    {
      MapAna[offset + i] = wany[y];
      MapAna[offset + i + 1] = wany[y];
      MapAna[offset + i + 2] = wany[y];
      MapAna[offset + i + 3] = wany[y];
      MapSyn[offset + i] = wsyny[y];
      MapSyn[offset + i + 1] = wsyny[y];
      MapSyn[offset + i + 2] = wsyny[y];
      MapSyn[offset + i + 3] = wsyny[y];
    }
    for (x = 0; x < ow; x += 2, i += 4)
    {
      MapAna[offset + i + 2] = wanx[x] * wany[y];
      MapAna[offset + i + 3] = wanx[x + 1] * wany[y];
      MapAna[offset + i] = wanx[(ow - 1) - x] * wany[y];
      MapAna[offset + i + 1] = wanx[ow - x - 2] * wany[y];
      MapSyn[offset + i + 2] = wsynx[x] * wsyny[y];
      MapSyn[offset + i + 3] = wsynx[x + 1] * wsyny[y];
      MapSyn[offset + i] = wsynx[(ow - 1) - x] * wsyny[y];
      MapSyn[offset + i + 1] = wsynx[ow - x - 2] * wsyny[y];
    }
  }
  for (int y = 0, x = 0, i = 0; y < bh - 2 * oh; y++, offset += bw * 2, x = 0, i = 0)
  {
    for (; x < ow; x += 2, i += 4)
    {
      MapAna[offset + i] = wanx[x];
      MapAna[offset + i + 1] = wanx[x + 1];
      MapAna[offset + i + 2] = wanx[(ow - 1) - x];
      MapAna[offset + i + 3] = wanx[ow - x - 2];
      MapSyn[offset + i] = wsynx[x];
      MapSyn[offset + i + 1] = wsynx[x + 1];
      MapSyn[offset + i + 2] = wsynx[(ow - 1) - x];
      MapSyn[offset + i + 3] = wsynx[ow - x - 2];
    }
    for (; x < bw - ow; x += 2, i += 4)
    {
      MapAna[offset + i] = 1;
      MapAna[offset + i + 1] = 1;
      MapAna[offset + i + 2] = 1;
      MapAna[offset + i + 3] = 1;
      MapSyn[offset + i] = 1;
      MapSyn[offset + i + 1] = 1;
      MapSyn[offset + i + 2] = 1;
      MapSyn[offset + i + 3] = 1;
    }
    for (x = 0; x < ow; x += 2, i += 4)
    {
      MapAna[offset + i + 2] = wanx[x];
      MapAna[offset + i + 3] = wanx[x + 1];
      MapAna[offset + i] = wanx[(ow - 1) - x];
      MapAna[offset + i + 1] = wanx[ow - x - 2];
      MapSyn[offset + i + 2] = wsynx[x];
      MapSyn[offset + i + 3] = wsynx[x + 1];
      MapSyn[offset + i] = wsynx[(ow - 1) - x];
      MapSyn[offset + i + 1] = wsynx[ow - x - 2];
    }
  }
  for (int y = oh - 1, x = 0, i = 0; y >= 0; y--, offset += bw * 2, x = 0, i = 0)
  {
    for (; x < ow; x += 2, i += 4)
    {
      MapAna[offset + i] = wanx[x] * wany[y];
      MapAna[offset + i + 1] = wanx[x + 1] * wany[y];
      MapAna[offset + i + 2] = wanx[(ow - 1) - x] * wany[y];
      MapAna[offset + i + 3] = wanx[ow - x - 2] * wany[y];
      MapSyn[offset + i] = wsynx[x] * wsyny[y];
      MapSyn[offset + i + 1] = wsynx[x + 1] * wsyny[y];
      MapSyn[offset + i + 2] = wsynx[(ow - 1) - x] * wsyny[y];
      MapSyn[offset + i + 3] = wsynx[ow - x - 2] * wsyny[y];
    }
    for (; x < bw - ow; x += 2, i += 4)
    {
      MapAna[offset + i] = wany[y];
      MapAna[offset + i + 1] = wany[y];
      MapAna[offset + i + 2] = wany[y];
      MapAna[offset + i + 3] = wany[y];
      MapSyn[offset + i] = wsyny[y];
      MapSyn[offset + i + 1] = wsyny[y];
      MapSyn[offset + i + 2] = wsyny[y];
      MapSyn[offset + i + 3] = wsyny[y];
    }
    for (x = 0; x < ow; x += 2, i += 4)
    {
      MapAna[offset + i + 2] = wanx[x] * wany[y];
      MapAna[offset + i + 3] = wanx[x + 1] * wany[y];
      MapAna[offset + i] = wanx[(ow - 1) - x] * wany[y];
      MapAna[offset + i + 1] = wanx[ow - x - 2] * wany[y];
      MapSyn[offset + i + 2] = wsynx[x] * wsyny[y];
      MapSyn[offset + i + 3] = wsynx[x + 1] * wsyny[y];
      MapSyn[offset + i] = wsynx[(ow - 1) - x] * wsyny[y];
      MapSyn[offset + i + 1] = wsynx[ow - x - 2] * wsyny[y];
    }
  }
  delete wanx;
  delete wany;
  delete wsynx;
  delete wsyny;

}



ImgStream2::~ImgStream2()
{
  if (FactorLUTana != FactorLUTsyn)
    delete FactorLUTana;
  delete FactorLUTsyn;
  delete i2f;
  delete f2i;
  delete conv;
  delete Imgd;
}

void ImgStream2::ImgToTexture(Texture* src, pTextureRTpair *dst)
{
  PROFILE_BLOCK
    i2f->Apply(src, FactorLUTana, dst->first, dst->last);

}

void ImgStream2::TextureToImg(pTextureRTpair *src, TextureRT* dst)
{
  PROFILE_BLOCK
    f2i->Apply(src->first, src->last, FactorLUTsyn, Imgd);
  conv->Apply(Imgd, dst);
}

void ImgStream2::TextureToSrc(pTextureRTpair *src, UCHAR* dst, int pitch)
{
  PROFILE_BLOCK
    pDevice->BeginScene();
  f2i->Apply(src->first, src->last, FactorLUTsyn, Imgd);
  pDevice->EndScene();
  DownloadFromTexture(Imgd, dst, pitch);

}

/*
 * KalmanFilter
 *	constructor, Setup filter to do the WiennerFiltering.
 *
 * Inputs:
 *	_x:[in] block width
 *	_xnum:[in] number of blocks in width
 *	_y:[in] block height
 *	_ynum:[in] number of blocks in height
 *	_bt:[in] temporal width
 *	_StreamPoolPointer:[out] Pointer to an empty textureRT stack
 *	_StreamPoolSize:[in] Size of the texturepool to create
 *  sigma:[in] sigma value
 *  beta:[in] beta value
 *	_pDevice:[in]	Direct3d device to use
 *  _gtype:[in] Texture information
 *	_useHalf:[in] use 16bit precision float texture/else 32 bit
 *  _hr:[out] return result
 *
 * Returns:
 *     None
 *
 * Remarks:
 *
 */
KalmanFilter::KalmanFilter(TexturePool *_StreamPoolPointer, float sigmaSquaredNoiseNormed2D, float kratio, bool useTexturepair, LPDIRECT3DDEVICE9 _pDevice) :
  pDevice(_pDevice), StreamPoolPointer(_StreamPoolPointer), covar1d(0), covar2d(0), covarprocess1d(0), covarprocess2d(0), lastd(0),
  covar1(0), covar2(0), covarprocess1(0), covarprocess2(0), last(0), _sigmaSquaredNoiseNormed2D(sigmaSquaredNoiseNormed2D), _pattern(0)
{
  Init(useTexturepair);
  D3DCAPS9 Cap;
  LOG("GetCaps...")
    pDevice->GetDeviceCaps(&Cap);
  if (Cap.NumSimultaneousRTs > 1)
    ps = NEW psKalmanSP(pDevice, StreamPoolPointer->top(), sigmaSquaredNoiseNormed2D, kratio*kratio, false);
  else
    ps = NEW psKalmanMP(pDevice, StreamPoolPointer->top(), sigmaSquaredNoiseNormed2D, kratio*kratio, false);
}


KalmanFilter::KalmanFilter(TexturePool *_StreamPoolPointer, float kratio, bool useTexturepair, LPDIRECT3DDEVICE9 _pDevice, TextureM *pattern) :
  pDevice(_pDevice), StreamPoolPointer(_StreamPoolPointer), covar1d(0), covar2d(0), covarprocess1d(0), covarprocess2d(0), lastd(0),
  covar1(0), covar2(0), covarprocess1(0), covarprocess2(0), last(0), _sigmaSquaredNoiseNormed2D(0), _pattern(pattern)
{
  Init(useTexturepair);
  D3DCAPS9 Cap;
  LOG("GetCaps...")
    pDevice->GetDeviceCaps(&Cap);
  if (Cap.NumSimultaneousRTs > 1)
    ps = NEW psKalmanSP(pDevice, StreamPoolPointer->top(), 0, kratio*kratio, true);
  else
    ps = NEW psKalmanMP(pDevice, StreamPoolPointer->top(), 0, kratio*kratio, true);

}


void KalmanFilter::Init(bool useTexturepair)
{
  float* f = NEW float[StreamPoolPointer->top()->Size()];
  if (_pattern)
  {
    D3DLOCKED_RECT f1d;
    float* f1;
    _pattern->SetData(&f1d, 0);
    f1 = (float*)f1d.pBits;
    for (int i = 0, j = 0; j < _pattern->Size() * 4; i += f1d.Pitch / 4)
      for (int x = 0; x < _pattern->GetWidth(); x++, j += 4)
        f[j] = f[j + 1] = f[j + 2] = f[j + 3] = f1[i + x];
    _pattern->SetDataEnd();
  }
  else
  {
    for (int i = 0; i < StreamPoolPointer->top()->GetHeight()*StreamPoolPointer->top()->GetWidth() * 4; i++)
      f[i] = _sigmaSquaredNoiseNormed2D;
  }
  if (useTexturepair)
  {
    covar1d = NEW pTextureRTpair();
    StreamPoolPointer->pop(*covar1d);
    covar2d = NEW pTextureRTpair();
    StreamPoolPointer->pop(*covar2d);
    covarprocess1d = NEW pTextureRTpair();
    StreamPoolPointer->pop(*covarprocess1d);
    covarprocess2d = NEW pTextureRTpair();
    StreamPoolPointer->pop(*covarprocess2d);

    UploadToTexture(covar1d->first, f);
    UploadToTexture(covarprocess1d->first, f);
    UploadToTexture(covar1d->last, f);
    UploadToTexture(covarprocess1d->last, f);

  }
  else
  {
    StreamPoolPointer->pop(covar1);
    StreamPoolPointer->pop(covar2);
    StreamPoolPointer->pop(covarprocess1);
    StreamPoolPointer->pop(covarprocess2);

    UploadToTexture(covar1, f);
    UploadToTexture(covarprocess1, f);

  }
  delete[] f;
}

KalmanFilter::~KalmanFilter()
{
  delete covar1;
  delete covar2;
  delete covarprocess1;
  delete covarprocess2;
  delete covar1d;
  delete covar2d;
  delete covarprocess1d;
  delete covarprocess2d;
  delete ps;
}

void KalmanFilter::Filter(Texture* src, TextureRT* dst)
{
  PROFILE_BLOCK
    if (!last)
      last = src;
  ps->Apply(src, last, covarprocess1, covar1, covarprocess2, covar2, dst, _pattern);
  std::swap(covar1, covar2);
  std::swap(covarprocess1, covarprocess2);
  last = dst;
}

void KalmanFilter::Filter(pTextureRTpair* src, pTextureRTpair* dst)
{
  PROFILE_BLOCK
    if (!lastd)
    {
      lastd = src;
      /*lastd=NEW pTextureRTpair();
      StreamPoolPointer->pop(*lastd);
      lastd->first->SetAsRenderTarget(0);
      lastd->last->SetAsRenderTarget(1);
      pDevice->Clear(0,0,D3DCLEAR_TARGET,0,0,0);*/
    }
  ps->Apply(src->first, lastd->first, covarprocess1d->first, covar1d->first, covarprocess2d->first, covar2d->first, dst->first, _pattern);
  ps->Apply(src->last, lastd->last, covarprocess1d->last, covar1d->last, covarprocess2d->last, covar2d->last, dst->last, _pattern);
  std::swap(covar1d, covar2d);
  std::swap(covarprocess1d, covarprocess2d);
  lastd = dst;
}

void KalmanFilter::Restore()
{
  if (covar1d)
  {
    float* f = NEW float[covar1d->first->GetHeight()*covar1d->first->GetWidth() * 4];
    if (_pattern)
    {
      D3DLOCKED_RECT f1d;
      float* f1;
      _pattern->SetData(&f1d, 0);
      f1 = (float*)f1d.pBits;
      for (int i = 0, j = 0; j < _pattern->Size() * 4; i += f1d.Pitch / 4)
        for (int x = 0; x < _pattern->GetWidth(); x++, j += 4)
          f[j] = f[j + 1] = f[j + 2] = f[j + 3] = f1[i + x];
      _pattern->SetDataEnd();
    }
    else
    {
      for (int i = 0; i < covar1d->first->GetHeight()*covar1d->first->GetWidth() * 4; i++)
        f[i] = _sigmaSquaredNoiseNormed2D;
    }
    UploadToTexture(covar1d->first, f);
    UploadToTexture(covarprocess1d->first, f);
    UploadToTexture(covar1d->last, f);
    UploadToTexture(covarprocess1d->last, f);
    delete[] f;
    lastd = 0;
  }
  else
  {
    float* f = NEW float[covar1->GetHeight()*covar1->GetWidth() * 4];
    if (_pattern)
    {
      D3DLOCKED_RECT f1d;
      float* f1;
      _pattern->SetData(&f1d, 0);
      f1 = (float*)f1d.pBits;
      for (int i = 0, j = 0; j < _pattern->Size() * 4; i += f1d.Pitch / 4)
        for (int x = 0; x < _pattern->GetWidth(); x++, j += 4)
          f[j] = f[j + 1] = f[j + 2] = f[j + 3] = f1[i + x];
      _pattern->SetDataEnd();
    }
    else
    {
      for (int i = 0; i < covar1->GetHeight()*covar1->GetWidth() * 4; i++)
        f[i] = _sigmaSquaredNoiseNormed2D;
    }
    UploadToTexture(covar1, f);
    UploadToTexture(covarprocess1, f);
    delete[] f;
    last = 0;
  }

}


/*
 * Sharpen
 *
 *		 Sharpens the image by increasing the high frequency values
 *
 * Inputs:
 *		strength:[in] sharpen strength. Negativ values will decrease high frequency values aka blur the image
 *		_x:[in] blockwidth
 *		_xnum:[in]number of blocks in width
 *		_y:[in]block height
 *		_ynum:[in]number of blocks in height
 *		StreamPoolPointer:[out] Pointer to an empty textureRT stack
 *		hr:[out] return result
 *		pDevice:[in]	Direct3d device to use
 *		gtype:[in] Texture information
 * Returns:
 *     None
 *
 * Remarks: This sharpening algoritm will produce ringing. I will change it to a unsharpen mask algoritm later
 *
 */
Sharpen::Sharpen(float strength, float svr, float scutoff, float sigmaSquaredSharpenMin, float sigmaSquaredSharpenMax,
  unsigned int _x, unsigned int _xnum, unsigned int _y, unsigned int _ynum, bool degrid, TexturePool *StreamPoolPointer, HRESULT &hr,
  LPDIRECT3DDEVICE9 pDevice, GPUTYPES* gtype) :
  FreeStreamPool(StreamPoolPointer)
{
  int bh = _y;
  int bw = _x;
  unsigned int x = (_x / 2 + 1);
  int width = x * _xnum;
  unsigned int y = _y;
  int height = y * _ynum;
  float* FactorMap = NEW  float[width*height];
  float* Factor = NEW  float[x*y];

  //from Fizick's code fft3dfilter 1.8
  for (unsigned int j = 0, offset = 0; j < y; j++, offset += x)
  {
    int dj = j;
    if (j >= y / 2)
      dj = y - j;
    float d2v = float(dj*dj)*(svr*svr) / ((bh / 2)*(bh / 2)); // v1.7
    for (unsigned int i = 0; i < x; i++)
    {
      float d2 = d2v + float(i*i) / ((bw / 2)*(bw / 2)); // distance_2 - v1.7
      Factor[i + offset] = strength * (1 - exp(-d2 / (2 * scutoff*scutoff)));
    }
  }

  int offset = 0;
  for (unsigned int yrep = 0, offset = 0; yrep < _ynum; yrep++)
    for (unsigned int j = 0; j < y; j++)
      for (unsigned int xrep = 0; xrep < _xnum; xrep++)
        for (unsigned int i = 0; i < x; i++)
          FactorMap[offset++] = Factor[i + j * x];

  FilterLUT = NEW  TextureM(pDevice, width, height, gtype->FLOAT(), hr);
  if (FAILED(hr)) { hr = DXTrace("fft.cpp", __LINE__, hr, "Sharpen::Create FilterLUT", true); return; }
  UploadToTexture(FilterLUT, FactorMap);
  delete[] Factor;
  delete[] FactorMap;
  Sharp = NEW  psSharpen(pDevice, FilterLUT, sigmaSquaredSharpenMax, sigmaSquaredSharpenMin, degrid);
}

Sharpen::~Sharpen()
{
  delete FilterLUT;
  delete Sharp;
}

void Sharpen::Filter(TextureRT* src, TextureRT* &dst, TextureRT* degrid) {
  PROFILE_BLOCK
    Sharp->Apply(src, FilterLUT, dst, degrid);
}

void Sharpen::Filter(pTextureRTpair* src, pTextureRTpair* dst, pTextureRTpair* degrid) {
  PROFILE_BLOCK
    if (degrid)
    {
      Sharp->Apply(src->first, FilterLUT, dst->first, degrid->first);
      Sharp->Apply(src->last, FilterLUT, dst->last, degrid->last);
    }
    else
    {
      Sharp->Apply(src->first, FilterLUT, dst->first);
      Sharp->Apply(src->last, FilterLUT, dst->last);
    }
}