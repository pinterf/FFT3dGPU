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


#define HEIGHT 224	

sampler Src : register(s0);// : s0;
sampler Src2 : register(s2);
sampler I : register(s3);
sampler J : register(s4);
sampler Factor : register(s1);
sampler FactorM : register(s5);
sampler LoadMap : register(s6);


float2 _beta;
float2 _sigma;
float _stage : c0;


struct PS_INPUT
{
    float2 texCoord:TEXCOORD0;
};

struct PS_INPUT2
{
    float2 texCoord[2]:TEXCOORD0;
};

struct PS_INPUT4
{
    float2 texCoord[4]:TEXCOORD0;
    #ifdef MODE2
    float4 inCol : COLOR;
    #endif
};


struct PS_OUTPUT
{
#ifndef MULTIPASS
	float4 o[2] : COLOR;
#else
	float4 o : COLOR;
#endif
};

struct PS_OUTPUT2
{
#ifndef MULTIPASS
	float4 o[3] : COLOR;
#else
	float4 o : COLOR;
#endif
};



float4 Img2toFloat4( PS_INPUT4 In) :COLOR 
{
	float4 src;
	float4 dst;
	float2 factor;
	#ifdef A8L8
	float2 img=tex2D(Src,In.texCoord[1]).xw;
	float2 img_shifted=tex2D(Src,In.texCoord[2]).xw;
	#else
	float2 img=tex2D(Src,In.texCoord[1]);
	float2 img_shifted=tex2D(Src,In.texCoord[2]);
	#endif
	factor.xy=tex2D(Factor,In.texCoord[0]);
	//factor.zw=tex2D(Factor,In.texCoord[1]);
	src.xy=img;
	src.zw=img_shifted;//z??
	dst.xzyw=src*factor.xyxy; //*factor.xyxy;
	return dst;
}

float4 Float4toImg2( PS_INPUT4 In) : COLOR
{

	float2 dst;
	//float2 src1=tex2D(Src,In.texCoord).yw;
	//float2 src2=tex2D(Src,In.texCoord+OFFSET).xz;
	//float2 factor1=tex2D(Factor,In.texCoord).xy;
	//float2 factor2=tex2D(Factor,In.texCoord+OFFSET).xy;
	//dst=src2*factor2+src1*factor1;
	#ifdef MODE2
	float2 src_shifted=tex2D(Src,In.texCoord[1]).xz;
	float2 factor_shifted=tex2D(FactorM,In.texCoord[3]);
	float2 src=tex2D(Src,In.texCoord[0]).yw;
	float2 factor=tex2D(FactorM,In.texCoord[2]);
	dst=tex2D(Src,In.texCoord[1]).xz*tex2D(FactorM,In.texCoord[3])+tex2D(Src,In.texCoord[0]).yw*tex2D(FactorM,In.texCoord[2]);
	//dst=tex2D(Src,In.texCoord[0]).xz*tex2D(FactorM,In.texCoord[2])+tex2D(Src,In.texCoord[0]).xz*tex2D(FactorM,In.texCoord[2]);
	//dst=tex2D(Src,In.texCoord[0]).yw*tex2D(FactorM,In.texCoord[2])+tex2D(Src,In.texCoord[0]).yw*tex2D(FactorM,In.texCoord[2]);
	//dst=tex2D(Src,In.texCoord[1]).xz*tex2D(FactorM,In.texCoord[3])+tex2D(Src,In.texCoord[1]).xz*tex2D(FactorM,In.texCoord[3]);
	dst=src_shifted*factor_shifted+src*factor;
	dst=src_shifted+src;
	#else
	dst=tex2D(Src,In.texCoord[1]).xz*tex2D(Factor,In.texCoord[1])+tex2D(Src,In.texCoord[0]).yw*tex2D(Factor,In.texCoord[0]);
	#endif
	return dst.xyyy;
}


//*******************************************************************************************************
float4 Img2toFloat4_2( PS_INPUT4 In) :COLOR 
{
	float4 src;
	float4 dst;
	float2 factor;
	#ifdef A8L8
	float2 img=tex2D(Src,In.texCoord[2]).xw;
	float2 img_shifted=tex2D(Src,In.texCoord[1]).xw;
	#else
	float2 img=tex2D(Src,In.texCoord[2]);
	float2 img_shifted=tex2D(Src,In.texCoord[1]);
	#endif
	factor.xy=tex2D(Factor,In.texCoord[0]);
	
	src.xy=img;
	src.zw=img_shifted;//z??
	dst.xzyw=src*factor.xyxy; //*factor.xyxy;
	return dst;
}

float4 Float4toImg2_2( PS_INPUT4 In) : COLOR
{
	float2 dst;
	//float2 src1=tex2D(Src,In.texCoord).yw;
	//float2 src2=tex2D(Src,In.texCoord+OFFSET).xz;
	//float2 factor1=tex2D(Factor,In.texCoord).xy;
	//float2 factor2=tex2D(Factor,In.texCoord+OFFSET).xy;
	//dst=src2*factor2+src1*factor1;
	
	dst=tex2D(Src,In.texCoord[1]).xz*tex2D(Factor,In.texCoord[1])+tex2D(Src,In.texCoord[0]).yw*tex2D(Factor,In.texCoord[0])
	+tex2D(I,In.texCoord[2]).xz*tex2D(Factor,In.texCoord[2])+tex2D(I,In.texCoord[3]).yw*tex2D(Factor,In.texCoord[3]);
	return dst.xyyy;
}
//*******************************************************************************************************

#ifdef PIXELOFFSET
PS_OUTPUT BitReverseButterFlyH(PS_INPUT In)
{
PS_OUTPUT dst;
float2 Index;
float2 LM;
float4 i;
float4 j;
Index=In.texCoord;
Index.y=PIXELOFFSET;
LM=tex2D(LoadMap,Index);
Index.y=In.texCoord.y;
Index.x=LM.x;
i=tex2D(Src,Index);
Index.x=LM.y;
j=tex2D(Src,Index);
#ifndef MULTIPASS
dst.o[1]=i-j;//J
dst.o[0]=i+j;//I
#else
#ifdef PASS1
dst.o=i+j;//I
#else
dst.o=i-j;//J
#endif
#endif
return dst;
}
#endif

#ifdef NORM
#ifdef PIXELOFFSET
PS_OUTPUT BitReverseButterFlyBH(PS_INPUT In)
{
PS_OUTPUT dst;
float2 Index;
float2 LM;
float4 i;
float4 j;
Index=In.texCoord;
Index.y=PIXELOFFSET;
LM=tex2D(LoadMap,Index);
Index.y=In.texCoord.y;
Index.x=LM.x;
i=tex2D(Src,Index)*NORM;
Index.x=LM.y;
j=tex2D(Src,Index)*NORM;
#ifndef MULTIPASS
dst.o[1]=i-j;//J
dst.o[0]=i+j;//I
#else
#ifdef PASS1
dst.o=i+j;//I
#else
dst.o=i-j;//J
#endif
#endif
return dst;
}
#endif
#endif

#ifdef PIXELOFFSET
PS_OUTPUT BitReverseButterFlyV(PS_INPUT In)
{
PS_OUTPUT dst;
float2 Index;
float2 LM;
float4 i;
float4 j;
Index.x=In.texCoord.y;
Index.y=PIXELOFFSET;
LM=tex2D(LoadMap,Index);
Index.x=In.texCoord.x;
Index.y=LM.x;
i=tex2D(Src,Index);
Index.y=LM.y;
j=tex2D(Src,Index);
#ifndef MULTIPASS
dst.o[1]=i-j;//J
dst.o[0]=i+j;//I
#else
#ifdef PASS1
	dst.o=i+j;//I
#else
	dst.o=i-j;//J
#endif
#endif
return dst;
}
#endif


float4 C2R(PS_INPUT In) : COLOR
{
	float4 dst;
	float4 M=tex2D(Factor,In.texCoord);
	float2 Index=In.texCoord;
	float4 F;
	float4 N;
	float4 FpN;
	float4 FmN;
	Index.x=M.x;
	F=tex2D(Src,Index);
	Index.x=M.y;
	N=tex2D(Src,Index);
	FpN=0.5*(F+N);
	FmN=0.5*(F-N);
	dst.xy=-FpN.zw*M.zz+FmN.xy*M.ww+FpN.xy;
	dst.zw=FmN.xy*M.zz+FpN.zw*M.ww+FmN.zw;
	return dst;
}

float4 R2C(PS_INPUT In) : COLOR
{
	float4 dst;
	float4 M=tex2D(Factor,In.texCoord);
	float2 Index=In.texCoord;
	float4 F;
	float4 N;
	float4 FpN;
	float4 FmN;
	Index.x=M.x;
	F=tex2D(Src,Index);
	Index.x=M.y;
	N=tex2D(Src,Index);
	FpN=0.5*(F+N);
	FmN=0.5*(F-N);
	dst.xy=FpN.zw*M.zz+FmN.xy*M.ww+FpN.xy;
	dst.zw=-FmN.xy*M.zz+FpN.zw*M.ww+FmN.zw;
	
	
	return dst;
}


#ifdef PIXELOFFSET
PS_OUTPUT ButterflyCollectH(PS_INPUT In)
{
PS_OUTPUT dst;
float2 Index;
float2 W;
float2 LM;
float4 i;
float4 j;
float2 Offset;
float4 temp;
Index.x=In.texCoord.x;
Index.y=_stage;
W=tex2D(Factor,Index);
LM=tex2D(LoadMap,Index);
Index.y=In.texCoord.y;
Index.x=LM.x;
Offset.x=PIXELOFFSET;
Offset.y=0;//1 pixel offset;
if(LM.y==0){
	i=tex2D(I,Index);
	j=tex2D(I,Index+Offset);}
else{
	i=tex2D(J,Index);
	j=tex2D(J,Index+Offset);}

temp.xy=W.xx*j.xy-W.yy*j.zw;
temp.zw=W.xx*j.zw+W.yy*j.xy;
#ifndef MULTIPASS
dst.o[1]=i-temp;
dst.o[0]=i+temp;
#else
#ifdef PASS1
	dst.o=i+temp;//I
#else
	dst.o=i-temp;//J
#endif
#endif
return dst;
}
#endif

#ifdef PIXELOFFSET
PS_OUTPUT ButterflyCollectV(PS_INPUT In)
{
PS_OUTPUT dst;
float2 Index;
float2 W;
float2 LM;
float4 i;
float4 j;
float2 Offset;
float4 temp;
Index.x=In.texCoord.y;
Index.y=_stage;
W=tex2D(Factor,Index);
LM=tex2D(LoadMap,Index);
Index.x=In.texCoord.x;
Index.y=LM.x;
Offset.y=PIXELOFFSET;
Offset.x=0;//1 pixel offset;
if(LM.y==0){
	i=tex2D(I,Index);
	j=tex2D(I,Index+Offset);}
else{
	i=tex2D(J,Index);
	j=tex2D(J,Index+Offset);}

temp.xy=W.xx*j.xy-W.yy*j.zw;
temp.zw=W.xx*j.zw+W.yy*j.xy;
#ifndef MULTIPASS
dst.o[1]=i-temp;
dst.o[0]=i+temp;
#else
#ifdef PASS1
	dst.o=i+temp;//I
#else
	dst.o=i-temp;//J
#endif
#endif
return dst;
}
#endif

float4 CollectH( PS_INPUT In ) : COLOR
{
float4 dst;
//float2 Offset;
float2 Index1;
float2 Index2;
Index1=In.texCoord;
Index1.y=_stage;
Index2=tex2D(LoadMap,Index1);
Index1.x=Index2.x;
Index1.y=In.texCoord.y;
if(Index2.y==0)
	dst=tex2D(I,Index1);
else
	dst=tex2D(J,Index1);
return dst;
}

float4 CollectV( PS_INPUT In ) : COLOR
{
float4 dst;
float2 Index1;
float2 Index2;
Index1=In.texCoord.y;
Index1.y=_stage;
Index2=tex2D(LoadMap,Index1);
Index1.y=Index2.x;
Index1.x=In.texCoord.x;
if(Index2.y==0)
	dst=tex2D(I,Index1);
else
	dst=tex2D(J,Index1);
return dst;
}
//*******************************************
PS_OUTPUT FFT2p( PS_INPUT In)
{
PS_OUTPUT dst;
float4 src1=tex2D(I,In.texCoord);
float4 src2=tex2D(J,In.texCoord); 
#ifndef MULTIPASS
dst.o[0]=src1+src2;
dst.o[1]=src1-src2;
#else
#ifdef PASS1
	dst.o=src1+src2;
#else
	dst.o=src1-src2;
#endif
#endif
return dst;
}

float4 iFFT2p( PS_INPUT In) : COLOR
{
float4 src1=tex2D(I,In.texCoord);
float4 src2=tex2D(J,In.texCoord); 
return src1 - src2;
}

PS_OUTPUT2 FFT3p( PS_INPUT In)
{
PS_OUTPUT2 dst;
float4 src1=tex2D(Src,In.texCoord);
float4 src2=tex2D(I,In.texCoord);
float4 src3=tex2D(J,In.texCoord);
float4 t1=src2+src3;
float4 y1=src1-0.5*t1;
//-0.8660254=sin(pi*4/3)
float4 y2;
y2.zwxy=float4(-0.8660254,-0.8660254,0.8660254,0.8660254)*(src2-src3);
#ifndef MULTIPASS
dst.o[0]=src1+t1;
dst.o[1]=y1+y2;
//dst.o[1].xy=y1.xy-y2.zw;
//dst.o[1].zw=y1.zw+y2.xy;
dst.o[2]=y1-y2;
//dst.o[2].xy=y1.xy+y2.zw;
//dst.o[2].zw=y1.zw-y2.xy;
#else

#ifdef PASS1
dst.o=src1+t1;
#endif
#ifdef PASS2
dst.o=y1+y2;
//dst.o.xy=y1.xy-y2.zw;
//dst.o.zw=y1.zw+y2.xy;
#endif
#ifdef PASS3
dst.o=y1-y2;
//dst.o.xy=y1.xy+y2.zw;
//dst.o.zw=y1.zw-y2.xy;
#endif

#endif

return dst;
}

float4 iFFT3p( PS_INPUT In) : COLOR
{
float4 src1=tex2D(Src,In.texCoord);
float4 src2=tex2D(I,In.texCoord);
float4 src3=tex2D(J,In.texCoord);
float4 dst;
float4 t1=src2+src3;
float4 y1=src1-0.5*t1;
float4 y2;//=0.8660254*(src2-src3);
y2.zwxy=float4(-0.8660254,-0.8660254,0.8660254,0.8660254)*(src2-src3);
//dst.xy=(y1.xy-y2.zw);
//dst.zw=(y1.zw+y2.xy);
dst=y1-y2;
return dst;
}



//*******************************************************************************************************
/*
#ifdef BETA
float4 WFilter( PS_INPUT In) : COLOR
{
float4 src=tex2D(Src,In.texCoord);
float4 MulFac=float4(BETA.x,BETA.x,BETA.x,BETA.x);

float2 PSDInv=1/(float2(src.x*src.x+src.z*src.z,src.y*src.y+src.w*src.w)+0.0000000000000000000000000000000000001);

if(1/SIGMA.x>PSDInv.x)
	MulFac.xz=1.0-SIGMA.y*PSDInv.x;
if(1/SIGMA.x>PSDInv.y)
	MulFac.yw=1.0-SIGMA.y*PSDInv.y;
	


//float2 PSD=float2(src.x*src.x+src.z*src.z,src.y*src.y+src.w*src.w);
//if(SIGMA.x<PSD.x)
//	MulFac.xz=((PSD.x-SIGMA.y)/(PSD.x+0.0000000000000000000000000000000000001));//((PSD.x-SIGMA.y)/PSD.x);
//if(SIGMA.x<PSD.y)
//	MulFac.yw=((PSD.y-SIGMA.y)/(PSD.y+0.0000000000000000000000000000000000001));//((PSD.y-SIGMA.y)/PSD.y);


//MulFac=float4(0.5,0.5,0.5,0.5);
return MulFac*src;
//return MulFac;+0.0000000000000000000000000000000000001
}
#endif

#ifdef BETA
float4 WFilterDegrid( PS_INPUT In) : COLOR
{
float4 s=tex2D(Src,In.texCoord);
float4 correction=tex2D(Factor,In.texCoord);
float4 src=s-correction;
float4 MulFac=float4(BETA.x,BETA.x,BETA.x,BETA.x);
float2 PSDInv=1/(float2(src.x*src.x+src.z*src.z,src.y*src.y+src.w*src.w)+0.0000000000000000000000000000000000001);

if(1/SIGMA.x>PSDInv.x)
	MulFac.xz=1.0-SIGMA.y*PSDInv.x;
if(1/SIGMA.x>PSDInv.y)
	MulFac.yw=1.0-SIGMA.y*PSDInv.y;

return MulFac*src+correction;
}
#endif*/
//*******************************************************************************************************
#ifdef SSMAX
float4 Sharpen( PS_INPUT In) :COLOR
{
float4 src=tex2D(Src,In.texCoord);
float factor=tex2D(Factor,In.texCoord).x;
#ifdef DEGRID
float4 degrid=tex2D(FactorM,In.texCoord);
src=src-degrid;
#endif
float4 psd=(float2(src.x*src.x+src.z*src.z,src.y*src.y+src.w*src.w)+0.0000000000000000000000000000000000001).xyxy;
float4 dst=src*(float4(1,1,1,1)+factor*sqrt( psd*SSMAX/((psd + SSMIN)*(psd + SSMAX))));
#ifdef DEGRID
dst=dst+degrid;
#endif
return dst; 
}
#endif

//******************************************************************************************************
float4 FFTtoFixed( PS_INPUT In) : COLOR
{
float4 src=tex2D(Src,In.texCoord);
#ifdef FFT
float4 dst=0.1*log(1.0f+length(src.xz));
#else
float4 dst=/*src.xzyw;+*/src.ywxz;
#endif
return dst;
}
//******************************************************************************************************
float4 Minimize( PS_INPUT In) :COLOR
{
return tex2D(Src,In.texCoord).xyxy;
}

float4 GridCorrection( PS_INPUT In) :COLOR
{
	float4 src=tex2D(Src,In.texCoord);
	float4 factor=tex2D(Factor,In.texCoord);
	#ifdef DGRID
	float4 dst=src/factor.xyxy*DGRID;
	#else
	float4 dst=src*factor;
	#endif
	return dst;
}
//******************************************************************************************************
#ifdef BETA

sampler prev2;// : s0;
sampler prev1;// : s0;
sampler current : s3;
sampler next : s4;
sampler degrid : s1;
sampler sigma;

float4 Wienner(float4 src,float2 texCoord)
{
#ifdef SIGMA
float4 MulFac=float4(BETA.x,BETA.x,BETA.x,BETA.x);
float2 PSDInv=1/(float2(src.x*src.x+src.z*src.z,src.y*src.y+src.w*src.w)+0.0000000000000000000000000000000000001);
if(1/SIGMA.x>PSDInv.x)
	MulFac.xz=1.0-SIGMA.y*PSDInv.x;
if(1/SIGMA.x>PSDInv.y)
	MulFac.yw=1.0-SIGMA.y*PSDInv.y;
return MulFac*src;
#else
float4 MulFac=float4(BETA.x,BETA.x,BETA.x,BETA.x);
float2 Sigma=float2(tex2D(sigma,texCoord).x*BETA.y,tex2D(sigma,texCoord).x);
float2 PSDInv=1/(float2(src.x*src.x+src.z*src.z,src.y*src.y+src.w*src.w)+0.0000000000000000000000000000000000001);
if(1/Sigma.x>PSDInv.x)
	MulFac.xz=1.0-Sigma.y*PSDInv.x;
if(1/Sigma.x>PSDInv.y)
	MulFac.yw=1.0-Sigma.y*PSDInv.y;
return MulFac*src;
#endif

}

float4 Wienner2d( PS_INPUT In) : COLOR
{
float4 retval;
float4 src=tex2D(current,In.texCoord);
#ifdef DEGRID
float4 correction=tex2D(degrid,In.texCoord);
src=src-correction;
#endif
/*
float4 MulFac=float4(BETA.x,BETA.x,BETA.x,BETA.x);
float2 PSDInv=1/(float2(src.x*src.x+src.z*src.z,src.y*src.y+src.w*src.w)+0.0000000000000000000000000000000000001);
if(1/SIGMA.x>PSDInv.x)
	MulFac.xz=1.0-SIGMA.y*PSDInv.x;
if(1/SIGMA.x>PSDInv.y)
	MulFac.yw=1.0-SIGMA.y*PSDInv.y;*/
retval=Wienner(src,In.texCoord);
#ifdef DEGRID
retval=retval+correction;
#endif
return retval;
}



float4 Wienner3d2( PS_INPUT In): COLOR
{
float4 retval;
float4 dst[2];	
float4 src1=tex2D(prev1,In.texCoord);
float4 src2=tex2D(current,In.texCoord); 
dst[0]=src2-src1;
dst[1]=src2+src1;
#ifdef DEGRID
float4 factor=tex2D(degrid,In.texCoord)*2;
dst[1]=dst[1]-factor;
#endif
src1=Wienner(dst[0],In.texCoord);
src2=Wienner(dst[1],In.texCoord);
retval=src2 + src1;
#ifdef DEGRID
retval=retval+factor;
#endif
//return src1 +src2; 
return retval;
}

float4 Wienner3d3( PS_INPUT In): COLOR
{
float4 retval;
float4 dst[3];
float4 p=tex2D(prev1,In.texCoord);
float4 c=tex2D(current,In.texCoord);
float4 n=tex2D(next,In.texCoord);
#ifdef DEGRID
float4 factor=tex2D(degrid,In.texCoord)*3;
#endif
float4 t1=p+n;
float4 y1=c-0.5*t1;
//-0.8660254=sin(pi*4/3)
float4 y2;
y2.zwxy=float4(-0.8660254,-0.8660254,0.8660254,0.8660254)*(n-p);
dst[0]=y1+y2;
dst[1]=c+t1;
dst[2]=y1-y2;
#ifdef DEGRID
dst[1]=dst[1]-factor;
#endif

/*
p=dst[0];
c=dst[1];
n=dst[2];
*/
p=Wienner(dst[0],In.texCoord);
c=Wienner(dst[1],In.texCoord);
n=Wienner(dst[2],In.texCoord);

retval=p+c+n;
#ifdef DEGRID
retval=retval+factor;
#endif
return retval;
}

//modified from Fizick fft3dfilter
float4 Wienner3d4( PS_INPUT In): COLOR
{
float4 retval;
float4 p2=tex2D(prev2,In.texCoord);
float4 p1=tex2D(prev1,In.texCoord);
float4 c=tex2D(current,In.texCoord);
float4 n=tex2D(next,In.texCoord);
#ifdef DEGRID
float4 factor=tex2D(degrid,In.texCoord)*4;
#endif
float4 fp1 = -p2 + float4(1.0,1.0,-1.0,-1.0)*p1.zwxy + c + float4(-1.0,-1.0,1.0,1.0)*n.zwxy;
float4 fc  = p2 + p1 + c + n;
float4 fn  = -p2 + float4(-1.0,-1.0,1.0,1.0)*p1.zwxy + c + float4(1.0,1.0,-1.0,-1.0)*n.zwxy;
float4 fp2 = p2 - p1 + c - n;
#ifdef DEGRID
fp1=fp1-factor;
#endif
fp2=Wienner(fp2,In.texCoord);
fp1=Wienner(fp1,In.texCoord);
fc=Wienner(fc,In.texCoord);
fn=Wienner(fn,In.texCoord);
retval=fp2+fp1+fc+fn;
#ifdef DEGRID
retval=retval+factor;
#endif
return retval;
}
#endif

#ifdef KRATIOSQUARED

sampler Last : s1;
sampler Covar : s2;
sampler CovarProcess : s3;
sampler CovarNoise :s4;

struct PS_RETVAL
{
	float4 dst : COLOR0;
	float4 covar :COLOR1;
	float4 covarprocess :COLOR2;
};

#ifndef COVARNOISE
#define COVARNOISE tex2D(CovarNoise,In.texCoord).x
#endif
//modified from Fizick fft3dfilter
PS_RETVAL Kalman(PS_INPUT In):COLOR
{
	PS_RETVAL retval;
	PS_RETVAL r_1;
	float4 covar=tex2D(Covar,In.texCoord);
	float4 covarprocess=tex2D(CovarProcess,In.texCoord);
	float4 src=tex2D(Src,In.texCoord);
	float4 last=tex2D(Last,In.texCoord);
	float4 mask=(src-last)*(src-last);
	mask.x=max(mask.x,mask.z);
	mask.y=max(mask.y,mask.w);
	
		r_1.dst=src;
		r_1.covar=COVARNOISE;
		r_1.covarprocess=COVARNOISE;
	
		float4 s=covar+covarprocess;
		float4 gain=s/(s+COVARNOISE);
		retval.covarprocess=gain*gain*COVARNOISE;
		retval.covar=(1-gain)*s;
		retval.dst=gain*src+(1-gain)*last;
	if(mask.x>KRATIOSQUARED*COVARNOISE){
		retval.dst.xz=r_1.dst.xz;
		retval.covar.xz=r_1.covar.xz;
		retval.covarprocess.xz=r_1.covarprocess.xz;
	 }
	if(mask.y>KRATIOSQUARED*COVARNOISE)
	{
		retval.dst.yw=r_1.dst.yw;
		retval.covar.yw=r_1.covar.yw;
		retval.covarprocess.yw=r_1.covarprocess.yw;
	 }
	
  
	return retval;
}

//PASS1=dst
//PASS2=covar
//PASS3=covarprocess

float4 KalmanMP(PS_INPUT In):COLOR
{
	float4 retval;
	float4 r_1;
	float4 covar=tex2D(Covar,In.texCoord);
	float4 covarprocess=tex2D(CovarProcess,In.texCoord);
	float4 src=tex2D(Src,In.texCoord);
	float4 last=tex2D(Last,In.texCoord);
	float4 mask=(src-last)*(src-last);
	mask.x=max(mask.x,mask.z);
	mask.y=max(mask.y,mask.w);
	
		#ifdef PASS1_K
		r_1=src;
		#else 
		r_1=COVARNOISE;
		#endif
		
		float4 s=covar+covarprocess;
		float4 gain=s/(s+COVARNOISE);
		
		#ifdef PASS1_K
		retval=gain*src+(1-gain)*last;
		#elif defined(PASS2_K)
		retval=(1-gain)*s;
		#else
		retval=gain*gain*COVARNOISE;
		#endif
	if(mask.x>KRATIOSQUARED*COVARNOISE){
		retval.xz=r_1.xz;
	 }
	if(mask.y>KRATIOSQUARED*COVARNOISE)
	{
		retval.yw=r_1.yw;
	 }
	
  
	return retval;
}
#endif
	
#ifdef CHROMA
#define CHROMANORM float4(128.0/255.0,128.0/255.0,128.0/255.0,128.0/255.0)
#else
#define CHROMANORM float4(0,0,0,0)
#endif


float4 Float4ToImg2Corner(PS_INPUT4 In):COLOR{
float4 dst;
float4 tex0;
float4 tex1;
float4 factortex0=tex2D(Factor,In.texCoord[2]);
float4 factortex1=tex2D(Factor,In.texCoord[3]);
tex0.xy=tex2D(Src,In.texCoord[0]).xz;
tex1.zw=tex2D(Src2,In.texCoord[1]).yw;
tex0.zw=tex2D(Src,float2(In.texCoord[1].x,In.texCoord[0].y)).yw;
tex1.xy=tex2D(Src2,float2(In.texCoord[0].x,In.texCoord[1].y)).xz;
float4 tmp=tex0*factortex0+tex1*factortex1;
dst=tmp.xyzw+tmp.zwxy;

//return float4(In.texCoord[0].y*HEIGHT,In.texCoord[1].y*HEIGHT,In.texCoord[2].y*16,round(dst.x));
return dst+CHROMANORM;
}

#ifdef XY
#define SW0 xzxz
#define SWF xyxy
#else
#define SW0 ywyw
#define SWF zwzw
#endif

float4 Float4ToImg2Nooverlap(PS_INPUT4 In):COLOR{
float4 dst=tex2D(Src,In.texCoord[0]).SW0;

//return float4(In.texCoord[0].y*HEIGHT,0,0,round(dst.x));
return dst+CHROMANORM;
}

float4 Float4ToImg2Horizontal(PS_INPUT4 In):COLOR{
float4 dst;
float4 factortex0=tex2D(Factor,In.texCoord[2]).SWF;
float4 factortex1=tex2D(Factor,In.texCoord[3]).SWF;
float4 src1=tex2D(Src,In.texCoord[0]).SW0;
float4 src2=tex2D(Src2,In.texCoord[1]).SW0;
dst=src1*factortex0+src2*factortex1;

//return float4(In.texCoord[0].y*HEIGHT,In.texCoord[1].y*HEIGHT,In.texCoord[2].y*16,round(dst.x));
return dst+CHROMANORM;
}

float4 Float4ToImg2Vertical(PS_INPUT4 In):COLOR{
float4 dst;
float4 factor=tex2D(Factor,In.texCoord[2]);
float4 src1=tex2D(Src,In.texCoord[0]).xzxz;
float4 src2=tex2D(Src,In.texCoord[1]).ywyw;
dst=src1*factor.xyxy+src2*factor.zwzw;

//return float4(In.texCoord[0].y*HEIGHT,In.texCoord[1].y*HEIGHT,In.texCoord[2].y*16,round(dst.x));
return dst+CHROMANORM;
}


#ifdef A8L8
#define SW1 xw
#else
#define SW1 xy
#endif



#ifdef OFFSET

PS_OUTPUT Img2ToFloat4(PS_INPUT4 In){
PS_OUTPUT dst;
float4 factor=tex2D(Factor,In.texCoord[2]).xxyy;
dst.o[0].xz=tex2D(Src,In.texCoord[0]).SW1;
dst.o[0].yw=tex2D(Src,In.texCoord[1]).SW1;
dst.o[1].xz=tex2D(Src,In.texCoord[0]+OFFSET).SW1;
dst.o[1].yw=tex2D(Src,In.texCoord[1]+OFFSET).SW1;
dst.o[0]=(dst.o[0]-CHROMANORM)*factor;
dst.o[1]=(dst.o[1]-CHROMANORM)*factor;
//dst.o[0]=float4(In.texCoord[2].y*16,In.texCoord[1].y*32,In.texCoord[0].y*32,(In.texCoord[0]+OFFSET).y*32);
return dst;
}

float4 Img2ToFloat4MP(PS_INPUT4 In):COLOR{
float4 dst;
float4 factor=tex2D(Factor,In.texCoord[2]).xxyy;
#ifdef PASS1
dst.xz=tex2D(Src,In.texCoord[0]).SW1;
dst.yw=tex2D(Src,In.texCoord[1]).SW1;
#else
dst.xz=tex2D(Src,In.texCoord[0]+OFFSET).SW1;
dst.yw=tex2D(Src,In.texCoord[1]+OFFSET).SW1;
#endif
dst=(dst-CHROMANORM);

return dst*factor;
}
#endif

#ifdef OFFSET
float4 Img2toImg4( PS_INPUT In ) :COLOR
{
float4 dst=float4(0,0,0,0);
dst.zy=tex2D(Src,In.texCoord).SW1;
dst.xw=tex2D(Src,In.texCoord+OFFSET).SW1;
return dst;
}
#endif

struct PS_OUTPUTD
{
float4 c:COLOR0;
float d:DEPTH0;
};

PS_OUTPUTD CopyTexToZBuffer(in float2 In:TEXCOORD0 )
{
  PS_OUTPUTD dst;
  dst.d=tex2D(Src,In).x;
  dst.c=float4(0,0,0,0);
  return dst;
}






//*********************************
// fft2
//2 1d fft either vertical or horizontal
float dir:f0;

struct VS_OUTPUT
{
    float4 Position   : POSITION0;   // vertex position 
    float2 texCoord[2]:TEXCOORD0;
    float LoadTwiddle:TEXCOORD02;	
};

struct VS_OUTPUT2
{
    float4 Position   : POSITION0;   // vertex position 
    float2 texCoord[5]:TEXCOORD0;
};

struct VS_OUTPUT3
{
    float4 Position   : POSITION0;   // vertex position 
    float2 texCoord[3]:TEXCOORD0;
};


struct VS_OUTPUT1
{
    float4 Position   : POSITION0;   // vertex position 
    float2 texCoord:TEXCOORD0;
};

struct VS_INPUT
{
    float4 Position   : POSITION0;   // vertex position 
    float2 texCoord[2]:TEXCOORD0;
};

struct VS_OUTPUT4
{
    float4 Position   : POSITION0;   // vertex position 
    float2 texCoord[2]:TEXCOORD0;
};

VS_OUTPUT4 fft2(VS_OUTPUT4 In)
{
	VS_OUTPUT4 Out=In;
	Out.texCoord[1].y=dir*Out.texCoord[1].y;
	return Out;
}

VS_OUTPUT4 fft0(VS_OUTPUT4 In)
{
	VS_OUTPUT4 Out=In;
	Out.texCoord[1].x=dir*Out.texCoord[1].x;
	return Out;
}

VS_OUTPUT3 passthough3(VS_OUTPUT3 In)
{
	return In;
}

VS_OUTPUT1 passthough(VS_OUTPUT1 In)
{
	return In;
}
//	  Input vertex v0,v1,v2,v3
//    v0-v1
//    | / |
//    v2-v3
//	  for a single 1d fft the vertexes will have the following values	
//	  v0.Position={-1,-1},v0.texCoord[0]={0,1},v0.texCoord[1]={0,0}   v1.Position={1,-1},v1.texCoord[0]={1,1},v1.texCoord[1]={1,1}
//	  v2.Position={-1,1},v2.texCoord[0]={0,0},v2.texCoord[1]={0,0}    v3.Position={-1,1},v3.texCoord[0]={1,0},v3.texCoord[1]={1,0}


	


//Does 2 fft in parallel. Data is stored in a float4 texture with Realfft1,Realfft2,Imagfft1,Imagfft2 in xyzw
sampler Src0:s0;
sampler Src1:s1;

bool stage0;


#define EPSILON 0.00001

#ifdef LOADOFFSET
//LOADOFFSET = 0.5 for single fft
PS_OUTPUT Butterfly(VS_OUTPUT4 In)
{
	PS_OUTPUT Out;
	float4 a;
	float4 b;
	float2 T;
	float2 I=In.texCoord[1];
	#ifdef STAGE0
	sincos(I.x,T.y,T.x);
	#else
	T=I;
	#endif
	a=tex2D(Src0,In.texCoord[0]);
	b=tex2D(Src0,In.texCoord[0]+LOADOFFSET);
	#ifdef MULTIPASS
		#ifdef PASS0
	Out.o=a+b;
		#else
	float4 temp=a-b;
	float4 T1=float4(T.x,T.x,-T.y,T.y);
	Out.o=temp.zwxy*T1.zzww+temp.xyzw*T1.xyyy;
		#endif
	#else
	Out.o[0]=a+b;
	float4 temp=a-b;
	float4 T1=float4(T.x,T.x,-T.y,T.y);
	Out.o[1]=temp.zwxy*T1.zzww+temp.xyzw*T1.xyyy;
	#endif
	return Out;
}
#endif	



#ifndef NORM
#define NORM 1.0
#endif

float4 Combine(VS_OUTPUT1 In):COLOR{
return tex2D(Src0,In.texCoord)*NORM;
}
	
float4 R2Cfft(VS_OUTPUT3 In):COLOR
{
float4 a,b;
float4 Out;
float2 T;
a=tex2D(Src0,In.texCoord[0]);
b=tex2D(Src0,In.texCoord[1]);
//calc twiddle
sincos(In.texCoord[2].x,T.y,T.x);
//Out.xy=((a.zw+b.zw)*T.x-(a.xy-b.xy)*T.y+(a.xy+b.xy))*0.5;
//Out.zw=(-(a.xy-b.xy)*T.x-(a.zw+b.zw)*T.y+(a.zw-b.zw))*0.5;
Out.xy=(-(a.zw+b.zw)*T.x+(b.xy-a.xy)*T.y+a.xy+b.xy)*0.5;
Out.zw=((a.xy-b.xy)*T.x-(a.zw+b.zw)*T.y+a.zw-b.zw)*0.5;
//Out=float4(In.texCoord[2].x,In.texCoord[0].x*W,In.texCoord[1].x*W,Out.x+Out.z);
return Out;
}


bool t00;
bool t11;

float4 C2Rfft_ps(VS_OUTPUT3 In):COLOR0
{
float4 Out;
float4 a,b;
float2 T;
if(t00)
{
a=tex2D(Src0,In.texCoord[0]);
Out.xy=a.xy+In.texCoord[2].x*a.zw;
Out.zw=float2(0,0);
}
if(t11)
{
a=tex2D(Src0,In.texCoord[0]);
Out.xy=a.xy;
Out.zw=-a.zw;
}
else if(!t00)
{
a=tex2D(Src0,In.texCoord[0]);
b=tex2D(Src1,In.texCoord[1]);
sincos(In.texCoord[2].x,T.y,T.x);
Out.xy=((a.zw+b.zw)*T.x-(a.xy-b.xy)*T.y+(a.xy+b.xy))*0.5;
Out.zw=(-(a.xy-b.xy)*T.x-(a.zw+b.zw)*T.y+(a.zw-b.zw))*0.5;
}
//Out=float4(Out.x+Out.z,W*In.texCoord[0].x,W*In.texCoord[1].x,In.texCoord[2].x);
return Out;
}

