# coding: utf-8

import os
import sys

path = os.getcwd()
path = path.replace("\\", "/")
path = path[0:path.find("VulkanTutorials")]
path = path + "/VulkanTutorials/"

exepath = path

if "win32" == sys.platform:
	exepath = exepath + "external/vulkan/windows/bin/x86/glslangvalidator.exe"
	pass
elif "linux" == sys.platform:
	exepath = exepath + "external/vulkan/linux/bin/glslangValidator"
	pass
elif "linux2" == sys.platform:
	exepath = exepath + "external/vulkan/linux/bin/glslangValidator"
	pass
elif "darwin" == sys.platform:
	exepath = exepath + "external/vulkan/macos/bin/glslangValidator"
	pass

files = []

for parentDir, _, fileNames in os.walk(os.getcwd()):
	for fileName in fileNames:
		filepath = os.path.join(parentDir, fileName)
		files.append(filepath)
pass

shaders = [".vert", ".frag", ".comp", ".tese", ".tesc", ".geom", ".rchit", ".rmiss", ".rgen", ".rahit", ".rint"]
shaderFiles = []

for file in files:
	_, ext = os.path.splitext(file)
	ext = ext.lower()
	if ext in shaders:
		shaderFiles.append(file.replace("\\", "/"))
	pass

for shader in shaderFiles:
	os.system(exepath + " -V " + shader + " -o " + shader + ".spv")
	pass