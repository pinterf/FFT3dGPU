#include "Debug class.h"
#include <dxerr.h>
#include "vertexshader.h"

Vertexshader::Vertexshader(LPDIRECT3DDEVICE9 pDevice, LPCSTR pSrcFile, LPCSTR pFunctionName, LPCSTR pProfile, D3DXMACRO* defs)
  :Shader(pDevice, pSrcFile, pFunctionName, pProfile, defs)
{
#ifdef _DEBUG
  OutputDebugString("Creating vertexshader ");
  OutputDebugString(pFunctionName);
  OutputDebugString("\n");
#endif
  _pDevice->CreateVertexShader((DWORD*)pShader->GetBufferPointer(), &_pVertexShader);
  if (pShader)
    pShader->Release();
  pShader = 0;
};

Vertexshader::~Vertexshader() {
  delete quad;
  quad = 0;
#ifdef _DEBUG
  OutputDebugString("Releasing vertexshader\n");
  cerrwin << std::endl << "_pVertexShader RefCount: " << _pVertexShader->Release() << std::endl;
#else
  _pVertexShader->Release();
#endif
  _pVertexShader = 0;
}

HRESULT Vertexshader::SetActive() {
  return _pDevice->SetVertexShader(_pVertexShader);
}