project ('spdlog')
    kind ('StaticLib')
    language ('C++')
    cppdialect ('C++20')

    targetdir (binaries)
    objdir (intermediate)

    files ({
        'include/**',
        'src/**'
    })

    includedirs ({
        '.',
        'include'
    })

    defines ({
        'SPDLOG_COMPILED_LIB'
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