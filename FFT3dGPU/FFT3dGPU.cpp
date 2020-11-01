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






//Use reference device (for debuging shaders)
//#define USE_REF

/*
//Log to file
//#define USE_LOG
*/

#ifdef PIXPROFILE
#define ADDFRAME(DEVICE) DEVICE->EndScene();DEVICE->Present( NULL, NULL, NULL, NULL );DEVICE->BeginScene();
#else
#define ADDFRAME(DEVICE)
#endif

#ifndef USE_REF
#define DEVICETYPE D3DDEVTYPE_HAL
#else
#define DEVICETYPE D3DDEVTYPE_REF
#endif


/*
#ifdef USE_LOG
#define LOG(a) outfile<<a<<std::endl;
#else
#define LOG(a)
#endif
*/


#include "stdafx.h"
#include "Dxinput.h"
#include "TexturePool.h"
#include "FFT2.h"
#include "filters.h"
#include "GPUCache.h"
#include "./core/Debug class.h"
#include "./core/ManageDirect3DWindow.h"
#include "FFT3dGPU.h"




//#define GFtest
/*
#ifdef USE_LOG
#include <fstream>
std::ofstream outfile("C:\FFT3dGPU_log.txt");
#endif
*/




//DllMain
//called every time the dll is loaded. It setups the module handle.
HINSTANCE  HM;

BOOL  APIENTRY  __stdcall DllMain(HINSTANCE hModule,
  DWORD  ul_reason_for_call,
  LPVOID lpReserved
)
{
  HM = hModule;
#ifdef _DEBUG
  //memory leak  detector
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
  return TRUE;
}

//template <class Type> void swap __inline 
ManageDirect3DWindow D3Dwindow;

int totproc = 0;
double tottime = 0.0;

double FFT3dGPU::MeasureFFT(FFT2d *_fft, TextureRT* In)
{
  double t;
  TextureRT* Out = 0;
  for (int i = 0; i < 5; i++)
  {
    D3Dwindow.BeginScene();
    _fft->CalcFFT(In, Out, true);
    _fft->CalcFFT(Out, In, false);
    D3Dwindow.EndScene();
  }
  Qstart();
  for (int i = 0; i < 10; i++)
  {
    D3Dwindow.BeginScene();
    _fft->CalcFFT(In, Out, true);
    _fft->CalcFFT(Out, In, false);
    D3Dwindow.EndScene();
  }
  D3Dwindow.StartGPU();
  D3Dwindow.SleepWhileGPUwork();
  t = Qendtime();
  delete Out;
  return t;
}
/*
 * FFT3dGPU
 *	constructor, Setup the filter and initialize the Wiennerfilter class(that does the fft/ifft and wiennerfiltering), the ImgStream class(that uploads/downloads the image to/from the GPU and applies the window function(custom/hanning window) and sharpen class
 *	also setups directx and check if the parameters is valid
 *
 * Inputs:
 *	cl1:[in] Input clip
 *	_sigma:[in] sigma value to pass to the wiennerfilter (defines how much noise should be removed)
 *	_beta:[in] beta value to pass to the wiennerfilter (defienes how much noise should be left after filtering)
 *	_bw:[in] block width (will be corrected to a power of 2)
 *	_bh:[in] block height(will be corrected to a power of 2)
 *	_bt:[in] temporal mode:(-1 sharpen,0 Kalman, 1-4 wiennerfilter with temporal size of 1-4 frames
 *	plane:[in] what plane to clean (0=Luma,1=chroma)
 *	_mode:[in] setup the overlap and window function to use(0=2 blocks per quaterblock,1=4 blocks per quaterblock,2= same as 0 with additional overlap
 *	border:[in] overlap size when using mode 2
 *	precision:[in]	specifies with precision to use. 16 bit float(half) (FLOAT16), 32 bit float (single precision) only in FFT(FLOAT32_FFT) or allways(FLOAT32_ALL)
 *  NVPerf:[in] use NVperfHUD
 *	_degrid:[in] apply degrid. Works best with mode=1
 *	scutoff:[in] low frequency cutoff for sharpen
 *	svr:[in]vertical sharpen strength ratio(vertical strength/horizontal strength)
 *	smin:[in]min sharpen limit
 *	smax:[in]max sharpen limit
 *	kratio:[in]threshold to sigma ratio to reset kalman filter
 *	getdst:[in]if not NULL the dst frame will be fetched from FFT3dGPUallPlane and the bitblt will be disabled
 *	ow:[in] block overlap width when using mode 2
 *	oh:[in] block overlap height when using mode 2
 *  env:[out] script enviroment
 *
 * Returns:
 *     None
 *
 * Remarks:
 *
 */

FFT3dGPU::FFT3dGPU(PClip cl1, float _sigma, float _beta, int _bw, int _bh, int _bt, float sharpen, int _plane, int _mode, int border, int precision,
  bool NVPerf, float _degrid, float scutoff, float svr, float smin, float smax, float kratio, int ow, int oh, int wintype, bool interlaced,
  float _sigma2, float _sigma3, float _sigma4, FFTCODE fftcode,
  FFT3dGPUallPlane* getdst, int _xRatio, int _yRatio, IScriptEnvironment* env) :
  GenericVideoFilter(cl1), 
  isLumaPlane(_plane <= 0), // _plane==0 => luma or RGBfirtPlane, _plane==1 => chroma U and V or RGB2nd3rdplane
  isYUY2(vi.IsYUY2()),
  isRGB(vi.IsRGB()),
  sigma(_sigma / 255), beta(_beta), bw(_bw), bh(_bh), height(vi.height / _yRatio), width(vi.width / _xRatio), img(0),
  FImg(0), FImg1(0), bt(_bt > 0 ? _bt : 1), bm(_bt), useHalf(precision == FLOAT16), sharpen(sharpen != 0), mode(_mode), caches{ 0,0,0 }, imgp(0), mutex(0),
  pDevice(d3ddevice.Device()), gtype(D3Dwindow.GetGPUType()), d3ddevice(D3Dwindow), NVPERF(NVPerf), fft(0),
  gridsample(0), mintex(0), selectDC(0), calcgridcorrection(0), degrid(_degrid), usePattern(_sigma != _sigma2 || _sigma3 != _sigma4 || _sigma != _sigma3),
  FImgDegrid(0), FImgDegridp(), current(bm == 1 ? 0 : bm == 4 ? 2 : 1),
  Pattern(0), sharp(0), wiennerfilter(0), kalmans{ 0,0,0 },
  FImg2(0), UploadImg{ 0,0,0 }, UploadImg1{ 0,0,0 }, UploadImg2{ 0,0,0 }, DownloadImg{ 0,0,0 },
  lastn(-1), nuploaded(-100), GetDst(getdst), convert2(0), convert(0),
  DI(NVPerf ? new Dxinput(HM, D3Dwindow.GetWindow()) : 0)
{
  // Check frame property support
  has_at_least_v8 = true;
  try { env->CheckVersion(8); }
  catch (const AvisynthError&) { has_at_least_v8 = false; }

  smin /= 255;
  smax /= 255;

  LOG("FFT3dGPU constructor address: " << std::hex << (unsigned int)this);
  imgp = 0;
  LOG("Thread ID: " << std::dec << GetCurrentThreadId() << std::hex);
  HRESULT hr = 1;
  if (!pDevice)
    env->ThrowError("Error Creating Direct3D Device");
  if (NVPerf)
  {
    D3Dwindow.Show();
    //pDevice->GetBackBuffer(0,0, D3DBACKBUFFER_TYPE_MONO,&backbuffer	);
  }
  /*
  if(!(mutex=CreateMutex(NULL,FALSE,"fft3dgpu_mutex")))
    env->ThrowError("Couldn't create mutex");
  WaitForSingleObject(mutex,INFINITE);*/
  D3DCAPS9 Cap;
  LOG("GetCaps...")
    pDevice->GetDeviceCaps(&Cap);
  LOG("done");
  LOG("Check pixelshader version")
    if (((unsigned char*)&Cap.PixelShaderVersion)[1] < 2)
      env->ThrowError("Only pixelshader 2.0 or greater supported");
  LOG("Setup device done");
  //Setup bw and bh to be a power of two
  unsigned int logn = 0;
  for (unsigned int i = (bw - 1); i > 0; i /= 2, logn++);
  bw = 1;
  for (unsigned int i = 1; i <= logn; i++)
    bw *= 2;
  if (bw < 4)
    bw = 4;
  if (bw > 512)
    bw = 512;
  logn = 0;
  for (unsigned int i = (bh - 1); i > 0; i /= 2, logn++);
  bh = 1;
  for (unsigned int i = 1; i <= logn; i++)
    bh *= 2;
  if (bh < 4)
    bh = 4;
  if (bh > 512)
    bh = 512;
  if (bm < -1 || bm>4)
    env->ThrowError("Valid modes for bt are -1,0,1,2,3,4");
  if (mode < 0 || mode>2)
    mode = 0;
  ow *= (float)bw / _bw;
  ow &= 0xFFFFFFFE;//makes sure that ow is even;
  oh *= (float)bh / _bh;
  if (oh * 2 > bh)
    oh = bh / 2;
  if (ow * 2 > bw)
    ow = bw / 2;
  if (precision<FLOAT16 || precision>FLOAT32_ALL)
    precision = FLOAT32_FFT;
  if (wintype < 0 || wintype>2)
    wintype = 0;

  LOG("Setup bw, bh done");
  //Calculate number of times the block(bw,bh) are repeated in x and y dimension
  if (mode != 2) {
    if (mode == 0 || (bw == ow * 2 && bh == oh * 2))//half overlap
    {
      if (mode == 1 && interlaced)
        ny = 2 * (height / 2 + bh * 1.5 - 1) / bh;
      else
        ny = (height + bh * 1.5 - 1) / bh;
      nx = (width + bw * 1.5 - 1) / bw;

    }
    else
    {
      nx = ((width + ow - 1) / (bw - ow) + 1 + 1) / 2;
      if (mode == 1 && interlaced)
        ny = 2 * (((height / 2 + oh - 1) / (bh - oh) + 1 + 1) / 2);
      else
        ny = ((height + oh - 1) / (bh - oh) + 1 + 1) / 2;
    }
  }
  else
  {
    nx = (width + (bw - 4 * border)*1.5 - 1) / (bw - 4 * border);
    ny = (height + (bh - 4 * border)*1.5 - 1) / (bh - 4 * border);
  }
  //Calculate total framesize
  totw = nx * bw;
  toth = ny * bh;
  LOG("Creating textures");
  if (isLumaPlane)
  {
    UploadImg[0] = NEW TextureM(pDevice, (width + 1) >> 1, height, gtype->FIXED2(), hr);
    UploadImg1[0] = NEW TextureM(pDevice, (width + 1) >> 1, height, gtype->FIXED2(), hr);
    UploadImg2[0] = NEW TextureM(pDevice, (width + 1) >> 1, height, gtype->FIXED2(), hr);
    DownloadImg[0] = NEW TextureRT(pDevice, (width + 3) >> 2, height, gtype->FIXED4(), hr);
  }
  else
  {
    UploadImg[1] = NEW TextureM(pDevice, (width + 1) / 2, height, gtype->FIXED2(), hr);
    UploadImg1[1] = NEW TextureM(pDevice, (width + 1) / 2, height, gtype->FIXED2(), hr);
    UploadImg2[1] = NEW TextureM(pDevice, (width + 1) / 2, height, gtype->FIXED2(), hr);
    DownloadImg[1] = NEW TextureRT(pDevice, (width + 3) / 4, height, gtype->FIXED4(), hr);
    UploadImg[2] = NEW TextureM(pDevice, (width + 1) / 2, height, gtype->FIXED2(), hr);
    UploadImg1[2] = NEW TextureM(pDevice, (width + 1) / 2, height, gtype->FIXED2(), hr);
    UploadImg2[2] = NEW TextureM(pDevice, (width + 1) / 2, height, gtype->FIXED2(), hr);
    DownloadImg[2] = NEW TextureRT(pDevice, (width + 3) / 4, height, gtype->FIXED4(), hr);
  }
  LOG("Setup texture Img & Imgp");
  if (useHalf)
    if (mode != 1)
      img = NEW TextureRT(pDevice, totw / 2, toth, gtype->HALF4(), hr);
    else
      imgp = NEW pTextureRTpair(NEW TextureRT(pDevice, totw / 2, toth, gtype->HALF4(), hr), NEW TextureRT(pDevice, totw / 2, toth, gtype->HALF4(), hr));

  else
    if (mode != 1)
      img = NEW TextureRT(pDevice, totw / 2, toth, gtype->FLOAT4(), hr);
    else
      imgp = NEW pTextureRTpair(NEW TextureRT(pDevice, totw / 2, toth, gtype->FLOAT4(), hr), NEW TextureRT(pDevice, totw / 2, toth, gtype->FLOAT4(), hr));
  if (FAILED(hr))
    env->ThrowError("Failed Creating FFT3dGPU::Texture img");

  LOG("Creating fft class..");
  switch (fftcode)
  {
  case RADIX2LUT:
    fft = NEW  FFT2dRR(bw, nx, bh, ny, bt, pFreeFImgdPool, pDevice, gtype, precision, hr);
    break;
  case STOCKHAM:
    fft = NEW  FFT2dRR2(bw, nx, bh, ny, bt, pFreeFImgdPool, pDevice, gtype, precision, hr);
    break;
  case MEASURE:
    fft = NEW  FFT2dRR(bw, nx, bh, ny, bt, pFreeFImgdPool, pDevice, gtype, precision, hr);

    NQuad::CreateVertexBuffer();
    double t = 0;
    TextureRT* In = img ? img : imgp->first;
    t = MeasureFFT(fft, In);
    delete fft;
    double t1;
    fft = NEW  FFT2dRR2(bw, nx, bh, ny, bt, pFreeFImgdPool, pDevice, gtype, precision, hr);
    t1 = MeasureFFT(fft, In);
    if (t1 > t)
    {
      delete fft;
      fft = NEW  FFT2dRR(bw, nx, bh, ny, bt, pFreeFImgdPool, pDevice, gtype, precision, hr);
    }
  }
  LOG("Creating WiennerFilter class..");
  D3DXVECTOR2 sigma2;
  D3DXVECTOR2 beta2;
  float s = sigma * sigma*bt*bw*bh;
  sigma2 = D3DXVECTOR2(beta*s, s);
  beta2 = D3DXVECTOR2((beta - 1.0) / beta, beta);
  if (usePattern&&bm > -1)
  {
    _sigma2 *= _sigma2 * bt*bw*bh / (255 * 255);
    _sigma3 *= _sigma3 * bt*bw*bh / (255 * 255);
    _sigma4 *= _sigma4 * bt*bw*bh / (255 * 255);
    SigmaToPatternTexture(s, _sigma2, _sigma3, _sigma4);
  }
  if (bm > 0)
    wiennerfilter = NEW psWiennerFilter(pDevice, pFreeFImgdPool->top(), beta2, sigma2, bt, degrid != 0, usePattern);
  LOG("done");
  if (FAILED(hr))
    env->ThrowError("Faillure creating WiennerFilter");
  LOG("Creating Sharpen...");
  if (sharpen)
    sharp = NEW Sharpen(sharpen, svr, scutoff, smin*smin*bw*bh*bt, smax*smax*bw*bh*bt, bw, nx, bh, ny, _degrid != 0, pFreeFImgdPool, hr, pDevice, gtype);
  LOG("Done");
  if (FAILED(hr))
    env->ThrowError("Faillure creating Sharp");
  //setup cache
  LOG("Setup GPUCACHE...");
  if (bm > 1) {
    if (isLumaPlane == 1) {
      caches[0] = NEW GPUCache(bt);
      caches[0]->StreamPoolPointer(pFreeFImgdPool);
    }
    else {
      caches[1] = NEW GPUCache(bt);
      caches[1]->StreamPoolPointer(pFreeFImgdPool);
      caches[2] = NEW GPUCache(bt);
      caches[2]->StreamPoolPointer(pFreeFImgdPool);
    }
  }
  if (bm == 0)
  {
    if (usePattern)
    {
      if (isLumaPlane == 1)
        kalmans[0] = NEW KalmanFilter(pFreeFImgdPool, kratio, mode == 1, pDevice, Pattern);
      else
      {
        kalmans[1] = NEW KalmanFilter(pFreeFImgdPool, kratio, mode == 1, pDevice, Pattern);
        kalmans[2] = NEW KalmanFilter(pFreeFImgdPool, kratio, mode == 1, pDevice, Pattern);
      }
    }
    else
    {
      if (isLumaPlane == 1)
        kalmans[0] = NEW KalmanFilter(pFreeFImgdPool, sigma*sigma*bw*bh, kratio, mode == 1, pDevice);
      else
      {
        kalmans[1] = NEW KalmanFilter(pFreeFImgdPool, sigma*sigma*bw*bh, kratio, mode == 1, pDevice);
        kalmans[2] = NEW KalmanFilter(pFreeFImgdPool, sigma*sigma*bw*bh, kratio, mode == 1, pDevice);
      }
    }
  }

  LOG("done")

    if (FAILED(hr))
      env->ThrowError("Failed Creating ImgStream");

  LOG("Setup ImgStream...");
  if (mode == 1) {
    const bool useChromaDisplacementMacro = !isLumaPlane && !isRGB;
    // later is HLSL: defines the CHROMANORM macro to 128.0/255.0 only for chroma, to shift it to the +/-0.5 range instead of 0..1.0
    convert2 = NEW ImgStream2(bw, bh, ow, oh, isLumaPlane ? UploadImg[0] : UploadImg[1], imgp->first, isLumaPlane ? DownloadImg[0] : DownloadImg[1], useChromaDisplacementMacro, wintype, interlaced, pDevice, gtype, useHalf, hr);
  }
  else
    convert = NEW ImgStream(bw, nx, bh, ny, mode, width, height, pDevice, gtype, useHalf, hr, border);
  LOG("done")

    LOG("Push texture vector");
  if (bt >= 2)
    if (mode != 1)
      for (unsigned int i = 0; i < bt; i++)
      {
        FImg2d.push_back(0);
        pFreeFImgdPool->pop(FImg2d[i]);
        FImgd.push_back(0);
        pFreeFImgdPool->pop(FImgd[i]);
        FImg1d.push_back(0);
        pFreeFImgdPool->pop(FImg1d[i]);
      }
    else
      for (unsigned int i = 0; i < bt; i++)
      {
        FImg2dp.push_back(NEW pTextureRTpair());
        pFreeFImgdPool->pop(*(FImg2dp[i]));
        FImgdp.push_back(NEW pTextureRTpair());
        pFreeFImgdPool->pop(*(FImgdp[i]));
        FImg1dp.push_back(NEW pTextureRTpair());
        pFreeFImgdPool->pop(*(FImg1dp[i]));
      }
  else
  {
    if (mode == 1)
    {
      pFreeFImgdPool->pop(FImgp);
      pFreeFImgdPool->pop(FImg1p);
      if (bm == 0)
        pFreeFImgdPool->pop(FImg2p);
    }
    else
    {
      pFreeFImgdPool->pop(FImg1);
      pFreeFImgdPool->pop(FImg);
      if (bm == 0)
        pFreeFImgdPool->pop(FImg2);
    }
  }

  //FFTtoFixed= NEW psFFTtoFixed(pDevice,img->GetRect(),false);
  /*
  sd=NEW TextureRT(pDevice,2,2,gtype->FLOAT(),hr);
  MeanSD=NEW psMeanSD(pDevice,pFreeFImgdPool->top()->GetRect());
  */
  psGridCorrection *setupgridcorrection = 0;
  if (degrid != 0) {
    mintex = NEW TextureRT(pDevice, nx, ny, gtype->FLOAT4(), hr);
    selectDC = NEW psMinimize(pDevice, mintex, pFreeFImgdPool->top());
    calcgridcorrection = NEW psGridCorrection(pDevice, pFreeFImgdPool->top());
    setupgridcorrection = NEW psGridCorrection(pDevice, pFreeFImgdPool->top(), degrid);
  }
  LOG("Creating VertexBuffer");
  hr = NQuad::CreateVertexBuffer();
  if (FAILED(hr))
    env->ThrowError("Failed creating vertexbuffer");
  CalcSD = false;

  //Setup degrid
  if (degrid != 0) {
    TextureRT* s = isLumaPlane ? DownloadImg[0] : DownloadImg[1];
    s->SetAsRenderTarget();
    pDevice->Clear(0, 0, D3DCLEAR_TARGET, 0xFFFFFFFF, 0, 0);
    pDevice->BeginScene();
    for (int i = 0; i < 6; i++)
    {
      pDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
      pDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
    }
    TextureRT* temp = pFreeFImgdPool->top();
    pFreeFImgdPool->pop();
    if (mode != 1)
    {
      convert->ImgToStream(s, img);
      //TextureRT *degrid_in=NEW TextureRT(pDevice,totw/2,toth,gtype->FLOAT4(),hr);
      FImgDegrid = pFreeFImgdPool->top();
      pFreeFImgdPool->pop();
      fft->CalcFFT(img, FImgDegrid, true);
      selectDC->Apply(FImgDegrid, mintex);
      setupgridcorrection->Apply(FImgDegrid, temp, mintex);
    }
    else
    {
      convert2->ImgToTexture(s, imgp);//!!
      FImgDegridp.last = pFreeFImgdPool->top();
      pFreeFImgdPool->pop();
      fft->CalcFFT(imgp->first, FImgDegridp.first, true);
      selectDC->Apply(FImgDegridp.first, mintex);
      setupgridcorrection->Apply(FImgDegridp.first, temp, mintex);

    }
    gridsample = NEW TextureM(pDevice, temp->GetWidth(), temp->GetHeight(), gtype->FLOAT4(), hr);
    float* t = NEW float[temp->GetWidth()*temp->GetHeight() * 4];
    DownloadFromTexture(temp, t, 0);
    pFreeFImgdPool->push(temp);
    UploadToTexture(gridsample, t, 0);
    delete[] t;
    pDevice->EndScene();
    //DownloadFromTexture(mintex,dg,0);
    //delete s;
    //delete dg;
    delete setupgridcorrection;
  }

  pingd = &FImgp;
  pongd = &FImg1p;
  lastd = &FImg2p;
  ping = FImg;
  pong = FImg1;
  lastt = FImg2;
  backbuffer = 0;
  if (NVPERF)
    pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
  //ReleaseMutex(mutex);
  LOG("End constructor" << std::endl << std::dec);

}

//Modified from fizick's code SigmaToPattern
void FFT3dGPU::SigmaToPatternTexture(float sigma, float sigma2, float sigma3, float sigma4)
{
  // it is not fast, but called only in constructor
  unsigned int fbw = bw / 2 + 1;//fft blockwidth
  float* pattern2d = NEW float[fbw*bh];
  float* pattern2drep = NEW float[fbw*bh*nx*ny];
  HRESULT hr;
  if (Pattern)
    delete Pattern;
  Pattern = NEW TextureM(pDevice, fbw*nx, bh*ny, gtype->FLOAT(), hr);

  float ft2 = sqrt(0.5f) / 2; // frequency for sigma2
  float ft3 = sqrt(0.5f) / 4; // frequency for sigma3
  for (unsigned int h = 0, offset = 0; h < bh; h++, offset += fbw)
  {
    float fy = (bh - 2.0f*abs(h - bh / 2.0f)) / bh; // normalized to 1
    for (unsigned int w = 0; w < fbw; w++)
    {
      float fx = (w*1.0f) / fbw;  // normalized to 1
      float f = sqrt((fx*fx + fy * fy)*0.5f); // normalized to 1
      if (f < ft3)
      { // low frequencies
        pattern2d[offset + w] = sigma4 + (sigma3 - sigma4)*f / ft3;
      }
      else if (f < ft2)
      { // middle frequencies
        pattern2d[offset + w] = sigma3 + (sigma2 - sigma3)*(f - ft3) / (ft2 - ft3);
      }
      else
      {// high frequencies
        pattern2d[offset + w] = sigma + (sigma2 - sigma)*(1 - f) / (1 - ft2);
      }
    }
  }
  int offset = 0;
  for (unsigned int repy = 0; repy < ny; repy++)
    for (unsigned int j = 0; j < bh; j++)
      for (unsigned int repx = 0; repx < nx; repx++)
        for (unsigned int i = 0; i < fbw; i++)
          pattern2drep[offset++] = pattern2d[i + fbw * j];
  UploadToTexture(Pattern, pattern2drep);
  delete pattern2drep;
  delete pattern2d;
}

/*
 * CalcMeanSD
 *	Calculates the mean standard deviation(sigma) from a frequency texture by summing up the PSD for all frequencies except the 0(the avererage pixelvalue) and divede with the total number of frequencies(minus the 0 freqency)
 *  this gives the the mean sd(sigma) for white noise on a uniform background
 *
 * Inputs:
 *	src:[in] source texture
 *
 * Returns:
 *     mean sd
 *
 * Remarks:
 *     Not currently used because the window function screws this function somewhat up
 */
float FFT3dGPU::CalcMeanSD(TextureRT* src) {
  float* M = NEW float[src->GetWidth()*src->GetHeight()*src->GetType()->elem_size];
  DownloadFromTexture(src, M);
  float mean = 0;
  int count = 0;
  int offset = 0;
  for (unsigned int i = 0; i < ny - 2; i++)
    for (unsigned int y = 0, o = 0; y < bh; y++, offset = 0, o += src->GetWidth()*src->GetType()->elem_size)
      for (unsigned int j = 0; j < (nx - 2); j++)
        for (unsigned int x = 0; x < bw / 2 + 1; x++) {
          if (y != 0 && x != 0) {
            mean += M[o + offset + 1] * M[o + offset + 1] + M[o + offset + 3] * M[o + offset + 3];
            count++;
          }
          offset += 4;
        }
  mean /= count;
  mean = sqrt(mean * 255 * 255 / bh / bw / bt);
  delete M;
  return(mean);
}

//destructor cleans up the mess
FFT3dGPU::~FFT3dGPU()
{
  //LOG("Destructor wait on MUTEX")
  //WaitForSingleObject(mutex,INFINITE);
  LOG("done")
    delete DI;
  delete img;
  delete mintex;
  delete gridsample;

  delete FImg;
  delete FImg1;
  delete FImg2;
  delete FImgDegrid;
  delete Pattern;

  for (int i = 0; i < 3; i++) {
    delete UploadImg[i];
    delete UploadImg1[i];
    delete UploadImg2[i];
    delete DownloadImg[i];
    delete caches[i];
    delete kalmans[i];
  }

  if (imgp)
    imgp->Release();


  if (bm >= 2)
    if (mode != 1)
      for (unsigned int i = 0; i < bt; i++) {
        delete (FImgd[i]);
        delete (FImg1d[i]);
        delete (FImg2d[i]);
      }
    else
      for (unsigned int i = 0; i < bt; i++) {
        FImgdp[i]->Release();
        FImg1dp[i]->Release();
        FImg2dp[i]->Release();
      }
  delete selectDC;
  delete calcgridcorrection;
  delete convert;
  delete convert2;
  delete sharp;
  delete wiennerfilter;
  delete fft;


  if (backbuffer)
    backbuffer->Release();
#ifdef _DEBUG
  FFT3dGPU::pDevice->AddRef();
  cerrwin << std::endl << "pDevice RefCount: " << FFT3dGPU::pDevice->Release() << std::endl;//
#endif

  //std::ofstream outfile("timelog",std::ios_base::app);
  /*double r=tottime/(double)totproc;
  std::stringstream s;
  s<<r;
  MessageBox(NULL,s.str().c_str(),"fft3dgpu time",MB_OK);*/
  //outfile<<std::endl<<r;

      //ReleaseMutex(mutex);
      //CloseHandle(mutex);
      //mutex=0;
  LOG("done destructing")
}





/*
 * GetFrame
 *	Avisynth GetFrame overload function. Filters the n'th frame and returns it.
 *
 * Inputs:
 *	n:[in] frame number
 *	env:[in] script enviroment
 *
 * Returns:
 *     the processed frame
 *
 * Remarks:
 *     None
 */
PVideoFrame FFT3dGPU::GetFrame(int n, IScriptEnvironment* env)
{

  //totproc++;
  //Qstart();
  LOG("Getframe " << n);
  unsigned int gt;
  LOG("time: " << (gt = GetTickCount()))
    LOG("Thread ID: " << GetCurrentThreadId());
  //WaitForSingleObject(mutex,INFINITE);
    //setup the nextframe struct that are passed to imgstream who again uses it when downloading the current frame to get the next frame in a NEW thread (for concurrent processing)

  //get destination frame
  PVideoFrame dst;
  PVideoFrame src;

  if (GetDst)
    dst = GetDst->GetDstFrame();//from FFT3dGPUallPlane so we don't need to bltbit the luma/chroma
  else
  {
    dst = env->NewVideoFrame(vi);//else create a new empty frame
    if (isYUY2)
    {
      src = child->GetFrame(n, env);
      env->BitBlt(dst->GetWritePtr(), dst->GetPitch(), src->GetReadPtr(), src->GetPitch(), src->GetRowSize(), src->GetHeight());
    }
  }

  const int planes_y[4] = { PLANAR_Y, PLANAR_U, PLANAR_V, PLANAR_A };
  const int planes_r[4] = { PLANAR_G, PLANAR_B, PLANAR_R, PLANAR_A };
  const int *planes = (vi.IsYUV() || vi.IsYUVA()) ? planes_y : planes_r;

  //loop trough the two chroma planes(U and V) or through the luma plane(Y)
  int planeNo = isLumaPlane ? 0 : 1;
  // used for skipping through chromas planeNo=1 -> U (or B); planeNo
  // for chroma or rgb planeNo will be 2 after 1
  while (1)
  {
    const int currentPlane = planes[planeNo]; // Y or U or V or G or B or R
    //pDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
    D3Dwindow.BeginScene();
    //		BYTE* dstp;
    //		dstp=retwritepointer(dst,planar);//dst->GetWritePtr(planar);
        //Setup the gpucache to the right plane
    GPUCache* cache = caches[planeNo];
    KalmanFilter* kalman = kalmans[planeNo];
    TextureM* &upload1 = UploadImg1[planeNo];
    //TextureM* &upload2=planar==1?UploadImgY2:planar==2?UploadImgU2:UploadImgV2;
    TextureM* &upload = UploadImg[planeNo];
    TextureRT* download = DownloadImg[planeNo];
    //42
    //Reset the render and sampler state just in case if any filters or programs has changed them
    pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
    LOG("SetSamplerState...");
    for (int i = 0; i < 6; i++)
    {
      pDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
      pDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
    }
    LOG("done");

    if (bm >= 2)
    {

      if (mode != 1)//mode 0 or 2
      {
        //get the 2d fft transform from cache or calc it if not in cache
        for (unsigned int i = 0; i < bt; i++)
        {
          //If not in cache calculate the 2D fft and save it in cache
          if (!cache->GetStream(n - current + i, FImg2d[i]))
          {
            PROFILE_BLOCK
              if (nuploaded != n - current + i)
              {
                src = child->GetFrame(n - current + i, env);
                if (!isYUY2) {
                  UploadToTexture(upload, src->GetReadPtr(currentPlane), src->GetPitch(currentPlane));
                }
                else
                  UploadInterleavedToFixedTexture(upload, src->GetReadPtr() + (planeNo == 0 ? 0 : planeNo == 1 ? 1 : 3), src->GetPitch(), planeNo == 0 ? 2 : 4);
                convert->ImgToStream(upload, img);
              }
              else
                convert->ImgToStream(upload1, img);
            //copy src to buffer and mirror the borders to get the right size
            //CopyFrameToBuffer(src->GetReadPtr(planar),src->GetPitch(planar));

            //convert->ImgToStream(src->GetReadPtr(planar),img,src->GetPitch(planar));
            fft->CalcFFT(img, FImg2d[i], true);
            cache->AddtoCache(n - current + i, FImg2d[i]);
          }
        }
        if (degrid) {
          selectDC->Apply(FImg2d[current], mintex);
          calcgridcorrection->Apply(mintex, FImgDegrid, gridsample);
        }
        wiennerfilter->Apply(FImg2d, FImg1d[current], FImgDegrid, Pattern);
        if (sharpen) {
          sharp->Filter(FImg1d[current], FImg1d[0], FImgDegrid);
          fft->CalcFFT(FImg1d[0], img, false);
        }
        else
          fft->CalcFFT(FImg1d[current], img, false);
      }
      else//mode==1
      {//only change from above is that the textures are contanied in texturepairs
        for (unsigned int i = 0; i < bt; i++)
        {
          //FImg2dp[i]->Release();
          //If the texture is not in cache calculate the 2D fft and save it in cache
          if (!cache->GetStream(n - current + i, FImg2dp[i]))
          {
            PROFILE_BLOCK
              if (nuploaded != n - current + i)
              {
                src = child->GetFrame(n - current + i, env);
                if (!isYUY2)
                  UploadToTexture(upload, src->GetReadPtr(currentPlane), src->GetPitch(currentPlane));
                else
                  UploadInterleavedToFixedTexture(upload, src->GetReadPtr() + (planeNo == 0 ? 0 : planeNo == 1 ? 1 : 3), src->GetPitch(), planeNo == 0 ? 2 : 4);
                convert2->ImgToTexture(upload, imgp);
              }
              else
                convert2->ImgToTexture(upload1, imgp);

            fft->CalcFFT(imgp->first, FImg2dp[i]->first, true);
            fft->CalcFFT(imgp->last, FImg2dp[i]->last, true);
            cache->AddtoCache(n - current + i, FImg2dp[i]);
          }
        }
        pTextureRTpair* Degrid = 0;
        if (degrid) {

          selectDC->Apply(FImg2dp[current]->first, mintex);
          calcgridcorrection->Apply(mintex, FImgDegridp.first, gridsample);
          selectDC->Apply(FImg2dp[current]->last, mintex);
          calcgridcorrection->Apply(mintex, FImgDegridp.last, gridsample);
          Degrid = &FImgDegridp;
        }
        LOG("Filter...")
          wiennerfilter->Apply(FImg2dp, FImg1dp[current], Degrid, Pattern);
        LOG("done")
          if (sharpen) {
            sharp->Filter(FImg1dp[current], FImg1dp[0], Degrid);
            fft->CalcFFT(FImg1dp[0]->first, imgp->first, false);
            fft->CalcFFT(FImg1dp[0]->last, imgp->last, false);
          }
          else {
            fft->CalcFFT(FImg1dp[current]->first, imgp->first, false);
            fft->CalcFFT(FImg1dp[current]->last, imgp->last, false);
          }
      }

    }
    else {//bm<2

      LOG("Getting srcframe");
      //src=child->GetFrame(n,env);


      if (mode != 1)//mode 0 or 2
      {
        PROFILE_BLOCK
          LOG("Uploading src to GPU...");
        if (nuploaded != n)
        {
          src = child->GetFrame(n, env);
          if (!isYUY2)
            UploadToTexture(upload, src->GetReadPtr(currentPlane), src->GetPitch(currentPlane));
          else
            UploadInterleavedToFixedTexture(upload, src->GetReadPtr() + (planeNo == 0 ? 0 : planeNo == 1 ? 1 : 3), src->GetPitch(), planeNo == 0 ? 2 : 4);
          convert->ImgToStream(upload, img);
        }
        else
          convert->ImgToStream(upload1, img);

        //convert->ImgToStream(src->GetReadPtr(planar),img,src->GetPitch(planar));
        LOG("done");

        LOG("Calcfft...")
          fft->CalcFFT(img, ping, true);
        LOG("done")
          if (degrid) {
            LOG("CalcDegridSample...")
              selectDC->Apply(ping, mintex);
            calcgridcorrection->Apply(mintex, FImgDegrid, gridsample);
            LOG("done")
          }
        LOG("Filter...")
          if (bm == 1)
          {
            wiennerfilter->Apply(ping, pong, FImgDegrid, Pattern);
            std::swap(ping, pong);
          }
          else if (bm == 0)
          {
            kalman->Filter(ping, pong);
            std::swap(ping, pong);
          }
        //wiennerfilter->Filter(FImg,FImg1);
        LOG("done")
          if (sharpen) {
            sharp->Filter(ping, pong, FImgDegrid);
            if (bm == 0)
              std::swap(ping, lastt);
            std::swap(ping, pong);
          }

        fft->CalcFFT(ping, img, false);
        if (!sharpen&&bm == 0)
          std::swap(ping, lastt);
        LOG("done")
      }

      else//mode=1		
      {//again only difference is that it is texturepairs instead of textures
        //convert->ImgToStream(src->GetReadPtr(planar),imgp,src->GetPitch(planar));

        if (nuploaded != n)
        {
          PROFILE_BLOCK
            src = child->GetFrame(n, env);
          if (!isYUY2)
            UploadToTexture(upload, src->GetReadPtr(currentPlane), src->GetPitch(currentPlane));
          else
            UploadInterleavedToFixedTexture(upload, src->GetReadPtr() + (planeNo == 0 ? 0 : planeNo == 1 ? 1 : 3), src->GetPitch(), planeNo == 0 ? 2 : 4);
          //upload->SaveTex("d:\\src.dds");
          convert2->ImgToTexture(upload, imgp);
        }
        else
          convert2->ImgToTexture(upload1, imgp);

        /**/
        fft->CalcFFT(imgp->first, pingd->first, true);
        fft->CalcFFT(imgp->last, pingd->last, true);

        pTextureRTpair* Degrid = 0;
        if (degrid) {
          selectDC->Apply(pingd->first, mintex);
          calcgridcorrection->Apply(mintex, FImgDegridp.first, gridsample);
          selectDC->Apply(pingd->last, mintex);
          calcgridcorrection->Apply(mintex, FImgDegridp.last, gridsample);
          LOG("done")
            Degrid = &FImgDegridp;
        }

        LOG("Filter...")
          if (bm == 1)
          {
            wiennerfilter->Apply(pingd, pongd, Degrid, Pattern);
            std::swap(pingd, pongd);
          }
          else if (bm == 0)
          {
            kalman->Filter(pingd, pongd);
            std::swap(pingd, pongd);

          }

        LOG("done")
          if (sharpen) {
            sharp->Filter(pingd, pongd, Degrid);
            if (bm == 0)
              std::swap(pingd, lastd);
            std::swap(pingd, pongd);
          }
        //pingd->first->SaveTex("d:\\src.dds");
        fft->CalcFFT(pingd->first, imgp->first, false);
        fft->CalcFFT(pingd->last, imgp->last, false);
        if (!sharpen&&bm == 0)
          std::swap(pingd, lastd);
        /**/

      }
    }
    LOG("download from GPU...")
      if (mode == 1)
        convert2->TextureToImg(imgp, download);
      else
        convert->StreamToImg(img, download);
    LOG("done")
      D3Dwindow.EndScene();

    //Test if pDevice is lost(ie. if a screensaver is running, the computer resumes from suspend/sleep or a fullscreen direct3d program is executed
    //if lost and recovered then all textureRT are erased so it is nececery to start over.
    if (d3ddevice.DeviceLost())
    {
      if (kalmans[0])
        kalmans[0]->Restore();
      else if (kalmans[1])
      {
        kalmans[1]->Restore();
        kalmans[2]->Restore();
      }
      if (GetDst)
      {
        GetDst->ReturnDstFrame(dst);
        dst = 0;
      }
      return this->GetFrame(n, env);

    }
    //goto Restart;
    if (isLumaPlane) // planeNo == 0
      break;
    planeNo++;
    if (planeNo > 2) // exit after 2nd pass (U and V is done - RGB last two planes are done)
      break;
  }

  D3Dwindow.StartGPU();

  src = child->GetFrame(n, env);

  if (has_at_least_v8) { // frame property from the nth position
    env->copyFrameProps(src, dst);
  }

  if (!GetDst && !isYUY2) // if process all plane no need to bitblt
  {
    LOG("bitblt...")
      if (isLumaPlane && !vi.IsY()) { // If calc Luma copy chromaplane
        env->BitBlt(dst->GetWritePtr(PLANAR_U), dst->GetPitch(PLANAR_U), src->GetReadPtr(PLANAR_U), src->GetPitch(PLANAR_U), src->GetRowSize(PLANAR_U), src->GetHeight(PLANAR_U));
        env->BitBlt(dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_V), src->GetReadPtr(PLANAR_V), src->GetPitch(PLANAR_V), src->GetRowSize(PLANAR_V), src->GetHeight(PLANAR_V));
      }
      else // else calc chroma copy luma
        env->BitBlt(dst->GetWritePtr(PLANAR_Y), dst->GetPitch(PLANAR_Y), src->GetReadPtr(PLANAR_Y), src->GetPitch(PLANAR_Y), src->GetRowSize(PLANAR_Y), src->GetHeight(PLANAR_Y));
    LOG("done")
  }

  //this part is used to calculate the offset for the next frame that is need so that we can upload it now
  int last = lastn;
  if (bt > 1)
    lastn = n - current + bt - 1;
  else
    lastn = n;

  int delta = lastn - last;
  if (n == 0)
    delta = 1;
  nuploaded = lastn + delta;
  src = child->GetFrame(nuploaded, env);
  if (isLumaPlane)
  {
    PROFILE_BLOCK
      //UploadToTexture(UploadImgY2,src->GetReadPtr(PLANAR_Y),src->GetPitch(PLANAR_Y));
      if (!isYUY2) {
        const int currentPlane = planes[0]; // Y or G
        UploadToTexture(UploadImg2[0], src->GetReadPtr(currentPlane), src->GetPitch(currentPlane));
      }
      else
        UploadInterleavedToFixedTexture(UploadImg2[0], src->GetReadPtr(), src->GetPitch(), 2);
    std::swap(UploadImg2[0], UploadImg1[0]);
  }
  else
  {
    PROFILE_BLOCK
      //UploadToTexture(UploadImgU2,src->GetReadPtr(PLANAR_U),src->GetPitch(PLANAR_U));
      if (!isYUY2)
      {
        int currentPlane = planes[1]; // 1: U or B  2: V or R
        UploadToTexture(UploadImg2[1], src->GetReadPtr(currentPlane), src->GetPitch(currentPlane));
        currentPlane = planes[2];
        UploadToTexture(UploadImg2[2], src->GetReadPtr(currentPlane), src->GetPitch(currentPlane));
      }
      else
      {
        UploadInterleavedToFixedTexture(UploadImg2[1], src->GetReadPtr() + 1, src->GetPitch(), 4);
        UploadInterleavedToFixedTexture(UploadImg2[2], src->GetReadPtr() + 3, src->GetPitch(), 4);
      }
    std::swap(UploadImg2[1], UploadImg1[1]);
    //UploadToTexture(UploadImgV2,src->GetReadPtr(PLANAR_V),src->GetPitch(PLANAR_V));
    std::swap(UploadImg2[2], UploadImg1[2]);
  }

  D3Dwindow.SleepWhileGPUwork();

  if (isLumaPlane)
  {
    PROFILE_BLOCK
      //convert2->TextureToSrc(imgp,dst->GetWritePtr(PLANAR_Y),dst->GetPitch(PLANAR_Y));
      if (!isYUY2) {
        const int currentPlane = planes[0]; // Y or G
        DownloadFromTexture(DownloadImg[0], dst->GetWritePtr(currentPlane), dst->GetPitch(currentPlane), true);
      }
      else
        DownloadFromFixedTextureInterleaved(DownloadImg[0], dst->GetWritePtr(), dst->GetPitch(), true, 2);
  }
  else
  {
    PROFILE_BLOCK
      if (!isYUY2)
      {
        int currentPlane = planes[1]; // 1: U or B  2: V or R
        DownloadFromTexture(DownloadImg[1], dst->GetWritePtr(currentPlane), dst->GetPitch(currentPlane), true);
        currentPlane = planes[2];
        DownloadFromTexture(DownloadImg[2], dst->GetWritePtr(currentPlane), dst->GetPitch(currentPlane), true);
      }
      else
      {
        DownloadFromFixedTextureInterleaved(DownloadImg[1], dst->GetWritePtr() + 1, dst->GetPitch(), true, 4);
        DownloadFromFixedTextureInterleaved(DownloadImg[2], dst->GetWritePtr() + 3, dst->GetPitch(), true, 4);
      }
  }
  //if nvperf present window so nvperfhud can do its magic
  if (NVPERF)
  {
    pDevice->SetRenderTarget(0, backbuffer);
    pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    pDevice->Present(NULL, NULL, NULL, NULL);
  }
#ifdef PIXPROFILE
  else
    pDevice->Present(NULL, NULL, NULL, NULL);

#endif
  //ReleaseMutex(mutex);
  LOG("process time: " << GetTickCount() - gt);
  LOG("time: " << GetTickCount());
  LOG(std::endl)
    //tottime+=Qendtime();
    //Qend();
    return(dst);
}

// FFT3dGPUallPlane: to save one BitBlt by using the very same frame in multiple
// FFT3dGPU filters (one FFT3dGPU filter can handle either luma or two chromas)
FFT3dGPUallPlane::FFT3dGPUallPlane(PClip _child, IScriptEnvironment* env) :
  GenericVideoFilter(_child)
{}

PVideoFrame FFT3dGPUallPlane::GetFrame(int n, IScriptEnvironment* env)
{
  // return value is a common clip
  dst_frame = env->NewVideoFrame(vi);
  // lumaplane->GetFrame will work into this just created dst_frame by getting it with GetDstFrame
  // and fill the Luma planes, then returning this dst_frame by ReturnDstFrame
  dst_frame = _lumaplane->GetFrame(n, env);
  // chromaplane->GetFrame will futher work into this dst_frame by getting it with GetDstFrame
  // and fill the chroma planes, then returning this dst_frame by ReturnDstFrame
  dst_frame = _chromaplane->GetFrame(n, env);
  // now both luma and chroma is filled
  return dst_frame;
}

void FFT3dGPUallPlane::SetChromaAndLumaClip(PClip lumaplane, PClip chromaplane)
{
  _lumaplane = lumaplane;
  _chromaplane = chromaplane;
  child = lumaplane;
  vi = child->GetVideoInfo();
}

PVideoFrame FFT3dGPUallPlane::GetDstFrame()
{
  PVideoFrame tmp = dst_frame;
  dst_frame = 0;
  return tmp;
  // This assignment sequence ensures that the dst_frame refcount is 1 = it's writable
  // So we can write in it in the next FFT3dGPU GetFrame.
}

// when all-planes mode, after filtering luma or chromas, the result is returned with this method
void FFT3dGPUallPlane::ReturnDstFrame(PVideoFrame dst)
{
  dst_frame = dst;
}


//creates the FFT3dGPU class and returns it
AVSValue __cdecl Create_fft3dGPU(AVSValue args, void* user_data, IScriptEnvironment* env) {
  int plane = args[7].AsInt(0); //  0 filters luma, 1,2 and 3 filters Chroma (both U and V). 4 filters both luma and chroma. Default 0. 
  FFTCODE fftcode = args[25].Defined() ? args[25].AsBool() ? RADIX2LUT : STOCKHAM : MEASURE;
  float sigma = args[1].AsFloat(2);
  bool allplane = (plane == 4);
  if (plane == 2 || plane == 3)
    plane = 1;
  // now plane: 0 (luma), 1 (chroma), 4 (all)

  const VideoInfo vi = args[0].AsClip()->GetVideoInfo();
  if (vi.IsRGB()) {
    if(!vi.IsPlanar())
      env->ThrowError("FFT3dGPU: no packed RGB, only planar RGB is supported");
    plane = 4;
    // for RGB: silently set to handle all planes, no luma or chroma
    // at the moment the first and second-third RGB planes will be handled separately (like Y and UV)
  }
    
  if (vi.IsY()) {
    if (plane == 1)
      env->ThrowError("FFT3dGPU: cannot set chroma processing for a greyscale clip");
    if (plane == 4)
      plane = 0; // all planes means luma only
  }

  if (vi.BitsPerComponent() != 8)
    env->ThrowError("FFT3dGPU: only 8 bit clips are supported");

  PClip retval;
  FFT3dGPUallPlane* getdst = 0;
  if (allplane)
  {
    getdst = NEW FFT3dGPUallPlane(args[0].AsClip(), env); // tricky helper to avoid a BitBlt
    plane = 0; // first we take luma
  }
  LOG("CREATE_fft3dGPU" << std::endl)
    bool d = args[8].AsInt(1) == 1;//mode=1?
  float degrid = (float)args[12].AsFloat(d ? 1.0 : 0.0);

  const int xRatioUV = (vi.IsY() || vi.IsRGB()) ? 1 : (1 << vi.GetPlaneWidthSubsampling(PLANAR_U));
  const int yRatioUV = (vi.IsY() || vi.IsRGB()) ? 1 : (1 << vi.GetPlaneHeightSubsampling(PLANAR_U));

  retval = NEW FFT3dGPU(args[0].AsClip()//Input Clip
    , sigma,//sigma
    args[2].AsFloat(1),//beta
    args[3].AsInt(32),//bw
    args[4].AsInt(32),//bh
    args[5].AsInt(3),//bt
    args[6].AsFloat(0.0),//sharpen
    plane,//plane  here:0:luma (incl. all plane) 1:chromaUandV
    args[8].AsInt(1),//mode
    args[9].AsInt(1),//border
    args[10].AsInt(FLOAT16),//precision
    args[11].AsBool(false),//NVperf
    //args[12].AsBool(true),//reduce cpu
    degrid,//degrid
    args[13].AsFloat(0.3),//scutoff 
    args[14].AsFloat(1.0),//svr 
    args[15].AsFloat(4.0),//smin 
    args[16].AsFloat(20.0),//smax 
    args[17].AsFloat(2.0),//kratio
    args[18].AsInt(args[3].AsInt(32) / 2),//ow
    args[19].AsInt(args[4].AsInt(32) / 2),//oh
    args[20].AsInt(0),//wintype
    args[21].AsBool(false),//interlaced
    args[22].AsFloat(sigma),//sigma2
    args[23].AsFloat(sigma),//sigma3
    args[24].AsFloat(sigma),//sigma4
    fftcode,//fftcode
    getdst,//getdst valid only for all-planes operation
    plane == 0 ? 1 : xRatioUV,
    plane == 0 ? 1 : yRatioUV,
    env//env
  );
  if (allplane)
  {
    //AVSValue r1=env->Invoke("InternalCache",retval.AsClip());
    plane = 1; // luma was already done, now comes chromaUandV part
    PClip chroma = NEW FFT3dGPU(args[0].AsClip()//Input Clip
      , sigma,//sigma
      args[2].AsFloat(1),//beta
      args[3].AsInt(32),//bw
      args[4].AsInt(32),//bh
      args[5].AsInt(3),//bt
      args[6].AsFloat(0.0),//sharpen
      plane,//plane
      args[8].AsInt(1),//mode
      args[9].AsInt(1),//border
      args[10].AsInt(FLOAT16),//precision
      args[11].AsBool(false),//NVperf
      //args[12].AsBool(true),//reduce cpu
      degrid,//degrid
      args[13].AsFloat(0.3),//scutoff 
      args[14].AsFloat(1.0),//svr 
      args[15].AsFloat(4.0),//smin 
      args[16].AsFloat(20.0),//smax 
      args[17].AsFloat(2.0),//kratio
      args[18].AsInt(args[3].AsInt(32) / 2),//ow
      args[19].AsInt(args[4].AsInt(32) / 2),//oh
      args[20].AsInt(0),//wintype
      args[21].AsBool(false),//interlaced
      args[22].AsFloat(sigma),//sigma2
      args[23].AsFloat(sigma),//sigma3
      args[24].AsFloat(sigma),//sigma4
      fftcode,//fftcode
      getdst, // valid only for all-planes operation
      xRatioUV,
      yRatioUV,
      env//env
    );
    getdst->SetChromaAndLumaClip(retval, chroma); // merge luma and chroma results
    retval = getdst;
  }
  LOG("CREATE_fft3dGPU done" << std::endl)
    return retval;
}


// The following function is the function that actually registers the filter in AviSynth
// It is called automatically, when the plugin is loaded to see which functions this filter contains.

// new V2.6 requirement
const AVS_Linkage *AVS_linkage = NULL;

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {

  AVS_linkage = vectors;

  LOG("AvisynthPluginInit2:" << std::endl)
    env->AddFunction("fft3dGPU", "c[sigma]f[beta]f[bw]i[bh]i[bt]i[sharpen]f[plane]i[mode]i[bordersize]i[precision]i[NVPerf]b[degrid]f[scutoff]f[svr]f[smin]f[smax]f[kratio]f[ow]i[oh]i[wintype]i[interlaced]b[sigma2]f[sigma3]f[sigma4]f[oldfft]b", Create_fft3dGPU, 0);
  LOG("AvisynthPluginInit2 Addfunction done");

  return "fft3dGPU ver 0.8.4";
}



D3DDevice* D3DDevice::first = 0;

D3DDevice::D3DDevice(ManageDirect3DWindow &d3dwnd) :_d3dwnd(d3dwnd), lost(false), pDevice(d3dwnd.GetDirectDirect3DDevice()) {
  if (first) {
    first->prev->next = this;
    this->prev = first->prev;
    this->next = first;
    first->prev = this;
  }
  else
  {
    first = this;
    this->prev = this;
    this->next = this;
  }
}

D3DDevice::~D3DDevice() {
  if (next == this)
    first = 0;
  else
  {
    if (first == this)
      first = next;
    prev->next = next;
    next->prev = prev;
  }
  _d3dwnd.ReleaseDirect3DDevice();
}

bool D3DDevice::DeviceLost() {
  if (!_d3dwnd.DeviceReady()) {
    _d3dwnd.ResetDevice();
    GPUCache::FlushAll();
    D3DDevice* temp = first;
    do
    {
      temp->lost = true;
      temp = temp->next;
    } while (temp != first);
  }

  bool retval = lost;
  lost = false;
  return retval;
}

LPDIRECT3DDEVICE9 D3DDevice::Device() { return pDevice; }

