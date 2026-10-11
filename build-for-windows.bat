set "PATH=%~dp0ChimbaBenchCL-Build-Tools\w64devkit-x64-250\bin;%PATH%"
gcc src\main.c -o output\ChimbaBenchCL.exe -DCL_TARGET_OPENCL_VERSION=100 -I.\ChimbaBenchCL-Build-Tools\OpenCL-SDK-v2026.05.29-Win-x64\include -L.\ChimbaBenchCL-Build-Tools\OpenCL-SDK-v2026.05.29-Win-x64\lib -lOpenCL
