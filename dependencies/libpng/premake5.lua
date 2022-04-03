project ('libpng')
    kind ('StaticLib')
    language ('C++')

    targetdir (binaries)
    objdir (intermediate)

    files ({
        'src/**',
        'include/**',
    })

    includedirs ({
        'include',
        "%{IncludeDir.zlib}"
    })

    defines ({
        '_CRT_SECURE_NO_WARNINGS'
    })

    links ({
        'zlib'
    })

    dependson ({
        'zlib'
    })

    staticruntime ('Off')

    filter ({ 'system:windows' })
        systemversion ('latest')
        cppdialect ('C++20')

    filter ({ 'system:linux' })
        cppdialect ('C++2a')

    filter ({ 'configurations:Debug' })
        runtime ('Debug')
        symbols ('On')

    filter ({ 'configurations:Release' })
        optimize ('Speed')
        runtime ('Release')
        symbols ('Off')

    filter ()