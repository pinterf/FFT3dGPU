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

#include "./core/texture.h"
#include <stack>

class TexturePool {
public:
  TexturePool(LPDIRECT3DDEVICE9 pDevice, int width, int height, Types& type);
  ~TexturePool();
  TextureRT* top();
  void pop();
  void pop(TextureRT* &texture);
  void pop(pTextureRTpair &texture);
  void push(TextureRT* &texture);
  void push(pTextureRTpair &texture);
protected:
  std::stack<TextureRT*> TextureStack;
  Types _type;
  int _height;
  int _width;
  LPDIRECT3DDEVICE9 _pDevice;
};

