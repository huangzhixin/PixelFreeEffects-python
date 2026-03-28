@echo off
setlocal

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 1

cl /nologo /LD /EHsc ^
  /I D:\LinearX\PixelFreeEffects\SMBeautyEngine_windows\include ^
  /I D:\LinearX\PixelFreeEffects\SMBeautyEngine_windows\pixelfreeLib\Include ^
  /I D:\LinearX\PixelFreeEffects\SMBeautyEngine_windows\third_party\glfw\include ^
  /I D:\LinearX\PixelFreeEffects\SMBeautyEngine_windows\third_party\glad\include ^
  /FeD:\LinearX\PixelFreeEffects\SMBeautyEngine_win_python\lib\pixel_free_bridge.dll ^
  D:\LinearX\PixelFreeEffects\SMBeautyEngine_win_python\native_bridge\pixel_free_bridge.cpp ^
  D:\LinearX\PixelFreeEffects\SMBeautyEngine_windows\third_party\glad\src\glad.cc ^
  D:\LinearX\PixelFreeEffects\SMBeautyEngine_windows\src\opengl.cpp ^
  /link ^
  D:\LinearX\PixelFreeEffects\SMBeautyEngine_windows\third_party\glfw\glfw3.lib ^
  D:\LinearX\PixelFreeEffects\SMBeautyEngine_windows\pixelfreeLib\PixelFree.lib ^
  opengl32.lib glu32.lib gdi32.lib user32.lib kernel32.lib dbghelp.lib shell32.lib
