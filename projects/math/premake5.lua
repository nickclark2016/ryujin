group('Engine')

project ('math')
    kind ('None')
    language ('C++')
    cppdialect ('C++20')

    targetdir (binaries)
    objdir (intermediate)

    files ({
        'include/**.hpp'
    })

    includedirs ({
        "%{IncludeDir.ryujin_core}",
        "%{IncludeDir.ryujin_math}",
    })

    dependson ({
        'core'
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

project ('math-test')
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
        "%{IncludeDir.ryujin_math}",
        "%{IncludeDir.gtest}"
    })

    links ({
        'math',
        'googletest',
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