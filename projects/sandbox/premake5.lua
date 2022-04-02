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
        'ryujin'
    })

    links ({
        'ryujin',
        'zlib'
    })

    filter ({ 'configurations:Debug' })
        optimize ('Off')

    filter ({ 'configurations:Release' })
        optimize ('Speed')

    filter ()