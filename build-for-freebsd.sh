#!/usr/bin/env bash
PathToScript="$( dirname "$(readlink -f "$0")")"
cd "$PathToScript"

gcc src/main.c -o output/ChimbaBenchCL-BSD \
  -DCL_TARGET_OPENCL_VERSION=100 \
  -I./ChimbaBenchCL-Build-Tools/OpenCL-SDK-v2026.05.29-freebsd-x64/include \
  -L./ChimbaBenchCL-Build-Tools/OpenCL-SDK-v2026.05.29-freebsd-x64/lib \
  -lOpenCL

if [ $? -eq 0 ]; then
    echo "Сборка успешно завершена!"
else
    echo "Ошибка компиляции."
fi
