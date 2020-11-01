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


class D3DDevice
{
public:
  D3DDevice(ManageDirect3DWindow &d3dwnd);
  ~D3DDevice();
  bool DeviceLost();
  LPDIRECT3DDEVICE9 Device();
private:
  ManageDirect3DWindow &_d3dwnd;
  LPDIRECT3DDEVICE9 pDevice;
  static D3DDevice* first;
  D3DDevice* next;
  D3DDevice* prev;
  bool lost;
};



class FFT3dGPUallPlane : public GenericVideoFilter {
public:
  FFT3dGPUallPlane(PClip _child, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  // Auto register AVS+ mode: serialized
  int __stdcall SetCacheHints(int cachehints, int frame_range) override {
    return cachehints == CACHE_GET_MTMODE ? MT_SERIALIZED : 0; // no MT!
  }

  void SetChromaAndLumaClip(PClip lumaplane, PClip chromaplane);
  PVideoFrame GetDstFrame();
  void ReturnDstFrame(PVideoFrame dst);
protected:
  PClip _lumaplane;
  PClip _chromaplane;
  PVideoFrame dst_frame;
};

enum FFTCODE {
  MEASURE = 0,
  RADIX2LUT = 1,
  STOCKHAM = 2
};


class FFT3dGPU : public GenericVideoFilter {
public:
  FFT3dGPU(PClip cl1, float _sigma, float _beta, int _bw, int _bh, int _bt, float sharpen, int _plane, int _mode, int border, int precision,
    bool NVPerf, float _degrid, float scutoff, float svr, float smin, float smax, float kratio, int ow, int oh, int wintype, bool interlaced,
    float _sigma2, float _sigma3, float _sigma4, FFTCODE fftcode,
    FFT3dGPUallPlane* getdst, int _xRatio, int _yRatio, IScriptEnvironment* env);
  ~FFT3dGPU();
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  // Auto register AVS+ mode: serialized
  int __stdcall SetCacheHints(int cachehints, int frame_range) override {
    //return cachehints == CACHE_GET_MTMODE ? (bt == 0 ? MT_SERIALIZED : MT_MULTI_INSTANCE) : 0; // FFT3dFilter settings
    return cachehints == CACHE_GET_MTMODE ? MT_SERIALIZED : 0; // no MT!
  }
protected:
  bool has_at_least_v8; // frame property support

  //To avoid unnecesary bltblit when processing both luma and chroma we ask GetDst to get the PVideoFrame we will use as the destination
  FFT3dGPUallPlane* GetDst;
  D3DDevice d3ddevice;	//contains informations about d3ddevice state(lost/reset device)
  bool isLumaPlane; //1=luma 2=chroma
  bool isYUY2;
  bool isRGB;

  //caches the 2d fft on the GPU
  GPUCache* caches[3];

  int lastn;//previous frame processed (used to calculate offset for the next frame to upload)
  int nuploaded;//current frame uploaded to the GPU memory (saved in UploadImgY/U/V)
  TextureM* UploadImg[3];	//contains the frame uploaded in the last call to getframe

  TextureM* UploadImg1[3];  //used if nuploaded was not the frame requested 
  TextureM* UploadImg2[3];  //the next needed frame is uploaded here (Y and Y2 are swaped)

  //the result is downloaded here
  TextureRT* DownloadImg[3];

  KalmanFilter* kalmans[3];

  //int _y,_u,_v;
  unsigned int height;//src height in bytes
  unsigned int width;//src width in bytes
  unsigned int nx, ny;//number of blocks
  unsigned int totw, toth;//buffer wide and height
  int mode;//how are the blocks created: valid values:0,1,2
  bool CalcSD;//not used
  unsigned int bt;//temporal block size(1-4)
  int bm;//block mode (sharpen(-1),kalman(0) or wienner(1-4))
  float sigma, beta;//used in wiennerfiltering
  unsigned int bw, bh;//block width and height
  int current;//when bt>1 current is the index in the vector<texture> that returns the current(n) frame
  psWiennerFilter* wiennerfilter;
  FFT2d* fft;//does the fft
//converts the src texture to the blocks that are fft'ed
  ImgStream* convert;//mode 0,2
  ImgStream2* convert2;//mode 1

  Sharpen* sharp;

  TextureRT *img;//this is the texture that contains the non-fft'ed blocks
  pTextureRTpair* imgp;//ditto for mode 2

  //Degrid
  TextureM *gridsample;//used by degrid.
  TextureRT* mintex;//ditto
  psMinimize* selectDC;//select the DC component of each block
  psGridCorrection *calcgridcorrection;
  /*
    TextureRT *sd;
    psMeanSD* MeanSD;*/
  float CalcMeanSD(TextureRT* src);

  double MeasureFFT(FFT2d *_fft, TextureRT* In);


  TexturePool *pFreeFImgdPool;//The texturePool delivers fft textures of the right size

  //vectors that contains fft textures that are used by the filters in mode 0 and 2 when bt>1
  std::vector<TextureRT* > FImgd;
  std::vector<TextureRT* > FImg1d;
  std::vector<TextureRT* > FImg2d;

  //vectors that contains fft tetxure pairs used by mode 2 and bt>1
  std::vector<pTextureRTpair* > FImgdp;
  std::vector<pTextureRTpair* > FImg1dp;
  std::vector<pTextureRTpair* > FImg2dp;

  //used when bt=1
  TextureRT *FImg;
  pTextureRTpair FImgp;
  TextureRT *FImg1;
  pTextureRTpair FImg1p;
  TextureRT *FImg2;
  pTextureRTpair FImg2p;
  TextureRT *FImgDegrid;
  pTextureRTpair FImgDegridp;

  //these texture pointers are used to avoid memory leakages so they are not deleted (they contains pointers to the above textures
  pTextureRTpair* pingd;
  pTextureRTpair* pongd;
  pTextureRTpair* lastd;
  TextureRT* ping;
  TextureRT* pong;
  TextureRT* lastt;


  TextureM* Pattern;
  void SigmaToPatternTexture(float sigma, float sigma2, float sigma3, float sigma4);

  bool sharpen;
  bool useHalf;
  bool NVPERF;
  float degrid;
  bool usePattern;

  HANDLE mutex;//this ensures that only one thread access fft3dgpu at a time
  LPDIRECT3DDEVICE9 pDevice;
  GPUTYPES* gtype;

  Dxinput* DI;
  IDirect3DSurface9* backbuffer;
};





