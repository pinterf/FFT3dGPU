# FFT3dGPU
FFT3dGPU

An attempt to compile FFT3dGPU under Visual Studio 2017 with current Avisynth+ headers for x86/x64.

Changelog (pinterf)
**v0.8.5 (20201101)**
- Avisynth+ 3.6 additional support: preserve frame properties
- Fix bt=0 Kalman mode warning in fft3dgpu.hlsl (Nuihc88) 

**v0.8.4 (20181121)**
- New color spaces besides YV12 and YUY2:
  Y8, YV16, YV411, YV24 (all 8 bits), 8 bit Planar RGB (Avisynth+)
- FFT3dGPU filter registers itself as MT_SERIALIZED automatically for Avisynth+: no need for SetFilterMTMode.

**v0.8.3 (20181118)**
- project hosted on https://github.com/pinterf/FFT3dGPU
- source and project files updated to build under Visual Studio 2017 (15.9) using DX SDK June 2010
- Moved to Avisynth Interface 6 (v2.6) - using headers from the Avisynth+ project
- added x64 platform

**v0.8.2 (x86: 2006, x64: 2010)**
- Initial source: 0.8.2

Forum
=====
https://forum.doom9.org/showthread.php?t=89941

Known sources of the plugin version 0.8.2
=========================================
x86: 
http://www.avisynth.nl/index.php/External_filters#Spatio-Temporal_Denoisers
http://www.avisynth.nl/users/tsp/fft3dgpu0.8.2.exe
http://avisynth.nl/users/tsp/fft3dgpu0.8.2.7z

And the latest (non-existant) one:
This one is non-existant (it is said to have the missing 4 files: filtersps.h, filtersps.cpp, dxinput.h, dxinput.cpp):
tsp provided them (https://forum.doom9.org/showthread.php?p=1102862#post1102862) in an updated version:
http://www.avisynth.org/tsp/fft3dgpu0.8.2a.exe
bit it disappeared from the site

x64: (binary only, dll and hlsl)
http://avisynth.nl/index.php/AviSynth%2B_x64_plugins
https://www.mediafire.com/download/2chnt1jkwwm/FFT3DGPU_3-15-2010.rar
Comment:
The HLSL (shader program) file is edited from the original to adhere to pixel shader 3.0 syntax rules. 
Please make sure to place the correct file in the same directory as the 64bit plugin. Compiled by Joshy D.

Ultim's comment:
================
2nd March 2014, 19:00
fft3dgpu sometimes requests negative frames without any reason (e.g. when seeking to a positive frame far from zero), which some other filters do no happen to like (for somewhat understandable reasons). See https://github.com/AviSynth/AviSynthPlus/issues/38 .
In case anybody picks up development, please have a look at this behavior and correct if possible.

Build Hints
===========
1.) Download DirectX 9 SDK (June 2010)
https://www.microsoft.com/en-us/download/details.aspx?id=6812

2.) Install it!
When problem occurs (Setup failed) at the end of the installation:
"S1023" error when you install the DirectX SDK (June 2010)
e.g. you have VC2010 Redist version 10.0.40249, while the setup wants to install 10.0.40219

Here is the help (in nutshell: uninstall vs2010 redist, install SDK, install newest redist)
https://support.microsoft.com/en-us/help/2728613/s1023-error-when-you-install-the-directx-sdk-june-2010

Resolution
----------
To resolve this issue, you must uninstall all versions of the Visual C++ 2010 Redistributable before installing the June 2010 DirectX SDK. You may have one or more of the following products installed:

    Microsoft Visual C++ 2010 x86 Redistributable
    Microsoft Visual C++ 2010 x64 Redistributable

After uninstalling the Microsoft Visual C++ 2010 Redistributable products, you may install the June 2010 DirectX SDK.

After installing the June 2010 DirectX SDK, you may then reinstall the most current version of the Visual C++ 2010 Redistributable Package. 