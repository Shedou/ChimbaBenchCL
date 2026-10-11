#!/usr/bin/env bash
PathToScript="$( dirname "$(readlink -f "$0")")"
cd "$PathToScript"

wine build-for-windows.bat
