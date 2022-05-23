project ('Sandbox')
    kind ('ConsoleApp')
    language ('C++')
    cppdialect ('C++20')

    targetdir (binaries)
    objdir (intermediate)
    debugdir (binaries)

    files ({
        'include/**.hpp',
        'src/**.cpp'
    })

    includedirs ({
        'include/',
        "%{IncludeDir.glfw}",
        "%{IncludeDir.ryujin}",
        "%{IncludeDir.vkbootstrap}",
        "%{IncludeDir.vulkan}",
        "%{IncludeDir.vma}"
    })

    dependson ({
        'glfw',
        'vma',
        'stb',
        'ryujin',
        'tinygltf',
        'vk-bootstrap'
    })

    links ({
        'glfw',
        'vma',
        'stb',
        'tinygltf',
        'vk-bootstrap',
        'ryujin',
    })

    defines ({
        'GLFW_DLL'
    })

    filter ({ 'configurations:Debug' })
        optimize ('Off')
        symbols ('On')

    filter ({ 'configurations:Release' })
        optimize ('Speed')

    filter ({ 'system:windows' })
        cppdialect ('C++20')
        defines ({ '_RYUJIN_WINDOWS' })

    filter ({ 'system:linux' })
        cppdialect ('C++2a')
        linkgroups ('On')
        defines ({ '_RYUJIN_LINUX' })
        links ({
            'dl',
            'X11',
            'pthread'
        })

    filter ()