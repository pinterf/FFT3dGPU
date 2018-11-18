#include "fft.h"
#include "fft2ps.h"
class TexturePool;



class FFT2dRR2 :public FFT2d{
public:
	FFT2dRR2(unsigned int x,unsigned int xrep,unsigned int y,unsigned int yrep,unsigned int z,TexturePool *&StreamPoolPointer,LPDIRECT3DDEVICE9 pDevice,	GPUTYPES* gtype,int precision,HRESULT &hr);
	~FFT2dRR2();
	void CalcFFT(TextureRT* src,TextureRT* (&dst),bool forward);
private:
	LPDIRECT3DDEVICE9 _pDevice;
	
	unsigned int _log2cx;
	unsigned int _log2y;

	int bstartstage;

	psButterfly* ButterFlyH;
	psButterfly* ButterFlyV;
	psC2Rfft*	  C2Rfft;
	psR2Cfft*	  R2Cfft;
	psCombine*	  CombineV;	
	psCombine*	  CombineH;

	TextureRT*	  TempVertical1;
	TextureRT*	  TempVertical2;
	TextureRT*	  TempVertical3;
	TextureRT*	  TempVertical4;
	TextureRT*	  TempHorizontal1;
	TextureRT*	  TempHorizontal2;
	TextureRT*	  TempHorizontal3;
	TextureRT*	  TempHorizontal4;


	TexturePool *FreeStreamPool;

	float norm;

	float* u;
	float* v;
	


	


};
