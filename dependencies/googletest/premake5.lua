project ('googletest')
    kind ('StaticLib')
    language ('C++')

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