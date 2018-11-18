# FFT3dGPU
FFT3dGPU

An attempt to compile FFT3dGPU under Visual Studio 2017 with current (2018) Avisynth+ headers.

Forum
=====
https://forum.doom9.org/showthread.php?t=89941

Known sources of the plugin
===========================
x86 (2006): 
http://www.avisynth.nl/index.php/External_filters#Spatio-Temporal_Denoisers
http://www.avisynth.nl/users/tsp/fft3dgpu0.8.2.exe (missing 4 files: filtersps.h, filtersps.cpp, dxinput.h, dxinput.cpp)
http://avisynth.nl/users/tsp/fft3dgpu0.8.2.7z
http://www.avisynth.org/tsp/fft3dgpu0.8.2a.exe (with filtersps.h, filtersps.cpp, dxinput.h, dxinput.cpp)
the link above is dead, but it was saved for the future: https://forum.doom9.org/showthread.php?p=1857809#post1857809

x64: (2010 build for avs2.5 x64, binary only, dll and hlsl)
http://avisynth.nl/index.php/AviSynth%2B_x64_plugins
https://www.mediafire.com/download/2chnt1jkwwm/FFT3DGPU_3-15-2010.rar
Comment:
The HLSL (shader program) file is edited from the original to adhere to pixel shader 3.0 syntax rules. 
Please make sure to place the correct file in the same directory as the 64bit plugin. Compiled by Joshy D.
