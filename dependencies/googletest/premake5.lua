project ('googletest')
    kind ('StaticLib')
    language ('C++')
    cppdialect ('C++20')

    targetdir (binaries)
    objdir (intermediate)

    files ({
        'src/gmock-all.cc',
        'src/gtest-all.cc',
    })

    includedirs ({
        '.',
        'include'
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