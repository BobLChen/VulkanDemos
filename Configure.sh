#!/bin/bash
if [ ! -d "build" ]; then
    mkdir build
else
    rm -rf build/*
fi
cd build
Local_Dir=$(cd "$(dirname "$0")"; pwd)
echo "Now work at Dir:$Local_Dir"
cmake .. -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF