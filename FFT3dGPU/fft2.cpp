
#include "windows.h"
#include <math.h>
#include "D3dx9tex.h."
#include "d3d9.h"
#include "d3dx9.h"
#include "TexturePool.h"
#include "./core/texture.h"
#include "./core/vertexbuffer.h"
#include "./core/pixelshader.h"
#include "./core/Debug class.h"
#include "./core/ManageDirect3DWindow.h"
#include "fft2.h"
#include "stdafx.h"

int log2(int n)
{
    int logn=0;
	for(int i=n;i>1;i/=2,logn++);
	return logn;
}

FFT2dRR2::FFT2dRR2(unsigned int x,unsigned int xrep,unsigned int y,unsigned int yrep,unsigned int z,TexturePool *&StreamPoolPointer,LPDIRECT3DDEVICE9 pDevice,	GPUTYPES* gtype,int precision,HRESULT &hr):
_log2cx(log2(x/2)),_log2y(log2(y)),_pDevice(pDevice)
,norm(2.0/(x*y*z))
{


bstartstage=1;
int cx=x/2;//complex fft width
int rx=cx+1;//real fft width after transform
int height=y*yrep;
int srcwidth=cx*xrep;//width before transform
int width=rx*xrep;//width after transform

Types float4=precision>FLOAT16?gtype->FLOAT4():gtype->HALF4();

TempVertical1=NEW TextureRT(pDevice,width,height/2,float4,hr);
TempVertical2=NEW TextureRT(pDevice,width,height/2,float4,hr);
TempVertical3=NEW TextureRT(pDevice,width,height/2,float4,hr);
TempVertical4=NEW TextureRT(pDevice,width,height/2,float4,hr);
TempHorizontal1=NEW TextureRT(pDevice,srcwidth/2,height,float4,hr);
TempHorizontal2=NEW TextureRT(pDevice,srcwidth/2,height,float4,hr);
TempHorizontal3=NEW TextureRT(pDevice,srcwidth/2,height,float4,hr);
TempHorizontal4=NEW TextureRT(pDevice,srcwidth/2,height,float4,hr);
D3DCAPS9 Cap;
pDevice->GetDeviceCaps(&Cap);
//Determ if card support MultipleRenderTargets(MRT) and setup the pixelshaders accordingly 
if(Cap.NumSimultaneousRTs==1)
{//don't support MRT
	ButterFlyH=NEW psButterflyHMP(pDevice,xrep,cx,yrep,y);
	ButterFlyV=NEW psButterflyVMP(pDevice,xrep,rx,yrep,y);
}
else
{
	ButterFlyH=NEW psButterflyHSP(pDevice,xrep,cx,yrep,y);
	ButterFlyV=NEW psButterflyVSP(pDevice,xrep,rx,yrep,y);
}
C2Rfft=NEW psC2Rfft(pDevice,xrep,cx,yrep,y);
R2Cfft=NEW psR2Cfft(pDevice,xrep,cx,yrep,y);
CombineV=NEW psCombine(pDevice,xrep,rx,yrep,y,false,1);
CombineH=NEW psCombine(pDevice,xrep,cx,yrep,y,true,norm);

//Setup the pool of free "frequency" textures that the result from the fft is returned in
FreeStreamPool = NEW TexturePool(pDevice,width,height,precision!=FLOAT32_ALL?gtype->HALF4():float4); 
StreamPoolPointer=FreeStreamPool;

}

FFT2dRR2::~FFT2dRR2()
{
	delete TempVertical1;
	delete TempVertical2;
	delete TempVertical3;
	delete TempVertical4;
	delete TempHorizontal1;
	delete TempHorizontal2;
	delete TempHorizontal3;
	delete TempHorizontal4;
	
	delete ButterFlyH;
	delete ButterFlyV;
	delete C2Rfft;
	delete R2Cfft;
	delete CombineH;
	delete CombineV;

	delete FreeStreamPool;
}

void FFT2dRR2::CalcFFT(TextureRT* src,TextureRT* (&dst),bool forward)
{
PROFILE_BLOCK

TextureRT	  *ping1,*ping2,*pong1,*pong2;
if(forward){
	if(!dst)		 
		 FreeStreamPool->pop(dst);	
	//Horizontal 1d fft
	ping1=TempHorizontal1;ping2=TempHorizontal2;pong1=TempHorizontal3;pong2=TempHorizontal4;
	ButterFlyH->Apply(src,pong1,pong2,true);
	std::swap(ping1,pong1);std::swap(ping2,pong2);

	for(unsigned int i=1;i<_log2cx;i++)
	{
		ButterFlyH->Apply(ping1,ping2,pong1,pong2,i,true);
		std::swap(ping1,pong1);std::swap(ping2,pong2);
	
	}
	//convert complex halfsize fft to real fullsize fft
	C2Rfft->Apply(ping1,ping2,dst);
	//Vertical 1d fft
	ping1=TempVertical1;ping2=TempVertical2;pong1=TempVertical3;pong2=TempVertical4;
	ButterFlyV->Apply(dst,pong1,pong2,true);
	std::swap(ping1,pong1);std::swap(ping2,pong2);
	for(unsigned int i=1;i<_log2y;i++)
	{
		ButterFlyV->Apply(ping1,ping2,pong1,pong2,i,true);
		std::swap(ping1,pong1);std::swap(ping2,pong2);
	}
	//combine result in ping1 and ping2
	CombineV->Apply(ping1,ping2,dst);
}
else
{
	//vertical
	ping1=TempVertical1;ping2=TempVertical2;pong1=TempVertical3;pong2=TempVertical4;
	ButterFlyV->Apply(src,pong1,pong2,false);
	std::swap(ping1,pong1);std::swap(ping2,pong2);
	for(unsigned int i=1;i<_log2y;i++)
	{
		ButterFlyV->Apply(ping1,ping2,pong1,pong2,i,false);
		std::swap(ping1,pong1);std::swap(ping2,pong2);
	}

	//convert from real fft to complex
	R2Cfft->Apply(ping1,ping2,dst);
	//horizontal 
	ping1=TempHorizontal1;ping2=TempHorizontal2;pong1=TempHorizontal3;pong2=TempHorizontal4;
	ButterFlyH->Apply(dst,pong1,pong2,false); 
	std::swap(ping1,pong1);std::swap(ping2,pong2);
	for(unsigned int i=1;i<_log2cx;i++)
	{
		ButterFlyH->Apply(ping1,ping2,pong1,pong2,i,false);
		std::swap(ping1,pong1);std::swap(ping2,pong2);
	}

	//combine
	CombineH->Apply(ping1,ping2,dst);
}
_pDevice->SetVertexShader(0);
}