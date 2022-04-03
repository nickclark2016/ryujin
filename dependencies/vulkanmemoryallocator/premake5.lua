project ('vma')
    kind ('StaticLib')
    language ('C++')

    targetdir (binaries)
    objdir (intermediate)

    files ({
        'include/**.h',
        'src/**.h',
        'src/**.cpp',
    })

    includedirs ({
        'include/',
        "%{IncludeDir.vulkan}"
    })

    defines ({
        'VK_NO_PROTOTYPES'
    })

    filter ({ 'system:windows' })
        systemversion ('latest')
        staticruntime ('Off')
        cppdialect ('C++20')

    filter ({ 'system:linux' })
        cppdialect ('C++2a')

    filter ({})

    filter ({ 'configurations:Debug' })
        optimize ('Off')
        runtime ('Debug')
        symbols ('Full')

    filter ({ 'configurations:Release' })
        optimize ('Speed')
        runtime ('Release')
        symbols ('Off')

    filter ({})