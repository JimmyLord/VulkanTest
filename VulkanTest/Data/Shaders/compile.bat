..\..\..\Libraries\Vulkan\Bin32\glslangValidator -V test.vert -o spv.test.vs
if errorlevel 1 pause
..\..\..\Libraries\Vulkan\Bin32\glslangValidator -V test.frag -o spv.test.fs
if errorlevel 1 pause
