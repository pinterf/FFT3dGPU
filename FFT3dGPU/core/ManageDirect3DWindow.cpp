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


#include "windows.h"
#include "../stdafx.h"
#include "Debug class.h"
#include "pixelshader.h"
#include "ManageDirect3DWindow.h"	

//#define USE_REF

#ifndef USE_REF
#define DEVICETYPE D3DDEVTYPE_HAL
#else
#define DEVICETYPE D3DDEVTYPE_REF
#endif

//function to get dxversion
extern HRESULT GetDXVersion( DWORD* pdwDirectXVersion, TCHAR* strDirectXVersion, int cchDirectXVersion );




ManageDirect3DWindow::ManageDirect3DWindow():
pDevice(0),hWnd(0),pD3D(0),
wc(),
d3dpp(),
gtype(0),einitdone(CreateEvent(0,false,false,0)),
refcount(0),
pQuery(0),
sceneCount(0)
{

	
}

void ManageDirect3DWindow::Init()
{
	
		WNDCLASSEX _wc= { sizeof(WNDCLASSEX), CS_CLASSDC, ManageDirect3DWindow::MsgProc, 0L, 0L,GetModuleHandle(NULL), NULL, NULL, NULL, NULL,"FFT3dGPU", NULL };
	wc=_wc;
	messagethread
		=
	CreateThread(NULL, 0, 
            (LPTHREAD_START_ROUTINE) ManageDirect3DWindow::MessageLoopWrapper, 
            this,  // pass parameters
            0, 0); 
	WaitForSingleObject(einitdone,INFINITE);
}

void ManageDirect3DWindow::BeginScene()
{
	if(sceneCount==0)
		pDevice->BeginScene();
	sceneCount++;
}

void ManageDirect3DWindow::EndScene()
{
	sceneCount--;
	if(sceneCount==0)
		pDevice->EndScene();
}
ManageDirect3DWindow::~ManageDirect3DWindow()
{
	CloseHandle(einitdone);
}

//CreateThread doesn't support non static class functions so this fixes that problem
VOID ManageDirect3DWindow::MessageLoopWrapper(ManageDirect3DWindow* _this)
{
	_this->MessageLoop();
}


bool ManageDirect3DWindow::DestroyDevice()
{
		if(gtype)
			delete gtype;
		gtype=0;
		if(pQuery)
			pQuery->Release();
		if(pDevice)
			pDevice->Release();
		pDevice=0;
		if(pD3D)
			pD3D->Release();
		pD3D=0;
		UnregisterClass( "FFT3dGPU", wc.hInstance );
		return true;
}


LPDIRECT3DDEVICE9 ManageDirect3DWindow::GetDirectDirect3DDevice()
{
	if(refcount++<=0)
	{
		Init();
	}
	if(pDevice)
		pDevice->AddRef();
	return pDevice;
}

void ManageDirect3DWindow::ReleaseDirect3DDevice()
{
    if(pDevice)
		pDevice->Release();
	if(--refcount<=0)
	{
	if(hWnd)
	{
		SendMessage(hWnd,EXIT,0,0);
		WaitForSingleObject(messagethread,15000);
	}
	else
		TerminateThread(messagethread,-1);
	CloseHandle(messagethread);
	}

}

HWND ManageDirect3DWindow::GetWindow()
{
	return hWnd;
}

GPUTYPES* ManageDirect3DWindow::GetGPUType()
{
	return gtype;
}

bool ManageDirect3DWindow::InitDevice()
{
//test Directx version (must be at least version 9.0c)
	HRESULT result;
	DWORD dwDirectXVersion = 0;
	TCHAR strDirectXVersion[10];

	if(SUCCEEDED(GetDXVersion( &dwDirectXVersion, strDirectXVersion, 10 )))
#ifndef DX9b
		if(dwDirectXVersion<0x00090003)
		{
			MessageBox(NULL,"FFT3dGPU need at least DirectX version 9.0c installed","FFT3dGPU",MB_OK);
			return false;
		}
#else
		if(dwDirectXVersion<0x00090002)
		{
			MessageBox(NULL,"FFT3dGPU need at least DirectX version 9.0b installed","FFT3dGPU",MB_OK);
			return false;
		}
#endif
		RegisterClassEx( &(ManageDirect3DWindow::wc) );
		ManageDirect3DWindow::hWnd = CreateWindow( "FFT3dGPU", "FFT3dGPU Debug",
		WS_OVERLAPPEDWINDOW , 100, 100, 1024, 768,
		GetDesktopWindow(), NULL, ManageDirect3DWindow::wc.hInstance, NULL );
		pD3D = Direct3DCreate9( D3D_SDK_VERSION );
		if(!pD3D){
			MessageBox(NULL,"Failed to create D3D","fft3dgpu",MB_OK);
			return false;
		}
		//D3DPRESENT_PARAMETERS d3dpp;
		//setup device options
		ZeroMemory( &d3dpp, sizeof(d3dpp) );
		d3dpp.BackBufferHeight=1024;
		d3dpp.BackBufferWidth=768;
		d3dpp.BackBufferCount=1;
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.EnableAutoDepthStencil = FALSE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;

		UINT AdapterToUse=D3DADAPTER_DEFAULT;
		D3DDEVTYPE DeviceType=DEVICETYPE;


		for (UINT Adapter=0;Adapter<pD3D->GetAdapterCount();Adapter++)
			{
				D3DADAPTER_IDENTIFIER9 Identifier;
				HRESULT Res=pD3D->GetAdapterIdentifier(Adapter,0,&Identifier);
				if (strcmp(Identifier.Description,"NVIDIA NVPerfHUD")==0)
				{
					AdapterToUse=Adapter;
					DeviceType=D3DDEVTYPE_REF;
					break;
				}
			}
				result=pD3D->CreateDevice( 
				AdapterToUse,
				DeviceType		, 
				ManageDirect3DWindow::hWnd,
				D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_FPU_PRESERVE|D3DCREATE_MULTITHREADED|D3DCREATE_PUREDEVICE,	//no need for D3DCREATE_MULTITHREADED when handling the lock.
				&d3dpp, &pDevice );

			if(FAILED(result))
			{DXTrace( "texture.cpp" ,__LINE__ ,result,"Creating Device",true);return false;}
			pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
			pDevice->SetRenderState(  D3DRS_ZFUNC, D3DCMP_LESS );
			for(int i=0;i<6;i++){
				pDevice->SetSamplerState(i,D3DSAMP_ADDRESSU,D3DTADDRESS_BORDER);
				pDevice->SetSamplerState(i,D3DSAMP_ADDRESSV,D3DTADDRESS_BORDER);
				pDevice->SetSamplerState(i,D3DSAMP_BORDERCOLOR,0x00000000/*0x00808080*/);
			}

			gtype=NEW GPUTYPES(pDevice);
			pDevice->CreateQuery(D3DQUERYTYPE_EVENT, &pQuery);
			return true;

}

void ManageDirect3DWindow::Show()
{
	ShowWindow(hWnd,SW_SHOW );
}

bool ManageDirect3DWindow::DeviceReady()
{
	return  D3D_OK==SendMessage(hWnd,TESTCOOPERATIVELEVEL,(WPARAM)this,0);
}

bool ManageDirect3DWindow::ResetDevice()
{
	HRESULT hr=SendMessage(hWnd,TESTCOOPERATIVELEVEL,(WPARAM)this,0);
//	for(int i=0;i<120&&hr!=D3DERR_DEVICENOTRESET;i++) //We got 2 min. to get the device back
while(hr!=D3DERR_DEVICENOTRESET&&hr!=D3D_OK)
	{
		Sleep(100);
		hr=SendMessage(hWnd,TESTCOOPERATIVELEVEL,(WPARAM)this,0);
	}
	if(hr==D3D_OK)
		return true;
	if(hr!=D3DERR_DEVICENOTRESET)
	{
		DXTrace( "FFT3dGPU.cpp" ,__LINE__ ,hr,"Reset Device",true);
		return false;//timeout or internal driver error
	}
	
	//Ready do reset...
	//Destroy Default_pool allocated objects (Render textures)
	hr=TextureRT::Reset(pDevice,true);
    if(FAILED(hr))
		return false;

	hr=Pixelshader::Reset(pDevice,true);
	if(FAILED(hr))
		return false;
	if(pQuery)
		pQuery->Release();
	hr=SendMessage(hWnd,RESETDEVICE,(WPARAM)this,0);
	if(FAILED(hr))
	{
		DXTrace( "FFT3dGPU.cpp" ,__LINE__ ,hr,"Reset Device",true);
		return false;
	}
	//Recreate them
	if(FAILED(hr=TextureRT::Reset(pDevice,false)))
	{
		DXTrace( "FFT3dGPU.cpp" ,__LINE__ ,hr,"Reset Device",true);
		return false;
	}
	pDevice->CreateQuery(D3DQUERYTYPE_EVENT, &pQuery);
	return(SUCCEEDED(Pixelshader::Reset(pDevice,false)));


}

LRESULT WINAPI ManageDirect3DWindow::MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case EXIT:
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		//Cleanup();
		LOG("WM_DESTROY recieved");
		PostQuitMessage( 0 );
		return 0;
	case WM_CLOSE:
		//if trying to close the window minimize it instead.
		ShowWindow( hWnd, SW_MINIMIZE );
		return 0;
	case RESETDEVICE:
		{
		ManageDirect3DWindow* _this=(ManageDirect3DWindow*)wParam;
		return _this->pDevice->Reset(&(_this->d3dpp));
		}
	case TESTCOOPERATIVELEVEL:
		{
		ManageDirect3DWindow* _this=(ManageDirect3DWindow*)wParam;
		return _this->pDevice->TestCooperativeLevel();
		}
/*	case WM_KEYUP:
		MessageBox(NULL,"hallo","halloooo",MB_OK);
		return 0;
*/
	}
	return DefWindowProc( hWnd, msg, wParam, lParam );
}

void ManageDirect3DWindow::MessageLoop()
{
	InitDevice();
	SetEvent(einitdone);
	MSG msg;
	while(WaitMessage())
	{
	if( GetMessage( &msg, hWnd, 0U, 0U )!=0 )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
	else
		break;
	}
	DestroyDevice();
	ExitThread(0);
}




void ManageDirect3DWindow::StartGPU()
{
	PROFILE_BLOCK
	LOG("Reduce CPU");
	//Flushes the command queue on the GPU
	LOG("ISSUE_END...")
		pQuery->Issue(D3DISSUE_END);
	LOG("done")
	LOG("GETDATA Flush...")
		pQuery->GetData( NULL, 0, D3DGETDATA_FLUSH );
	LOG("done")
	
}
void ManageDirect3DWindow::SleepWhileGPUwork()
{
	PROFILE_BLOCK
	//While GPU working sleep
	DWORD endtick=GetTickCount()+10000;//assure that we don't sleep for more than 10 sec.
	LOG("sleep while query not flushed...")
	while((S_FALSE == pQuery->GetData( NULL, 0, D3DGETDATA_FLUSH ))&&(GetTickCount()<endtick))
		Sleep(1);
	LOG("done")
}

