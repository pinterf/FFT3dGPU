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

#include "texture.h"
#include "Debug class.h"
#include <dxerr.h>

int Texture::_ID = 0;


/*
 * GPUTYPES
 *
 *		Constructor, setups the supported texture format
 *
 * Inputs:
 *		pDevice:[in]	Direct3d device to use
  * Returns:
 *     None
 *
 *	Remarks:
 *		None
 */
GPUTYPES::GPUTYPES(LPDIRECT3DDEVICE9 pDevice) {

  D3DDEVICE_CREATION_PARAMETERS Parameters;
  LPDIRECT3D9 pD3D;
  D3DDISPLAYMODE mode;
  pDevice->GetCreationParameters(&Parameters);
  pDevice->GetDirect3D(&pD3D);
  pDevice->GetDisplayMode(0, &mode);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A32B32G32R32F)))
    _float4.RenderTarget = NEW  types(4, 4, D3DFMT_A32B32G32R32F, _FLOAT);
  else
    _float4.RenderTarget = NEW  types(4, 0, D3DFMT_UNKNOWN, _FLOAT);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_G32R32F)))
    _float2.RenderTarget = NEW  types(2, 2, D3DFMT_G32R32F, _FLOAT);
  else
    _float2.RenderTarget = NEW  types(2, _float4.RenderTarget->real_elem_size, _float4.RenderTarget->format, _FLOAT);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_R32F)))
    _float1.RenderTarget = NEW  types(1, 1, D3DFMT_R32F, _FLOAT);
  else
    _float1.RenderTarget = NEW  types(1, _float2.RenderTarget->real_elem_size, _float2.RenderTarget->format, _FLOAT);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F)))
    _half4.RenderTarget = NEW  types(4, 4, D3DFMT_A16B16G16R16F, _HALF);
  else
    _half4.RenderTarget = NEW  types(4, 0, D3DFMT_UNKNOWN, _HALF);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_G16R16F)))
    _half2.RenderTarget = NEW  types(2, 2, D3DFMT_G16R16F, _HALF);
  else
    _half2.RenderTarget = NEW  types(2, _half4.RenderTarget->real_elem_size, _half4.RenderTarget->format, _HALF);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_R16F)))
    _half1.RenderTarget = NEW  types(1, 1, D3DFMT_R16F, _HALF);
  else
    _half1.RenderTarget = NEW  types(1, _half2.RenderTarget->real_elem_size, _half2.RenderTarget->format, _HALF);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8)))
    _fixed4.RenderTarget = NEW  types(4, 4, D3DFMT_A8R8G8B8, _FIXED);
  else
    _fixed4.RenderTarget = NEW  types(4, 0, D3DFMT_UNKNOWN, _FIXED);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A8L8)))
    _fixed2.RenderTarget = NEW  types(2, 2, D3DFMT_A8L8, _FIXED);
  else
    _fixed2.RenderTarget = NEW  types(2, _fixed4.RenderTarget->real_elem_size, _fixed4.RenderTarget->format, _FIXED);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_L8)))
    _fixed1.RenderTarget = NEW  types(1, 1, D3DFMT_L8, _FIXED);
  else
    _fixed1.RenderTarget = NEW  types(1, _fixed2.RenderTarget->real_elem_size, _fixed2.RenderTarget->format, _FIXED);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_A32B32G32R32F)))
    _float4.Managed = NEW  types(4, 4, D3DFMT_A32B32G32R32F, _FLOAT);
  else
    _float4.Managed = NEW  types(4, 0, D3DFMT_UNKNOWN, _FLOAT);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_G32R32F)))
    _float2.Managed = NEW  types(2, 2, D3DFMT_G32R32F, _FLOAT);
  else
    _float2.Managed = NEW  types(2, _float4.Managed->real_elem_size, _float4.Managed->format, _FLOAT);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_R32F)))
    _float1.Managed = NEW  types(1, 1, D3DFMT_R32F, _FLOAT);
  else
    _float1.Managed = NEW  types(1, _float2.Managed->real_elem_size, _float2.Managed->format, _FLOAT);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_A16B16G16R16F)))
    _half4.Managed = NEW  types(4, 4, D3DFMT_A16B16G16R16F, _HALF);
  else
    _half4.Managed = NEW  types(4, 0, D3DFMT_UNKNOWN, _HALF);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_G16R16F)))
    _half2.Managed = NEW  types(2, 2, D3DFMT_G16R16F, _HALF);
  else
    _half2.Managed = NEW  types(2, _half4.Managed->real_elem_size, _half4.Managed->format, _HALF);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_R16F)))
    _half1.Managed = NEW  types(1, 1, D3DFMT_R16F, _HALF);
  else
    _half1.Managed = NEW  types(1, _half2.Managed->real_elem_size, _half2.Managed->format, _HALF);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8)))
    _fixed4.Managed = NEW  types(4, 4, D3DFMT_A8R8G8B8, _FIXED);
  else
    _fixed4.Managed = NEW  types(4, 0, D3DFMT_UNKNOWN, _FIXED);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_A8L8)))
    _fixed2.Managed = NEW  types(2, 2, D3DFMT_A8L8, _FIXED);
  else
    _fixed2.Managed = NEW  types(2, _fixed4.Managed->real_elem_size, _fixed4.Managed->format, _FIXED);

  if (SUCCEEDED(pD3D->CheckDeviceFormat(Parameters.AdapterOrdinal, Parameters.DeviceType, mode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_L8)))
    _fixed1.Managed = NEW  types(1, 1, D3DFMT_L8, _FIXED);
  else
    _fixed1.Managed = NEW  types(1, _fixed2.Managed->real_elem_size, _fixed2.Managed->format, _FIXED);

  if (pD3D)
    pD3D->Release();
}

GPUTYPES::~GPUTYPES()
{
  delete(_float1.Managed);
  delete(_float2.Managed);
  delete(_float4.Managed);
  delete(_half1.Managed);
  delete(_half2.Managed);
  delete(_half4.Managed);
  delete(_fixed1.Managed);
  delete(_fixed2.Managed);
  delete(_fixed4.Managed);

  delete(_float1.RenderTarget);
  delete(_float2.RenderTarget);
  delete(_float4.RenderTarget);
  delete(_half1.RenderTarget);
  delete(_half2.RenderTarget);
  delete(_half4.RenderTarget);
  delete(_fixed1.RenderTarget);
  delete(_fixed2.RenderTarget);
  delete(_fixed4.RenderTarget);
}

/*
 * Texture
 *
 *		Constructor, setups parent texture class
 *
 * Inputs:
 *		pDevice:[in]	Direct3d device to use
 *		width:[in] texture width
 *		height:[in] texture height
 *		pType:[in] texture type containing information about the texture format to use
 * Returns:
 *     None
 *
 *	Remarks:
 *		None
 */
Texture::Texture(LPDIRECT3DDEVICE9 pDevice, int width, int height, types* pType)
  :_width(width), _height(height), _pDevice(pDevice), _pType(pType), _MemUsage(width*height*_pType->PixelSizeInByte) {}

//std::list<TextureRT*> TextureRT::texRTList;
TextureRT::texList TextureRT::LastUsedList;






/*
 * TextureRT
 *
 *		Constructor, setups the Rendertexture class
 *
 * Inputs:
 *		pDevice:[in]	Direct3d device to use
 *		width:[in] texture width
 *		height:[in] texture height
 *		pType:[in] texture type containing information about the texture format to use
 *		hr:[out] result of the initialization
 * Returns:
 *     None
 *
 *	Remarks:
 *		None
 */
TextureRT::TextureRT(LPDIRECT3DDEVICE9 pDevice, int width, int height, Types& Types, HRESULT &hr)
  :Texture(pDevice, width, height, Types.RenderTarget), _RefCount(1), ID(_ID++), _SwapedToMem(false)
{

  HRESULT result;


#ifdef _DEBUG
  char BUF[10];
  itoa(ID, BUF, 10);
  OutputDebugString("Creating TextureRT ID:");
  OutputDebugString(BUF);
  OutputDebugString("\n");
#endif
  //Creates the shadow texture in system mem. It is in this texture the rendertexture is downloaded to
  result = pDevice->CreateTexture(_width, _height, 1, NULL, _pType->format, D3DPOOL_SYSTEMMEM, &_pShadowTexture, NULL);
  if (FAILED(result)) { hr = DXTrace("texture.cpp", __LINE__, result, "Create TextureRT:ShadowTexture", true); return; }
  //Creates the rendertexture in the GPU
  result = CreateVideoTexture();
  V(result);
  //Creates surfaces (~same as planes in avisynth)
  result = _pShadowTexture->GetSurfaceLevel(0, &_pShadowSurface);
  if (FAILED(result)) { hr = DXTrace("texture.cpp", __LINE__, result, "Create TextureRT:ShadowSurface", true); return; }
  result = _pVideoTexture->GetSurfaceLevel(0, &_pVideoSurface);
  if (FAILED(result)) { hr = DXTrace("texture.cpp", __LINE__, result, "Create TextureRT:VideoSurface", true); return; }

  //texRTList.push_back(this);
  ListElem = new texList(this, &LastUsedList);

}

//copy constructor
TextureRT::TextureRT(TextureRT* src, HRESULT &hr)
  :Texture(src->_pDevice, src->_width, src->_height, src->_pType), _RefCount(1), ID(_ID++), _SwapedToMem(false)
{


  HRESULT result;
#ifdef _DEBUG
  char BUF[10];
  itoa(ID, BUF, 10);
  OutputDebugString("Creating TextureRT ID:");
  OutputDebugString(BUF);
  OutputDebugString("\n");
#endif
  //Creates the shadow texture in system mem. It is in this texture the rendertexture is downloaded to
  result = _pDevice->CreateTexture(_width, _height, 1, NULL, _pType->format, D3DPOOL_SYSTEMMEM, &_pShadowTexture, NULL);
  if (FAILED(result)) { hr = DXTrace("texture.cpp", __LINE__, result, "Create TextureRT:ShadowTexture", true); return; }
  //Creates the rendertexture in the GPU
  //result=_pDevice->CreateTexture(_width,_height,1,D3DUSAGE_RENDERTARGET,_pType->format,D3DPOOL_DEFAULT,&_pVideoTexture,NULL);
  result = CreateVideoTexture();
  V(result);
  //Creates surfaces (~same as planes in avisynth)
  result = _pShadowTexture->GetSurfaceLevel(0, &_pShadowSurface);
  if (FAILED(result)) { hr = DXTrace("texture.cpp", __LINE__, result, "Create TextureRT:ShadowSurface", true); return; }
  result = _pVideoTexture->GetSurfaceLevel(0, &_pVideoSurface);
  if (FAILED(result)) { hr = DXTrace("texture.cpp", __LINE__, result, "Create TextureRT:VideoSurface", true); return; }
  //copy
  _pDevice->StretchRect(src->_pVideoSurface, NULL, _pVideoSurface, NULL, D3DTEXF_NONE);
  _pDevice->GetRenderTargetData(_pVideoSurface, _pShadowSurface);
  //texRTList.push_back(this);
  ListElem = new texList(this, &LastUsedList);
}
//destructor decreases the refcount
TextureRT::~TextureRT()
{
  int n = 0;
  if (_pVideoSurface)
    _pVideoSurface->Release();
  if (_pShadowSurface)
    _pShadowSurface->Release();
  if (_pVideoTexture)
    n = _pVideoTexture->Release();
  if (_pShadowTexture)
    n += _pShadowTexture->Release();

  _RefCount--;
#ifdef _DEBUG
  if (_RefCount < 1)
    if (n != 0) {
      char BUF[10];
      itoa(ID, BUF, 10);
      itoa(n, BUF + 5, 5);
      OutputDebugString("Not released TextureRT ID:");
      OutputDebugString(BUF);
      OutputDebugString("\n");
    }
#endif
}

//delete overload. Only deallocate the texture if refcount<1
void TextureRT::operator delete(void *p) {
  TextureRT* _this = static_cast<TextureRT*>(p);
  if (_this->_RefCount <= 0) {
#ifdef _DEBUG
    char BUF[10];
    itoa(_this->ID, BUF, 10);
    OutputDebugString("Deleting TextureRT ID:");
    OutputDebugString(BUF);
    OutputDebugString("\n");
#endif
    _this->ListElem->Unlink();
    delete _this->ListElem;
    //texRTList.remove(_this);
    ::delete(p);
  }
}

//release fucntion. Will delete textureRT if refcount<1
void TextureRT::Release() {
  int n = 0;
  if (_pVideoSurface)
    _pVideoSurface->Release();
  _pShadowSurface->Release();
  if (_pVideoTexture)
    n = _pVideoTexture->Release();
  n += _pShadowTexture->Release();
  _RefCount--;
  if (_RefCount < 1) {
#ifdef _DEBUG
    if (n != 0) {
      char BUF[10];
      itoa(ID, BUF, 10);
      itoa(n, BUF + 5, 5);
      OutputDebugString("Not released TextureRT ID:");
      OutputDebugString(BUF);
      OutputDebugString("\n");
    }
#endif

    _pVideoSurface = NULL;
    _pShadowSurface = NULL;
    _pVideoTexture = NULL;
    _pShadowTexture = NULL;
    delete this;
  }
}

void TextureRT::AddRef() {
  if (_pVideoSurface)
    _pVideoSurface->AddRef();
  _pShadowSurface->AddRef();
  if (_pVideoTexture)
    _pVideoTexture->AddRef();
  _pShadowTexture->AddRef();
  _RefCount++;
}



/*
 * CreateVideoTexture
 *
 *		This function creates the Rendertaget texture on the GPU
 *		It will free enough memory to do so or fail
 *
 * Inputs:
 *		none
  * Returns:
 *     Returns the HRESULT of createTexture
 *
 *	Remarks:
 */

HRESULT TextureRT::CreateVideoTexture()
{
  HRESULT result;
  int memfreed = 1;
  result = _pDevice->CreateTexture(_width, _height, 1, D3DUSAGE_RENDERTARGET, _pType->format, D3DPOOL_DEFAULT, &_pVideoTexture, NULL);
  //while there are still mem free and insuffisent memory to create the texture try again
  while (result == D3DERR_OUTOFVIDEOMEMORY && memfreed)
  {
    memfreed = ReleaseMem(_MemUsage);
    result = _pDevice->CreateTexture(_width, _height, 1, D3DUSAGE_RENDERTARGET, _pType->format, D3DPOOL_DEFAULT, &_pVideoTexture, NULL);
  }
  if (result == D3DERR_OUTOFVIDEOMEMORY)
    MessageBox(NULL, "fft3dgpu needs more memory than there are available on the graphics card. So\n	either get a graphics card with more memory or try lowering the resolution,bt,bh,bw,ow,oh or \n	use usefloat16=true or mode 0 or 2. Have a pleasant day.", "fft3dgpu", MB_ICONERROR | MB_OK);
  return result;
}

/*
 * SwapToMem
 *
 *		This deletes the VideoTexture to free mem on the GPU
 *
 * Inputs:
 *		none
  * Returns:
 *     Return number of bytes freed
 *
 *	Remarks:
 */

int TextureRT::SwapToMem()
{
  if (_SwapedToMem) //Allready swaped to main memory
    return 0;
#ifdef _DEBUG
  cerrwin << "Swap TextureRT " << ID << " to system memory" << std::endl;
#endif
  _pDevice->GetRenderTargetData(_pVideoSurface, _pShadowSurface);//Save content to system mem
  int RC = _RefCount;
  while (RC > 0)
  {
    int n = _pVideoSurface->Release();
    n = _pVideoTexture->Release();
    RC--;
  }
  _pVideoSurface = 0;
  _pVideoTexture = 0;
  ListElem->Unlink();

  _SwapedToMem = true;
  return _MemUsage;
}

/*
 * SwapToGPU
 *
 *		This recreates the VideoTexture on the GPU and refresh the content from system memory
 *
 * Inputs:
 *		none
  * Returns:
 *     HRESULT of texture creation
 *
 *	Remarks:
 */

HRESULT TextureRT::SwapToGPU() {
  if (!_SwapedToMem)
    return S_OK;
#ifdef _DEBUG
  cerrwin << "Swap TextureRT " << ID << " back to GPU memory" << std::endl;
#endif
  HRESULT result;
  result = CreateVideoTexture();
  V_RETURN(result);
  //Creates surfaces (~same as planes in avisynth)
  result = _pVideoTexture->GetSurfaceLevel(0, &_pVideoSurface);
  V_RETURN(result);
  int RC = _RefCount;
  while (RC > 1)//createTexture/GetSurfaceLevel already increased the counter by 1;
  {
    _pVideoSurface->AddRef();
    _pVideoTexture->AddRef();
    RC--;
  }
  _SwapedToMem = false;
  _pDevice->UpdateTexture(_pShadowTexture, _pVideoTexture);
  ListElem->InsertAtFront(&LastUsedList);
  return result;
}

HRESULT TextureRT::ReadyTexture() {
  HRESULT result;
  if (_SwapedToMem)
  {
    result = SwapToGPU();
    V_RETURN(result);
  }
  ListElem->Unlink();
  ListElem->InsertAtFront(&LastUsedList);

  return S_OK;
}

int TextureRT::ReleaseMem(int size) {
  int freed = 0;
  texList* iter = LastUsedList.Prev();
  while (freed < size&&iter->T())
  {
    texList* temp = iter->Prev();
    freed += iter->T()->SwapToMem();
    iter = temp;
  }
  return freed;
}


/*
 * SetData
 *
 *		This returns a pointer to the texture so data can be uploaded
 *
 * Inputs:
 *		pLockedRect:[out] Return object containing the pitch and pointer
 *		pRect:[in] rectangle containing the area to upload to. can be NULL
  * Returns:
 *     Result of operation
 *
 *	Remarks:
 *		Remember to call SetDataEnd() when done
 */
HRESULT TextureRT::SetData(D3DLOCKED_RECT* pLockedRect, CONST RECT *pRect) {
  return _pShadowTexture->LockRect(0, pLockedRect, pRect, 0);
}



/*
* GetData
*
*		returns a pointer to the contents of the texture
*
* Inputs:
*		pLockedRect:[out] Return object containing the pitch and pointer
*		pRect:[in] rectangle containing the area to download. can be NULL
* Returns:
*     Result of operation
*
*	Remarks:
*		Remember to call GeTDataEnd when done with downloading
*/
HRESULT TextureRT::GetData(D3DLOCKED_RECT* pLockedRect, CONST RECT *pRect) {

  LOG("Thread ID: " << GetCurrentThreadId());
  LOG("GetData")
    HRESULT result;
  ReadyTexture();
  LOG("GetRenderTargetData...")
    result = _pDevice->GetRenderTargetData(_pVideoSurface, _pShadowSurface);
  LOG("done")
    if (FAILED(result))
      return result;
  LOG("lock rect");
  return _pShadowTexture->LockRect(0, pLockedRect, pRect, D3DLOCK_READONLY);
}

/*
 * SetDataEnd
 *
 *		Called then done uploading to texture
 *
 * Inputs:
 *
 * Returns:
 *     Result of operation
 *
 *	Remarks:
 *
 */
HRESULT TextureRT::SetDataEnd() {

  HRESULT result;
  result = _pShadowTexture->UnlockRect(0);
  if (FAILED(result))
    return result;
  ReadyTexture();
  return _pDevice->UpdateTexture(_pShadowTexture, _pVideoTexture);


}

/*
 * SetDataEnd
 *
 *		Called then done downloading from texture
 *
 * Inputs:
 *
 * Returns:
 *     Result of operation
 *
 *	Remarks:
 *
 */
HRESULT TextureRT::GetDataEnd() {
  return _pShadowTexture->UnlockRect(0);
}

/*
 * SetAsRenderTarget
 *
 *		Set texture as rendertarget
 *
 * Inputs:
 *		RenderTargetIndex: Select the index to assign the texture to.
 * Returns:
 *     Result of operation
 *
 *	Remarks:
 *
 */
HRESULT TextureRT::SetAsRenderTarget(DWORD RenderTargetIndex) {
  ReadyTexture();
  return _pDevice->SetRenderTarget(RenderTargetIndex, _pVideoSurface);
}

/*
 * SetAsRenderTarget
 *
 *		Set as texture
 *
 * Inputs:
 *		Stage: Texture index.
 * Returns:
 *     Result of operation
 *
 *	Remarks:
 *
 */
HRESULT TextureRT::SetAsTexture(DWORD Stage) {
  ReadyTexture();
  return _pDevice->SetTexture(Stage, _pVideoTexture);
}

/*
 * Reset
 *
 *		Function to reset the texture when before and after reseting the device
 *
 * Inputs:
 *		pDevice:[in] device to reset
 *		firstpass:[in]if true the textures are destroyed if false they are recreated
 * Returns:
 *     Result of operation
 *
 *	Remarks:
 *
 */

HRESULT TextureRT::Reset(LPDIRECT3DDEVICE9 pDevice, bool firstpass)
{
  //std::list<TextureRT*>::iterator iter;
  HRESULT hr = S_OK;
  /*for(iter=texRTList.begin();iter!=texRTList.end()&&SUCCEEDED(hr);iter++)
    hr=(*iter)->ResetTexture(pDevice,firstpass);*/
  texList* t = LastUsedList.Next();
  while (t->T() && SUCCEEDED(hr))
  {
    hr = t->T()->ResetTexture(pDevice, firstpass);
    t = t->Next();
  }
  return hr;
}

void TextureRT::SaveTex(const char* filename)
{
  D3DXSaveSurfaceToFile(filename, D3DXIFF_DDS, _pVideoSurface, 0, 0);
}
/*
 * ResetTexture
 *
 *		Function to reset the texture when before and after reseting the device
 *
 * Inputs:
 *		pDevice:[in] device to reset
 *		firstpass:[in]if true the textures are destroyed if false they are recreated
 * Returns:
 *     Result of operation
 *
 *	Remarks:
 *		called by TextureRT::Reset
 */
HRESULT TextureRT::ResetTexture(LPDIRECT3DDEVICE9 pDevice, bool firstpass)
{
  if (_pDevice != pDevice)
    return S_OK;
  if (_SwapedToMem)
    return S_OK;
  int RC = _RefCount;
  HRESULT result = 2;
  if (firstpass)
  {

    while (RC > 0)
    {
      _pVideoSurface->Release();
      _pVideoTexture->Release();
      RC--;
    }
    _pVideoSurface = 0;
    _pVideoTexture = 0;
#ifdef _DEBUG
    cerrwin << std::endl << "Reset TextureRT ID:" << ID << std::endl;
#endif

  }
  else
  {
    result = CreateVideoTexture();
    if (FAILED(result)) { DXTrace("texture.cpp", __LINE__, result, "Recreate TextureRT:VideoTexture", true); return result; }
    result = _pDevice->UpdateTexture(_pShadowTexture, _pVideoTexture);
    if (FAILED(result)) { DXTrace("texture.cpp", __LINE__, result, "Update TextureRT:VideoTexture", true); return result; }
    result = _pVideoTexture->GetSurfaceLevel(0, &_pVideoSurface);
    if (FAILED(result)) { DXTrace("texture.cpp", __LINE__, result, "Recreate TextureRT:VideoSurface", true); return result; }
    while (RC > 1)//createTexture/GetSurfaceLevel already increased the counter by 1;
    {
      _pVideoSurface->AddRef();
      _pVideoTexture->AddRef();
      RC--;
    }
#ifdef _DEBUG
    cerrwin << std::endl << "Reset TextureRT ID:" << ID << std::endl;
#endif
  }
  return result;
}


TextureM::TextureM(LPDIRECT3DDEVICE9 pDevice, int width, int height, Types& Types, HRESULT &hr)
  :Texture(pDevice, width, height, Types.Managed), _RefCount(1), ID(_ID++)
{
  HRESULT result;
#ifdef _DEBUG
  char BUF[10];
  itoa(ID, BUF, 10);
  OutputDebugString("Creating TextureM ID:");
  OutputDebugString(BUF);
  OutputDebugString("\n");
#endif
  result = pDevice->CreateTexture(_width, _height, 1, NULL, _pType->format, D3DPOOL_MANAGED, &_pVideoTexture, NULL);
  if (FAILED(result))
    hr = DXTrace("texture.cpp", __LINE__, result, "Create TextureM:Texture", true);
  LPDIRECT3DSURFACE9 surf;
  _pVideoTexture->GetSurfaceLevel(0, &surf);//this enables PIX to show the texture
  surf->Release();
}

TextureM::~TextureM()
{
#ifdef _DEBUG
  cerrwin << std::endl << "TextureM ID:" << ID << " _pVideoTexture RefCount: " << _pVideoTexture->Release() << std::endl;
#else
  _pVideoTexture->Release();
#endif
  _RefCount--;
}

void TextureM::SaveTex(const char* filename)
{
  LPDIRECT3DSURFACE9 surf;
  _pVideoTexture->GetSurfaceLevel(0, &surf);//this enables PIX to show the texture
  D3DXSaveSurfaceToFile(filename, D3DXIFF_DDS, surf, 0, 0);
  surf->Release();
}
void TextureM::Release() {
#ifdef _DEBUG
  cerrwin << std::endl << "TextureM ID:" << ID << " _pVideoTexture RefCount: " << _pVideoTexture->Release() << std::endl;
#else
  _pVideoTexture->Release();
#endif
  _RefCount--;
  if (_RefCount < 1)
    delete this;
}

void TextureM::AddRef() {
  _pVideoTexture->AddRef();
  _RefCount++;
}

void TextureM::operator delete(void *p) {
  if (static_cast<TextureM*>(p)->_RefCount <= 0) {
#ifdef _DEBUG
    char BUF[10];
    itoa(static_cast<TextureM*>(p)->ID, BUF, 10);
    OutputDebugString("Deleting TextureM ID:");
    OutputDebugString(BUF);
    OutputDebugString("\n");
#endif
    ::delete(p);
  }
}

HRESULT TextureM::SetAsTexture(DWORD Stage) {
  return _pDevice->SetTexture(Stage, _pVideoTexture);
}

HRESULT TextureM::SetData(D3DLOCKED_RECT* pLockedRect, CONST RECT *pRect) {
  HRESULT hr;
  hr = _pVideoTexture->LockRect(0, pLockedRect, pRect, 0);
  return hr;
}

HRESULT TextureM::SetDataEnd() {
  HRESULT hr;
  hr = _pVideoTexture->UnlockRect(0);

  return hr;
}

/*
 * UploadToTexture
 *
 *		Upload src to texture. Source must have same width and height as texture (but can have different pitch)
 *
 * Inputs:
 *		src:[in] source to upload. Must have same width and height as texture and same type: unsigned char=fixed, float = float but float=half (will be converted)
 *      pitch:[in] src pitch in bytes
 * Returns:
 *     None
 *
 *	Remarks:
 *
 */
void UploadToTexture(Texture *texture, const void* src, int pitch) {
  types* ptype = texture->GetType();
  D3DLOCKED_RECT LockedRect;
  if (!ptype->real_elem_size)
    return;
  int width = texture->GetWidth();
  int height = texture->GetHeight();
  texture->SetData(&LockedRect, NULL);


  switch (ptype->type) {
  case _FLOAT: {
    const float* srcp = (const float*)src;
    float* dstp = (float*)LockedRect.pBits;
    int srcInc = ptype->elem_size;
    int dstInc = ptype->real_elem_size;
    int dstpitch = LockedRect.Pitch / sizeof(float);
    //int srcpitch=width*ptype->elem_size;//<<(elem_size<<1)
    int srcpitch = (pitch ? pitch : width * ptype->elem_size);
    for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch)
      for (int elem = 0; elem < srcInc; elem++)
        for (int x = 0, doffset =/*dstInc-1-*/elem, soffset = elem; x < width; x++, doffset += dstInc, soffset += srcInc)
          *(dstp + doffset) = *(srcp + soffset);
    break; }
  case _HALF: {
    const float* srcp = (const float*)src;
    D3DXFLOAT16* dstp = (D3DXFLOAT16*)LockedRect.pBits;
    int srcInc = ptype->elem_size;
    int dstInc = ptype->real_elem_size;
    int dstpitch = LockedRect.Pitch / sizeof(D3DXFLOAT16);
    //int srcpitch=width*ptype->elem_size;//<<(elem_size<<1)
    int srcpitch = (pitch ? pitch : width * ptype->elem_size);
    for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch)
      for (int elem = 0; elem < srcInc; elem++)
        for (int x = 0, doffset =/*dstInc-1-*/elem, soffset = elem; x < width; x++, doffset += dstInc, soffset += srcInc)
          *(dstp + doffset) = *(srcp + soffset);
    break; }
  case _FIXED: {
    const unsigned char* srcp = (const unsigned char*)src;
    unsigned char* dstp = (unsigned char*)LockedRect.pBits;
    int srcInc = ptype->elem_size;
    int dstInc = ptype->real_elem_size;

    int dstpitch = LockedRect.Pitch / sizeof(unsigned char);
    //int srcpitch=width*ptype->elem_size;//<<(elem_size<<1)
    int srcpitch = (pitch ? pitch : width * ptype->elem_size);
    if (dstInc == 2 && srcInc == 2 && dstpitch == srcpitch) {
      memcpy(dstp, srcp, srcpitch*height);
      break;
    }
    if (dstInc >= 2) {
      int alpha = srcInc == dstInc ? dstInc - 1 : dstInc - 1 - srcInc;
      for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch) {
        for (int elem = 1; elem < srcInc; elem++)
          for (int x = 0, doffset = dstInc - 1 - elem, soffset = elem - 1; x < width; x++, doffset += dstInc, soffset += srcInc)
            *(dstp + doffset) = *(srcp + soffset);
        for (int x = 0, doffset = alpha, soffset = srcInc - 1; x < width; x++, doffset += dstInc, soffset += srcInc)
          *(dstp + doffset) = *(srcp + soffset);
      }
    }
    else
      for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch)
        for (int elem = 0; elem < srcInc; elem++)
          for (int x = 0, doffset = dstInc - 1 - elem, soffset = elem; x < width; x++, doffset += dstInc, soffset += srcInc)
            *(dstp + doffset) = *(srcp + soffset);
    break; }
  }
  texture->SetDataEnd();
}

/*
 * DownloadFromTexture
 *
 *		Download texture to dst. Destination must have same width and height as texture (but can have different pitch)
 *
 * Inputs:
 *		dst:[out] Destination pointer. Must have same width and height as texture and same type: unsigned char=fixed, float = float but float=half (will be converted)
 *		pitch:[in] Destination pitch in bytes.
 *		reduceCPU:[in] call sleep while downloading
 *		nextframe:[in] struct containing information about the frame to be cached
 * Returns:
 *     None
 *
 *	Remarks:
 *
 */
void DownloadFromTexture(TextureRT *texture, void* dst, int pitch, bool IgnoreAlphaChannel) {
  LOG("Thread ID: " << GetCurrentThreadId());

  types* ptype = texture->GetType();
  D3DLOCKED_RECT LockedRect;
  if (!ptype->real_elem_size)
    return;
  int width = texture->GetWidth();
  int height = texture->GetHeight();
  LOG("GetData...");
  //Qstart();
  if (FAILED(texture->GetData(&LockedRect, NULL)))
    return;
  //Qend();
  LOG("done")
    LOG("Copy data...")
    switch (ptype->type) {
    case _FLOAT: {
      float* dstp = (float*)dst;
      float* srcp = (float*)LockedRect.pBits;
      int dstInc = ptype->elem_size;
      int srcInc = ptype->real_elem_size;
      int srcpitch = LockedRect.Pitch / sizeof(float);
      int dstpitch = (pitch ? pitch : width * ptype->elem_size);//<<(elem_size<<1)
      for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch)
        for (int elem = 0; elem < dstInc; elem++)
          for (int x = 0, soffset =/*srcInc-1-*/elem, doffset = elem; x < width; x++, doffset += dstInc, soffset += srcInc)
            *(dstp + doffset) = *(srcp + soffset);
      break; }
    case _HALF: {
      float* dstp = (float*)dst;
      D3DXFLOAT16* srcp = (D3DXFLOAT16*)LockedRect.pBits;
      int dstInc = ptype->elem_size;
      int srcInc = ptype->real_elem_size;
      int srcpitch = LockedRect.Pitch / sizeof(D3DXFLOAT16);
      int dstpitch = (pitch ? pitch : width * ptype->elem_size);//<<(elem_size<<1)
      for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch)
        for (int elem = 0; elem < dstInc; elem++)
          for (int x = 0, soffset =/*srcInc-1-*/elem, doffset = elem; x < width; x++, doffset += dstInc, soffset += srcInc)
            *(dstp + doffset) = *(srcp + soffset);
      break; }
    case _FIXED: {
      unsigned char* dstp = (unsigned char*)dst;
      unsigned char* srcp = (unsigned char*)LockedRect.pBits;
      int dstInc = ptype->elem_size;
      int srcInc = ptype->real_elem_size;
      int srcpitch = LockedRect.Pitch / sizeof(unsigned char);
      int dstpitch = (pitch ? pitch : width * ptype->elem_size); //*ptype->elem_size;//<<(elem_size<<1)
      if (dstInc == 4 && srcInc == 4 && IgnoreAlphaChannel)
      {
        for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch)
          memcpy(dstp, srcp, srcpitch);
        break;
      }

      if (srcInc >= 2) {
        int alpha = srcInc == dstInc ? srcInc - 1 : srcInc - 1 - dstInc;
        for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch) {
          for (int elem = 1; elem < dstInc; elem++)
            for (int x = 0, soffset = srcInc - 1 - elem, doffset = elem - 1; x < width; x++, doffset += dstInc, soffset += srcInc)
              *(dstp + doffset) = *(srcp + soffset);
          for (int x = 0, soffset = alpha, doffset = dstInc - 1; x < width; x++, doffset += dstInc, soffset += srcInc)
            *(dstp + doffset) = *(srcp + soffset);
        }
      }
      else
        for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch)
          for (int elem = 0; elem < srcInc; elem++)
            for (int x = 0, doffset = dstInc - 1 - elem, soffset = elem; x < width; x++, doffset += dstInc, soffset += srcInc)
              *(dstp + doffset) = *(srcp + soffset);
      break; }
    }
  LOG("done")
    LOG("GetDataEnd...")
    texture->GetDataEnd();
  LOG("done")
}



/*
 * UploadInterleavedToFixedTexture
 *
 *		Upload interleaved src to a fixed texture. Source must have same height as texture (but can have different pitch)
 *
 * Inputs:
 *		src:[in] source to upload. Must have same width and height as texture and same type: unsigned char=fixed, float = float but float=half (will be converted)
 *      pitch:[in] src pitch in bytes
 *		srcoffset:[in] width in byte between each srcbyte
 * Returns:
 *     None
 *
 *	Remarks:
 *
 */
void UploadInterleavedToFixedTexture(Texture *texture, const void* src, int pitch, int srcoffset) {
  types* ptype = texture->GetType();
  D3DLOCKED_RECT LockedRect;
  if (!ptype->real_elem_size)
    return;
  int width = texture->GetWidth();
  int height = texture->GetHeight();
  texture->SetData(&LockedRect, NULL);
  const unsigned char* srcp = (const unsigned char*)src;
  unsigned char* dstp = (unsigned char*)LockedRect.pBits;
  int srcInc = ptype->elem_size;
  int dstInc = ptype->real_elem_size;

  int dstpitch = LockedRect.Pitch / sizeof(unsigned char);
  //int srcpitch=width*ptype->elem_size;//<<(elem_size<<1)
  int srcpitch = pitch;
  if (dstInc >= 2) {
    int alpha = srcInc == dstInc ? dstInc - 1 : dstInc - 1 - srcInc;
    for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch) {
      for (int elem = 1; elem < srcInc; elem++)
        for (int x = 0, doffset = dstInc - 1 - elem, soffset = (elem - 1)*srcoffset; x < width; x++, doffset += dstInc, soffset += srcInc * srcoffset)
          *(dstp + doffset) = *(srcp + soffset);
      for (int x = 0, doffset = alpha, soffset = (srcInc - 1)*srcoffset; x < width; x++, doffset += dstInc, soffset += srcInc * srcoffset)
        *(dstp + doffset) = *(srcp + soffset);
    }
  }
  else
    for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch)
      for (int elem = 0; elem < srcInc; elem++)
        for (int x = 0, doffset = dstInc - 1 - elem, soffset = elem * srcoffset; x < width; x++, doffset += dstInc, soffset += srcInc * srcoffset)
          *(dstp + doffset) = *(srcp + soffset);
  texture->SetDataEnd();
}

/*
 * DownloadFromFixedTextureInterleaved
 *
 *		Download fixed texture to interleaved dst. Destination must have same height as texture (but can have different pitch)
 *
 * Inputs:
 *		dst:[out] Destination pointer. Must have same width and height as texture and same type: unsigned char=fixed, float = float but float=half (will be converted)
 *		pitch:[in] Destination pitch in bytes.
 *		reduceCPU:[in] call sleep while downloading
 *		nextframe:[in] struct containing information about the frame to be cached
 * Returns:
 *     None
 *
 *	Remarks:
 *
 */
void DownloadFromFixedTextureInterleaved(TextureRT *texture, void* dst, int pitch, bool IgnoreAlphaChannel, int dstoffset) {
  LOG("Thread ID: " << GetCurrentThreadId());

  types* ptype = texture->GetType();
  D3DLOCKED_RECT LockedRect;
  if (!ptype->real_elem_size)
    return;
  int width = texture->GetWidth();
  int height = texture->GetHeight();
  LOG("GetData...");
  //Qstart();
  if (FAILED(texture->GetData(&LockedRect, NULL)))
    return;
  //Qend();
  LOG("done")
    LOG("Copy data...")
  {
    unsigned char* dstp = (unsigned char*)dst;
    unsigned char* srcp = (unsigned char*)LockedRect.pBits;
    int dstInc = ptype->elem_size;
    int srcInc = ptype->real_elem_size;
    int srcpitch = LockedRect.Pitch / sizeof(unsigned char);
    int dstpitch = pitch; //*ptype->elem_size;//<<(elem_size<<1)
    if (dstInc == 4 && srcInc == 4 && IgnoreAlphaChannel)
    {

      for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch)
        for (int x = 0; x < width * 4; x++)
          *(dstp + x * dstoffset) = *(srcp + x);
      texture->GetDataEnd();
      return;
    }

    if (srcInc >= 2) {
      int alpha = srcInc == dstInc ? srcInc - 1 : srcInc - 1 - dstInc;
      for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch) {
        for (int elem = 1; elem < dstInc; elem++)
          for (int x = 0, soffset = srcInc - 1 - elem, doffset = (elem - 1)*dstoffset; x < width; x++, doffset += dstInc * dstoffset, soffset += srcInc)
            *(dstp + doffset) = *(srcp + soffset);
        for (int x = 0, soffset = alpha, doffset = (dstInc - 1)*dstoffset; x < width; x++, doffset += dstInc * dstoffset, soffset += srcInc)
          *(dstp + doffset) = *(srcp + soffset);
      }
    }
    else
      for (int y = 0; y < height; y++, srcp += srcpitch, dstp += dstpitch)
        for (int elem = 0; elem < srcInc; elem++)
          for (int x = 0, doffset = (dstInc - 1 - elem)*dstoffset, soffset = elem; x < width; x++, doffset += dstInc * dstoffset, soffset += srcInc)
            *(dstp + doffset) = *(srcp + soffset);
  }
  LOG("done")
    LOG("GetDataEnd...")
    texture->GetDataEnd();
  LOG("done")
}