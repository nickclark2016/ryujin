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
        "%{IncludeDir.ryujin_assets}",
        "%{IncludeDir.ryujin_core}",
        "%{IncludeDir.ryujin_entities}",
        "%{IncludeDir.ryujin_graphics}",
        "%{IncludeDir.ryujin_input}",
        "%{IncludeDir.ryujin_math}",
        "%{IncludeDir.glfw}", -- work to hide this dep
        "%{IncludeDir.vkbootstrap}", -- work to hide this dep
        "%{IncludeDir.vma}", -- work to hide this dep
        "%{IncludeDir.vulkan}", -- work to hide this dep
    })

    dependson ({
        'assets',
        'core',
        'entities',
        'graphics',
        'input',
        'math',
        'glfw',
        'vma',
        'stb',
        'tinygltf',
        'vk-bootstrap'
    })

    links ({
        'assets',
        'core',
        'entities',
        'graphics',
        'input',
        'math',
        'glfw',
        'vma',
        'stb',
        'tinygltf',
        'vk-bootstrap',
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