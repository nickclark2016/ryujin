project ('zlib')
    kind ('SharedLib')
    language ('C++')

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