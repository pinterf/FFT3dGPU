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

#define RESETDEVICE WM_USER + 0
#define TESTCOOPERATIVELEVEL WM_USER + 1
#define EXIT WM_USER + 2



class ManageDirect3DWindow {
public:
  ManageDirect3DWindow();
  ~ManageDirect3DWindow();
  LPDIRECT3DDEVICE9 GetDirectDirect3DDevice();
  void ReleaseDirect3DDevice();
  bool ResetDevice();
  bool DeviceReady();
  void Show();
  void StartGPU();
  void SleepWhileGPUwork();
  void BeginScene();
  void EndScene();
  HWND GetWindow();
  GPUTYPES* GetGPUType();
protected:
  void Init();
  LPDIRECT3DDEVICE9 pDevice;
  IDirect3DQuery9* pQuery;
  HWND hWnd;
  LPDIRECT3D9 pD3D;
  WNDCLASSEX wc;
  D3DPRESENT_PARAMETERS d3dpp;
  static VOID MessageLoopWrapper(ManageDirect3DWindow* _this);
  void MessageLoop();
  static LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
  bool InitDevice();
  bool DestroyDevice();
  GPUTYPES* gtype;
  HANDLE messagethread;
  HANDLE einitdone;
  bool init;
  int sceneCount;
  int refcount;
};
