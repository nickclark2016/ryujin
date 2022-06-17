group('Engine')

project ('entities')
    kind ('StaticLib')
    language ('C++')
    cppdialect ('C++20')

    targetdir (binaries)
    objdir (intermediate)

    files ({
        'include/**.hpp',
        'src/**.cpp'
    })

    includedirs ({
        "%{IncludeDir.ryujin_core}",
        "%{IncludeDir.ryujin_entities}",
        "%{IncludeDir.ryujin_math}",
    })

    links ({
        'core'
    })

    dependson ({
        'math'
    })

    filter ({ 'system:windows' })
        cppdialect ('C++20')
        defines ({ '_RYUJIN_WINDOWS' })

    filter ({ 'system:linux' })
        cppdialect ('C++2a')
        defines ({ '_RYUJIN_LINUX' })
        links ({
            'pthread'
        })

group('Tests')

project ('entities-test')
    kind ('ConsoleApp')
    language ('C++')
    cppdialect ('C++20')

    targetdir (binaries)
    objdir (intermediate)

    files ({
        'tests/**.cpp'
    })

    includedirs ({
        "%{IncludeDir.ryujin_core}",
        "%{IncludeDir.ryujin_entities}",
        "%{IncludeDir.ryujin_math}",
        "%{IncludeDir.gtest}"
    })

    links ({
        'core',
        'entities',
        'googletest',
        'spdlog',
    })

    filter ({ 'system:windows' })
        cppdialect ('C++20')
        defines ({ '_RYUJIN_WINDOWS' })

    filter ({ 'system:linux' })
        cppdialect ('C++2a')
        defines ({ '_RYUJIN_LINUX' })
        links ({
            'pthread'
        })