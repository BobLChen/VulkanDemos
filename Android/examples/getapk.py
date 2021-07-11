#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import os.path
import sys
import struct
import hashlib
import shutil

if __name__ == "__main__":

    filelist = []

    for parentDir, _, fileNames in os.walk(os.getcwd()):
        for fileName in fileNames:
            filepath = os.path.join(parentDir, fileName)
            filepath = filepath.replace("\\", "/")
            if filepath.endswith(".apk") and "debug" in filepath:
                filelist.append(filepath)
                pass
            pass
        pass

    for filepath in filelist:
        srcPath = filepath
        dstPath = os.getcwd() + "/" + "apks/" + os.path.split(srcPath)[1]
        srcPath = srcPath.replace("\\", "/")
        dstPath = dstPath.replace("\\", "/")
        shutil.copyfile(srcPath, dstPath)
        pass

