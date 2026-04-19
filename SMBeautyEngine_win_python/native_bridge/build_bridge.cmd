@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "PYTHON_DIR=%SCRIPT_DIR%.."
set "SDK_DIR=%PYTHON_DIR%\..\SMBeautyEngine_windows"
set "OUTPUT_DIR=%PYTHON_DIR%\lib"
set "OBJ_DIR=%OUTPUT_DIR%\obj"

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"
if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"

set "VSDEVCMD=D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
if not exist "%VSDEVCMD%" (
  set "VSDEVCMD=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
)
if not exist "%VSDEVCMD%" (
  set "VSDEVCMD=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
)
if not exist "%VSDEVCMD%" (
  echo Visual Studio VsDevCmd.bat not found.
  exit /b 1
)

call "%VSDEVCMD%" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 1

cl /nologo /LD /EHsc ^
  /Fo"%OBJ_DIR%\\" ^
  /I "%SDK_DIR%\include" ^
  /I "%SDK_DIR%\pixelfreeLib\Include" ^
  /I "%SDK_DIR%\third_party\glfw\include" ^
  /I "%SDK_DIR%\third_party\glad\include" ^
  /Fe"%OUTPUT_DIR%\pixel_free_bridge.dll" ^
  "%SCRIPT_DIR%\pixel_free_bridge.cpp" ^
  "%SDK_DIR%\third_party\glad\src\glad.cc" ^
  "%SDK_DIR%\src\opengl.cpp" ^
  /link ^
  "%SDK_DIR%\third_party\glfw\glfw3.lib" ^
  "%SDK_DIR%\pixelfreeLib\PixelFree.lib" ^
  opengl32.lib glu32.lib gdi32.lib user32.lib kernel32.lib dbghelp.lib shell32.lib
