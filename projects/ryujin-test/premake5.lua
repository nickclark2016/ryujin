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

    defines ({
        'RYUJIN_PROVIDE_STRUCTURED_BINDINGS'
    })

    filter ({ 'configurations:Debug' })
        optimize ('Off')

    filter ({ 'configurations:Release' })
        optimize ('Speed')

    filter ({ 'system:windows' })
        cppdialect ('C++20')
        defines ({ '_RYUJIN_WINDOWS' })

    filter ({ 'system:linux' })
        cppdialect ('C++2a')
        defines ({ '_RYUJIN_LINUX' })
        links ({
            'pthread'
        })


    filter ()