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


#include "./core/Debug class.h"
#include <dxerr.h>
#include "FFTps.h"



//********************************************************************************************************
psBitReverseButterFly::psBitReverseButterFly(LPDIRECT3DDEVICE9 pDevice,LPCSTR pSrcFile, LPCSTR pFunctionName,LPCSTR pProfile,D3DXMACRO* defs):
	Pixelshader(pDevice,pSrcFile,pFunctionName,pProfile,defs)
	{}

psBitReverseButterFlyH::psBitReverseButterFlyH(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset):
psBitReverseButterFly(pDevice,SRC_SHADER,"BitReverseButterFlyH",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",CreateMacroArray(1))),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sLoadMap(_pConstantTable->GetConstantByName(NULL,"LoadMap"))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psBitReverseButterFlyH::Apply(Texture* src,Texture *BitReverseLUT,TextureRT* dst1,TextureRT* dst2){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH",true); 
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(1)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH",true); 
	//if(FAILED(hr=BitReverseLUT->SetAsTexture(GetSamplerIndex(sFactor))))
	if(FAILED(hr=BitReverseLUT->SetAsTexture(GetSamplerIndex(sLoadMap))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH",true); 
	if(FAILED(hr=_pDevice->SetRenderTarget(1,NULL)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH",true); 
	return hr;//=_pDevice->EndScene();
}


psBitReverseButterFlyH_MP::psBitReverseButterFlyH_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset):
psBitReverseButterFly(pDevice,SRC_SHADER,"BitReverseButterFlyH",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",SetMacroArray("MULTIPASS",SetMacroArray("PASS1",CreateMacroArray(3)),1),2)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sLoadMap(_pConstantTable->GetConstantByName(NULL,"LoadMap")),
pass2(pDevice,SRC_SHADER,"BitReverseButterFlyH",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",SetMacroArray("MULTIPASS",SetMacroArray("PASS2",CreateMacroArray(3)),1),2))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psBitReverseButterFlyH_MP::Apply(Texture* src,Texture *BitReverseLUT,TextureRT* dst1,TextureRT* dst2){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH_MP",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=dst1->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH_MP",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH_MP",true); 
	if(FAILED(hr=BitReverseLUT->SetAsTexture(GetSamplerIndex(sLoadMap))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH_MP",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH_MP",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH_MP",true); 
	if(FAILED(hr=pass2.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyH_MP",true); 
	return hr;
}

psBitReverseButterFlyV::psBitReverseButterFlyV(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset):
psBitReverseButterFly(pDevice,SRC_SHADER,"BitReverseButterFlyV",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",CreateMacroArray(1))),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sLoadMap(_pConstantTable->GetConstantByName(NULL,"LoadMap"))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psBitReverseButterFlyV::Apply(Texture* src,Texture *BitReverseLUT,TextureRT* dst1,TextureRT* dst2){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV",true); 
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(1)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV",true); 
	//if(FAILED(hr=BitReverseLUT->SetAsTexture(GetSamplerIndex(sFactor))))
	if(FAILED(hr=BitReverseLUT->SetAsTexture(GetSamplerIndex(sLoadMap))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV",true); 
	if(FAILED(hr=_pDevice->SetRenderTarget(1,NULL)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV",true); 
	return hr;//=_pDevice->EndScene();
}

psBitReverseButterFlyV_MP::psBitReverseButterFlyV_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset):
psBitReverseButterFly(pDevice,SRC_SHADER,"BitReverseButterFlyV",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",SetMacroArray("MULTIPASS",SetMacroArray("PASS1",CreateMacroArray(3)),1),2)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sLoadMap(_pConstantTable->GetConstantByName(NULL,"LoadMap")),
pass2(pDevice,SRC_SHADER,"BitReverseButterFlyV",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",SetMacroArray("MULTIPASS",SetMacroArray("PASS2",CreateMacroArray(3)),1),2))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psBitReverseButterFlyV_MP::Apply(Texture* src,Texture *BitReverseLUT,TextureRT* dst1,TextureRT* dst2){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV_MP",true); 
	if(FAILED(hr=quad->SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,__FUNCTION__,true); 
	if(FAILED(hr=dst1->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV_MP",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV_MP",true); 
	if(FAILED(hr=BitReverseLUT->SetAsTexture(GetSamplerIndex(sLoadMap))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV_MP",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV_MP",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV_MP",true); 
	if(FAILED(hr=pass2.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyV_MP",true); 
	return hr;
}

psBitReverseButterFlyBH::psBitReverseButterFlyBH(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float norm,float pixeloffset):
psBitReverseButterFly(pDevice,SRC_SHADER,"BitReverseButterFlyBH",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",FloatToMacroArray(norm,"NORM",CreateMacroArray(2)),1)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sLoadMap(_pConstantTable->GetConstantByName(NULL,"LoadMap"))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psBitReverseButterFlyBH::Apply(Texture* src,Texture* BitReverseLUT,TextureRT* dst1,TextureRT* dst2){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH",true); 
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(1)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH",true); 
	if(FAILED(hr=BitReverseLUT->SetAsTexture(GetSamplerIndex(sLoadMap))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH",true); 
	if(FAILED(hr=_pDevice->SetRenderTarget(1,NULL)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH",true); 
	return hr;//=_pDevice->EndScene();
}

psBitReverseButterFlyBH_MP::psBitReverseButterFlyBH_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float norm,float pixeloffset):
psBitReverseButterFly(pDevice,SRC_SHADER,"BitReverseButterFlyBH",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",SetMacroArray("MULTIPASS",SetMacroArray("PASS1",FloatToMacroArray(norm,"NORM",CreateMacroArray(4)),1),2),3)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sLoadMap(_pConstantTable->GetConstantByName(NULL,"LoadMap")),
pass2(pDevice,SRC_SHADER,"BitReverseButterFlyBH",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",SetMacroArray("MULTIPASS",SetMacroArray("PASS2",FloatToMacroArray(norm,"NORM",CreateMacroArray(4)),1),2),3))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psBitReverseButterFlyBH_MP::Apply(Texture* src,Texture *BitReverseLUT,TextureRT* dst1,TextureRT* dst2){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH_MP",true); 
	if(FAILED(hr=dst1->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH_MP",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH_MP",true); 
	if(FAILED(hr=BitReverseLUT->SetAsTexture(GetSamplerIndex(sLoadMap))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH_MP",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH_MP",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH_MP",true); 
	if(FAILED(hr=pass2.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"BitReverseButterFlyBH_MP",true); 
	return hr;
}

psC2R::psC2R(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget):
Pixelshader(pDevice,SRC_SHADER,"C2R",D3DXGetPixelShaderProfile(pDevice)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	//sSrc=_pConstantTable->GetConstantByName(NULL,"Src2");
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}



HRESULT psC2R::Apply(Texture* src,Texture *C2RLUT,TextureRT *dst){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"C2R",true); 
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"C2R",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"C2R",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"C2R",true); 
	if(FAILED(hr=C2RLUT->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"C2R",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"C2R",true); 
	if(FAILED(quad->Draw()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"C2R",true); 
	return hr;//=_pDevice->EndScene();
}

psR2C::psR2C(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget):
Pixelshader(pDevice,SRC_SHADER,"R2C",D3DXGetPixelShaderProfile(pDevice)),
sSrc(_pConstantTable->GetConstantByName(NULL,"Src")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
//,sLoadMap(_pConstantTable->GetConstantByName(NULL,"LoadMap"))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psR2C::Apply(Texture* src,Texture *R2CLUT,TextureRT *dst){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"R2C",true); 
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"R2C",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"R2C",true); 
	if(FAILED(hr=src->SetAsTexture(GetSamplerIndex(sSrc))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"R2C",true); 
	if(FAILED(hr=R2CLUT->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"R2C",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"R2C",true); 
	if(FAILED(quad->Draw()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"R2C",true); 
	return hr;//=_pDevice->EndScene();
}


psButterflyCollect::psButterflyCollect(LPDIRECT3DDEVICE9 pDevice,LPCSTR pSrcFile, LPCSTR pFunctionName,LPCSTR pProfile,D3DXMACRO* defs):
	Pixelshader(pDevice,pSrcFile,pFunctionName,pProfile,defs)
	{}

psButterflyCollectH::psButterflyCollectH(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset):
psButterflyCollect(pDevice,SRC_SHADER,"ButterflyCollectH",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",CreateMacroArray(1))),
_stage(_pConstantTable->GetConstantByName(NULL,"_stage")),
sSrc1(_pConstantTable->GetConstantByName(NULL,"I")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"J")),
sLoadLUT(_pConstantTable->GetConstantByName(NULL,"LoadMap")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psButterflyCollectH::Apply(Texture* src1,Texture* src2,Texture *TwiddleLUT,Texture *LoadLUT,float stage,TextureRT *dst1,TextureRT *dst2){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH",true); 
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(1)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH",true); 
	if(FAILED(hr=_pConstantTable->SetFloat(_pDevice,_stage,stage)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH",true); 
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH",true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH",true); 
	if(FAILED(hr=LoadLUT->SetAsTexture(GetSamplerIndex(sLoadLUT))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH",true); 
	if(FAILED(hr=TwiddleLUT->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH",true); 
	if(FAILED(hr=_pDevice->SetRenderTarget(1,NULL)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH",true); 
	return hr;//=_pDevice->EndScene();
}

psButterflyCollectH_MP::psButterflyCollectH_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset):
psButterflyCollect(pDevice,SRC_SHADER,"ButterflyCollectH",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",SetMacroArray("MULTIPASS",SetMacroArray("PASS1",CreateMacroArray(3)),1),2)),
_stage(_pConstantTable->GetConstantByName(NULL,"_stage")),
sSrc1(_pConstantTable->GetConstantByName(NULL,"I")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"J")),
sLoadLUT(_pConstantTable->GetConstantByName(NULL,"LoadMap")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor")),
pass2(pDevice,SRC_SHADER,"ButterflyCollectH",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",SetMacroArray("MULTIPASS",SetMacroArray("PASS2",CreateMacroArray(3)),1),2))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psButterflyCollectH_MP::Apply(Texture* src1,Texture* src2,Texture *TwiddleLUT,Texture *LoadLUT,float stage,TextureRT *dst1,TextureRT *dst2){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH_MP",true); 
	if(FAILED(hr=dst1->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH_MP",true); 
	if(FAILED(hr=_pConstantTable->SetFloat(_pDevice,_stage,stage)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH_MP",true); 
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH_MP",true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH_MP",true); 
	if(FAILED(hr=LoadLUT->SetAsTexture(GetSamplerIndex(sLoadLUT))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH_MP",true); 
	if(FAILED(hr=TwiddleLUT->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH_MP",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH_MP",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH_MP",true); 
	if(FAILED(hr=pass2.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectH_MP",true); 
	return hr;
}

psButterflyCollectV::psButterflyCollectV(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset):
psButterflyCollect(pDevice,SRC_SHADER,"ButterflyCollectV",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",CreateMacroArray(1))),
_stage(_pConstantTable->GetConstantByName(NULL,"_stage")),
sSrc1(_pConstantTable->GetConstantByName(NULL,"I")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"J")),
sLoadLUT(_pConstantTable->GetConstantByName(NULL,"LoadMap")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor"))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psButterflyCollectV::Apply(Texture* src1,Texture* src2,Texture *TwiddleLUT,Texture *LoadLUT,float stage,TextureRT *dst1,TextureRT *dst2){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV",true); 
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(1)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV",true); 
	if(FAILED(hr=_pConstantTable->SetFloat(_pDevice,_stage,stage)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV",true); 
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV",true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV",true); 
	if(FAILED(hr=LoadLUT->SetAsTexture(GetSamplerIndex(sLoadLUT))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV",true); 
	if(FAILED(hr=TwiddleLUT->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV",true); 
	if(FAILED(hr=_pDevice->SetRenderTarget(1,NULL)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV",true); 
	return hr;//=_pDevice->EndScene();
}

psButterflyCollectV_MP::psButterflyCollectV_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset):
psButterflyCollect(pDevice,SRC_SHADER,"ButterflyCollectV",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",SetMacroArray("MULTIPASS",SetMacroArray("PASS1",CreateMacroArray(3)),1),2)),
_stage(_pConstantTable->GetConstantByName(NULL,"_stage")),
sSrc1(_pConstantTable->GetConstantByName(NULL,"I")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"J")),
sLoadLUT(_pConstantTable->GetConstantByName(NULL,"LoadMap")),
sFactor(_pConstantTable->GetConstantByName(NULL,"Factor")),
pass2(pDevice,SRC_SHADER,"ButterflyCollectV",D3DXGetPixelShaderProfile(pDevice),FloatToMacroArray(pixeloffset,"PIXELOFFSET",SetMacroArray("MULTIPASS",SetMacroArray("PASS2",CreateMacroArray(3)),1),2))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psButterflyCollectV_MP::Apply(Texture* src1,Texture* src2,Texture *TwiddleLUT,Texture *LoadLUT,float stage,TextureRT *dst1,TextureRT *dst2){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV_MP",true); 
	if(FAILED(hr=dst1->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV_MP",true); 
	if(FAILED(hr=_pConstantTable->SetFloat(_pDevice,_stage,stage)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV_MP",true); 
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV_MP",true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV_MP",true); 
	if(FAILED(hr=LoadLUT->SetAsTexture(GetSamplerIndex(sLoadLUT))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV_MP",true); 
	if(FAILED(hr=TwiddleLUT->SetAsTexture(GetSamplerIndex(sFactor))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV_MP",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV_MP",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV_MP",true); 
	if(FAILED(hr=pass2.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"ButterflyCollectV_MP",true); 
	return hr;
}

psCollectH::psCollectH(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget):
Pixelshader(pDevice,SRC_SHADER,"CollectH",D3DXGetPixelShaderProfile(pDevice)),
_stage(_pConstantTable->GetConstantByName(NULL,"_stage")),
sSrc1(_pConstantTable->GetConstantByName(NULL,"I")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"J")),
sLoadLUT(_pConstantTable->GetConstantByName(NULL,"LoadMap"))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psCollectH::Apply(Texture* src1,Texture *src2,Texture *SaveLUT,float stage,TextureRT *dst){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectH",true); 
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectH",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectH",true); 
	if(FAILED(hr=_pConstantTable->SetFloat(_pDevice,_stage,stage)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectH",true); 
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectH",true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectH",true); 
	if(FAILED(hr=SaveLUT->SetAsTexture(GetSamplerIndex(sLoadLUT))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectH",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectH",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectH",true); 
	return hr;//=_pDevice->EndScene();
}

psCollectV::psCollectV(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget):
Pixelshader(pDevice,SRC_SHADER,"CollectV",D3DXGetPixelShaderProfile(pDevice)),
_stage(_pConstantTable->GetConstantByName(NULL,"_stage")),
sSrc1(_pConstantTable->GetConstantByName(NULL,"I")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"J")),
sLoadLUT(_pConstantTable->GetConstantByName(NULL,"LoadMap"))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psCollectV::Apply(Texture* src1,Texture *src2,Texture *SaveLUT,float stage,TextureRT *dst){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectV",true); 
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectV",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectV",true); 
	if(FAILED(hr=_pConstantTable->SetFloat(_pDevice,_stage,stage)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectV",true); 
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectV",true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectV",true); 
	if(FAILED(hr=SaveLUT->SetAsTexture(GetSamplerIndex(sLoadLUT))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectV",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectV",true); 
	if(FAILED(quad->Draw())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"psCollectV",true); return hr;//=_pDevice->EndScene();
}



psFFT2::psFFT2(LPDIRECT3DDEVICE9 pDevice,LPCSTR pSrcFile, LPCSTR pFunctionName,LPCSTR pProfile,D3DXMACRO* defs):
	Pixelshader(pDevice,pSrcFile,pFunctionName,pProfile,defs)
	{}


psFFT2p::psFFT2p(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget):
psFFT2(pDevice,SRC_SHADER,"FFT2p",D3DXGetPixelShaderProfile(pDevice)),
sSrc1(_pConstantTable->GetConstantByName(NULL,"I")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"J"))

{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psFFT2p::Apply(Texture* src1,Texture* src2,TextureRT *dst1,TextureRT *dst2){
	PROFILE_BLOCK
	HRESULT hr;
	/*if(!SceneBegin)
		if(FAILED(hr=_pDevice->BeginScene())) 
			return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p",true); 
		else
			SceneBegin=true;*/
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(1)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p",true); 
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p",true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p",true); 
	if(FAILED(hr=_pDevice->SetRenderTarget(1,NULL)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p",true); 	
	return hr;//=_pDevice->EndScene();
}

psFFT2p_MP::psFFT2p_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget):
psFFT2(pDevice,SRC_SHADER,"FFT2p",D3DXGetPixelShaderProfile(pDevice),SetMacroArray("MULTIPASS",SetMacroArray("PASS1",CreateMacroArray(2)),1)),
sSrc1(_pConstantTable->GetConstantByName(NULL,"I")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"J")),
pass2(pDevice,SRC_SHADER,"FFT2p",D3DXGetPixelShaderProfile(pDevice),SetMacroArray("MULTIPASS",SetMacroArray("PASS2",CreateMacroArray(2)),1))

{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psFFT2p_MP::Apply(Texture* src1,Texture* src2,TextureRT *dst1,TextureRT *dst2){
	PROFILE_BLOCK
	HRESULT hr;
	/*if(!SceneBegin)
		if(FAILED(hr=_pDevice->BeginScene()))  
			return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p_MP",true);
		else
			SceneBegin=true;*/
	//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p_MP",true); 
	if(FAILED(hr=dst1->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p_MP",true); 
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p_MP",true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p_MP",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p_MP",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p_MP",true); 
	if(FAILED(hr=pass2.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT2p_MP",true); 
	return hr;//=_pDevice->EndScene();
}

psiFFT2p::psiFFT2p(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget):
Pixelshader(pDevice,SRC_SHADER,"iFFT2p",D3DXGetPixelShaderProfile(pDevice)),
sSrc1(_pConstantTable->GetConstantByName(NULL,"I")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"J"))

{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psiFFT2p::Apply(Texture* src1,Texture* src2,TextureRT *dst){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT2p",true); 
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT2p",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT2p",true); 
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT2p",true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT2p",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT2p",true); 
	if(FAILED(quad->Draw())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT2p",true); return hr;//=_pDevice->EndScene();
}

psFFT3::psFFT3(LPDIRECT3DDEVICE9 pDevice,LPCSTR pSrcFile, LPCSTR pFunctionName,LPCSTR pProfile,D3DXMACRO* defs):
	Pixelshader(pDevice,pSrcFile,pFunctionName,pProfile,defs)
	{}

psFFT3p::psFFT3p(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget):
psFFT3(pDevice,SRC_SHADER,"FFT3p",D3DXGetPixelShaderProfile(pDevice)),
sSrc1(_pConstantTable->GetConstantByName(NULL,"Src")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"I")),
sSrc3(_pConstantTable->GetConstantByName(NULL,"J"))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psFFT3p::Apply(Texture* src1,Texture* src2,Texture* src3,TextureRT *dst1,TextureRT *dst2,TextureRT *dst3){
	PROFILE_BLOCK
	HRESULT hr;
	/*if(!SceneBegin)
		if(FAILED(hr=_pDevice->BeginScene())) 
			return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p",true); 
		else
			SceneBegin=true;*/
	if(FAILED(hr=dst1->SetAsRenderTarget(0)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget(1)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p",true); 
	if(FAILED(hr=dst3->SetAsRenderTarget(2)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p",true); 
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p",true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p",true); 
	if(FAILED(hr=src3->SetAsTexture(GetSamplerIndex(sSrc3))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p",true); 
	if(FAILED(hr=_pDevice->SetRenderTarget(1,NULL)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p",true); 
	if(FAILED(hr=_pDevice->SetRenderTarget(2,NULL)))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p",true); 
	return hr;//=_pDevice->EndScene();
}

psFFT3p_MP::psFFT3p_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget):
psFFT3(pDevice,SRC_SHADER,"FFT3p",D3DXGetPixelShaderProfile(pDevice),SetMacroArray("MULTIPASS",SetMacroArray("PASS1",CreateMacroArray(2)),1)),
sSrc1(_pConstantTable->GetConstantByName(NULL,"Src")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"I")),
sSrc3(_pConstantTable->GetConstantByName(NULL,"J")),
pass2(pDevice,SRC_SHADER,"FFT3p",D3DXGetPixelShaderProfile(pDevice),SetMacroArray("MULTIPASS",SetMacroArray("PASS2",CreateMacroArray(2)),1)),
pass3(pDevice,SRC_SHADER,"FFT3p",D3DXGetPixelShaderProfile(pDevice),SetMacroArray("MULTIPASS",SetMacroArray("PASS3",CreateMacroArray(2)),1))
{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psFFT3p_MP::Apply(Texture* src1,Texture* src2,Texture* src3,TextureRT *dst1,TextureRT *dst2,TextureRT *dst3){
	PROFILE_BLOCK
	HRESULT hr;
	/*if(!SceneBegin)
		if(FAILED(hr=_pDevice->BeginScene())) 
			return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
		else
			SceneBegin=true;*/
	if(FAILED(hr=dst1->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
	if(FAILED(hr=src3->SetAsTexture(GetSamplerIndex(sSrc3))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
	if(FAILED(hr=dst2->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
	if(FAILED(hr=pass2.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
	if(FAILED(hr=dst3->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
	if(FAILED(hr=pass3.SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
	if(FAILED(quad->Draw())) 
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"FFT3p_MP",true); 
	return hr;
}

psiFFT3p::psiFFT3p(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget):
Pixelshader(pDevice,SRC_SHADER,"iFFT3p",D3DXGetPixelShaderProfile(pDevice)),
sSrc1(_pConstantTable->GetConstantByName(NULL,"Src")),
sSrc2(_pConstantTable->GetConstantByName(NULL,"I")),
sSrc3(_pConstantTable->GetConstantByName(NULL,"J"))

{
	quad=NEW  NQuad(_pDevice,&(_Rendertarget->GetRect()),1);
}

HRESULT psiFFT3p::Apply(Texture* src1,Texture* src2,Texture* src3,TextureRT *dst){
	PROFILE_BLOCK
	HRESULT hr;//if(FAILED(hr=_pDevice->BeginScene())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT3p",true); 
	if(FAILED(hr=dst->SetAsRenderTarget()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT3p",true); 
	//if(FAILED(hr=quad->SetActive()))
//		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT3p",true); 
	if(FAILED(hr=src1->SetAsTexture(GetSamplerIndex(sSrc1))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT3p",true); 
	if(FAILED(hr=src2->SetAsTexture(GetSamplerIndex(sSrc2))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT3p",true); 
	if(FAILED(hr=src3->SetAsTexture(GetSamplerIndex(sSrc3))))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT3p",true); 
	if(FAILED(hr=SetActive()))
		return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT3p",true); 
	if(FAILED(quad->Draw())) return DXTrace( "pixelshader.cpp" ,__LINE__ ,hr,"iFFT3p",true); return hr;//=_pDevice->EndScene();
}