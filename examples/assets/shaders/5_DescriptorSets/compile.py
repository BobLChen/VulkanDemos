# coding: utf-8

import os
import sys

# os.system("../glslangvalidator.exe")

path = os.getcwd()
path = path.replace("\\", "/")
path = path[0:path.find("VulkanTutorials")]
path = path + "/VulkanTutorials/"

exepath = path

if "win" in sys.platform:
	exepath = exepath + "external/vulkan/windows/bin/x86/glslangvalidator.exe"
	pass
elif "linux" in sys.platform:
	exepath = exepath + "external/linux/bin/glslangValidator"
	pass
elif "darwin" in sys.platform:
	exepath = exepath + "external/macos/bin/glslangvalidator"
	pass

shaders = [
	"examples/assets/shaders/5_DescriptorSets/solid.vert",
	"examples/assets/shaders/5_DescriptorSets/solid.frag"
]

for shader in shaders:
	os.system(exepath + " -V " + (path + shader) + " -o " + (path + shader + ".spv"))
	pass