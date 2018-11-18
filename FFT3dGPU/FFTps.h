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


#include "./core/pixelshader.h"

//*******************************************************************************************
class psBitReverseButterFly : public Pixelshader{
	public:
		psBitReverseButterFly(LPDIRECT3DDEVICE9 pDevice,LPCSTR pSrcFile, LPCSTR pFunctionName,LPCSTR pProfile,D3DXMACRO* defs=0);
		virtual HRESULT Apply(Texture* src,Texture *BitReverseLUT,TextureRT *dst1,TextureRT *dst2)=0;
};

class psBitReverseButterFlyH : public psBitReverseButterFly{
	public:
		psBitReverseButterFlyH(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset);
		HRESULT Apply(Texture* src,Texture *BitReverseLUT,TextureRT *dst1,TextureRT *dst2);
	protected:
		D3DXHANDLE sSrc;
		D3DXHANDLE sLoadMap;
};

class psBitReverseButterFlyH_MP : public psBitReverseButterFly{
	public:
		psBitReverseButterFlyH_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset);
		HRESULT Apply(Texture* src,Texture *BitReverseLUT,TextureRT *dst1,TextureRT *dst2);
	protected:
		D3DXHANDLE sSrc;
		D3DXHANDLE sLoadMap;
		Pixelshader pass2;
};

class psBitReverseButterFlyBH : public psBitReverseButterFly{
	public:
		psBitReverseButterFlyBH(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float norm,float pixeloffset);
		HRESULT Apply(Texture* src,Texture *BitReverseLUT,TextureRT *dst1,TextureRT *dst2);
	protected:
		D3DXHANDLE sSrc;
		D3DXHANDLE sLoadMap;
};

class psBitReverseButterFlyBH_MP : public psBitReverseButterFly{
	public:
		psBitReverseButterFlyBH_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float norm,float pixeloffset);
		HRESULT Apply(Texture* src,Texture *BitReverseLUT,TextureRT *dst1,TextureRT *dst2);
	protected:
		D3DXHANDLE sSrc;
		D3DXHANDLE sLoadMap;
		Pixelshader pass2;
};

class psBitReverseButterFlyV : public psBitReverseButterFly{
	public:
		psBitReverseButterFlyV(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset);
		HRESULT Apply(Texture* src,Texture *BitReverseLUT,TextureRT *dst1,TextureRT *dst2);
	protected:
		D3DXHANDLE sSrc;
		D3DXHANDLE sLoadMap;
};

class psBitReverseButterFlyV_MP : public psBitReverseButterFly{
	public:
		psBitReverseButterFlyV_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset);
		HRESULT Apply(Texture* src,Texture *BitReverseLUT,TextureRT *dst1,TextureRT *dst2);
	protected:
		D3DXHANDLE sSrc;
		D3DXHANDLE sLoadMap;
		Pixelshader pass2;
};



class psC2R : public Pixelshader{
	public:
		psC2R(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget);
		HRESULT Apply(Texture* src,Texture *C2RLUT,TextureRT *dst);
	protected:
		D3DXHANDLE sSrc;
		D3DXHANDLE sFactor;
};

class psR2C : public Pixelshader{
	public:
		psR2C(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget);
		HRESULT Apply(Texture* src,Texture *R2CLUT,TextureRT *dst);
	
	protected:
		D3DXHANDLE sSrc;
		D3DXHANDLE sFactor;

};

class psButterflyCollect : public Pixelshader{
	public:
		psButterflyCollect(LPDIRECT3DDEVICE9 pDevice,LPCSTR pSrcFile, LPCSTR pFunctionName,LPCSTR pProfile,D3DXMACRO* defs=0);
		virtual HRESULT Apply(Texture* src1,Texture* src2,Texture *TwiddleLUT,Texture *LoadLUT,float stage,TextureRT *dst1,TextureRT *dst2)=0;
};

class psButterflyCollectH : public psButterflyCollect{
	public:
		psButterflyCollectH(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset);
		HRESULT Apply(Texture* src1,Texture* src2,Texture *TwiddleLUT,Texture *LoadLUT,float stage,TextureRT *dst1,TextureRT *dst2);
	protected:
		D3DXHANDLE _stage;
		D3DXHANDLE sSrc1;
		D3DXHANDLE sSrc2;
		D3DXHANDLE sFactor;
		D3DXHANDLE sLoadLUT;
};

class psButterflyCollectH_MP : public psButterflyCollect{
	public:
		psButterflyCollectH_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset);
		HRESULT Apply(Texture* src1,Texture* src2,Texture *TwiddleLUT,Texture *LoadLUT,float stage,TextureRT *dst1,TextureRT *dst2);
	protected:
		D3DXHANDLE _stage;
		D3DXHANDLE sSrc1;
		D3DXHANDLE sSrc2;
		D3DXHANDLE sFactor;
		D3DXHANDLE sLoadLUT;
		Pixelshader pass2;
};

class psButterflyCollectV : public psButterflyCollect{
	public:
		psButterflyCollectV(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset);
		HRESULT Apply(Texture* src1,Texture* src2,Texture *TwiddleLUT,Texture *LoadLUT,float stage,TextureRT *dst1,TextureRT *dst2);
	protected:
		D3DXHANDLE _stage;
		D3DXHANDLE sSrc1;
		D3DXHANDLE sSrc2;
		D3DXHANDLE sFactor;
		D3DXHANDLE sLoadLUT;
};

class psButterflyCollectV_MP : public psButterflyCollect{
	public:
		psButterflyCollectV_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget,float pixeloffset);
		HRESULT Apply(Texture* src1,Texture* src2,Texture *TwiddleLUT,Texture *LoadLUT,float stage,TextureRT *dst1,TextureRT *dst2);
	protected:
		D3DXHANDLE _stage;
		D3DXHANDLE sSrc1;
		D3DXHANDLE sSrc2;
		D3DXHANDLE sFactor;
		D3DXHANDLE sLoadLUT;
		Pixelshader pass2;
};

class psCollectH : public Pixelshader{
	public:
		psCollectH(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget);
		HRESULT Apply(Texture* src1,Texture *src2,Texture *SaveLUT,float stage,TextureRT *dst);
	protected:
		D3DXHANDLE _stage;
		D3DXHANDLE sSrc1;
		D3DXHANDLE sSrc2;
		D3DXHANDLE sLoadLUT;
};

class psCollectV : public Pixelshader{
	public:
		psCollectV(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget);
		HRESULT Apply(Texture* src1,Texture *src2,Texture *SaveLUT,float stage,TextureRT *dst);
	protected:
		D3DXHANDLE _stage;
		D3DXHANDLE sSrc1;
		D3DXHANDLE sSrc2;
		D3DXHANDLE sLoadLUT;
};

//******************************************************************************************************

class psFFT2 : public Pixelshader{
	public:
		virtual HRESULT Apply(Texture* src1,Texture* src2,TextureRT *dst1,TextureRT *dst2)=0;
		psFFT2(LPDIRECT3DDEVICE9 pDevice,LPCSTR pSrcFile, LPCSTR pFunctionName,LPCSTR pProfile,D3DXMACRO* defs=0);
};

class psFFT2p : public psFFT2{
	public:
		psFFT2p(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget);
		HRESULT Apply(Texture* src1,Texture* src2,TextureRT *dst1,TextureRT *dst2);
	protected:
		D3DXHANDLE sSrc1;
		D3DXHANDLE sSrc2;
}; 

class psFFT2p_MP : public psFFT2{
	public:
		psFFT2p_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget);
		HRESULT Apply(Texture* src1,Texture* src2,TextureRT *dst1,TextureRT *dst2);
	protected:
		D3DXHANDLE sSrc1;
		D3DXHANDLE sSrc2;
		Pixelshader pass2;
}; 


class psiFFT2p : public Pixelshader{
	public:
		psiFFT2p(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget);
		HRESULT Apply(Texture* src1,Texture* src2,TextureRT *dst);
	protected:
		D3DXHANDLE sSrc1;
		D3DXHANDLE sSrc2;
}; 

class psFFT3 : public Pixelshader{
	public:
		virtual HRESULT Apply(Texture* src1,Texture* src2,Texture* src3,TextureRT *dst1,TextureRT *dst2,TextureRT *dst3)=0;
		psFFT3(LPDIRECT3DDEVICE9 pDevice,LPCSTR pSrcFile, LPCSTR pFunctionName,LPCSTR pProfile,D3DXMACRO* defs=0);
};

class psFFT3p : public psFFT3{
	public:
		psFFT3p(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget);
		HRESULT Apply(Texture* src1,Texture* src2,Texture* src3,TextureRT *dst1,TextureRT *dst2,TextureRT *dst3);
	protected:
		D3DXHANDLE sSrc1;
		D3DXHANDLE sSrc2;
		D3DXHANDLE sSrc3;
}; 

class psFFT3p_MP : public psFFT3{
	public:
		psFFT3p_MP(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget);
		HRESULT Apply(Texture* src1,Texture* src2,Texture* src3,TextureRT *dst1,TextureRT *dst2,TextureRT *dst3);
	protected:
		D3DXHANDLE sSrc1;
		D3DXHANDLE sSrc2;
		D3DXHANDLE sSrc3;
		Pixelshader pass2;
		Pixelshader pass3;
}; 

class psiFFT3p : public Pixelshader{
	public:
		psiFFT3p(LPDIRECT3DDEVICE9 pDevice,TextureRT* _Rendertarget);
		HRESULT Apply(Texture* src1,Texture* src2,Texture* src3,TextureRT *dst);
	protected:
		D3DXHANDLE sSrc1;
		D3DXHANDLE sSrc2;
		D3DXHANDLE sSrc3;
}; 

