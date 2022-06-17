group('Engine')

project ('assets')
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
        "%{IncludeDir.ryujin_assets}",
        "%{IncludeDir.ryujin_core}",
        "%{IncludeDir.ryujin_entities}",
        "%{IncludeDir.ryujin_graphics}",
        "%{IncludeDir.ryujin_math}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.tinygltf}"
    })

    links ({
        'core',
        'entities',
        'math',
        'stb',
        'tinygltf'
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

project ('assets-test')
    kind ('ConsoleApp')
    language ('C++')
    cppdialect ('C++20')

    targetdir (binaries)
    objdir (intermediate)

    files ({
        'tests/**.cpp'
    })

    includedirs ({
        "%{IncludeDir.ryujin_assets}",
        "%{IncludeDir.ryujin_core}",
        "%{IncludeDir.ryujin_entities}",
        "%{IncludeDir.ryujin_graphics}",
        "%{IncludeDir.ryujin_math}",
        "%{IncludeDir.gtest}"
    })

    links ({
        'assets',
        'core',
        'entities',
        'googletest',
        'stb',
        'stb',
        'tinygltf'
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