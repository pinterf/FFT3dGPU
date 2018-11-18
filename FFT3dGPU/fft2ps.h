#include "./core/pixelshader.h"
#include "./core/vertexshader.h"

class psButterfly : public Pixelshader {
public:
	psButterfly(LPDIRECT3DDEVICE9 pDevice,D3DXMACRO *defs,LPCSTR functionname);
	virtual HRESULT Apply(Texture* src1,Texture* src2,TextureRT *dst1,TextureRT *dst2,int stage,bool forward)=0;
	virtual HRESULT Apply(Texture* src,TextureRT *dst1,TextureRT *dst2,bool forward)=0;
};


class psButterflyHSP : public psButterfly {
public:
	psButterflyHSP(LPDIRECT3DDEVICE9 pDevice,int xn,int x,int yn,int y);
	HRESULT Apply(Texture* src1,Texture* src2,TextureRT *dst1,TextureRT *dst2,int stage,bool forward);
	HRESULT Apply(Texture* src,TextureRT *dst1,TextureRT *dst2,bool forward);
protected:
	Vertexshader vs;
	D3DXHANDLE sSrc;
	D3DXHANDLE dir;
	Vertexshader vs0;
	Pixelshader ps0;
	FFTQuad0 q0;
	FFTQuad2* q;
};

class psButterflyHMP : public psButterfly {
public:
	psButterflyHMP(LPDIRECT3DDEVICE9 pDevice,int xn,int x,int yn,int y);
	HRESULT Apply(Texture* src1,Texture* src2,TextureRT *dst1,TextureRT *dst2,int stage,bool forward);
	HRESULT Apply(Texture* src,TextureRT *dst1,TextureRT *dst2,bool forward);
protected:
	Vertexshader vs;
	D3DXHANDLE sSrc;
	D3DXHANDLE dir;
	Vertexshader vs0;
	Pixelshader ps0_pass0;
	Pixelshader ps0_pass1;
	Pixelshader ps_pass1;
	FFTQuad0 q0;
	FFTQuad2* q;
};


class psButterflyVSP : public psButterfly {
public:
	psButterflyVSP(LPDIRECT3DDEVICE9 pDevice,int xn,int x,int yn,int y);
	HRESULT Apply(Texture* src1,Texture* src2,TextureRT *dst1,TextureRT *dst2,int stage,bool forward);
	HRESULT Apply(Texture* src,TextureRT *dst1,TextureRT *dst2,bool forward);
protected:
	Vertexshader vs;
	D3DXHANDLE sSrc;
	D3DXHANDLE dir;
	Vertexshader vs0;
	Pixelshader ps0;
	FFTQuad0 q0;
	FFTQuad2* q;
};

class psButterflyVMP : public psButterfly {
public:
	psButterflyVMP(LPDIRECT3DDEVICE9 pDevice,int xn,int x,int yn,int y);
	HRESULT Apply(Texture* src1,Texture* src2,TextureRT *dst1,TextureRT *dst2,int stage,bool forward);
	HRESULT Apply(Texture* src,TextureRT *dst1,TextureRT *dst2,bool forward);
protected:
	Vertexshader vs;
	D3DXHANDLE sSrc;
	D3DXHANDLE dir;
	Vertexshader vs0;
	Pixelshader ps0_pass0;
	Pixelshader ps0_pass1;
	Pixelshader ps_pass1;
	FFTQuad0 q0;
	FFTQuad2* q;
};

class psC2Rfft : public Pixelshader {
public:
	psC2Rfft(LPDIRECT3DDEVICE9 pDevice,int xn,int x,int yn,int y);
	HRESULT Apply(Texture* src1,Texture* src2,TextureRT *dst);
protected:
	C2RQuad *q;
	Vertexshader vs;
	/*Pixelshader p00;
	Pixelshader p11;*/
	D3DXHANDLE t00;
	D3DXHANDLE t11;
	D3DXHANDLE sSrc1;
	D3DXHANDLE sSrc2;
};


class psR2Cfft : public Pixelshader {
public:
	psR2Cfft(LPDIRECT3DDEVICE9 pDevice,int xn,int x,int yn,int y);
	HRESULT Apply(Texture* src1,Texture* src2,TextureRT *dst);
protected:
	Vertexshader vs;
	D3DXHANDLE sSrc;
	R2CFFTQuad *q;
};

class psCombine : public Pixelshader {
public:
	psCombine(LPDIRECT3DDEVICE9 pDevice,int xn,int x,int yn,int y,bool horizontal,float norm);
	HRESULT Apply(Texture* src1,Texture* src2,TextureRT *dst);
protected:
	Vertexshader vs;
	D3DXHANDLE sSrc;
	CollectQuad *q;
};
	
