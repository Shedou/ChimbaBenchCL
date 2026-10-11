#!/usr/bin/env bash
PathToScript="$( dirname "$(readlink -f "$0")")"
cd "$PathToScript"

gcc src/main.c -o output/ChimbaBenchCL-Linux \
  -DCL_TARGET_OPENCL_VERSION=100 \
  -I./ChimbaBenchCL-Build-Tools/OpenCL-SDK-v2026.05.29-Lin-CL-x64/include \
  -L./ChimbaBenchCL-Build-Tools/OpenCL-SDK-v2026.05.29-Lin-CL-x64/lib \
  -Wl,--wrap=__libc_start_main \
  -lOpenCL

if [ $? -eq 0 ]; then
    echo "Complete!"
else
    echo "Error"
fi
