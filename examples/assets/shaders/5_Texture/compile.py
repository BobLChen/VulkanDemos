# coding: utf-8

import os
import sys

# os.system("../glslangvalidator.exe")

path = os.getcwd()
path = path.replace("\\", "/")
path = path[0:path.find("VulkanTutorials")]
path = path + "/VulkanTutorials/"

exepath = path

if "win" == sys.platform:
	exepath = exepath + "external/vulkan/windows/bin/x86/glslangvalidator.exe"
	pass
elif "linux" == sys.platform:
	exepath = exepath + "external/vulkan/linux/bin/glslangValidator"
	pass
elif "darwin" == sys.platform:
	exepath = exepath + "external/vulkan/macos/bin/glslangValidator"
	pass

shaders = [
	"examples/assets/shaders/5_Texture/solid.vert",
	"examples/assets/shaders/5_Texture/solid.frag"
]

for shader in shaders:
	os.system(exepath + " -V " + (path + shader) + " -o " + (path + shader + ".spv"))
	pass