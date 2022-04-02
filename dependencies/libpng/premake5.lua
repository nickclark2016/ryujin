project ('libpng')
    kind ('StaticLib')
    language ('C++')
    cppdialect ('C++20')

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

    filter ({ 'configurations:Debug' })
        runtime ('Debug')
        symbols ('On')

    filter ({ 'configurations:Release' })
        optimize ('Speed')
        runtime ('Release')
        symbols ('Off')

    filter ()