#define DIRECTINPUT_VERSION  0x0800
#include "Dinput.h"

class Dxinput {
public:
	Dxinput(HINSTANCE hmodule,HWND hwnd);
	~Dxinput();
protected:
	LPDIRECTINPUT8  g_lpDI; 
	LPDIRECTINPUTDEVICE8 g_pMouse; 
	LPDIRECTINPUTDEVICE8  g_lpDIDevice;
	HANDLE g_hMouseEvent; 
	DIPROPDWORD dipdw;

	HWND g_hwndMain;

};