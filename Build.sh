#!/bin/bash
if [ ! -d "build" ]; then
    echo "Configure fist"
    exit
else
    cd build
fi
Local_Dir=$(cd "$(dirname "$0")"; pwd)
echo "Now work at Dir:$Local_Dir"
make