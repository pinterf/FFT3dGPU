#include "Dxinput.h"
#define SAMPLE_BUFFER_SIZE  16


Dxinput::Dxinput(HINSTANCE hmodule, HWND hwnd) :
  g_hwndMain(hwnd)
{
  HINSTANCE  g_hinst = hmodule;
  HRESULT         hr;

  hr = DirectInput8Create(g_hinst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_lpDI, NULL);
  //setup keyboard
  hr = g_lpDI->CreateDevice(GUID_SysKeyboard, &g_lpDIDevice, NULL);
  hr = g_lpDIDevice->SetDataFormat(&c_dfDIKeyboard);
  hr = g_lpDIDevice->SetCooperativeLevel(g_hwndMain, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
  if (g_lpDIDevice) hr = g_lpDIDevice->Acquire();
  //setup mouse
  hr = g_lpDI->CreateDevice(GUID_SysMouse, &g_pMouse, NULL);
  hr = g_pMouse->SetDataFormat(&c_dfDIMouse);
  hr = g_pMouse->SetCooperativeLevel(g_hwndMain, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

  g_hMouseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  hr = g_pMouse->SetEventNotification(g_hMouseEvent);

  // the header
  dipdw.diph.dwSize = sizeof(DIPROPDWORD);
  dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
  dipdw.diph.dwObj = 0;
  dipdw.diph.dwHow = DIPH_DEVICE;
  // the data
  dipdw.dwData = SAMPLE_BUFFER_SIZE;

  hr = g_pMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

  hr = g_pMouse->Acquire();

}

Dxinput::~Dxinput() {
  //g_lpDIDevice->Release();//first Acquire
  g_lpDIDevice->Release();//final release
  g_pMouse->Release();
  g_lpDI->Release();//final release
  CloseHandle(g_hMouseEvent);
}
