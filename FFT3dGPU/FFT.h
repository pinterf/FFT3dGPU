// FFT3DGPU plugin for Avisynth 2.5
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

#include "FFTps.h"
#include "windows.h"

class TexturePool;
class FFT2d{
public:
	virtual void CalcFFT(TextureRT* src,TextureRT* (&dst),bool forward)=0;
	virtual ~FFT2d(){};
};


void WriteMemFloatToFile(float* src,int width,int height,int elemsize,const char* SaveFilename,const char* Tittle,bool append=false);
void WriteMemFixedToFile(const unsigned char* src,int width,int height,int elemsize,const char* SaveFilename,const char* Tittle,bool append=false);
enum {
	FLOAT16=0,
	FLOAT32_FFT=1,
	FLOAT32_ALL=2
};
class FFT2dRR:public FFT2d{
	public:
	FFT2dRR(unsigned int _x,unsigned int _xnum,unsigned int _y,unsigned int _ynum,unsigned int _bt,TexturePool *&StreamPoolPointer,LPDIRECT3DDEVICE9 _pDevice,GPUTYPES* _gtype,int precision,HRESULT &hr);
	~FFT2dRR();
	void CalcFFT(TextureRT* src,TextureRT* (&dst),bool forward);
	void CalcFFT1d(std::vector< TextureRT* > src,std::vector<TextureRT* > &dst,int n,bool forward);
	void CalcFFT1d(std::vector< pTextureRTpair* > src,std::vector<pTextureRTpair* > &dst,int n,bool forward);
protected:
	void CreateC2RMap(float *Map,unsigned int n,bool forward,unsigned int rep);
	void CalcFFTVert(bool forward);
	void CalcFFTHori(bool forward);
	void CreateLoadMap(float *Map,unsigned int n,unsigned int rep);
	void CreateLoadMap2(float *Map,float* BitReverseMap,float* LoadMap,float* SaveMap,int width,int height);
	void CreateSaveMap(float *Map,unsigned int n,unsigned int rep);
	void CreateTwiddleMap(float *Map,unsigned int n,bool forward,unsigned int rep);
	void CreateBitReverseMap(float *Map,unsigned int n,unsigned int rep);
	int ReverseBit(int data2invert,unsigned char revbits);
  int ReverseBit_c(int data2invert, unsigned char revbits);

	template <class Type> Type log2(Type n){
	Type logn=0;
	for(Type i=n;i>1;i/=2,logn++);
	return logn;
}
	
	//TextureM *BitReverseLUT[2];
	TextureM *TwiddleLUTforward[2];
	TextureM *TwiddleLUTbackward[2];
	//TextureM *LoadLUT[2];
	TextureM *LoadLUT2[2];
	TextureM *SaveLUT[2];

	TextureM *R2CLUT;
    TextureM *C2RLUT;

	//::brook::iter *it2[2];
	//::brook::iter *it1[2];

	TexturePool *FreeStreamPool;

    TextureRT *Img;
	TextureRT *TempI[2];
	TextureRT *TempIpong[2];
	TextureRT *TempJ[2];
	TextureRT *TempJpong[2];
	TextureRT *TempF[2];
	TextureRT *Temp;
	TextureRT *FixedTemp;
//	psFFTtoFixed* FFTtoFixed;

	unsigned int x;
	unsigned int log2x;
	float* nnx;
	float* stagex;
	unsigned int xV;
	unsigned int xVn;
	unsigned int xnum;


	unsigned int y;
	unsigned int yn;
	unsigned int log2y;
	float* nny;
	float* stagey;
	unsigned int ynum;

	unsigned int bt;

	float norm;

	LPDIRECT3DDEVICE9 pDevice;
	GPUTYPES* gtype;

	psCollectH*		CollectH;	
	psCollectV*		CollectV;
	psC2R*			C2R;
	psR2C*			R2C;

	psBitReverseButterFly* BitReverseButterFlyBH;
	psBitReverseButterFly* BitReverseButterFlyH;
	psButterflyCollect*  ButterflyCollectH;
	psBitReverseButterFly* BitReverseButterFlyV;
	psButterflyCollect*  ButterflyCollectV;

	psFFT2*		FFT2p;
	psiFFT2p*		iFFT2p;
	psFFT3*		FFT3p;
	psiFFT3p*		iFFT3p;


};

