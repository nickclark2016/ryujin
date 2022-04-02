project ('ryujin')
    kind ('StaticLib')
    language ('C++')
    cppdialect ('C++20')

    targetdir (binaries)
    objdir (intermediate)

    files ({
        'include/**.hpp',
        'src/**.hpp',
        'src/**.cpp'
    })

    includedirs ({
        'include/',
        "%{IncludeDir.glfw}",
        "%{IncludeDir.libpng}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.vkbootstrap}",
        "%{IncludeDir.vulkan}",
        "%{IncludeDir.vma}"
    })

    links ({
        'glfw',
        'libpng',
        'spdlog',
        'vk-bootstrap',
        'vma'
    })

    dependson ({
        'glfw',
        'libpng',
        'spdlog',
        'vk-bootstrap',
        'vma'
    })

    filter ({ 'configurations:Debug' })
        optimize ('Off')

    filter ({ 'configurations:Release' })
        optimize ('Speed')

    filter ()
    
    dataDirectory = path.join(binaries, "data")
    projectDataDirectory = path.join("%{root}", "projects", "ryujin", "data")

    filter ({ 'action:vs*' })

        postBuildBatchFile = path.join("%{root}", "projects", "ryujin", "postbuildcmds.bat")

        postbuildcommands({
            "%{postBuildBatchFile} %{projectDataDirectory} %{dataDirectory} %{cfg.buildcfg}"
        })

    filter ()