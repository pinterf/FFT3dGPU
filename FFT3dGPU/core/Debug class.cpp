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


#include "Debug class.h"
dostream cerrwin;

void WriteMemFloatToFile(float* src, int width, int height, int elemsize, const char* SaveFilename, const char* Tittle, bool append) {
  std::ofstream outfile(SaveFilename, (append ? std::ios_base::app : std::ios_base::out));
  outfile << std::endl << std::endl << std::endl << src << std::endl << Tittle << std::endl;
  for (int j = 0, offset = 0; j < height; j++) {
    outfile << std::endl;
    for (int i = 0; i < width*elemsize; i += 1) {
      outfile << std::showpos << std::fixed << std::setprecision(0) << std::setw(4) << src[offset++] * 255 << ((i + 1) % elemsize ? " " : "   ");
      /*
          for(int i=0;i<width*elemsize;i+=4){
            outfile <<std::showpos<<std::fixed<<std::setprecision(4)<<std::setw(8)<<(src[offset]*src[offset]+src[offset+2]*src[offset+2])<<((i+1)%elemsize?" ":"   ");
            offset+=4;
            if(!((i+1)%20))
              outfile <<"    ";*/
    }
  }
}

void WriteMemFixedToFile(const unsigned char* src, int width, int height, int elemsize, const char* SaveFilename, const char* Tittle, bool append) {
  std::ofstream outfile(SaveFilename, (append ? std::ios_base::app : std::ios_base::out));
  outfile << std::endl << std::endl << std::endl << src << std::endl << Tittle << std::endl;
  for (int j = 0, offset = 0; j < height; j++) {
    outfile << std::endl;
    for (int i = 0; i < width*elemsize; i += 1) {
      outfile << std::showpos << std::setw(8) <</*static_cast<int>*/(sqrt((float)src[offset++] / 255)) << ((i + 1) % elemsize ? " " : "   ");
    }
  }
}

void WriteMemFixed16ToFile(const uint16_t* src, int width, int height, int elemsize, const char* SaveFilename, const char* Tittle, bool append) {
  std::ofstream outfile(SaveFilename, (append ? std::ios_base::app : std::ios_base::out));
  outfile << std::endl << std::endl << std::endl << src << std::endl << Tittle << std::endl;
  for (int j = 0, offset = 0; j < height; j++) {
    outfile << std::endl;
    for (int i = 0; i < width * elemsize; i += 1) {
      outfile << std::showpos << std::setw(8) <</*static_cast<int>*/(sqrt((float)src[offset++] / 65535)) << ((i + 1) % elemsize ? " " : "   ");
    }
  }
}

#ifdef _DEBUG
// Remark for C4290 warning:
// A function is declared using exception specification, which Visual C++ accepts but does not 
// implement.Code with exception specifications that are ignored during compilation may need to 
// be recompiled and linked to be reused in future versions supporting exception specifications.
void *__cdecl operator new(size_t size, const char* file, int line) throw(std::bad_alloc)
{	// try to allocate size bytes
  void *p;
  while ((p = _malloc_dbg(size, _NORMAL_BLOCK, file, line)) == 0)
    if (_callnewh(size) == 0)
      std::bad_alloc();
  return (p);
}



/*void operator delete(
        void *pUserData,
    const char* file,
    int line
        )
{
    delete(pUserData);


        return;
}*/

#endif

LARGE_INTEGER timerB;
LARGE_INTEGER timerA;
LARGE_INTEGER clockpersec;
int _s_init_clockpersec = QueryPerformanceFrequency(&clockpersec);

void Qstart()
{
  QueryPerformanceCounter(&timerB);
}

double Qendtime()
{
  QueryPerformanceCounter(&timerA);
  LARGE_INTEGER diff;
  diff.QuadPart = timerA.QuadPart - timerB.QuadPart;
  double t = (double)diff.QuadPart / (double)clockpersec.QuadPart;
  return t;
}

void Qend()
{
  QueryPerformanceCounter(&timerA);
  LARGE_INTEGER diff;
  diff.QuadPart = timerA.QuadPart - timerB.QuadPart;
  double t = (double)diff.QuadPart / (double)clockpersec.QuadPart;

  unsigned int time = timerA.HighPart - timerB.HighPart;
  char ctime[100];
  MessageBox(NULL, _gcvt(t, 6, ctime), "time in sec", MB_OK);
  /*if(time){
    itoa
    itoa(time,ctime,10);
    _gcvt(t,16,ctime)
    MessageBox(NULL,ctime,"timerHigh",MB_OK);
  }
  else{
    time=timerA.LowPart-timerB.LowPart;
    itoa(time,ctime,10);
    MessageBox(NULL,ctime,"timer",MB_OK);
  }*/
}


#ifdef USE_LOG
#include <fstream>
std::ofstream outfile("C://FFT3dGPU_log.txt");
#endif

#ifdef PIXPROFILE
#include "strsafe.h"
namespace D3DUtils
{
  // Class constructor. Takes the necessary information and
  // composes a string that will appear in PIXfW.
  ScopeProfiler::ScopeProfiler(WCHAR* Name, int Line)
  {
    WCHAR wc[MAX_PATH];
    StringCchPrintfW(wc, MAX_PATH, L"%s @ Line %d.\0", Name, Line);
    D3DPERF_BeginEvent(D3DCOLOR_XRGB(rand() % 255, rand() % 255, rand() % 255), wc);
    srand(static_cast<unsigned>(time(NULL)));
  }

  // Makes sure that the BeginEvent() has a matching EndEvent()
  // if used via the macro in D3DUtils.h this will be called when
  // the variable goes out of scope.
  ScopeProfiler::~ScopeProfiler()
  {
    D3DPERF_EndEvent();
  }
}
#endif