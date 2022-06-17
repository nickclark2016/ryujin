group('Engine')

project ('core')
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
        "%{IncludeDir.spdlog}",
    })

    links ({
        "spdlog"
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

project ('core-test')
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
        "%{IncludeDir.gtest}"
    })

    links ({
        'core',
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