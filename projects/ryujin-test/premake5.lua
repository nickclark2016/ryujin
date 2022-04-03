project ('ryujin-test')
    kind ('ConsoleApp')
    language ('C++')
    cppdialect ('C++20')

    targetdir (binaries)
    objdir (intermediate)

    files ({
        'src/**.cpp'
    })

    includedirs ({
        "%{IncludeDir.ryujin}",
        "%{IncludeDir.gtest}"
    })

    links ({
        'googletest',
        'ryujin'
    })

    dependson ({
        'googletest',
        'ryujin'
    })

    filter ({ 'configurations:Debug' })
        optimize ('Off')

    filter ({ 'configurations:Release' })
        optimize ('Speed')


    filter ({ 'system:windows' })
        cppdialect ('C++20')

    filter ({ 'system:linux' })
        cppdialect ('C++2a')
        links ({
            'pthread'
        })


    filter ()