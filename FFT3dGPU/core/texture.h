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

#ifndef __TEXTURE__
#define __TEXTURE__

#include <assert.h>

#include <d3d9.h>
#include <d3dx9.h>
#include <stack>
#include <list>


enum gpu_types {
  _FLOAT = 4,
  _HALF = 2,
  _FIXED = 1
};


class types {
public:
  types(int es, int res, D3DFORMAT form, gpu_types gt) :
    elem_size(es), real_elem_size(res), format(form), type(gt), PixelSizeInByte(res*gt) {}
  const int elem_size;
  const int real_elem_size;
  const D3DFORMAT format;
  const gpu_types type;
  const int PixelSizeInByte;
};

struct Types {
  types* RenderTarget;
  types* Managed;
};

class GPUTYPES {

public:
  GPUTYPES(LPDIRECT3DDEVICE9 pDevice);
  ~GPUTYPES();

  Types FLOAT() { return _float1; }
  Types FLOAT2() { return _float2; }
  Types FLOAT4() { return _float4; }
  Types FIXED() { return _fixed1; }
  Types FIXED2() { return _fixed2; }
  Types FIXED4() { return _fixed4; }
  Types HALF() { return _half1; }
  Types HALF2() { return _half2; }
  Types HALF4() { return _half4; }

private:
  Types _float1;
  Types _float2;
  Types _float4;
  Types _half1;
  Types _half2;
  Types _half4;
  Types _fixed1;
  Types _fixed2;
  Types _fixed4;

};


class Texture {
protected:
  int _width;
  int _height;
  types* _pType;
  LPDIRECT3DDEVICE9 _pDevice;
  static int _ID;
  int _MemUsage;
public:
  Texture(LPDIRECT3DDEVICE9 pDevice, int width, int height, types* pType);
  virtual ~Texture() {}
  types* GetType() { return _pType; }
  virtual void AddRef() = 0;
  virtual void Release() = 0;
  int Size() { return(_width*_height*_pType->elem_size); }
  int GetHeight() { return(_height); }
  int GetWidth() { return(_width); }
  RECT GetRect() { RECT retval; retval.top = 0; retval.bottom = _height; retval.left = 0; retval.right = _width; return retval; }
  virtual HRESULT SetAsTexture(DWORD Stage) = 0;
  virtual HRESULT SetData(D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect) = 0;
  virtual HRESULT SetDataEnd() = 0;


};

class FFT3dGPU;

class TextureRT :public Texture {
  friend FFT3dGPU;
public:
  TextureRT(LPDIRECT3DDEVICE9 pDevice, int width, int height, Types& Types, HRESULT &hr);
  TextureRT(TextureRT* src, HRESULT &hr);
  ~TextureRT();
  void operator delete(void *p);
  void AddRef();
  void Release();
  HRESULT SetAsRenderTarget(DWORD RenderTargetIndex = 0);
  HRESULT SetAsTexture(DWORD Stage);
  HRESULT GetData(D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect);
  HRESULT GetDataEnd();
  HRESULT SetData(D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect);
  HRESULT SetDataEnd();
  static HRESULT Reset(LPDIRECT3DDEVICE9 pDevice, bool firstpass);
  void CopyTo(IDirect3DSurface9* pDestSurface)
  {
    _pDevice->StretchRect(_pVideoSurface, NULL, pDestSurface, NULL, D3DTEXF_LINEAR);
  }
  IDirect3DSurface9* GetSurface() { return _pVideoSurface; }
  void SaveTex(const char* filename);
protected:
  HRESULT ResetTexture(LPDIRECT3DDEVICE9 pDevice, bool firstpass);
  int SwapToMem();//swap Texture from GPU mem to main mem
  HRESULT SwapToGPU();//swap Texture back to GPU mem
  HRESULT ReadyTexture();//Check if texture is in GPU mem if not move it to there. Move Texture to front in LastUsedList
  static int ReleaseMem(int size); //Release mem by moving infreq used Texture to main memory
  HRESULT CreateVideoTexture();//Create videotexture and free memory if not enough memory is available
  IDirect3DTexture9 *_pShadowTexture;
  IDirect3DSurface9 *_pShadowSurface;
  IDirect3DTexture9 *_pVideoTexture;
  IDirect3DSurface9 *_pVideoSurface;

  //static std::list<TextureRT*> texRTList;

  class texList {
  protected:
    texList *prev;
    texList *next;
    TextureRT* t;
  public:
    texList() :t(0) { prev = this; next = this; };
    texList(TextureRT* texture, texList* list) ://insert int front of list
      t(texture)
    {
      next = list->next;
      prev = list;
      list->next->prev = this;
      list->next = this;
    }
    void Unlink() {
      prev->next = next;
      next->prev = prev;
      prev = this;
      next = this;
    }
    void InsertAtFront(texList* list) {
      prev = list;
      next = list->next;
      list->next->prev = this;
      list->next = this;
    }
    texList* Next() const { return next; }
    texList* Prev() const { return prev; }
    TextureRT* T() const { return t; }
  };

  static texList LastUsedList;
  texList *ListElem;
  bool _SwapedToMem;
  void Remove();
  void MoveToFront();
  void InsertAtFront();
private:
  int _RefCount;
  int ID;

};


class pTextureRTpair {
public:
  pTextureRTpair(TextureRT* _first, TextureRT* _last) :first(_first), last(_last), RefCount(1) {}
  pTextureRTpair() :first(0), last(0), RefCount(1) {}
  ~pTextureRTpair() {
    if (RefCount > 0) {
      if (first)
        first->Release();
      if (last)
        last->Release();
    }

  }
  TextureRT* first;
  TextureRT* last;
  void AddRef() {
    first->AddRef();
    last->AddRef();
    RefCount++;
  }
  void Release() {
    if (first)
      first->Release();
    if (last)
      last->Release();
    RefCount--;
    if (RefCount <= 0) {
      first = NULL;
      last = NULL;
      delete(this);
    }
  }

private:
  int RefCount;
};

class TextureM :public Texture {
public:
  TextureM(LPDIRECT3DDEVICE9 pDevice, int width, int height, Types& Types, HRESULT &hr);
  ~TextureM();
  void operator delete(void *p);
  void AddRef();
  void Release();
  HRESULT SetAsTexture(DWORD Stage);
  HRESULT SetData(D3DLOCKED_RECT *pLockedRect, CONST RECT *pRect);
  HRESULT SetDataEnd();
  void SaveTex(const char* filename);
private:
  int _RefCount;
  int ID;
protected:
  IDirect3DTexture9 *_pVideoTexture;
  //IDirect3DSurface9 *_pVideoSurface;
};



void UploadToTexture(Texture *texture, const void* src, int pitch = 0);
void DownloadFromTexture(TextureRT *texture, void* dst, int pitch = 0, bool IgnoreAlphaChannel = false);

void UploadInterleavedToFixedTexture(Texture *texture, const void* src, int pitch, int srcoffset);
void DownloadFromFixedTextureInterleaved(TextureRT *texture, void* dst, int pitch, bool IgnoreAlphaChannel, int dstoffset);

void Qstart();
void Qend();
#endif