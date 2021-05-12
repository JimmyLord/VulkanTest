------------------------------------------------ Solution
workspace "VulkanTest"
    configurations  { "Debug", "Release" }
    location        "build"
    startproject    "VulkanTest"

    filter "system:windows"
        platforms       { "x64" }
        characterset    "MBCS"

------------------------------------------------ Game Project
project "VulkanTest"
    location    "build"
    kind        "WindowedApp"
    language    "C++"
    debugdir    "VulkanTest"

    includedirs {
        "VulkanTest/Source",
        os.getenv("VULKAN_SDK") .. "/Include",
    }

    files {
        "VulkanTest/Source/**.cpp",
        "VulkanTest/Source/**.h",
        "VulkanTest/Data/Shaders/**.vert",
        "VulkanTest/Data/Shaders/**.frag",
        "premake5.lua",
        "PremakeGenerateBuildFiles.bat",
        ".gitignore",
    }

    vpaths {
        [""] = {
            "*.*",
        },
        ["Source*"] = {
            "Game/Source",
        },
    }

    links {
        os.getenv("VULKAN_SDK") .. "/lib/vulkan-1",
    }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
