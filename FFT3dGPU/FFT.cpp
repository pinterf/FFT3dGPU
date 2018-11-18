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


#include "./core/Debug class.h"
#include "TexturePool.h"
#include <dxerr.h>
#include "FFT.h"


const double pi(acos(-1.0));


/*
 * FFT2dRR
 *	constructor, Setup the neccesary textures and pixelshaders. Also initialize the texture stack.
 *	FFT2dRR performs the 2d fast fourier transform on a texture that contains 2 packed real vaule arrays	
 *  It can also perform the 2 or 3 point 1d fft on complex arrays packed in 2 or 3 textures. (combined with the 2d tranform
 *  we get the 3d fft on 2 real value arrays)	
 *
 * Inputs:
 *	_x:[in] block width
 *	_xnum:[in] number of blocks in width
 *	_y:[in] block height
 *	_ynum:[in] number of blocks in height
 *	_bt:[in] temporal width
 *	_StreamPoolPointer:[out] Pointer to an empty textureRT stack
 *	_StreamPoolSize:[in] Size of the texturepool to create
 *	_pDevice:[in]	Direct3d device to use
 *  _gtype:[in] Texture information
 *	precision:[in] FLOAT16=use 16bit precision float texture/fft FLOAT32_FFT=32 bit precision  fft result stored in 16 bit precision FLOAT32_ALL 32 bit precision
 *  _hr:[out] return result
 *
 * Returns:
 *     None
 *
 * Remarks:
 *     
 */

FFT2dRR::FFT2dRR(unsigned int _x,unsigned int _xnum,unsigned int _y,unsigned int _ynum,unsigned int _bt,TexturePool *&StreamPoolPointer,LPDIRECT3DDEVICE9 _pDevice,	GPUTYPES* _gtype,int precision,HRESULT &hr)
:x(_x/2),log2x(log2(_x/2)),xnum(_xnum),y(_y),log2y(log2(_y)),ynum(_ynum),xV((_x/2)+1),yn(_ynum*y),xVn(_xnum*xV),Img(0),bt(_bt)
,norm(1.0/(_x*_y*_bt)),pDevice(_pDevice),gtype(_gtype)
{




float* R2CMap;
float* C2RMap;
float* TwiddleMap; 
float* brMap;
float* SaveMap;
float* LoadMap; 



//setup the dimensions
unsigned int n[2]={x,y};

unsigned int xn=x*xnum;
unsigned int x2=x/2;
unsigned int x2n=xn/2;

unsigned int y2=y/2;
unsigned int y2n=yn/2;


unsigned int nn[2]={xn,yn};
unsigned int n2[2]={x2,y2};
unsigned int n2n[2]={x2n,y2n};

unsigned int log2n[2]={log2x,log2y};

unsigned int nrep[2]={xnum,ynum};

//the normalization factor for the FFT
norm=1.0/(x*y*bt);

Types _float4;
Types _float2;
Types _float1;
if(precision>FLOAT16)
{
_float4=gtype->FLOAT4();
_float2=gtype->FLOAT2();
_float1=gtype->FLOAT();
}
else
{
_float4=gtype->HALF4();
_float2=gtype->HALF2();
_float1=gtype->HALF();
}

//Setup the look up table for the conversion from complex fft to real fft(the complex number consist of 1 real number for the real part and 1 real number for the imaginary part meaning a half size complex fft is converted to a full size real fft) and reverse
R2CLUT= NEW  TextureM(pDevice,xVn,1,_float4,hr);
if(FAILED(hr)){	hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create R2CLUT",true);return;}
C2RLUT= NEW  TextureM(pDevice,xn,1,_float4,hr);
if(FAILED(hr)){	hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create C2RLUT",true);return;}
R2CMap=NEW  float[xVn*4];
C2RMap=NEW  float[xn*4];
CreateC2RMap(R2CMap,x,true,xnum);
UploadToTexture(R2CLUT,R2CMap);

delete[] R2CMap;
CreateC2RMap(C2RMap,x,false,xnum);
UploadToTexture(C2RLUT,C2RMap);

delete[] C2RMap;

//Setup the temporary textures need for the fft
TempF[0]= NEW  TextureRT(pDevice,xn,yn,_float4,hr);
if(FAILED(hr)){	hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create TempF[0]",true);return;}
TempI[0]= NEW  TextureRT(pDevice,x2n,yn,_float4,hr);
if(FAILED(hr)){hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create TempI[0]",true);return;}
TempJ[0]= NEW  TextureRT(pDevice,x2n,yn,_float4,hr);
if(FAILED(hr)){hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create TempJ[0]",true);return;}
TempIpong[0]= NEW  TextureRT(pDevice,x2n,yn,_float4,hr);
if(FAILED(hr)){hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create TempIpong[0]",true);return;}
TempJpong[0]= NEW  TextureRT(pDevice,x2n,yn,_float4,hr);
if(FAILED(hr)){hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create TempJpong[0]",true);return;}
TempF[1]= NEW  TextureRT(pDevice,xVn,yn,_float4,hr);
if(FAILED(hr)){hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create TempF[1]",true);return;}
TempI[1]= NEW  TextureRT(pDevice,xVn,y2n,_float4,hr);
if(FAILED(hr)){hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create TempI[1]",true);return;}
TempJ[1]= NEW  TextureRT(pDevice,xVn,y2n,_float4,hr);
if(FAILED(hr)){hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create TempJ[1]",true);return;}
TempIpong[1]= NEW  TextureRT(pDevice,xVn,y2n,_float4,hr);
if(FAILED(hr)){hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create TempIpong[1]",true);return;}
TempJpong[1]= NEW  TextureRT(pDevice,xVn,y2n,_float4,hr);
if(FAILED(hr)){hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create TempJpong[1]",true);return;}

//Setup the pool of free "frequency" textures that the result from the fft is returned in
FreeStreamPool = NEW TexturePool(pDevice,xVn,yn,precision!=FLOAT32_ALL?gtype->HALF4():_float4); 
StreamPoolPointer=FreeStreamPool;
/*for(int i=0;i<StreamPoolSize;i++)
{
	FreeStreamPool.push(NEW  TextureRT(pDevice,xVn,yn,_float4,hr));
	if(FAILED(hr)){hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create FreeStreamPool",true);return;}
}*/


//This part setups the LUT for fft. One for the horizontal fft and one for the vertical fft
for(unsigned int i=0;i<2;i++)
{

TwiddleLUTforward[i]= NEW  TextureM(pDevice,n2n[i],log2n[i],_float2,hr);
if(FAILED(hr)){	hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create TwiddleLUTforward",true);return;}
TwiddleLUTbackward[i]= NEW  TextureM(pDevice,n2n[i],log2n[i],_float2,hr);
if(FAILED(hr)){	hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create TwiddleLUTbackward",true);return;}
LoadLUT2[i] = NEW  TextureM(pDevice,n2n[i],log2n[i],_float2,hr);
if(FAILED(hr)){	hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create LoadLUT2",true);return;}
SaveLUT[i]= NEW  TextureM(pDevice,nn[i],log2n[i],_float2,hr);
if(FAILED(hr)){	hr=DXTrace( "fft.cpp" ,__LINE__ ,hr,"FFT2dRR::Create SaveLUT",true);return;}
 

TwiddleMap = NEW  float[nn[i] * log2n[i]];
 CreateTwiddleMap(TwiddleMap,n[i],true,nrep[i]);
 UploadToTexture(TwiddleLUTforward[i],TwiddleMap);
 CreateTwiddleMap(TwiddleMap,n[i],false,nrep[i]);
 UploadToTexture(TwiddleLUTbackward[i],TwiddleMap);
  delete[] TwiddleMap;

 brMap = NEW  float[nn[i]];
 CreateBitReverseMap(brMap,n[i],nrep[i]);


 SaveMap = NEW  float[2 * nn[i] * log2n[i]];
 CreateSaveMap(SaveMap,n[i],nrep[i]);
 UploadToTexture(SaveLUT[i],SaveMap);



 LoadMap = NEW  float[n2n[i] * log2n[i]];
CreateLoadMap(LoadMap,n[i],nrep[i]);
 float* LoadMap2=NEW  float[2*n2n[i]*log2n[i]];
 CreateLoadMap2(LoadMap2,brMap,LoadMap,SaveMap,nn[i],log2n[i]);
 UploadToTexture(LoadLUT2[i],LoadMap2);
delete[] LoadMap2;
delete[] LoadMap;
delete[] brMap;	
delete[] SaveMap;
}
nnx=NEW  float[log2x];
nny=NEW  float[log2y];
stagex=NEW  float[log2x];
stagey=NEW  float[log2y];
for (unsigned int i = 0,nn=1; i < log2x;nn<<=1, i++)
{
	nnx[i]=(float)nn/xn;
	stagex[i]=(i+0.5)/log2x;
}
for (unsigned int i = 0,nn=1; i < log2y;nn<<=1, i++)
{
	nny[i]=(float)nn/yn;
	stagey[i]=(i+0.5)/log2y;
}
	//Setup pixelshaders
	C2R=NEW  psC2R(pDevice,TempF[0]);
	CollectH=NEW  psCollectH(pDevice,TempF[0]);
	R2C=NEW  psR2C(pDevice,TempF[1]);
	CollectV=NEW  psCollectV(pDevice,TempF[1]);

	D3DCAPS9 Cap;
	pDevice->GetDeviceCaps(&Cap);
	//Determ if card support MultipleRenderTargets(MRT) and setup the pixelshaders accordingly 
	if(Cap.NumSimultaneousRTs==1)
	{//don't support MRT
	BitReverseButterFlyBH=NEW  psBitReverseButterFlyBH_MP(pDevice,TempI[0],norm,stagex[0]);
	BitReverseButterFlyH=NEW  psBitReverseButterFlyH_MP(pDevice,TempI[0],stagex[0]);
	ButterflyCollectH= NEW  psButterflyCollectH_MP(pDevice,TempI[0],1.0/x2n);
	BitReverseButterFlyV=NEW  psBitReverseButterFlyV_MP(pDevice,TempI[1],stagey[0]);
	ButterflyCollectV= NEW  psButterflyCollectV_MP(pDevice,TempI[1],1.0/y2n);

	FFT2p=NEW  psFFT2p_MP(pDevice,TempF[1]);
	FFT3p=NEW  psFFT3p_MP(pDevice,TempF[1]);
	}
	else
	{//Does support MRT
	BitReverseButterFlyBH=NEW  psBitReverseButterFlyBH(pDevice,TempI[0],norm,stagex[0]);
	BitReverseButterFlyH=NEW  psBitReverseButterFlyH(pDevice,TempI[0],stagex[0]);
	ButterflyCollectH= NEW  psButterflyCollectH(pDevice,TempI[0],1.0/x2n);
	BitReverseButterFlyV=NEW  psBitReverseButterFlyV(pDevice,TempI[1],stagey[0]);
	ButterflyCollectV= NEW  psButterflyCollectV(pDevice,TempI[1],1.0/y2n);

	FFT2p=NEW  psFFT2p(pDevice,TempF[1]);
	FFT3p=NEW  psFFT3p(pDevice,TempF[1]);
	}
	iFFT2p=NEW  psiFFT2p(pDevice,TempF[1]);
	iFFT3p=NEW  psiFFT3p(pDevice,TempF[1]);
  
}

FFT2dRR::~FFT2dRR(){
	for(unsigned int i=0;i<2;i++){

  delete TwiddleLUTforward[i];
  delete TwiddleLUTbackward[i];
  delete LoadLUT2[i];
  delete SaveLUT[i];

  delete TempF[i];
  delete TempI[i];
  delete TempJ[i];
  delete TempIpong[i];
  delete TempJpong[i];
	}

  
  delete R2CLUT;
  delete C2RLUT;
  delete FreeStreamPool;
	
	delete CollectH;	
	delete CollectV;
	delete C2R;
	delete R2C;
	delete BitReverseButterFlyBH;
	delete BitReverseButterFlyH;
	delete ButterflyCollectH;
	delete BitReverseButterFlyV;
	delete ButterflyCollectV;
	delete FFT2p;
	delete iFFT2p;
	delete FFT3p;
	delete iFFT3p;
	
	delete stagex;
	delete stagey;
	delete nnx;
	delete nny;

}

/*
 * CalcFFT1d
 *
 *		 Performs the 2 or 3 point 1d fft on complex arrays packed in 2 or 3 textures
 *
 * Inputs:
 *     src:[in] vector containing the 2 or 3 textures to be read from 
 *     dst:[out] vector containing the 2 or 3 textures to be converted to 
 *     n:[in] number of textures
 *	   forward:[in] forward fft? 
 *
 * Returns:
 *     None
 *
 * Remarks:
 *     n must be 2 or 3
 */


void FFT2dRR::CalcFFT1d(std::vector< TextureRT* > src,std::vector< TextureRT* > &dst,int n,bool forward){
	if(forward){
		for(int i=0;i<n;i++)
			//if dst=0 get a Texture from the pool
			if(!dst[i]){
				dst[i]=FreeStreamPool->top();
				FreeStreamPool->pop();
			}	
		if(n==2)
			FFT2p->Apply(src[0],src[1],dst[0],dst[1]);
		else
			FFT3p->Apply(src[0],src[1],src[2],dst[0],dst[1],dst[2]);
	}
	else{
		if(n==2)
			iFFT2p->Apply(src[0],src[1],dst[1]);
		else
			iFFT3p->Apply(src[0],src[1],src[2],dst[1]);
	}
}

/*
 * CalcFFT1d
 *
 *		 Performs the 2 or 3 point 1d fft on complex arrays packed in 2 or 3 texture pairs
 *
 * Inputs:
 *     src:[in] vector containing the 2 or 3 texturepairs to be read from 
 *     dst:[out] vector containing the 2 or 3 texturepairs to be converted to 
 *     n:[in] number of texturepairs
 *	   forward:[in] forward fft? 
 *
 * Returns:
 *     None
 *
 * Remarks:
 *     n must be 2 or 3. This function is intented for mode 1
 */
void FFT2dRR::CalcFFT1d(std::vector< pTextureRTpair* > src,std::vector< pTextureRTpair* > &dst,int n,bool forward){
	if(forward){
		for(int i=0;i<n;i++)
			//if dst=0 get a Texture from the pool
			if(!dst[i]->first){
				dst[i]->first=FreeStreamPool->top();
				FreeStreamPool->pop();
				dst[i]->last=FreeStreamPool->top();
				FreeStreamPool->pop();
			}	
		if(n==2){
			FFT2p->Apply(src[0]->first,src[1]->first,dst[0]->first,dst[1]->first);
			FFT2p->Apply(src[0]->last,src[1]->last,dst[0]->last,dst[1]->last);
		}
		else{
			FFT3p->Apply(src[0]->first,src[1]->first,src[2]->first,dst[0]->first,dst[1]->first,dst[2]->first);
			FFT3p->Apply(src[0]->last,src[1]->last,src[2]->last,dst[0]->last,dst[1]->last,dst[2]->last);
		}
	}
	else{
		if(n==2){
			iFFT2p->Apply(src[0]->first,src[1]->first,dst[1]->first);
			iFFT2p->Apply(src[0]->last,src[1]->last,dst[1]->last);
		}
		else{
			iFFT3p->Apply(src[0]->first,src[1]->first,src[2]->first,dst[1]->first);
			iFFT3p->Apply(src[0]->last,src[1]->last,src[2]->last,dst[1]->last);
		}
	}
}


/*
 * CalcFFT
 *
 *		 Performs the 2d fft on 2 real value arrays packed in 1 float4 texture
 *
 * Inputs:
 *     src:[in] texture to convert 
 *     dst:[out] save result in this texture. If no texture is supplied return one from the pool
 *	   forward:[in] forward fft? 
 *
 * Returns:
 *     None
 *
 * Remarks:
 *     the real arrayes a and b must be packed like this in one float 4  a[0] b[0] a[1] b[1] 
 *     and the complex result like this a_real b_real a_imag b_imag
 */
void FFT2dRR::CalcFFT(TextureRT* src,TextureRT* (&dst),bool forward){
	PROFILE_BLOCK
	if(forward){	
	 
	 Img=src;
	 Temp=TempF[1];
	 if(!dst){		 
		 TempF[1]=FreeStreamPool->top();
		 FreeStreamPool->pop();	
	 }
	 else
		TempF[1]=dst;
	 //D3DLOCKED_RECT LockedRect;
	 CalcFFTHori(forward);
	 
	 CalcFFTVert(forward);

	 //FFTtoFixed->Apply(TempF[1],FixedTemp);

	 dst=TempF[1];
     TempF[1]=Temp;

	}
	else{	
	 Img=src;
	 Temp=TempF[0];
	 TempF[0]=dst;
	 CalcFFTVert(forward);
	 CalcFFTHori(forward);
	 dst=TempF[0];
	 TempF[0]=Temp;
	}
}

/*
 * CalcFFTHori
 *
 *		 Performs the horizontal 1d fft. The input is threated as complex values of half the length and converted to a full lengthreal value fft without the negative frequencies 
 *
 * Inputs:
 *	   forward:[in] forward fft? 
 *
 * Returns:
 *     None
 *
 * Remarks:
 *	
 */

void FFT2dRR::CalcFFTHori(bool forward){
	PROFILE_BLOCK
	TextureM *TwiddleLUT;
	
	TextureRT *I=TempI[0];
	TextureRT *J=TempJ[0];
//	D3DLOCKED_RECT r;

	if(forward){
		TwiddleLUT=TwiddleLUTforward[0];

		//Bitreverse stage combined with the first butterfly
		BitReverseButterFlyH->Apply(Img,LoadLUT2[0],TempI[0],TempJ[0]);

		}
	else{
		//If backward transform convert the real value fft to complex fft of half size
		C2R->Apply(TempF[1],C2RLUT,Temp);

		//Bitreverse stage combined with the first butterfly
		TwiddleLUT=TwiddleLUTbackward[0];
		BitReverseButterFlyBH->Apply(Temp,LoadLUT2[0],TempI[0],TempJ[0]);

	}


//Do log2 blockwidth - 1 butterfly and collect operations
	for (unsigned int i = 1; i < log2x;i++)
	{

		ButterflyCollectH->Apply(TempI[0],TempJ[0],TwiddleLUT,LoadLUT2[0],stagex[i],TempIpong[0],TempJpong[0]);
		I=TempIpong[0];
		J=TempJpong[0];
        TempIpong[0]=TempI[0];
		TempJpong[0]=TempJ[0];
		TempI[0]=I;
		TempJ[0]=J;
	}
	//Finaly combine the two textures to one
	
	
	CollectH->Apply(TempI[0],TempJ[0],SaveLUT[0],stagex[log2x-1],TempF[0]);
	if(forward){
		//If forward transform convert the complex fft to real value fft
		R2C->Apply(TempF[0],R2CLUT,Temp);

	}

}

/*
 * CalcFFTVert
 *
 *		 Performs the vertical 1d fft. 
 *
 * Inputs:
 *	   forward:[in] forward fft? 
 *
 * Returns:
 *     None
 *
 * Remarks:
 *	
 */
void FFT2dRR::CalcFFTVert(bool forward){
	PROFILE_BLOCK
	 Texture *TwiddleLUT;
	 TextureRT *I=TempI[1];
	TextureRT *J=TempJ[1];

	if(forward){
		TwiddleLUT=TwiddleLUTforward[1];
		BitReverseButterFlyV->Apply(Temp,LoadLUT2[1],TempI[1],TempJ[1]);

			
	}
	else{
		TwiddleLUT=TwiddleLUTbackward[1];
		BitReverseButterFlyV->Apply(Img,LoadLUT2[1],TempI[1],TempJ[1]);
	}


  for (unsigned int i = 1; i < log2y; i++)
  {	

	ButterflyCollectV->Apply(TempI[1],TempJ[1],TwiddleLUT,LoadLUT2[1],stagey[i],TempIpong[1],TempJpong[1]);
		I=TempIpong[1];
		J=TempJpong[1];
		TempIpong[1]=TempI[1];
		TempJpong[1]=TempJ[1];
		TempI[1]=I;
		TempJ[1]=J;


  }
	CollectV->Apply(TempI[1],TempJ[1],SaveLUT[1],stagey[log2y-1],TempF[1]); 

 
}

/*
 * CreateSaveMap
 *
 *		 Create the savemap to be used when combining the two textures from the butterfly stage 
 *		 The returned map containes texture coordinates 
 *
 * Inputs:
 *		Map:[out] The map to return. Must have at least size n*rep*log2 n
 *		n:[in] blocksize
 *		rep:[in] number of blocks
 *
 * Returns:
 *     None
 *
 * Remarks:
 *	 
 */

void FFT2dRR::CreateSaveMap(float *Map,unsigned int n,unsigned int rep){
	float div=n/2*rep;//n=x|y I=n/2 OK?
	for(unsigned int m=2,y=0;m<n*2;m<<=1){
		for(unsigned int r=0,roff=0;r<rep;r++,roff+=n/2,y+=2*n){
			for(unsigned int o=0,x=0;o<m;o+=2){
				for(unsigned int i=o;i<n*2;i+=m*2){
					unsigned int j=i+m;				
					Map[y+i]=(x+roff+0.5)/div;
					Map[y+i+1]=0; //Texture I
					Map[y+j]=(x+roff+0.5)/div;
					Map[y+j+1]=1; //Texture J
					x++;
				}
			}
		}
	}
}

/*
 * CreateLoadMap2
 *
 *		 Create the loadmap to be used when combining the two textures (I and J) in the butterfly stage (stage 1 and greater)
 *		 and the location to load from in stage 0 (bitreverse)
 *		 The returned map containes texture coordinates 
 *
 * Inputs:
 *		Map:[out] The map to return. Must have at least size width*height
 *		BitReverseMap:[in] Map containing the texture coordinates for the bitreverse stage(stage 0)
 *		LoadMap:[in] Map containing the index(integer) to load the texture coordinates from the savemap
 *		SaveMap:[in] Contains the location of the texturecoordinate to load from and from what texture(I or J) to load from 
 *		width:[in] total length of transform
 *		height:[in] number of stages(log2(blocksize))
 *
 * Returns:
 *     None
 *
 * Remarks:
 *	 
 */
void FFT2dRR::CreateLoadMap2(float *Map,float* BitReverseMap,float* LoadMap,float* SaveMap,int width,int height){
	
	for(int x=0;x<width;x++)
		Map[x]=BitReverseMap[x];
	for(int y=1,offset=width/2,offset1=0;y<height;y++,offset+=width/2,offset1+=2*width)
		for(int x=0;x<width/2;x++){
			int index=LoadMap[x+offset]*2;
			Map[2*x+offset*2]=SaveMap[index+offset1];
			Map[2*x+1+offset*2]=SaveMap[index+1+offset1];
		}
}
	
/*
 * CreateLoadMap
 *
 *		 Creates the loadmap to be used in loadmap2
 *
 * Inputs:
 *		Map:[out] The map to return. Must have at least size n*rep
 *		n:[in] blocksize
 *		rep:[in] number of blocks
 *
 * Returns:
 *     None
 *
 * Remarks:
 *	 
 */
void FFT2dRR::CreateLoadMap(float *Map,unsigned int n,unsigned int rep){
	for(unsigned int m=1,y=0;m<n;m<<=1){
		for(unsigned int r=0,roff=0;r<rep;r++,roff+=n,y+=n/2){
			for(unsigned int o=0,x=0;o<m;o+=1){
				for(unsigned int i=o;i<n;i+=m*2){
								
					Map[y+x]=i+roff;
					
					x++;
				}
			}
		}
	}
}


/*
 * ReverseBit
 *
 *		 Reverse the bits
 *
 * Inputs:
 *		data2invert:[in] the number to be reversed
 *		revbit:[in] number of bits to reverse
 *
 * Returns:
 *     the reversed number
 *
 * Remarks:
 *	   taken from http://www.df.lth.se/~john_e/gems/gem0018.html
 */
int FFT2dRR::ReverseBit_c(int data2invert, unsigned char revbits) {
  unsigned int  NO_OF_BITS = sizeof(data2invert) * 8;
  unsigned int reverse_num = 0, i, temp;

  for (i = 0; i < NO_OF_BITS; i++)
  {
    temp = (data2invert & (1 << i));
    if (temp)
      reverse_num |= (1 << ((NO_OF_BITS - 1) - i));
  }

  return reverse_num;
}

int FFT2dRR::ReverseBit(int data2invert,unsigned char revbits){
#if _M_X64
  return ReverseBit_c(data2invert, revbits);
#else
  int retval;
	__asm{
	    xor     eax,eax
        mov     ebx,data2invert ; your input data
        mov     cl,revbits      ; number of bits to reverse
revloop:
		ror     ebx,1
		rcl     eax,1
		dec     cl
		jnz     revloop
		mov		retval,eax
	}
	return retval;
#endif
}

/*
 * CreateBitReverseMap
 *
 *		 Creates the bitreversemap to be used in loadmap2
 *
 * Inputs:
 *		Map:[out] The map to return. Must have at least size n*rep
 *		n:[in] blocksize
 *		rep:[in] number of blocks
 *
 * Returns:
 *     None
 *
 * Remarks:
 *	 
 */
void FFT2dRR::CreateBitReverseMap(float *Map,unsigned int n,unsigned int rep){
	unsigned int logn=0;
	float div=n*rep;
	for(unsigned int i=(n-1);i>0;i/=2,logn++);
	for(unsigned int r=0,y=0,roff=0;r<rep;r++,roff+=n,y+=n){
			for(unsigned int i=0;i<n;i++){
				Map[y+i]=(ReverseBit(i,logn)+roff+0.5)/div;
			}
	}
}


/*
 * CreateTwiddleMap
 *
 *		 Creates the twiddlemap to be used in the butterfly stages
 *
 * Inputs:
 *		Map:[out] The map to return. Must have at least size n*rep*log2(n)
 *		n:[in] blocksize
 *		forward:[in] forward transform?
 *		rep:[in] number of blocks
 *
 * Returns:
 *     None
 *
 * Remarks:
 *	 
 */
void FFT2dRR::CreateTwiddleMap(float *Map,unsigned int n,bool forward,unsigned int rep){
	//WR,WI0 WR,WI1 WR,WI2 ... ... WR,WIn
	int isign=forward*1-1*!forward;

	double theta,wtemp,wpr,wpi,wr,wi;
	for(unsigned int m=2,y=0;m<n*2;m<<=1){
		for(unsigned int r=0;r<rep;r++,y+=n){	
			theta=(2*pi/m*isign);
			wtemp=sin(0.5*theta);
			wpr=-2*wtemp*wtemp;
			wpi=sin(theta);
			wr=1;
			wi=0;
			for(unsigned int o=0,x=0;o<m;o+=2){
				for(unsigned int i=o;i<n;i+=m){
					Map[y+x]=wr;
					Map[y+x+1]=wi;
					x+=2;
				}
				wr=(wtemp=wr)*wpr-wi*wpi+wr;
				wi=wi*wpr+wtemp*wpi+wi;
			}
		}
	}

}
	
/*
 * CreateC2RMap
 *
 *		 Creates a map to be used when converting a half size complex fft to a full size real fft
 *		 or the reverse transform
 *
 * Inputs:
 *		Map:[out] The map to return. Must have at least size n*rep*4
 *		n:[in] blocksize
 *		forward:[in] forward transform (meaning transforming from complex to real)?
 *		rep:[in] number of blocks
 *
 * Returns:
 *     None
 *
 * Remarks:
 *	 
 */
void FFT2dRR::CreateC2RMap(float *Map,unsigned int n,bool forward,unsigned int rep){
	unsigned int N=2*n;
	float div=(n+!forward)*rep;
	//float sign;
	for(unsigned int r=0,y=0,roff=0;r<rep;r++,roff+=n+!forward,y+=(n+forward)*4){	
			if(forward){
				//sign=1.0;
				Map[0+y]=(roff+0.5)/div;
				Map[1+y]=(roff+0.5)/div;
				Map[2+y]=1;
				Map[3+y]=0;
			}
			else{
				//sign=-1.0;
				Map[0+y]=(roff+0.5)/div;
				Map[1+y]=(n+roff+0.5)/div;
				Map[2+y]=1;
				Map[3+y]=0;
			}
			for(unsigned int i=1,o=4;i<n;i++,o+=4){
				Map[o+y]=(i+roff+0.5)/div;
				Map[o+1+y]=(n-i+roff+0.5)/div;
				Map[o+2+y]=cos(2*pi*i/N);
				Map[o+3+y]=sin(2*pi*i/N);
			}
			if(forward){
				Map[n*4+y]=(roff+0.5)/div;
				Map[n*4+1+y]=(roff+0.5)/div;
				Map[n*4+2+y]=cos(2*pi*n/N);
				Map[n*4+3+y]=sin(2*pi*n/N);
			}
		}
		}




