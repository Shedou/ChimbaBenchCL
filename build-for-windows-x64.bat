@echo off
cd /d "%~dp0"
set "OutputDir=%~dp0output"

echo Checking output dir...
if not exist "%OutputDir%" mkdir "%OutputDir%"

echo Set paths...
set "PATH=%~dp0ChimbaBenchCL-Build-Tools\w64devkit-x64-250\bin;%PATH%"

echo Compiling...
gcc src\main.c -o "%OutputDir%\ChimbaBenchCL.exe" -DCL_TARGET_OPENCL_VERSION=100 -I.\ChimbaBenchCL-Build-Tools\OpenCL-SDK-v2026.05.29-Win-x64\include -L.\ChimbaBenchCL-Build-Tools\OpenCL-SDK-v2026.05.29-Win-x64\lib -lOpenCL

if %errorlevel% equ 0 (
	echo Complete!
) else (
	echo Error
)
