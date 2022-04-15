project ('ryujin')
    kind ('StaticLib')
    language ('C++')

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
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.tinygltf}",
        "%{IncludeDir.vkbootstrap}",
        "%{IncludeDir.vulkan}",
        "%{IncludeDir.vma}"
    })

    links ({
        'glfw',
        'spdlog',
        'stb',
        'vk-bootstrap',
        'vma'
    })

    dependson ({
        'glfw',
        'spdlog',
        'stb',
        'vk-bootstrap',
        'vma'
    })

    filter ({ 'configurations:Debug' })
        optimize ('Off')
        symbols ('On')
        defines ({
            '_DEBUG'
        })

    filter ({ 'configurations:Release' })
        optimize ('Speed')

    filter ({ 'system:windows' })
        cppdialect ('C++20')
        defines ({ '_RYUJIN_WINDOWS' })

    filter ({ 'system:linux' })
        cppdialect ('C++2a')
        defines ({ '_RYUJIN_LINUX' })
        -- linkgroups ('On')

    filter ({})
    
    dataDirectory = path.join(binaries, "data")
    projectDataDirectory = path.join("%{root}", "projects", "ryujin", "data")

    filter ({ 'action:vs*' })

        postBuildBatchFile = path.join("%{root}", "projects", "ryujin", "postbuildcmds.bat")

        postbuildcommands({
            "%{postBuildBatchFile} %{projectDataDirectory} %{dataDirectory} %{cfg.buildcfg}"
        })

    filter ({ 'system:linux' })

        postBuildShellFile = path.join("%{root}", "projects", "ryujin", "postbuildcmds.sh")

        postbuildcommands({
            "sh %{postBuildShellFile} %{projectDataDirectory} %{dataDirectory} %{cfg.buildcfg}"
        })

    filter ()