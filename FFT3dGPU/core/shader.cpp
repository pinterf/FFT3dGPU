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

#include "Shader.h"
#include "Debug class.h"
#include <dxerr.h>



#ifdef _DEBUG
#define COMPILEOPTION D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION
#else
#define COMPILEOPTION D3DXSHADER_DEBUG
#endif

extern HINSTANCE HM;


std::list<Shader*> Shader::psList;
/*
 * Shader
 *
 *		Constructor of the parent shader class that provides the common framework for the child pixel/vertexshader classes. It compiles the shader and setup the constant table
 *
 * Inputs:
 *		pDevice:[in]	Direct3d device to use
 *		pSrcFile:[in] Filename that contains the HLSL shader
 *		pFunctionName:[in] The name of the function to compile
 *		pProfile:[in] pixelshader profile to use
 *		defs:[in] preprocessor definition to use
 * Returns:
 *     None
 *
 *	Remarks:
 *		None
 */
Shader::Shader(LPDIRECT3DDEVICE9 pDevice,LPCSTR pSrcFile, LPCSTR pFunctionName,LPCSTR pProfile,D3DXMACRO* defs):_pDevice(pDevice),quad(0),macro(defs)
{
//Get the directory Fft3dgpu is in
char Buf[400];
GetModuleFileName(HM,Buf,400);
std::string SrcFile(Buf);
//Removes the filename
std::basic_string <char>::size_type indexCh2a;
indexCh2a=SrcFile.find_last_of("\\",SrcFile.size());
SrcFile.erase(indexCh2a+1,SrcFile.size());
SrcFile+=pSrcFile;
HRESULT result;
LPD3DXBUFFER pDebugMsg;
//Compile shader
result=D3DXCompileShaderFromFile(SrcFile.c_str(),defs,NULL,pFunctionName,pProfile,COMPILEOPTION,&pShader,&pDebugMsg,&_pConstantTable);
//result=D3DXCompileShaderFromResource(NULL,MAKEINTRESOURCE(102),defs,NULL,pFunctionName,pProfile,COMPILEOPTION,&pShader,&pDebugMsg,&_pConstantTable);
if(pDebugMsg){
	MessageBox(NULL,(LPCSTR)pDebugMsg->GetBufferPointer(),pFunctionName,MB_ICONWARNING|MB_OK);
	pDebugMsg->Release();
	pDebugMsg=0;
}

}


Shader::~Shader(){
	delete quad;
	quad=0;
#ifdef _DEBUG
	OutputDebugString("Releasing Shader\n");
	cerrwin<<std::endl<<"_pConstantTable RefCount: "<<_pConstantTable->Release()<<std::endl;
#else
	_pConstantTable->Release();
#endif
	_pConstantTable=0;
	delete macro;
}
	

/*
 * GetSamplerIndex
 *
 *		Returns the sampler index of the supplied constant
 *
 * Inputs:
 *		hConstant:[in] handle to the constant
 * Returns:
 *     The sampler index
 *
 *	Remarks:
 *		This function exist only in DirectX 9.0c so I had to add it to be compatible with 9.0b
 */
UINT Shader::GetSamplerIndex(D3DXHANDLE hConstant)
{

#ifdef DX9c
	return _pConstantTable->GetSamplerIndex(hConstant);
#else
		 D3DXCONSTANT_DESC hConstDesc;
	 unsigned int c=1;
	 _pConstantTable->GetConstantDesc(hConstant,&hConstDesc,&c);
	return hConstDesc.RegisterIndex;
#endif
}

/*
 * VecToMacroArray
 *
 *		Converts the 2d vector to a macroarray to be used in the preprocessor definition =#define name float2(vector)
 *		ie. the vector (2.0,1.0) is converted to the char* "float2(2.0,1.0)"
 *
 * Inputs:
 *		vector:[in] input vector (2d)
 *		name:[in]name of the the definition	
 *		macroarray:[in][out]input macroarray to put the definition in
 *		offset:[in]the location of this definition inside the array
 * Returns:
 *     Pointer to the input macroarray
 *
 *	Remarks:
 *		
 */
D3DXMACRO* Shader::VecToMacroArray(D3DXVECTOR2 &vector,const char* name,D3DXMACRO* macroarray,int offset){
	std::stringstream ss;
	static std::string str[5];
	ss<<"float2("<<std::fixed<<vector.x<<","<<vector.y<<")";
    str[0+offset]=ss.str();
	macroarray[0+offset].Name=name;
	macroarray[0+offset].Definition=str[0+offset].c_str();
	//macroarray[1+offset].Name=0;
	//macroarray[1+offset].Definition=0;
	return macroarray;
}

/*
 * FloatToMacroArray
 *
 *		Converts a float to a macroarray to be used in the preprocessor definition #define name f
 *		
 *
 * Inputs:
 *		f:[in] input float
 *		name:[in]name of the the definition	
 *		macroarray:[in][out]input macroarray to put the definition in
 *		offset:[in]the location of this definition inside the array
 * Returns:
 *     Pointer to the input macroarray
 *
 *	Remarks:
 *		
 */
D3DXMACRO* Shader::FloatToMacroArray(float &f,const char* name,D3DXMACRO* macroarray,int offset){
	std::stringstream ss;
	static std::string str[5];
	ss<<std::fixed<<f;
    str[0+offset]=ss.str();
	macroarray[0+offset].Name=name;
	macroarray[0+offset].Definition=str[0+offset].c_str();
	//macroarray[1+offset].Name=0;
	//macroarray[1+offset].Definition=0;
	return macroarray;
}

/*
 * SetMacroArray
 *
 *		Creates an empty define = #define name  
 *
 * Inputs:
 *		name:[in]name of the the definition	
 *		macroarray:[in][out]input macroarray to put the definition in
 *		offset:[in]the location of this definition inside the array
 * Returns:
 *     Pointer to the input macroarray
 *
 *	Remarks:
 *		
 */
D3DXMACRO* Shader::SetMacroArray(const char* name,D3DXMACRO* macroarray,int offset){
	macroarray[0+offset].Name=name;
	macroarray[0+offset].Definition="";
	//macroarray[1+offset].Name=0;
	//macroarray[1+offset].Definition=0;
	return macroarray;
}


HRESULT Shader::Reset(LPDIRECT3DDEVICE9 pDevice,bool firstpass)
{
	std::list<Shader*>::iterator iter;
	HRESULT hr=1;
	for(iter=psList.begin();iter!=psList.end()&&SUCCEEDED(hr);iter++)
		hr=(*iter)->ResetShader(pDevice,firstpass);
	return hr;
}

/*
 * CreateMacroArray
 *		Create a macro array to be used for the #define
 *
 * Inputs
 *  size:[in]size of the macroarray
 *
 * Returns:
 *    Pointer to the macroarray
 *
 * Remarks
 *	The macroarray is destroyed with the Shader class
 */

D3DXMACRO* Shader::CreateMacroArray(unsigned int size)
{
	D3DXMACRO* retval=NEW D3DXMACRO[size+1];
	for(unsigned int i=0;i<=size;i++)
	{
		retval[i].Name=0;
		retval[i].Definition=0;
	}
	return retval;
}

