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
#include <dxerr.h>
#include "filtersps.h"



psMinimize::psMinimize(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,Texture* _src):
Pixelshader(pDevice,SRC_SHADER,"Minimize",D3DXGetPixelShaderProfile(pDevice),0),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src"))
{
	RECT rec[2]={_Rendertarget->GetRect(),_src->GetRect()};
	quad=NEW  NQuad(_pDevice,rec,1,1,true);
}


HRESULT psMinimize::Apply(Texture* src,TextureRT *dst){
	PROFILE_BLOCK
	HRESULT hr;
	quad->SetActive();
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psMinimize",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psMinimize",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psMinimize",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psMinimize",true); 
	return hr;
}

psGridCorrection::psGridCorrection(LPDIRECT3DDEVICE9 pDevice,Texture* target):
Pixelshader(pDevice,SRC_SHADER,"GridCorrection",D3DXGetPixelShaderProfile(pDevice)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	quad=NEW  NQuad(_pDevice,&(target->GetRect()),1,0,false);
}

psGridCorrection::psGridCorrection(LPDIRECT3DDEVICE9 pDevice,Texture* target,float degrid):
Pixelshader(pDevice,SRC_SHADER,"GridCorrection",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(degrid,"DGRID",CreateMacroArray(1))),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))

{
	quad=NEW  NQuad(_pDevice,&(target->GetRect()),1,0,false);
}

HRESULT psGridCorrection::Apply(Texture* src,TextureRT *dst,Texture* degridsample){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	quad->SetActive();
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"GridCorrection",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"GridCorrection",true); 
	if(FAILED(hr=degridsample->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"GridCorrection",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"GridCorrection",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"GridCorrection",true); 
	return hr;
}

//************************************************************************************************************
psWiennerFilter::psWiennerFilter(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,D3DXVECTOR2 &beta,D3DXVECTOR2 &sigma,int n,bool degrid,bool pattern):
Pixelshader(pDevice,SRC_SHADER,n==1?"Wienner2d":n==2?"Wienner3d2":n==3?"Wienner3d3":"Wienner3d4",D3DXGetPixelShaderProfile(pDevice),
			SetMacroArray(degrid?"DEGRID":" ",VecToMacroArray(sigma,pattern?"":"SIGMA",VecToMacroArray(beta,"BETA",CreateMacroArray(3)),1),2))
,_degrid(degrid)
{
	switch(n)
	{
		case 4:
			sSrc[3]=_pConstantTable->GetConstantByName(NULL,"prev2");
		case 3:
			sSrc[2]=_pConstantTable->GetConstantByName(NULL,"prev1");
			sSrc[1]=_pConstantTable->GetConstantByName(NULL,"current");
			sSrc[0]=_pConstantTable->GetConstantByName(NULL,"next");
			break;
		case 2:
			sSrc[1]=_pConstantTable->GetConstantByName(NULL,"prev1");
		case 1:
			sSrc[0]=_pConstantTable->GetConstantByName(NULL,"current");
	}
	if(degrid)
		sFactor=_pConstantTable->GetConstantByName(NULL,"degrid");
	if(pattern)
		sSigma=_pConstantTable->GetConstantByName(NULL,"sigma");
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}



HRESULT psWiennerFilter::Apply(Texture* src,TextureRT *dst,Texture* degrid,Texture* sigma)
{
	PROFILE_BLOCK
		HRESULT hr;
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc[0]))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(degrid)
		if(FAILED(hr=degrid->SetAsTexture(GetSamplerIndex(sFactor))))
			return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(sigma)
		V_RETURN(sigma->SetAsTexture(GetSamplerIndex(sSigma)));
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(quad->Draw()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	return hr;
}

HRESULT psWiennerFilter::Apply(std::vector<TextureRT*> src,TextureRT* dst,Texture* degrid,Texture* sigma)
{
	PROFILE_BLOCK
			HRESULT hr;
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	for(int i=0,j=src.size()-1;j>=0;i++,j--)
	{
	if(FAILED(hr=src[i]->SetAsTexture(GetSamplerIndex(sSrc[j]))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	}
	if(degrid)
		if(FAILED(hr=degrid->SetAsTexture(GetSamplerIndex(sFactor))))
			return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(sigma)
		V_RETURN(sigma->SetAsTexture(GetSamplerIndex(sSigma)));
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(quad->Draw()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	return hr;
}


HRESULT psWiennerFilter::Apply(std::vector<pTextureRTpair*> src,pTextureRTpair* dst,pTextureRTpair* degrid,Texture* sigma)
{
	PROFILE_BLOCK
			HRESULT hr;
	if(FAILED(hr=dst->first->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	for(int i=0,j=src.size()-1;j>=0;i++,j--)
	{
	if(FAILED(hr=src[i]->first->SetAsTexture(GetSamplerIndex(sSrc[j]))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	}
	if(sigma)
		V_RETURN(sigma->SetAsTexture(GetSamplerIndex(sSigma)));
	if(degrid)
		if(FAILED(hr=degrid->first->SetAsTexture(GetSamplerIndex(sFactor))))
			return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(quad->Draw()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 

	if(FAILED(hr=dst->last->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	for(int i=0,j=src.size()-1;j>=0;i++,j--)
	{
	if(FAILED(hr=src[i]->last->SetAsTexture(GetSamplerIndex(sSrc[j]))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	}
	if(sigma)
		V_RETURN(sigma->SetAsTexture(GetSamplerIndex(sSigma)));
	if(degrid)
		if(FAILED(hr=degrid->last->SetAsTexture(GetSamplerIndex(sFactor))))
			return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(quad->Draw()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	return hr;
}

HRESULT psWiennerFilter::Apply(pTextureRTpair* src,pTextureRTpair* dst,pTextureRTpair* degrid,Texture* sigma)
{
	PROFILE_BLOCK
			HRESULT hr;
	if(FAILED(hr=dst->first->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=src->first->SetAsTexture(GetSamplerIndex(sSrc[0]))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(sigma)
		V_RETURN(sigma->SetAsTexture(GetSamplerIndex(sSigma)));
	if(degrid)
		if(FAILED(hr=degrid->first->SetAsTexture(GetSamplerIndex(sFactor))))
			return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(quad->Draw()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=dst->last->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=src->last->SetAsTexture(GetSamplerIndex(sSrc[0]))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(sigma)
		V_RETURN(sigma->SetAsTexture(GetSamplerIndex(sSigma)));
	if(degrid)
		if(FAILED(hr=degrid->last->SetAsTexture(GetSamplerIndex(sFactor))))
			return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(quad->Draw()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	return hr;
}
//************************************************************************************************************************************************************

psKalmanSP::psKalmanSP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float covarNoise,float kratiosquared,bool pattern):
psKalman(pDevice,SRC_SHADER,"Kalman",D3DXGetPixelShaderProfile(pDevice),
		 FloatToMacroArray(covarNoise,pattern?" ":"COVARNOISE",FloatToMacroArray(kratiosquared,"KRATIOSQUARED",CreateMacroArray(2)),1)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sLast(_pConstantTable->GetConstantByName(NULL,"Last")),
sCovar(_pConstantTable->GetConstantByName(NULL,"Covar")),
sCovarProcess(_pConstantTable->GetConstantByName(NULL,"CovarProcess"))
{
	if(pattern)
		sCovarNoise=_pConstantTable->GetConstantByName(NULL,"CovarNoise");
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psKalmanSP::Apply(Texture* src,Texture* last,Texture* covarprocesslast,Texture* covarlast,TextureRT* covarprocess,TextureRT* covar,TextureRT *dst,Texture* pattern){
	PROFILE_BLOCK
	HRESULT hr;
	if(pattern)
		V_RETURN(pattern->SetAsTexture(GetSamplerIndex(sCovarNoise)));
	if(FAILED(hr=dst->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=covar->SetAsRenderTarget(1)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=covarprocess->SetAsRenderTarget(2)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=last->SetAsTexture(GetSamplerIndex(sLast))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=covarprocesslast->SetAsTexture(GetSamplerIndex(sCovarProcess))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=covarlast->SetAsTexture(GetSamplerIndex(sCovar))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(quad->Draw()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	_pDevice->SetRenderTarget(1,0);
	_pDevice->SetRenderTarget(2,0);
	return hr;
}

psKalmanMP::psKalmanMP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float covarNoise,float kratiosquared,bool pattern):
psKalman(pDevice,SRC_SHADER,"KalmanMP",D3DXGetPixelShaderProfile(pDevice),
		 SetMacroArray("PASS1_K",FloatToMacroArray(covarNoise,pattern?"":"COVARNOISE",FloatToMacroArray(kratiosquared,"KRATIOSQUARED",CreateMacroArray(3)),1),2)),
pass2(pDevice,SRC_SHADER,"KalmanMP",D3DXGetPixelShaderProfile(pDevice),
	  SetMacroArray("PASS2_K",FloatToMacroArray(covarNoise,pattern?"":"COVARNOISE",FloatToMacroArray(kratiosquared,"KRATIOSQUARED",CreateMacroArray(3)),1),2)),
pass3(pDevice,SRC_SHADER,"KalmanMP",D3DXGetPixelShaderProfile(pDevice),
	  FloatToMacroArray(covarNoise,pattern?"":"COVARNOISE",FloatToMacroArray(kratiosquared,"KRATIOSQUARED",CreateMacroArray(3)),1)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sLast(_pConstantTable->GetConstantByName(NULL,"Last")),
sCovar(_pConstantTable->GetConstantByName(NULL,"Covar")),
sCovarProcess(_pConstantTable->GetConstantByName(NULL,"CovarProcess"))
{
	if(pattern)
		sCovarNoise=_pConstantTable->GetConstantByName(NULL,"CovarNoise");
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psKalmanMP::Apply(Texture* src,Texture* last,Texture* covarprocesslast,Texture* covarlast,TextureRT* covarprocess,TextureRT* covar,TextureRT *dst,Texture* pattern){
	PROFILE_BLOCK
	HRESULT hr;
	if(pattern)
		V_RETURN(pattern->SetAsTexture(GetSamplerIndex(sCovarNoise)));
	if(FAILED(hr=dst->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=last->SetAsTexture(GetSamplerIndex(sLast))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=covarprocesslast->SetAsTexture(GetSamplerIndex(sCovarProcess))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=covarlast->SetAsTexture(GetSamplerIndex(sCovar))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(quad->Draw()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=pass2.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=covar->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(quad->Draw()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(hr=covarprocess->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true);
	if(FAILED(hr=pass3.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
	if(FAILED(quad->Draw()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"WFilter",true); 
//	_pDevice->SetRenderTarget(1,0);
//	_pDevice->SetRenderTarget(2,0);
	return hr;
}

//*****************************************************************************************************************


psImg2toImg4::psImg2toImg4(LPDIRECT3DDEVICE9 pDevice,Texture* Src,Texture* Dst):
Pixelshader(pDevice,SRC_SHADER,"Img2toImg4",D3DXGetPixelShaderProfile(pDevice),
	VecToMacroArray((D3DXVECTOR2(1.0/Src->GetWidth(),0)),"OFFSET",(Src->GetType()->real_elem_size==2?SetMacroArray("A8L8",CreateMacroArray(2),1):CreateMacroArray(1)))),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src"))
{
	RECT rect[2]={Dst->GetRect(),Src->GetRect()};
	quad=NEW NQuad(_pDevice,rect,2,1,true);
}

HRESULT psImg2toImg4::Apply(Texture* src,TextureRT* dst)
{
	PROFILE_BLOCK
	HRESULT hr;
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	return hr;
}

psImg2toFloat4m0::psImg2toFloat4m0(LPDIRECT3DDEVICE9 pDevice,RECT _Rendertarget,RECT Src,D3DXVECTOR2 &offset,Texture* src):
psImg2toFloat4(pDevice,SRC_SHADER,"Img2toFloat4",D3DXGetPixelShaderProfile(pDevice),
			   (src->GetType()->real_elem_size==2?SetMacroArray("A8L8",CreateMacroArray(1)):0)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	RECT rect[3]={_Rendertarget,Src,Src};
	rect[2].left+=offset.x;
	rect[2].right+=offset.x;
	rect[2].top+=offset.y;
	rect[2].bottom+=offset.y;

	quad=NEW  NQuad(_pDevice,rect,3);
}


HRESULT psImg2toFloat4m0::Apply(Texture* src,Texture* FactorLUT,TextureRT* dst){
	HRESULT hr;
	PROFILE_BLOCK
	/*if(!SceneBegin){
	_pDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
	
    if(FAILED(hr=_pDevice->BeginScene())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	
	SceneBegin=true;
	}*/
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	

	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 

	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	if(FAILED(hr=FactorLUT->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	
	return hr;//=_pDevice->EndScene();
}

psImg2toFloat4m2::psImg2toFloat4m2(LPDIRECT3DDEVICE9 pDevice,int bw,int bh,int repx,int repy,int border,Texture* src):
psImg2toFloat4(pDevice,SRC_SHADER,"Img2toFloat4",D3DXGetPixelShaderProfile(pDevice),
			   (src->GetType()->real_elem_size==2?SetMacroArray("A8L8",CreateMacroArray(1)):0)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	MQuad* mquad=NEW  MQuad(_pDevice);
	mquad->CreateVB(src->GetRect().right,src->GetRect().bottom,bw,bh,repx,repy,border);
	quad=mquad;
}



HRESULT psImg2toFloat4m2::Apply(Texture* src,Texture* FactorLUT,TextureRT* dst){
	PROFILE_BLOCK
	HRESULT hr;
	/*
	if(!SceneBegin){
	_pDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
	
    if(FAILED(hr=_pDevice->BeginScene())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	
	SceneBegin=true;
	
	
	}*/
if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 

	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 

	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	if(FAILED(hr=FactorLUT->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	
	if(FAILED(hr=quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4",true); 
	
	//dst->CopyTo(pback);
	//_pDevice->Present( NULL, NULL, NULL, NULL );
//_pDevice->SetRenderTarget(0,pback);
	
	return hr;//=_pDevice->EndScene();
}

psFloat4toImg2m0::psFloat4toImg2m0(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,Texture* _FactorMap,D3DXVECTOR2 &offset):
psFloat4toImg2(pDevice,SRC_SHADER,"Float4toImg2",D3DXGetPixelShaderProfile(pDevice)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	RECT rect[3]={_Rendertarget->GetRect(),_FactorMap->GetRect(),_FactorMap->GetRect()};
	rect[2].left+=offset.x;
	rect[2].right+=offset.x;
	rect[2].top+=offset.y;
	rect[2].bottom+=offset.y;
	quad=NEW  NQuad(_pDevice,rect,2,1);
	_pDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pback);
	psList.push_back(this);
}

psFloat4toImg2m0::~psFloat4toImg2m0(){
	if(pback){
		pback->Release();
		pback=0;
		//_pDevice->Release();
	}
	psList.remove(this);
}

HRESULT psFloat4toImg2m0::ResetShader(LPDIRECT3DDEVICE9 pDevice,bool firstpass)
{
	if(pDevice!=_pDevice)
		return 2;
	if(firstpass)
	{
	if(pback){
		pback->Release();
		pback=0;
		return 1;
		//_pDevice->Release();
	}
	}
	else
		return _pDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pback);
	return 1;
}

HRESULT psFloat4toImg2m0::Apply(Texture* src,Texture* FactorLUT,TextureRT* dst){
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	PROFILE_BLOCK
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 

	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	if(FAILED(hr=FactorLUT->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	_pDevice->SetRenderTarget(0,pback);
	//	_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
	//quad->Draw();
		//quad->Draw();
	/*_pDevice->EndScene();
	SceneBegin=false;*/
	
	return hr;//=_pDevice->Present( NULL, NULL, NULL, NULL );
}

psFloat4toImg2m2::psFloat4toImg2m2(LPDIRECT3DDEVICE9 pDevice,int bw,int bh,int repx,int repy,int border,TextureRT *dst):
psFloat4toImg2(pDevice,SRC_SHADER,"Float4toImg2",D3DXGetPixelShaderProfile(pDevice),
			   SetMacroArray("MODE2",CreateMacroArray(1))),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"FactorM"))
{
//	pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSU,D3DTADDRESS_MIRROR);
//	pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSV,D3DTADDRESS_MIRROR);
	MQuad* mquad=NEW  MQuad(pDevice);
	mquad->CreateVBi(dst->GetRect().right,dst->GetRect().bottom,bw,bh,repx,repy,border);
	quad=mquad;
	_pDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pback);
	psList.push_back(this);
}

psFloat4toImg2m2::~psFloat4toImg2m2(){
	if(pback){
		pback->Release();
		pback=0;
		//_pDevice->Release();
	}
	psList.remove(this);
}

HRESULT psFloat4toImg2m2::ResetShader(LPDIRECT3DDEVICE9 pDevice,bool firstpass)
{

	if(pDevice!=_pDevice)
		return 2;
	if(firstpass)
	{
	if(pback){
		pback->Release();
		pback=0;
		return 1;
		//_pDevice->Release();
	}
	}
	else
		return _pDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pback);
	return 1;
}

HRESULT psFloat4toImg2m2::Apply(Texture* src,Texture* FactorLUT,TextureRT* dst){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
//_pDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 

	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	/*if(FAILED(hr=FactorLUT->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); */
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	//dst->CopyTo(pback);
	_pDevice->SetRenderTarget(0,pback);

//_pDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);
		//quad->Draw();
	/*_pDevice->EndScene();
	SceneBegin=false;*/
	
	return hr;//=_pDevice->Present( NULL, NULL, NULL, NULL );
}
/**/
//*****************************************************************************************************************
psImg2toFloat4_2::psImg2toFloat4_2(LPDIRECT3DDEVICE9 pDevice,RECT _Rendertarget,RECT Src,D3DXVECTOR2 &offset,Texture* src):
Pixelshader(pDevice,SRC_SHADER,"Img2toFloat4_2",D3DXGetPixelShaderProfile(pDevice),
			(src->GetType()->real_elem_size==2?SetMacroArray("A8L8",CreateMacroArray(1)):0)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	RECT rect[3]={_Rendertarget,Src,Src};
	rect[1].left+=offset.x;
	rect[1].right+=offset.x;
	rect[2].top+=offset.y;
	rect[2].bottom+=offset.y;

	quad=NEW  NQuad(_pDevice,rect,3);
}

HRESULT psImg2toFloat4_2::Apply(Texture* src,Texture* FactorLUT,TextureRT* dst){
	PROFILE_BLOCK
	HRESULT hr;
	/*
	if(!SceneBegin){
	_pDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
	
    if(FAILED(hr=_pDevice->BeginScene())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4_2",true); 
	
	SceneBegin=true;
	}*/
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4_2",true); 
	

	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4_2",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4_2",true); 

	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4_2",true); 
	if(FAILED(hr=FactorLUT->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4_2",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4_2",true); 
	
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Img2toFloat4_2",true); 
	
	return hr;//=_pDevice->EndScene();
}


psFloat4toImg2_2::psFloat4toImg2_2(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,Texture* _FactorMap,D3DXVECTOR2 &offset):
Pixelshader(pDevice,SRC_SHADER,"Float4toImg2_2",D3DXGetPixelShaderProfile(pDevice)),
sSrc1(_pConstantTable->GetConstantByName(NULL,"Src")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"I")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
RECT rect[5]={_Rendertarget->GetRect(),_FactorMap->GetRect(),_FactorMap->GetRect(),_FactorMap->GetRect(),_FactorMap->GetRect()};
	rect[2].left+=offset.x;
	rect[2].right+=offset.x;
	rect[2].top+=offset.y;
	rect[2].bottom+=offset.y;
	rect[3].left+=offset.x;
	rect[3].right+=offset.x;
	rect[4].top+=offset.y;
	rect[4].bottom+=offset.y;
	quad=NEW  NQuad(_pDevice,rect,4,1);
	_pDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pback);
	psList.push_back(this);
}

psFloat4toImg2_2::~psFloat4toImg2_2(){
	if(pback){
		pback->Release();
		pback=0;
		//_pDevice->Release();
	}
	psList.remove(this);
}


HRESULT psFloat4toImg2_2::ResetShader(LPDIRECT3DDEVICE9 pDevice,bool firstpass)
{
	if(pDevice!=_pDevice)
		return 2;
	if(firstpass)
	{
	if(pback){
		pback->Release();
		pback=0;
		return 1;
		//_pDevice->Release();
	}
	}
	else
		return _pDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pback);
	return 1;
}

HRESULT psFloat4toImg2_2::Apply(TextureRT* src1,TextureRT* src2,Texture* FactorLUT,TextureRT* dst){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 

	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	if(FAILED(hr=FactorLUT->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"Float4toImg2",true); 
	_pDevice->SetRenderTarget(0,pback);
		//quad->Draw();
	/*_pDevice->EndScene();
	SceneBegin=false;*/
	
	return hr;//=_pDevice->Present( NULL, NULL, NULL, NULL );
}


//*************************************************************************************************************
psSharpen::psSharpen(LPDIRECT3DDEVICE9 pDevice,Texture* _Rendertarget,float sigmaSquaredSharpenMax,float sigmaSquaredSharpenMin,bool degrid):
Pixelshader(pDevice,SRC_SHADER,"Sharpen",D3DXGetPixelShaderProfile(pDevice),
SetMacroArray(degrid?"DEGRID":" ",FloatToMacroArray(sigmaSquaredSharpenMax,"SSMAX",FloatToMacroArray(sigmaSquaredSharpenMin,"SSMIN",CreateMacroArray(3)),1),2)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	if(degrid)
		sDegrid=_pConstantTable->GetConstantByName(NULL,"FactorM");
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psSharpen::Apply(Texture* src,Texture* Factor,TextureRT *dst,Texture* degrid){
	PROFILE_BLOCK
	HRESULT hr;
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(degrid)
		if(FAILED(hr=degrid->SetAsTexture(GetSamplerIndex(sDegrid))))
			return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=Factor->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	return hr;
}



//************************************************************************************************************


//**********************************************************************************************
psFFTtoFixed::psFFTtoFixed(LPDIRECT3DDEVICE9 pDevice,RECT src,RECT dst,bool FFT):
Pixelshader(pDevice,SRC_SHADER,"FFTtoFixed",D3DXGetPixelShaderProfile(pDevice),FFT?SetMacroArray("FFT",CreateMacroArray(1),0):0),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src"))
{
	RECT rec[2];
	rec[1]=src;
	rec[0]=dst;
	quad=NEW  NQuad(_pDevice,rec,1,1,true);
	//quad=NEW  NQuad(_pDevice,rec,1);
	_pDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pback);
	psList.push_back(this);
}

psFFTtoFixed::~psFFTtoFixed(){
	if(pback){
		pback->Release();
		pback=0;
		//_pDevice->Release();
	}
	psList.remove(this);
}

HRESULT psFFTtoFixed::ResetShader(LPDIRECT3DDEVICE9 pDevice,bool firstpass)
{
	if(pDevice!=_pDevice)
		return 2;
	if(firstpass)
	{
	if(pback){
		pback->Release();
		pback=0;
		return 1;
		//_pDevice->Release();
	}
	}
	else
		return _pDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pback);
	return 1;
}

HRESULT psFFTtoFixed::Apply(Texture* src,TextureRT *dst,bool backbuffer,bool endscene){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFTtoFixed",true); 
	_pDevice->SetRenderTarget(0,pback);
	if(!backbuffer)
		dst->SetAsRenderTarget(0);
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFTtoFixed",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFTtoFixed",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT3p",true); 
	if(FAILED(hr=quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFTtoFixed",true); 
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFTtoFixed",true); 
	if(endscene){
			_pDevice->EndScene();
			}
	if(backbuffer)
		_pDevice->Present( NULL, NULL, NULL, NULL );
	return hr;//=_pDevice->EndScene();
}

/*
psMeanSD::psMeanSD(LPDIRECT3DDEVICE9 pDevice,RECT src):
Pixelshader(pDevice,SRC_SHADER,"MeanSD",D3DXGetPixelShaderProfile(pDevice)),
sSrc(_pConstantTable->GetConstantByName(NULL,"FactorM"))
{
	RECT rec[2];
	rec[1]=src;
	rec[0].top=0;
	rec[0].left=0;
	rec[0].right=2;
	rec[0].bottom=2;
	quad=NEW  NQuad(_pDevice,rec,1,1,true);
}

HRESULT psMeanSD::Apply(Texture* src,TextureRT* dst){
	HRESULT hr;
	_pDevice->SetSamplerState(GetSamplerIndex(sSrc),D3DSAMP_MINFILTER,D3DTEXF_NONE);
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFTtoFixed",true); 
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFTtoFixed",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFTtoFixed",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT3p",true); 
	if(FAILED(hr=quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFTtoFixed",true); 
	_pDevice->SetSamplerState(GetSamplerIndex(sSrc),D3DSAMP_MINFILTER,D3DTEXF_POINT);
	return hr;
}
*/
//**************
psImg2ToFloat4oSP::psImg2ToFloat4oSP(LPDIRECT3DDEVICE9 pDevice,Texture* src,int bw,int bh,int ow,int oh,bool chroma):
psImg2ToFloat4(pDevice,SRC_SHADER,"Img2ToFloat4",D3DXGetPixelShaderProfile(pDevice),
	VecToMacroArray((D3DXVECTOR2(0,(bh-oh)/(float)src->GetHeight())),"OFFSET",(src->GetType()->real_elem_size==2?SetMacroArray("A8L8",chroma?SetMacroArray("CHROMA",CreateMacroArray(3),2):CreateMacroArray(2),1):chroma?SetMacroArray("CHROMA",CreateMacroArray(2),1):CreateMacroArray(1)))),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor")),
oquad(NEW OQuad(pDevice))
{
	oquad->CreateVB(src->GetWidth(),src->GetHeight(),bw,bh,ow,oh);
}

psImg2ToFloat4oSP::~psImg2ToFloat4oSP()
{
	delete oquad;
}

HRESULT psImg2ToFloat4oSP::Apply(Texture* src,Texture *factor, TextureRT *dst1,TextureRT *dst2)
{
	HRESULT hr;
	PROFILE_BLOCK
	oquad->SetActive();
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(1)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=factor->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oquad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->SetRenderTarget(1,0);
	return hr;
}

//***
psImg2ToFloat4oMP::psImg2ToFloat4oMP(LPDIRECT3DDEVICE9 pDevice,Texture* src,int bw,int bh,int ow,int oh,bool chroma):
psImg2ToFloat4(pDevice,SRC_SHADER,"Img2ToFloat4MP",D3DXGetPixelShaderProfile(pDevice),
	VecToMacroArray((D3DXVECTOR2(0,(bh/2)/(float)src->GetHeight())),"OFFSET",SetMacroArray("PASS1",(src->GetType()->real_elem_size==2?SetMacroArray("A8L8",chroma?SetMacroArray("CHROMA",CreateMacroArray(4),3):CreateMacroArray(3),2):chroma?SetMacroArray("CHROMA",CreateMacroArray(3),2):CreateMacroArray(2)),1))),
pass2(pDevice,SRC_SHADER,"Img2ToFloat4MP",D3DXGetPixelShaderProfile(pDevice),
	  VecToMacroArray((D3DXVECTOR2(0,(bh-oh)/(float)src->GetHeight())),"OFFSET",(src->GetType()->real_elem_size==2?SetMacroArray("A8L8",chroma?SetMacroArray("CHROMA",CreateMacroArray(3),2):CreateMacroArray(2),1):chroma?SetMacroArray("CHROMA",CreateMacroArray(2),1):CreateMacroArray(1)))),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor")),
oquad(NEW OQuad(pDevice))
{
	oquad->CreateVB(src->GetWidth(),src->GetHeight(),bw,bh,ow,oh);
}

psImg2ToFloat4oMP::~psImg2ToFloat4oMP()
{
	delete oquad;
}

HRESULT psImg2ToFloat4oMP::Apply(Texture* src,Texture *factor, TextureRT *dst1,TextureRT *dst2)
{
	HRESULT hr;
	PROFILE_BLOCK
	oquad->SetActive();
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 

	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=factor->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oquad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=pass2.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oquad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	return hr;
}

//****
psImg2ToFloat4oInterlacedSP::psImg2ToFloat4oInterlacedSP(LPDIRECT3DDEVICE9 pDevice,Texture* src,TextureRT* dst,int bw,int bh,int ow,int oh,bool chroma):
psImg2ToFloat4(pDevice,SRC_SHADER,"Img2ToFloat4",D3DXGetPixelShaderProfile(pDevice),
	VecToMacroArray((D3DXVECTOR2(0,2*(bh-oh)/(float)src->GetHeight())),"OFFSET",
	(src->GetType()->real_elem_size==2?
		SetMacroArray("A8L8",chroma?
			SetMacroArray("CHROMA",CreateMacroArray(3),2):
			CreateMacroArray(2),1):
		chroma?SetMacroArray("CHROMA",CreateMacroArray(2),1):
	CreateMacroArray(1)))),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor")),
oiquad(NEW OIQuad(pDevice))
{
	oiquad->CreateVB(src->GetWidth(),src->GetHeight(),bw,bh,ow,oh);

	HRESULT hr;
	int dstwidth=dst->GetWidth();
	int dstheight=dst->GetHeight();
	int bottomheight=bh-oh;
	

	int ny=((src->GetHeight()+oh-1)/(bh-oh)+1+1)/2;
	bottomheight=ny/2*(bh-oh)-src->GetHeight()/2;
	temptex=NEW TextureRT(dst,hr);

	r[0].top=0;r[0].bottom=oh;r[0].left=0;r[0].right=dstwidth;
	r[1].top=0;r[1].bottom=oh;r[1].left=0;r[1].right=dstwidth;
	r[2].top=dstheight/2;r[2].bottom=dstheight/2+oh;r[2].left=0;r[2].right=dstwidth;
	r[3].top=oh;r[3].bottom=2*oh;r[3].left=0;r[3].right=dstwidth;

	r[4].top=dstheight/2-bottomheight;r[4].bottom=dstheight/2;r[4].left=0;r[4].right=dstwidth;
	r[5].top=0;r[5].bottom=bottomheight;r[5].left=0;r[5].right=dstwidth;
	r[6].top=dstheight-bottomheight;r[6].bottom=dstheight;r[6].left=0;r[6].right=dstwidth;
	r[7].top=bottomheight;r[7].bottom=bottomheight*2;r[7].left=0;r[7].right=dstwidth;
	
	bottomheight+=oh;
	r[8].top=dstheight/2-bottomheight;r[8].bottom=dstheight/2;r[8].left=0;r[8].right=dstwidth;
	r[9].top=0;r[9].bottom=bottomheight;r[9].left=0;r[9].right=dstwidth;
	r[10].top=dstheight-bottomheight;r[10].bottom=dstheight;r[10].left=0;r[10].right=dstwidth;
	r[11].top=bottomheight;r[11].bottom=bottomheight*2;r[11].left=0;r[11].right=dstwidth;

}

psImg2ToFloat4oInterlacedSP::~psImg2ToFloat4oInterlacedSP()
{
	delete temptex;
	delete oiquad;
}

HRESULT psImg2ToFloat4oInterlacedSP::Apply(Texture* src,Texture *factor, TextureRT *dst1,TextureRT *dst2)
{
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	/*
	Types ty;
	ty.Managed=dst1->GetType();
	TextureM s1(_pDevice,src->GetWidth(),src->GetHeight(),ty,hr);
	float* f1=NEW float[s1.Size()];
	for(int y=0;y<s1.GetHeight()/2;y++)
	{
		for(int x=0;x<s1.GetWidth();x++)
		{
			f1[x*4+y*s1.GetWidth()*8]=y;
			f1[x*4+y*s1.GetWidth()*8+s1.GetWidth()*4]=-y;
				f1[x*4+y*s1.GetWidth()*8+1]=y;
			f1[x*4+y*s1.GetWidth()*8+s1.GetWidth()*4+1]=-y;
				f1[x*4+y*s1.GetWidth()*8+2]=y;
			f1[x*4+y*s1.GetWidth()*8+s1.GetWidth()*4+2]=-y;
				f1[x*4+y*s1.GetWidth()*8+3]=y;
			f1[x*4+y*s1.GetWidth()*8+s1.GetWidth()*4+3]=-y;
		}
	}
	UploadToTexture(&s1,f1);
	delete f1;*/
	oiquad->SetActive();
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(1)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))//UNDONE bad factor?
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	//s1.SetAsTexture(GetSamplerIndex(sSrc));
	if(FAILED(hr=factor->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oiquad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->SetRenderTarget(1,0);
	//float *f=NEW float[dst1->Size()];
	//DownloadFromTexture(dst1,f);
	
/**/
	_pDevice->StretchRect(dst1->GetSurface(),r+0,temptex->GetSurface(),r+1,D3DTEXF_NONE);
	_pDevice->StretchRect(dst1->GetSurface(),r+2,temptex->GetSurface(),r+3,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+1,dst1->GetSurface(),r+2,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+3,dst1->GetSurface(),r+0,D3DTEXF_NONE);
	
	//DownloadFromTexture(dst1,f);
	_pDevice->StretchRect(dst1->GetSurface(),r+4,temptex->GetSurface(),r+5,D3DTEXF_NONE);
	_pDevice->StretchRect(dst1->GetSurface(),r+6,temptex->GetSurface(),r+7,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+5,dst1->GetSurface(),r+6,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+7,dst1->GetSurface(),r+4,D3DTEXF_NONE);

	//DownloadFromTexture(dst1,f);
	//delete f;
	_pDevice->StretchRect(dst2->GetSurface(),r+8,temptex->GetSurface(),r+9,D3DTEXF_NONE);
	_pDevice->StretchRect(dst2->GetSurface(),r+10,temptex->GetSurface(),r+11,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+9,dst2->GetSurface(),r+10,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+11,dst2->GetSurface(),r+8,D3DTEXF_NONE);
/**/
	return hr;
}
//****
psImg2ToFloat4oInterlacedMP::psImg2ToFloat4oInterlacedMP(LPDIRECT3DDEVICE9 pDevice,Texture* src,TextureRT* dst,int bw,int bh,int ow,int oh,bool chroma):
psImg2ToFloat4(pDevice,SRC_SHADER,"Img2ToFloat4MP",D3DXGetPixelShaderProfile(pDevice),
	VecToMacroArray((D3DXVECTOR2(0,2*(bh-oh)/(float)src->GetHeight())),"OFFSET",
		SetMacroArray("PASS1",
		(src->GetType()->real_elem_size==2?
			SetMacroArray("A8L8",chroma?
				SetMacroArray("CHROMA",CreateMacroArray(4),3):
				CreateMacroArray(3),2):
			chroma?SetMacroArray("CHROMA",CreateMacroArray(3),2):
		CreateMacroArray(2))),1)),
pass2(pDevice,SRC_SHADER,"Img2ToFloat4MP",D3DXGetPixelShaderProfile(pDevice),
	VecToMacroArray((D3DXVECTOR2(0,2*(bh-oh)/(float)src->GetHeight())),"OFFSET",
	(src->GetType()->real_elem_size==2?
		SetMacroArray("A8L8",chroma?
			SetMacroArray("CHROMA",CreateMacroArray(3),2):
			CreateMacroArray(2),1):
		chroma?SetMacroArray("CHROMA",CreateMacroArray(2),1):
	CreateMacroArray(1)))),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor")),
oiquad(NEW OIQuad(pDevice))
{
	oiquad->CreateVB(src->GetWidth(),src->GetHeight(),bw,bh,ow,oh);

	HRESULT hr;
	int dstwidth=dst->GetWidth();
	int dstheight=dst->GetHeight();
	int bottomheight=bh-oh;
	

	int ny=((src->GetHeight()+oh-1)/(bh-oh)+1+1)/2;
	bottomheight=ny/2*(bh-oh)-src->GetHeight()/2;
	temptex=NEW TextureRT(dst,hr);

	r[0].top=0;r[0].bottom=oh;r[0].left=0;r[0].right=dstwidth;
	r[1].top=0;r[1].bottom=oh;r[1].left=0;r[1].right=dstwidth;
	r[2].top=dstheight/2;r[2].bottom=dstheight/2+oh;r[2].left=0;r[2].right=dstwidth;
	r[3].top=oh;r[3].bottom=2*oh;r[3].left=0;r[3].right=dstwidth;

	r[4].top=dstheight/2-bottomheight;r[4].bottom=dstheight/2;r[4].left=0;r[4].right=dstwidth;
	r[5].top=0;r[5].bottom=bottomheight;r[5].left=0;r[5].right=dstwidth;
	r[6].top=dstheight-bottomheight;r[6].bottom=dstheight;r[6].left=0;r[6].right=dstwidth;
	r[7].top=bottomheight;r[7].bottom=bottomheight*2;r[7].left=0;r[7].right=dstwidth;
	
	bottomheight+=oh;
	r[8].top=dstheight/2-bottomheight;r[8].bottom=dstheight/2;r[8].left=0;r[8].right=dstwidth;
	r[9].top=0;r[9].bottom=bottomheight;r[9].left=0;r[9].right=dstwidth;
	r[10].top=dstheight-bottomheight;r[10].bottom=dstheight;r[10].left=0;r[10].right=dstwidth;
	r[11].top=bottomheight;r[11].bottom=bottomheight*2;r[11].left=0;r[11].right=dstwidth;

}

psImg2ToFloat4oInterlacedMP::~psImg2ToFloat4oInterlacedMP()
{
	delete temptex;
	delete oiquad;
}

HRESULT psImg2ToFloat4oInterlacedMP::Apply(Texture* src,Texture *factor, TextureRT *dst1,TextureRT *dst2)
{
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	/*
	Types ty;
	ty.Managed=dst1->GetType();
	TextureM s1(_pDevice,src->GetWidth(),src->GetHeight(),ty,hr);
	float* f1=NEW float[s1.Size()];
	for(int y=0;y<s1.GetHeight()/2;y++)
	{
		for(int x=0;x<s1.GetWidth();x++)
		{
			f1[x*4+y*s1.GetWidth()*8]=y;
			f1[x*4+y*s1.GetWidth()*8+s1.GetWidth()*4]=-y;
				f1[x*4+y*s1.GetWidth()*8+1]=y;
			f1[x*4+y*s1.GetWidth()*8+s1.GetWidth()*4+1]=-y;
				f1[x*4+y*s1.GetWidth()*8+2]=y;
			f1[x*4+y*s1.GetWidth()*8+s1.GetWidth()*4+2]=-y;
				f1[x*4+y*s1.GetWidth()*8+3]=y;
			f1[x*4+y*s1.GetWidth()*8+s1.GetWidth()*4+3]=-y;
		}
	}
	UploadToTexture(&s1,f1);
	delete f1;*/
	oiquad->SetActive();
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))//UNDONE bad factor?
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	//s1.SetAsTexture(GetSamplerIndex(sSrc));
	if(FAILED(hr=factor->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oiquad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	V_RETURN(dst2->SetAsRenderTarget(0));
	V_RETURN(pass2.SetActive());
	V_RETURN(oiquad->Draw());
		
	//float *f=NEW float[dst1->Size()];
	//DownloadFromTexture(dst1,f);
	
/**/
	_pDevice->StretchRect(dst1->GetSurface(),r+0,temptex->GetSurface(),r+1,D3DTEXF_NONE);
	_pDevice->StretchRect(dst1->GetSurface(),r+2,temptex->GetSurface(),r+3,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+1,dst1->GetSurface(),r+2,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+3,dst1->GetSurface(),r+0,D3DTEXF_NONE);
	
	//DownloadFromTexture(dst1,f);
	_pDevice->StretchRect(dst1->GetSurface(),r+4,temptex->GetSurface(),r+5,D3DTEXF_NONE);
	_pDevice->StretchRect(dst1->GetSurface(),r+6,temptex->GetSurface(),r+7,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+5,dst1->GetSurface(),r+6,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+7,dst1->GetSurface(),r+4,D3DTEXF_NONE);

	//DownloadFromTexture(dst1,f);
	//delete f;
	_pDevice->StretchRect(dst2->GetSurface(),r+8,temptex->GetSurface(),r+9,D3DTEXF_NONE);
	_pDevice->StretchRect(dst2->GetSurface(),r+10,temptex->GetSurface(),r+11,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+9,dst2->GetSurface(),r+10,D3DTEXF_NONE);
	hr=_pDevice->StretchRect(temptex->GetSurface(),r+11,dst2->GetSurface(),r+8,D3DTEXF_NONE);
/**/
	return hr;
}
//********
psImg2ToFloat4ohalfSP::psImg2ToFloat4ohalfSP(LPDIRECT3DDEVICE9 pDevice,Texture* src,int bw,int bh,bool chroma):
psImg2ToFloat4(pDevice,SRC_SHADER,"Img2ToFloat4",D3DXGetPixelShaderProfile(pDevice),
	VecToMacroArray((D3DXVECTOR2(0,(bh/2)/(float)src->GetHeight())),"OFFSET",(src->GetType()->real_elem_size==2?SetMacroArray("A8L8",chroma?SetMacroArray("CHROMA",CreateMacroArray(3),2):CreateMacroArray(2),1):chroma?SetMacroArray("CHROMA",CreateMacroArray(2),1):CreateMacroArray(1)))),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	int dstwidth=((int)(src->GetWidth()+bw*1.5-1))/bw*bw;
	int dstheight=((int)(src->GetHeight()+bh*1.5-1))/bh*bh;
	RECT rec[4];
	rec[0].left=0;
	rec[0].right=dstwidth;
	rec[0].top=0;
	rec[0].bottom=dstheight;
	rec[1].left=-bw/2;//xy
	rec[1].right=src->GetWidth()-bw/2;
	rec[1].top=-bh/2;
	rec[1].bottom=src->GetHeight()-bh/2;
	rec[2].left=0;//zw
	rec[2].top=-bh/2;
	rec[2].right=src->GetWidth();
	rec[2].bottom=src->GetHeight()-bh/2;
	rec[3].left=0;	//factor
	rec[3].right=bw;
	rec[3].top=0;
	rec[3].bottom=bh;

	quad=NEW NQuad(pDevice,rec,3,1,false);
}

HRESULT psImg2ToFloat4ohalfSP::Apply(Texture* src,Texture* factor,TextureRT *dst1,TextureRT *dst2)
{
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	quad->SetActive();
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(1)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
	if(FAILED(hr=factor->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSU,D3DTADDRESS_MIRROR );
	_pDevice->SetRenderTarget(1,0);
	return hr;
}
//****
psImg2ToFloat4ohalfInterlacedSP::psImg2ToFloat4ohalfInterlacedSP(LPDIRECT3DDEVICE9 pDevice,Texture* src,TextureRT* dst,int bw,int bh,bool chroma):
psImg2ToFloat4(pDevice,SRC_SHADER,"Img2ToFloat4",D3DXGetPixelShaderProfile(pDevice),
	VecToMacroArray((D3DXVECTOR2(0,(bh)/(float)src->GetHeight())),"OFFSET",(src->GetType()->real_elem_size==2?SetMacroArray("A8L8",chroma?SetMacroArray("CHROMA",CreateMacroArray(3),2):CreateMacroArray(2),1):chroma?SetMacroArray("CHROMA",CreateMacroArray(2),1):CreateMacroArray(1)))),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	int dstwidth=dst->GetWidth();
	int dstheight=dst->GetHeight();
	RECT rec[4];
	rec[0].left=0;
	rec[0].right=dstwidth;
	rec[0].top=0;
	rec[0].bottom=dstheight;
	rec[1].left=-bw/2;//xy
	rec[1].right=src->GetWidth()-bw/2;
	rec[1].top=-bh;
	rec[1].bottom=src->GetHeight()-bh;
	rec[2].left=0;//zw
	rec[2].top=-bh;
	rec[2].right=src->GetWidth();
	rec[2].bottom=src->GetHeight()-bh;
	rec[3].left=0;	//factor
	rec[3].right=bw;
	rec[3].top=0;
	rec[3].bottom=bh*2;

	RECT qrec={0,0,dstwidth,dstheight/2};
	quad=NEW NQuad(pDevice,rec,3,1,qrec,false);
	rec[1].top+=1;
	rec[1].bottom+=1;
	rec[2].top+=1;
	rec[2].bottom+=1;
	qrec.top+=dstheight/2;
	qrec.bottom+=dstheight/2;
	bottomquad=NEW NQuad(pDevice,rec,3,1,qrec,false);
	HRESULT hr;
	int bottomheight=(dstheight-bh-src->GetHeight())/2;
	temptex=NEW TextureRT(dst,hr);

	r[0].top=0;r[0].bottom=bh/2;r[0].left=0;r[0].right=dstwidth;
	r[1].top=0;r[1].bottom=bh/2;r[1].left=0;r[1].right=dstwidth;
	r[2].top=dstheight/2;r[2].bottom=dstheight/2+bh/2;r[2].left=0;r[2].right=dstwidth;
	r[3].top=bh/2;r[3].bottom=bh;r[3].left=0;r[3].right=dstwidth;

	r[4].top=dstheight/2-bottomheight;r[4].bottom=dstheight/2;r[4].left=0;r[4].right=dstwidth;
	r[5].top=0;r[5].bottom=bottomheight;r[5].left=0;r[5].right=dstwidth;
	r[6].top=dstheight-bottomheight;r[6].bottom=dstheight;r[6].left=0;r[6].right=dstwidth;
	r[7].top=bottomheight;r[7].bottom=bottomheight*2;r[7].left=0;r[7].right=dstwidth;
	
	bottomheight+=bh/2;
	r[8].top=dstheight/2-bottomheight;r[8].bottom=dstheight/2;r[8].left=0;r[8].right=dstwidth;
	r[9].top=0;r[9].bottom=bottomheight;r[9].left=0;r[9].right=dstwidth;
	r[10].top=dstheight-bottomheight;r[10].bottom=dstheight;r[10].left=0;r[10].right=dstwidth;
	r[11].top=bottomheight;r[11].bottom=bottomheight*2;r[11].left=0;r[11].right=dstwidth;
	
}

psImg2ToFloat4ohalfInterlacedSP::~psImg2ToFloat4ohalfInterlacedSP()
{
	delete bottomquad;
	delete temptex;
}

HRESULT psImg2ToFloat4ohalfInterlacedSP::Apply(Texture* src,Texture* factor,TextureRT *dst1,TextureRT *dst2)
{
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	V_RETURN(quad->SetActive());
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(1)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	

	_pDevice->Clear(0,0,D3DCLEAR_TARGET,0x00880000,0,0);
	
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
	if(FAILED(hr=factor->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	
	if(FAILED(bottomquad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 


	_pDevice->StretchRect(dst1->GetSurface(),r+0,temptex->GetSurface(),r+1,D3DTEXF_NONE);
	_pDevice->StretchRect(dst1->GetSurface(),r+2,temptex->GetSurface(),r+3,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+1,dst1->GetSurface(),r+2,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+3,dst1->GetSurface(),r+0,D3DTEXF_NONE);

	_pDevice->StretchRect(dst1->GetSurface(),r+4,temptex->GetSurface(),r+5,D3DTEXF_NONE);
	_pDevice->StretchRect(dst1->GetSurface(),r+6,temptex->GetSurface(),r+7,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+5,dst1->GetSurface(),r+6,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+7,dst1->GetSurface(),r+4,D3DTEXF_NONE);

	_pDevice->StretchRect(dst2->GetSurface(),r+8,temptex->GetSurface(),r+9,D3DTEXF_NONE);
	_pDevice->StretchRect(dst2->GetSurface(),r+10,temptex->GetSurface(),r+11,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+9,dst2->GetSurface(),r+10,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+11,dst2->GetSurface(),r+8,D3DTEXF_NONE);

	_pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSU,D3DTADDRESS_MIRROR );
	_pDevice->SetRenderTarget(1,0);

	return hr;
}
                  
//********
psImg2ToFloat4ohalfMP::psImg2ToFloat4ohalfMP(LPDIRECT3DDEVICE9 pDevice,Texture* src,int bw,int bh,bool chroma):
psImg2ToFloat4(pDevice,SRC_SHADER,"Img2ToFloat4MP",D3DXGetPixelShaderProfile(pDevice),
	VecToMacroArray((D3DXVECTOR2(0,(bh/2)/(float)src->GetHeight())),"OFFSET",SetMacroArray("PASS1",(src->GetType()->real_elem_size==2?SetMacroArray("A8L8",chroma?SetMacroArray("CHROMA",CreateMacroArray(4),3):CreateMacroArray(3),2):chroma?SetMacroArray("CHROMA",CreateMacroArray(3),2):CreateMacroArray(2)),1))),
pass2(pDevice,SRC_SHADER,"Img2ToFloat4MP",D3DXGetPixelShaderProfile(pDevice),
	  VecToMacroArray((D3DXVECTOR2(0,(bh/2)/(float)src->GetHeight())),"OFFSET",(src->GetType()->real_elem_size==2?SetMacroArray("A8L8",chroma?SetMacroArray("CHROMA",CreateMacroArray(3),2):CreateMacroArray(2),1):chroma?SetMacroArray("CHROMA",CreateMacroArray(2),1):CreateMacroArray(1)))),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	int dstwidth=((int)(src->GetWidth()+bw*1.5-1))/bw*bw;
	int dstheight=((int)(src->GetHeight()+bh*1.5-1))/bh*bh;
	RECT rec[4];
	rec[0].left=0;
	rec[0].right=dstwidth;
	rec[0].top=0;
	rec[0].bottom=dstheight;
	rec[1].left=-bw/2;//xy
	rec[1].right=src->GetWidth()-bw/2;
	rec[1].top=-bh/2;
	rec[1].bottom=src->GetHeight()-bh/2;
	rec[2].left=0;//zw
	rec[2].top=-bh/2;
	rec[2].right=src->GetWidth();
	rec[2].bottom=src->GetHeight()-bh/2;
	rec[3].left=0;	//factor
	rec[3].right=dstwidth;
	rec[3].top=0;
	rec[3].bottom=dstheight;

	quad=NEW NQuad(pDevice,rec,3,1,false);
}

HRESULT psImg2ToFloat4ohalfMP::Apply(Texture* src,Texture* factor,TextureRT *dst1,TextureRT *dst2)
{
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	quad->SetActive();
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
	if(FAILED(hr=factor->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=pass2.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSU,D3DTADDRESS_MIRROR );
	return hr;
}

//****
psImg2ToFloat4ohalfInterlacedMP::psImg2ToFloat4ohalfInterlacedMP(LPDIRECT3DDEVICE9 pDevice,Texture* src,TextureRT* dst,int bw,int bh,bool chroma):
psImg2ToFloat4(pDevice,SRC_SHADER,"Img2ToFloat4MP",D3DXGetPixelShaderProfile(pDevice),
	VecToMacroArray((D3DXVECTOR2(0,(bh)/(float)src->GetHeight())),"OFFSET",SetMacroArray("PASS1",(src->GetType()->real_elem_size==2?SetMacroArray("A8L8",chroma?SetMacroArray("CHROMA",CreateMacroArray(4),3):CreateMacroArray(3),2):chroma?SetMacroArray("CHROMA",CreateMacroArray(3),2):CreateMacroArray(2)),1))),
pass2(pDevice,SRC_SHADER,"Img2ToFloat4MP",D3DXGetPixelShaderProfile(pDevice),
	  VecToMacroArray((D3DXVECTOR2(0,(bh)/(float)src->GetHeight())),"OFFSET",(src->GetType()->real_elem_size==2?SetMacroArray("A8L8",chroma?SetMacroArray("CHROMA",CreateMacroArray(3),2):CreateMacroArray(2),1):chroma?SetMacroArray("CHROMA",CreateMacroArray(2),1):CreateMacroArray(1)))),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	int dstwidth=dst->GetWidth();
	int dstheight=dst->GetHeight();
	RECT rec[4];
	rec[0].left=0;
	rec[0].right=dstwidth;
	rec[0].top=0;
	rec[0].bottom=dstheight;
	rec[1].left=-bw/2;//xy
	rec[1].right=src->GetWidth()-bw/2;
	rec[1].top=-bh;
	rec[1].bottom=src->GetHeight()-bh;
	rec[2].left=0;//zw
	rec[2].top=-bh;
	rec[2].right=src->GetWidth();
	rec[2].bottom=src->GetHeight()-bh;
	rec[3].left=0;	//factor
	rec[3].right=dstwidth;
	rec[3].top=0;
	rec[3].bottom=dstheight*2;//?

	RECT qrec={0,0,dstwidth,dstheight/2};
	quad=NEW NQuad(pDevice,rec,3,1,qrec,false);
	rec[1].top+=1;
	rec[1].bottom+=1;
	rec[2].top+=1;
	rec[2].bottom+=1;
	qrec.top+=dstheight/2;
	qrec.bottom+=dstheight/2;
	bottomquad=NEW NQuad(pDevice,rec,3,1,qrec,false);
	HRESULT hr;
	int bottomheight=(dstheight-bh-src->GetHeight())/2;
	temptex=NEW TextureRT(dst,hr);

	r[0].top=0;r[0].bottom=bh/2;r[0].left=0;r[0].right=dstwidth;
	r[1].top=0;r[1].bottom=bh/2;r[1].left=0;r[1].right=dstwidth;
	r[2].top=dstheight/2;r[2].bottom=dstheight/2+bh/2;r[2].left=0;r[2].right=dstwidth;
	r[3].top=bh/2;r[3].bottom=bh;r[3].left=0;r[3].right=dstwidth;

	r[4].top=dstheight/2-bottomheight;r[4].bottom=dstheight/2;r[4].left=0;r[4].right=dstwidth;
	r[5].top=0;r[5].bottom=bottomheight;r[5].left=0;r[5].right=dstwidth;
	r[6].top=dstheight-bottomheight;r[6].bottom=dstheight;r[6].left=0;r[6].right=dstwidth;
	r[7].top=bottomheight;r[7].bottom=bottomheight*2;r[7].left=0;r[7].right=dstwidth;
	
	bottomheight+=bh/2;
	r[8].top=dstheight/2-bottomheight;r[8].bottom=dstheight/2;r[8].left=0;r[8].right=dstwidth;
	r[9].top=0;r[9].bottom=bottomheight;r[9].left=0;r[9].right=dstwidth;
	r[10].top=dstheight-bottomheight;r[10].bottom=dstheight;r[10].left=0;r[10].right=dstwidth;
	r[11].top=bottomheight;r[11].bottom=bottomheight*2;r[11].left=0;r[11].right=dstwidth;
	
}

psImg2ToFloat4ohalfInterlacedMP::~psImg2ToFloat4ohalfInterlacedMP()
{
	delete bottomquad;
	delete temptex;
}

HRESULT psImg2ToFloat4ohalfInterlacedMP::Apply(Texture* src,Texture* factor,TextureRT *dst1,TextureRT *dst2)
{
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	quad->SetActive();
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 

	_pDevice->Clear(0,0,D3DCLEAR_TARGET,0x00880000,0,0);
	
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
	if(FAILED(hr=factor->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(bottomquad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=pass2.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(bottomquad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 

	_pDevice->StretchRect(dst1->GetSurface(),r+0,temptex->GetSurface(),r+1,D3DTEXF_NONE);
	_pDevice->StretchRect(dst1->GetSurface(),r+2,temptex->GetSurface(),r+3,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+1,dst1->GetSurface(),r+2,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+3,dst1->GetSurface(),r+0,D3DTEXF_NONE);

	_pDevice->StretchRect(dst1->GetSurface(),r+4,temptex->GetSurface(),r+5,D3DTEXF_NONE);
	_pDevice->StretchRect(dst1->GetSurface(),r+6,temptex->GetSurface(),r+7,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+5,dst1->GetSurface(),r+6,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+7,dst1->GetSurface(),r+4,D3DTEXF_NONE);

	_pDevice->StretchRect(dst2->GetSurface(),r+8,temptex->GetSurface(),r+9,D3DTEXF_NONE);
	_pDevice->StretchRect(dst2->GetSurface(),r+10,temptex->GetSurface(),r+11,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+9,dst2->GetSurface(),r+10,D3DTEXF_NONE);
	_pDevice->StretchRect(temptex->GetSurface(),r+11,dst2->GetSurface(),r+8,D3DTEXF_NONE);

	_pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSU,D3DTADDRESS_MIRROR );
	return hr;
}
//************



psFloat4ToImg2o::psFloat4ToImg2o(LPDIRECT3DDEVICE9 pDevice,int width,int height,int bw,int bh,int ow,int oh,int srcwidth,int srcheight,bool chroma):
psFloat4ToImg2(pDevice,SRC_SHADER,"Float4ToImg2Corner",D3DXGetPixelShaderProfile(pDevice),chroma?SetMacroArray("CHROMA",CreateMacroArray(1),0):0),
sSrc1(_pConstantTable->GetConstantByName(NULL,"Src")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"Src2")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor")),
nooverlapxy(pDevice,SRC_SHADER,"Float4ToImg2Nooverlap",D3DXGetPixelShaderProfile(pDevice),
			SetMacroArray("XY",chroma?SetMacroArray("CHROMA",CreateMacroArray(2),1):CreateMacroArray(1),0)),
nooverlapzw(pDevice,SRC_SHADER,"Float4ToImg2Nooverlap",D3DXGetPixelShaderProfile(pDevice),chroma?SetMacroArray("CHROMA",CreateMacroArray(1),0):0),
horizontalxy(pDevice,SRC_SHADER,"Float4ToImg2Horizontal",D3DXGetPixelShaderProfile(pDevice),
			 SetMacroArray("XY",chroma?SetMacroArray("CHROMA",CreateMacroArray(2),1):CreateMacroArray(1),0)),
horizontalzw(pDevice,SRC_SHADER,"Float4ToImg2Horizontal",D3DXGetPixelShaderProfile(pDevice),chroma?SetMacroArray("CHROMA",CreateMacroArray(1),0):0),
vertical(pDevice,SRC_SHADER,"Float4ToImg2Vertical",D3DXGetPixelShaderProfile(pDevice),chroma?SetMacroArray("CHROMA",CreateMacroArray(1),0):0),
oquad(NEW OQuad(pDevice))
{
	oquad->CreateVBi(width,height,bw,bh,ow,oh,srcwidth,srcheight);
}

HRESULT psFloat4ToImg2o::Apply(Texture* src1,Texture* src2,Texture *factor, TextureRT *dst)
{
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	oquad->SetActive();
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->Clear(0,0,D3DCLEAR_TARGET,0xFFFFFFFF,0,0);
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=factor->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oquad->DrawCorners())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); //1+2:ok
	if(FAILED(hr=horizontalxy.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(oquad->DrawHorizontalxy())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); //1+2:-
	if(FAILED(hr=horizontalzw.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(oquad->DrawHorizontalzw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); //1+2:-
	if(FAILED(hr=nooverlapxy.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oquad->DrawNonoverlaptex1xy())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);//1:-
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oquad->DrawNonoverlaptex2xy())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);//2:ok
	if(FAILED(hr=nooverlapzw.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oquad->DrawNonoverlaptex2zw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);//2:ok
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(oquad->DrawNonoverlaptex1zw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);//1:ok
	if(FAILED(hr=vertical.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oquad->DrawVerticaltex1())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);//1:-
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
 	if(FAILED(oquad->DrawVerticaltex2())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);//2:ok
	return hr;
}


psFloat4ToImg2o::~psFloat4ToImg2o()
{
	delete oquad;
}
//*******
psFloat4ToImg2oInterlaced::psFloat4ToImg2oInterlaced(LPDIRECT3DDEVICE9 pDevice,Texture* src,TextureRT* dst,int bw,int bh,GPUTYPES* _gtype,int ow,int oh,bool chroma):
psFloat4ToImg2(pDevice,SRC_SHADER,"Float4ToImg2Corner",D3DXGetPixelShaderProfile(pDevice),
			chroma?SetMacroArray("CHROMA",CreateMacroArray(1),0):0),
sSrc1(_pConstantTable->GetConstantByName(NULL,"Src")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"Src2")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor")),
nooverlapxy(pDevice,SRC_SHADER,"Float4ToImg2Nooverlap",D3DXGetPixelShaderProfile(pDevice),
			SetMacroArray("XY",chroma?SetMacroArray("CHROMA",CreateMacroArray(2),1):CreateMacroArray(1),0)),
nooverlapzw(pDevice,SRC_SHADER,"Float4ToImg2Nooverlap",D3DXGetPixelShaderProfile(pDevice),
			chroma?SetMacroArray("CHROMA",CreateMacroArray(1),0):0),
horizontalxy(pDevice,SRC_SHADER,"Float4ToImg2Horizontal",D3DXGetPixelShaderProfile(pDevice),
			 SetMacroArray("XY",chroma?SetMacroArray("CHROMA",CreateMacroArray(2),1):CreateMacroArray(1),0)),
horizontalzw(pDevice,SRC_SHADER,"Float4ToImg2Horizontal",D3DXGetPixelShaderProfile(pDevice),
			 chroma?SetMacroArray("CHROMA",CreateMacroArray(1),0):0),
vertical(pDevice,SRC_SHADER,"Float4ToImg2Vertical",D3DXGetPixelShaderProfile(pDevice),
		 chroma?SetMacroArray("CHROMA",CreateMacroArray(1),0):0),
oevenquad(NEW OIQuad(pDevice)),
ooddquad(NEW OIQuad(pDevice))
{
	int width=dst->GetWidth();
	int height=dst->GetHeight();
	int srcwidth=src->GetWidth();
	int srcheight=src->GetHeight();

	HRESULT hr;
	pDevice->CreateDepthStencilSurface(width,height,D3DFMT_D24X8,D3DMULTISAMPLE_NONE,0,false,&zbuffer,NULL);
	ztex=NEW TextureRT(pDevice,width,height,_gtype->FIXED(),hr);
	UCHAR *t=NEW UCHAR[width*height];
	for(int i=0;i<height;i++)
		memset(t+i*width,(i&1)*255,width);
	UploadToTexture(ztex,t);
		delete t;
	
	//Setup z buffer
	RECT r={0,0,width,height};
	NLQuad q(pDevice,&r,1,0);
	Pixelshader p(pDevice,SRC_SHADER,"CopyTexToZBuffer",D3DXGetPixelShaderProfile(pDevice));
	pDevice->BeginScene();
	q.SetActive();
	p.SetActive();
	dst->SetAsRenderTarget(0);
	ztex->SetAsTexture(0);
	pDevice->SetRenderState(D3DRS_ZENABLE ,D3DZB_TRUE );
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE ,TRUE);
	pDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);
	pDevice->SetDepthStencilSurface(zbuffer);
	q.Draw();
	pDevice->EndScene();
	pDevice->SetDepthStencilSurface(0);
	pDevice->SetRenderState(D3DRS_ZENABLE ,D3DZB_FALSE );
	pDevice->SetTexture(0,0);

	oevenquad->CreateVBi(width,height,bw,bh,ow,oh,srcwidth,srcheight,true);
	ooddquad->CreateVBi(width,height,bw,bh,ow,oh,srcwidth,srcheight,false);
	psList.push_back(this);

}

HRESULT psFloat4ToImg2oInterlaced::ResetShader(LPDIRECT3DDEVICE9 pDevice,bool firstpass)
{
	if(pDevice!=_pDevice)
		return 2;
	if(firstpass)
	{
		if(zbuffer){
			zbuffer->Release();
			zbuffer=0;
			return 1;
			//_pDevice->Release();
		}
	}
	else
	{
		//Setup z buffer
		HRESULT hr;
				int width=ztex->GetWidth();
		int height=ztex->GetHeight();
			UCHAR *t=NEW UCHAR[width*height];
	for( int i=0;i<height;i++)
		memset(t+i*width,(i&1)*255,width);
	UploadToTexture(ztex,t);
		delete t;
		TextureRT dst(ztex,hr);

		pDevice->CreateDepthStencilSurface(width,height,D3DFMT_D24X8,D3DMULTISAMPLE_NONE,0,false,&zbuffer,NULL);
		RECT r={0,0,width,height};
		NLQuad q(pDevice,&r,1,0);
		Pixelshader p(pDevice,SRC_SHADER,"CopyTexToZBuffer",D3DXGetPixelShaderProfile(pDevice));
		pDevice->BeginScene();
		q.SetActive();
		p.SetActive();
		dst.SetAsRenderTarget(0);
		ztex->SetAsTexture(0);
		pDevice->SetRenderState(D3DRS_ZENABLE ,D3DZB_TRUE );
		pDevice->SetRenderState(D3DRS_ZWRITEENABLE ,TRUE);
		pDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);
		pDevice->SetDepthStencilSurface(zbuffer);
		q.Draw();
		pDevice->EndScene();
		pDevice->SetDepthStencilSurface(0);
		pDevice->SetRenderState(D3DRS_ZENABLE ,D3DZB_FALSE );
		return pDevice->SetTexture(0,0);
	}

	return 1;
}

HRESULT psFloat4ToImg2oInterlaced::Apply(Texture* src1,Texture* src2,Texture *factor, TextureRT *dst)
{
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	oevenquad->SetActive();
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	/*Types ft;
	ft.RenderTarget=(src1->GetType());
	TextureRT f=TextureRT(_pDevice,dst->GetWidth(),dst->GetHeight(),ft,hr);
	
	f.SetAsRenderTarget();
	float* fd=NEW float[f.Size()];*/
	_pDevice->Clear(0,0,D3DCLEAR_TARGET,0x00880000,0,0);
	_pDevice->SetRenderState(D3DRS_ZENABLE ,D3DZB_TRUE );
	_pDevice->SetRenderState(D3DRS_ZWRITEENABLE ,FALSE);
	_pDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESS);
	_pDevice->SetDepthStencilSurface(zbuffer);
//Err
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=factor->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oevenquad->DrawCorners())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); //Corners
	if(FAILED(hr=horizontalxy.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(oevenquad->DrawHorizontalxy())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); //Horizontal block xy
	if(FAILED(hr=horizontalzw.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(oevenquad->DrawHorizontalzw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); //Horizontal zw
	if(FAILED(hr=nooverlapxy.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oevenquad->DrawNonoverlaptex1xy())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oevenquad->DrawNonoverlaptex2xy())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(hr=nooverlapzw.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oevenquad->DrawNonoverlaptex2zw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(oevenquad->DrawNonoverlaptex1zw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(hr=vertical.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(oevenquad->DrawVerticaltex1())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
 	if(FAILED(oevenquad->DrawVerticaltex2())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
//DownloadFromTexture(&f,fd);
	_pDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_GREATER);
	ooddquad->SetActive();
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(ooddquad->DrawCorners())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); //Corners
	if(FAILED(hr=horizontalxy.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(ooddquad->DrawHorizontalxy())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); //Horizontal block xy
	if(FAILED(hr=horizontalzw.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(ooddquad->DrawHorizontalzw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); //Horizontal zw
	if(FAILED(hr=nooverlapxy.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(ooddquad->DrawNonoverlaptex1xy())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(ooddquad->DrawNonoverlaptex2xy())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(hr=nooverlapzw.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(ooddquad->DrawNonoverlaptex2zw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(ooddquad->DrawNonoverlaptex1zw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(hr=vertical.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(ooddquad->DrawVerticaltex1())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
 	if(FAILED(ooddquad->DrawVerticaltex2())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true);
	_pDevice->SetRenderState(D3DRS_ZENABLE ,D3DZB_FALSE );
	_pDevice->SetDepthStencilSurface(0);
	/*DownloadFromTexture(&f,fd);
	delete fd;*/
	return hr;
}


psFloat4ToImg2oInterlaced::~psFloat4ToImg2oInterlaced()
{
	delete oevenquad;
	delete ooddquad;
	zbuffer->Release();
	delete ztex;
	psList.remove(this);
}

//**************

psFloat4ToImg2ohalf::psFloat4ToImg2ohalf(LPDIRECT3DDEVICE9 pDevice,int width,int height,int bw,int bh,int srcwidth,int srcheight,bool chroma,bool fullsizefactor):
psFloat4ToImg2(pDevice,SRC_SHADER,"Float4ToImg2Corner",D3DXGetPixelShaderProfile(pDevice),
			   chroma?SetMacroArray("CHROMA",CreateMacroArray(1),0):0),
sSrc1(_pConstantTable->GetConstantByName(NULL,"Src")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"Src2")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	RECT rec[5];
	rec[0].left=0;//src
	rec[0].right=width;
	rec[0].top=0;
	rec[0].bottom=height;
	rec[1].left=bw/2;//tex0xy
	rec[1].right=srcwidth+bw/2;
	rec[1].top=bh/2;
	rec[1].bottom=srcheight+bh/2;
	rec[2].left=0;//tex1zw
	rec[2].right=srcwidth;
	rec[2].top=0;
	rec[2].bottom=srcheight;
	rec[3].left=bw/2;//factortex0
	rec[3].top=bh/2;
	rec[3].right=(fullsizefactor?srcwidth:bw)+bw/2;
	rec[3].bottom=(fullsizefactor?srcheight:bh)+bh/2;
	rec[4].left=bw/2;//factortex1
	rec[4].top=0;
	rec[4].right=(fullsizefactor?srcwidth:bw)+bw/2;
	rec[4].bottom=(fullsizefactor?srcheight:bh);
	quad=NEW NQuad(pDevice,rec,5,1,false);
}

HRESULT psFloat4ToImg2ohalf::Apply(Texture* src1,Texture* src2,Texture *factor, TextureRT *dst)
{
	PROFILE_BLOCK
	HRESULT hr;
	quad->SetActive();
	if(FAILED(hr=dst->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
	if(FAILED(hr=factor->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSU,D3DTADDRESS_MIRROR );
	return hr;
}

//*****
psFloat4ToImg2ohalfInterlaced::psFloat4ToImg2ohalfInterlaced(LPDIRECT3DDEVICE9 pDevice,TextureRT* src,TextureRT* dst,int bw,int bh,GPUTYPES* _gtype,bool chroma,bool fullsizefactor):
psFloat4ToImg2(pDevice,SRC_SHADER,"Float4ToImg2Corner",D3DXGetPixelShaderProfile(pDevice),
			   chroma?SetMacroArray("CHROMA",CreateMacroArray(1),0):0),
sSrc1(_pConstantTable->GetConstantByName(NULL,"Src")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"Src2")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	PROFILE_BLOCK
	HRESULT hr;
	int srcwidth=src->GetWidth();
	int srcheight=src->GetHeight();
	int width=dst->GetWidth();
	int height=dst->GetHeight();
	
	pDevice->CreateDepthStencilSurface(width,height,D3DFMT_D24X8,D3DMULTISAMPLE_NONE,0,false,&zbuffer,NULL);
	ztex=NEW TextureRT(pDevice,width,height,_gtype->FIXED(),hr);
	UCHAR *t=NEW UCHAR[width*height];
	for( int i=0;i<height;i++)
		memset(t+i*width,(i&1)*255,width);
	UploadToTexture(ztex,t);
		delete t;
	
	//Setup z buffer
	RECT r={0,0,width,height};
	NLQuad q(pDevice,&r,1,0);
	Pixelshader p(pDevice,SRC_SHADER,"CopyTexToZBuffer",D3DXGetPixelShaderProfile(pDevice));
	pDevice->BeginScene();
	q.SetActive();
	p.SetActive();
	dst->SetAsRenderTarget(0);
	ztex->SetAsTexture(0);
	pDevice->SetRenderState(D3DRS_ZENABLE ,D3DZB_TRUE );
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE ,TRUE);
	pDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);
	pDevice->SetDepthStencilSurface(zbuffer);
	q.Draw();
	pDevice->EndScene();
	pDevice->SetDepthStencilSurface(0);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE ,FALSE);
	pDevice->SetRenderState(D3DRS_ZENABLE ,D3DZB_FALSE );
	pDevice->SetTexture(0,0);

	RECT rec[5];
	RECT qrec={0,0,width,height};
	rec[0].left=0;//src
	rec[0].right=width;
	rec[0].top=0;
	rec[0].bottom=height/2;
	rec[1].left=bw/2;//tex0xy
	rec[1].right=srcwidth+bw/2;
	rec[1].top=bh/2;
	rec[1].bottom=srcheight+bh/2;
	rec[2].left=0;//tex1zw
	rec[2].right=srcwidth;
	rec[2].top=0;
	rec[2].bottom=srcheight;
	rec[3].left=bw/2;//factortex0
	rec[3].top=bh/2;
	rec[3].right=(fullsizefactor?srcwidth:bw)+bw/2;
	rec[3].bottom=(fullsizefactor?srcheight:bh)+bh/2;
	rec[4].left=bw/2;//factortex1
	rec[4].top=0;
	rec[4].right=(fullsizefactor?srcwidth:bw)+bw/2;
	rec[4].bottom=(fullsizefactor?srcheight:bh);

	
	quad=NEW NQuad(pDevice,rec,5,1,qrec,false);
//inc to get correct eta
	qrec.top++;
	qrec.bottom++;
	rec[1].top+=srcheight/2;rec[1].bottom+=srcheight/2;
	rec[2].top+=srcheight/2;rec[2].bottom+=srcheight/2;
	/*if(fullsizefactor)
	{
		rec[3].top+=srcheight/2;rec[3].bottom+=srcheight/2;
		rec[4].top+=srcheight/2;rec[4].bottom+=srcheight/2;
	}*/
	evenfieldquad=NEW NQuad(pDevice,rec,5,1,qrec,false);
	psList.push_back(this);
}

psFloat4ToImg2ohalfInterlaced::~psFloat4ToImg2ohalfInterlaced()
{
	delete evenfieldquad;
	zbuffer->Release();
	delete ztex;
	psList.remove(this);
}


HRESULT psFloat4ToImg2ohalfInterlaced::ResetShader(LPDIRECT3DDEVICE9 pDevice,bool firstpass)
{
	if(pDevice!=_pDevice)
		return 2;
	if(firstpass)
	{
		if(zbuffer){
			zbuffer->Release();
			zbuffer=0;
			return 1;
			//_pDevice->Release();
		}
	}
	else
	{
		//Setup z buffer
		HRESULT hr;
				int width=ztex->GetWidth();
		int height=ztex->GetHeight();
			UCHAR *t=NEW UCHAR[width*height];
	for( int i=0;i<height;i++)
		memset(t+i*width,(i&1)*255,width);
	UploadToTexture(ztex,t);
		delete t;
		TextureRT dst(ztex,hr);

		pDevice->CreateDepthStencilSurface(width,height,D3DFMT_D24X8,D3DMULTISAMPLE_NONE,0,false,&zbuffer,NULL);
		RECT r={0,0,width,height};
		NLQuad q(pDevice,&r,1,0);
		Pixelshader p(pDevice,SRC_SHADER,"CopyTexToZBuffer",D3DXGetPixelShaderProfile(pDevice));
		pDevice->BeginScene();
		q.SetActive();
		p.SetActive();
		dst.SetAsRenderTarget(0);
		ztex->SetAsTexture(0);
		pDevice->SetRenderState(D3DRS_ZENABLE ,D3DZB_TRUE );
		pDevice->SetRenderState(D3DRS_ZWRITEENABLE ,TRUE);
		pDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);
		pDevice->SetDepthStencilSurface(zbuffer);
		q.Draw();
		pDevice->EndScene();
		pDevice->SetDepthStencilSurface(0);
		pDevice->SetRenderState(D3DRS_ZWRITEENABLE ,FALSE);
		pDevice->SetRenderState(D3DRS_ZENABLE ,D3DZB_FALSE );
		return pDevice->SetTexture(0,0); 
	}

	return 1;
}

HRESULT psFloat4ToImg2ohalfInterlaced::Apply(Texture* src1,Texture* src2,Texture *factor, TextureRT *dst)
{
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 

	quad->SetActive();
	if(FAILED(hr=dst->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 

	_pDevice->Clear(0,0,D3DCLEAR_TARGET,0x00880000,0,0);
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP );
    if(FAILED(hr=factor->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->SetRenderState(D3DRS_ZENABLE ,D3DZB_TRUE );
	_pDevice->SetRenderState(D3DRS_ZWRITEENABLE ,FALSE);
	_pDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_GREATER);
	_pDevice->SetDepthStencilSurface(zbuffer);
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	_pDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESS);	

	
	evenfieldquad->Draw();


	_pDevice->SetRenderState(D3DRS_ZENABLE ,D3DZB_FALSE );
	_pDevice->SetSamplerState(GetSamplerIndex(sFactor),D3DSAMP_ADDRESSU,D3DTADDRESS_MIRROR );
	_pDevice->SetDepthStencilSurface(0);
	_pDevice->SetTexture(GetSamplerIndex(sSrc2),0);
	_pDevice->SetTexture(GetSamplerIndex(sSrc1),0);

	return hr;
}