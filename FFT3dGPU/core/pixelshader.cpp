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


#include "Pixelshader.h"
#include "Debug class.h"
#include <dxerr.h>
#include <iomanip>



#ifdef _DEBUG
#define COMPILEOPTION D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION
#else
#define COMPILEOPTION 0
//D3DXSHADER_PREFER_FLOW_CONTROL
#endif

extern HINSTANCE HM;


/*
 * Pixelshader
 *
 *		Constructor of the parent pixelshader class that provides the common framework for the child pixelshader classes. It compiles the shader and setup the constant table
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
Pixelshader::Pixelshader(LPDIRECT3DDEVICE9 pDevice,LPCSTR pSrcFile, LPCSTR pFunctionName,LPCSTR pProfile,D3DXMACRO* defs)
:Shader( pDevice, pSrcFile,  pFunctionName, pProfile,defs)
{

#ifdef _DEBUG
OutputDebugString("Creating pixelshader ");
OutputDebugString(pFunctionName);
OutputDebugString("\n");
#endif
if(!pShader)
	return;
_pDevice->CreatePixelShader((DWORD*)pShader->GetBufferPointer(),&_pPixelShader);
if(pShader)
	pShader->Release();
pShader=0;
}


Pixelshader::~Pixelshader(){
#ifdef _DEBUG
	OutputDebugString("Releasing pixelshader\n");
	cerrwin<<std::endl<<"_pPixelShader RefCount: "<<_pPixelShader->Release()<<std::endl;
#else
	_pPixelShader->Release();
#endif
	_pPixelShader=0;
}
	
/*
 * SetActive
 *
 *		Activates the pixelshader
 *
 * Inputs:
 *
 * Returns:
 *     Result of the operation
 *
 *	Remarks:
 *		None
 */
HRESULT Pixelshader::SetActive(){
	return _pDevice->SetPixelShader(_pPixelShader);
}

