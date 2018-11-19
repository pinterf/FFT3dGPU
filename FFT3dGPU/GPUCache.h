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

class GPUCacheNode {
public:
  GPUCacheNode(TextureRT *stream1, TextureRT *stream2, int n, TexturePool *StreamPool) :_next(0), _stream1(stream1), _stream2(stream2), _n(n), _StreamPool(StreamPool) { _stream1->AddRef(); _stream2->AddRef(); }
  GPUCacheNode(TextureRT *stream, int n, TexturePool *StreamPool) :_next(0), _stream1(stream), _stream2(0), _n(n), _StreamPool(StreamPool) { _stream1->AddRef(); }
  GPUCacheNode* next() { return _next; }
  void next(GPUCacheNode* next) { _next = next; }
  int n() { return _n; }
  void stream(TextureRT* &texture1, TextureRT* &texture2) { texture1 = _stream1; texture2 = _stream2; }
  void stream(TextureRT* &texture) { texture = _stream1; }
  ~GPUCacheNode() { /*delete _stream;*/ _StreamPool->push(_stream1); if (_stream2)_StreamPool->push(_stream2); }
private:
  GPUCacheNode* _next;
  TextureRT* _stream1;
  TextureRT* _stream2;
  int _n;
  TexturePool *_StreamPool;
};


class GPUCache {
public:
  GPUCache(int cachesize);
  //~GPUCache(){deleteall(List);}
  ~GPUCache();
  //bool InCache(int n);
  void StreamPoolPointer(TexturePool *StreamPool) { _StreamPool = StreamPool; }
  void AddtoCache(int n, TextureRT *stream);
  void AddtoCache(int n, pTextureRTpair *stream);
  void FlushCache();
  static void FlushAll();
  bool GetStream(int n, TextureRT* &texture);
  bool GetStream(int n, pTextureRTpair* texture);
private:
  int _cachesize;
  int _maxcachesize;
  GPUCacheNode* List;
  TexturePool *_StreamPool;

  bool deletelast(GPUCacheNode* node);
  void deleteall(GPUCacheNode* node);
  static GPUCache* first;
  GPUCache* prev;
  GPUCache* next;
};
