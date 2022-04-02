project ('zlib')
    kind ('SharedLib')
    language ('C++')
    cppdialect ('C++20')

    targetdir (binaries)
    objdir (intermediate)

    files ({
        'src/**'
    })

    includedirs ({
        'include'
    })

    disablewarnings ({
        '4244'
    })

    defines ({
        'ZLIB_DLL'
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