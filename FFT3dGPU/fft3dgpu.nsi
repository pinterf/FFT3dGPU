; example2.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install example2.nsi into a directory that the user selects,

!define VERSION "0.8.2"
!system 'd:\programmer\7-Zip\7z.exe a -t7z "fft3dgpu${VERSION}.7z" fft3dgpu.dll -m0=BCJ2 -m1=LZMA:d25:mf=pat4h -m2=LZMA:d22:mf=pat4h -m3=LZMA:d22:mf=pat4h -mb0:1 -mb0s1:2 -mb0s2:3 -mx=9 -mmt=on'
!system 'd:\programmer\7-Zip\7z.exe a -t7z "fft3dgpu${VERSION}.7z" fft3dgpu.hlsl fft3dgpu.htm -m0=PPMd -mx=9 -mmt=on'


 SetCompressor /SOLID lzma
;--------------------------------

; The name of the installer
Name "FFT3dGPU ${VERSION}"

; The file to write
OutFile "fft3dgpu${VERSION}.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Avisynth 2.5\plugins"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\AviSynth" "plugindir2_5"

LicenseData "Copying.txt"
;--------------------------------

; Pages

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Install fft3dgpu"

  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "fft3dgpu.dll"
  File "fft3dgpu.hlsl"
  File "fft3dgpu.htm"
 
  SetOutPath $SYSDIR
   SetOverwrite off
  File "d3dx9_30.dll"
   SetOverwrite on
  
SectionEnd

; Optional section (can be disabled by the user)
Section /o "Install sourcecode"

  CreateDirectory "$INSTDIR\fft3dgpu_src"
  SetOutPath "$INSTDIR\fft3dgpu_src"


  File "avisynth.h"
  File "Dxinput.cpp"
  File "Dxinput.h"
  File "FFT.cpp"
  File "FFT.h"
  File "FFT2.cpp"
  File "FFT2.h"
  File "FFTps.cpp"
  File "FFTps.h"
  File "FFT2ps.cpp"
  File "FFT2ps.h"
  File "Filters.cpp"
  File "Filters.h"
  File "Filtersps.cpp"
  File "Filtersps.h"
  File "GPUCache.h"
  File "GPUCache.cpp" 
  File "resource.h"
  File "TexturePool.h"
  File "TexturePool.cpp"
  File "stdafx.cpp"
  File "stdafx.h"
  File "fft3dgpu.rc"
  File "fft3dgpu.cpp"
  File "fft3dgpu.h"
  File "Copying.txt"
  File "fft3dgpu.nsi"
  File "FFT3dGPU.vcproj"

  CreateDirectory "$INSTDIR\fft3dgpu_src\core"
  SetOutPath "$INSTDIR\fft3dgpu_src\core"
  File "core\*.*"
  
  
SectionEnd


