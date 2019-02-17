#!/bin/bash
if [ ! -d "build" ]; then
    mkdir build
else
    rm -rf build/*
fi
cd build
Local_Dir=$(cd "$(dirname "$0")"; pwd)
echo "Now work at Dir:$Local_Dir"
cmake ..