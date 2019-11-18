#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import os.path
import sys
import struct
import hashlib
import shutil

if __name__ == "__main__":

    for parentDir, _, fileNames in os.walk(os.getcwd()):
        for fileName in fileNames:
            filePath = os.path.join(parentDir, fileName)
            filePath = filePath.replace("\\", "/")
            if filePath.endswith(".iml"):
                os.remove(filePath)
                pass
            pass
        pass

    subDirList = []
    for subDirName in os.listdir(os.getcwd()):
        if os.path.isdir(subDirName):
            subDirList.append(subDirName)
            pass
        pass


    deleteNames = [".externalNativeBuild", "assets", "build"]

    for exampleDir in subDirList:
        for name in deleteNames:
            filepath = os.getcwd() + "/" + exampleDir + "/" + name
            filepath = filepath.replace("\\", "/")
            if os.path.exists(filepath):
                if os.path.isdir(filepath):
                    shutil.rmtree(filepath)
                    pass
                if os.path.isfile(filepath):
                    os.remove(filepath)
                    pass
                pass
            pass
        pass

