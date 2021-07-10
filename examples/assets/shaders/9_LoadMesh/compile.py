# coding: utf-8

import os
import sys

def IsExe(path):
    return os.path.isfile(path) and os.access(path, os.X_OK)

def FindGlslang():
    exeName = "glslangvalidator"
    if os.name == "nt":
        exeName += ".exe"
    
    for exeDir in os.environ["PATH"].split(os.pathsep):
        fullPath = os.path.join(exeDir, exeName)
        if IsExe(fullPath):
            return fullPath

    sys.exit("Could not find glslangvalidator on PATH.")

files = []

for parentDir, _, fileNames in os.walk(os.getcwd()):
	for fileName in fileNames:
		filepath = os.path.join(parentDir, fileName)
		files.append(filepath)
pass

shaders = [".vert", ".frag", ".comp", ".tese", ".tesc", ".geom", ".rgen", ".rchit", ".rmiss", ".rahit"]
shaderFiles = []
glslangPath = FindGlslang()

for file in files:
	_, ext = os.path.splitext(file)
	ext = ext.lower()
	if ext in shaders:
		shaderFiles.append(file.replace("\\", "/"))
	pass

for shader in shaderFiles:
	os.system(glslangPath + " -V " + shader + " -o " + shader + ".spv")
	pass