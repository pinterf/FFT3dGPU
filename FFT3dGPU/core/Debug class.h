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


#ifndef DEBUGCLASS
#define DEBUGCLASS

//generate log
//#define USE_LOG
//create PIX profile information
//#define PIXPROFILE
//#define DEBUG_RETVAL



#pragma warning(disable: 4244 4291)
#include <Windows.h>
#include <math.h>
#include <fstream>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include "Dxerr.h"






#ifdef PIXPROFILE
#include "d3dx9.h"
#include "time.h"
// These first two macros are taken from the
// VStudio help files - necessary to convert the
// __FUNCTION__ symbol from char to wchar_t.
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

// Only the first of these macro's should be used. The _INTERNAL
// one is so that the sp##id part generates "sp1234" type identifiers
// instead of always "sp__LINE__"...
#define PROFILE_BLOCK PROFILE_BLOCK_INTERNAL( __LINE__ )
#define PROFILE_BLOCK_INTERNAL(id) D3DUtils::ScopeProfiler sp##id ( WIDEN(__FUNCTION__), __LINE__ );

// To avoid polluting the global namespace,
// all D3D utility functions/classes are wrapped
// up in the D3DUtils namespace.
namespace D3DUtils
{
	class ScopeProfiler
	{
		public:
			ScopeProfiler( WCHAR *Name, int Line );
			~ScopeProfiler( );

		private:
			ScopeProfiler( );
	};
}

#else
#define PROFILE_BLOCK
#define PROFILE_BLOCK_INTERNAL(id)
#endif






#if defined(DEBUG) | defined(_DEBUG) | defined(DEBUG_RETVAL)
//	#define D3D_DEBUG_INFO
    #define V( x )\
        if( FAILED(x) )\
        {\
		DXTrace(__FILE__,__LINE__,x,__FUNCTION__,true);\
        }
    #define V_RETURN( x )\
        if( FAILED(x) )\
        {\
            return DXTrace(__FILE__,__LINE__,x,__FUNCTION__,true);\
        }
#else
    #define V(x)\
        x;
    #define V_RETURN(x)\
    if( FAILED(x) )\
        {\
            return x;\
       }
#endif


#ifdef USE_LOG
#define LOG(a) outfile<<a<<std::endl;
extern std::ofstream outfile;
#else
#define LOG(a)
#endif

#ifdef _DEBUG


//Memory leak detector. Overloads the new operator to display Line and filename of leak if it is a debug build
	#include <crtdbg.h> 

#ifndef _THROW0
#define _THROW0()    throw ()
#endif

#ifndef _THROW1
#define _THROW1(x)    throw (x)
#endif

#ifndef _THROW
#define _THROW(x, y)    throw x(y)
#endif

#ifdef __cplusplus
#include <cstdlib>
#include <new>
#else
#include <stddef.h>
#include <stdint.h>
#endif /* __cplusplus */

/* NAMING PROPERTIES */
#if defined(__cplusplus)
#define _C_LIB_DECL extern "C" {
#define _END_C_LIB_DECL }
#else
#define _C_LIB_DECL
#define _END_C_LIB_DECL
#endif /* __cplusplus */

_C_LIB_DECL
int __cdecl _callnewh(size_t size) throw(std::bad_alloc);
_END_C_LIB_DECL

void *__cdecl operator new(size_t size, const char* file, int line) throw(std::bad_alloc);
//void __cdecl operator delete(void *pUserData,const char* file,int line) throw(std::bad_alloc);

	#define NEW  new(__FILE__,__LINE__)
#else
	#define NEW  new
#endif

template <class CharT, class TraitsT = std::char_traits<CharT> >
class basic_debugbuf : 
    public std::basic_stringbuf<CharT, TraitsT>
{
public:

    virtual ~basic_debugbuf()
    {
        sync();
    }

protected:

    int sync()
    {
        output_debug_string(str().c_str());
        str(std::basic_string<CharT>());    // Clear the string buffer

        return 0;
    }

    void output_debug_string(const CharT *text) {}
};

template<>
void basic_debugbuf<char>::output_debug_string(const char *text)
{
    ::OutputDebugStringA(text);
}

template<>
void basic_debugbuf<wchar_t>::output_debug_string(const wchar_t *text)
{
    ::OutputDebugStringW(text);
}

template<class CharT, class TraitsT = std::char_traits<CharT> >
class basic_dostream : 
    public std::basic_ostream<CharT, TraitsT>
{
public:

    basic_dostream() : std::basic_ostream<CharT, TraitsT>
                (new basic_debugbuf<CharT, TraitsT>()) {}
    ~basic_dostream() 
    {
        delete rdbuf(); 
    }
};

typedef basic_dostream<char>    dostream;
typedef basic_dostream<wchar_t> wdostream;
extern dostream cerrwin;

void Qstart();
void Qend();

double Qendtime();

void WriteMemFixedToFile(const unsigned char* src,int width,int height,int elemsize,const char* SaveFilename,const char* Tittle,bool append);
void WriteMemFloatToFile(float* src,int width,int height,int elemsize,const char* SaveFilename,const char* Tittle,bool append);
#endif