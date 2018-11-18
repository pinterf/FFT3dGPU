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
#ifndef __SHADER__
#define __SHADER__
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9shader.h>
#include <sstream>
#include <list>
#include <string>
#include "windows.h"
#include "vertexbuffer.h"
#include "texture.h"


#define SRC_SHADER "fft3dgpu.hlsl"


class Shader {
public:
	Shader(LPDIRECT3DDEVICE9 pDevice,LPCSTR pSrcFile, LPCSTR pFunctionName,LPCSTR pProfile,D3DXMACRO* defs);
	virtual ~Shader();
	virtual HRESULT SetActive()=0;
	static HRESULT Reset(LPDIRECT3DDEVICE9 pDevice,bool firstpass);
	LPD3DXCONSTANTTABLE _pConstantTable;
protected:
	LPD3DXBUFFER pShader;
	UINT GetSamplerIndex(D3DXHANDLE hConstant);
	LPDIRECT3DDEVICE9 _pDevice;
	
	

	Quad* quad;
	static std::list<Shader*> psList;
	
	virtual HRESULT ResetShader(LPDIRECT3DDEVICE9 pDevice,bool firstpass){return 1;}
	D3DXMACRO* macro;
	D3DXMACRO* VecToMacroArray(D3DXVECTOR2 &vector,const char* name,D3DXMACRO* macroarray,int offset=0);
	D3DXMACRO* FloatToMacroArray(float &f,const char* name,D3DXMACRO* macroarray,int offset=0);
	D3DXMACRO* SetMacroArray(const char* name,D3DXMACRO* macroarray,int offset=0);
	D3DXMACRO* EndMacroArray();
	D3DXMACRO* CreateMacroArray(unsigned int size);

};


void Qstart();
void Qend();
#endif