group('Engine')

project ('graphics')
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
        "%{IncludeDir.glfw}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.tinygltf}",
        "%{IncludeDir.vkbootstrap}",
        "%{IncludeDir.vulkan}",
        "%{IncludeDir.vma}"
    })

    links ({
        'assets',
        'core',
        'entities',
        'math',
        'glfw',
        'spdlog',
        'stb',
        'vk-bootstrap',
        'vma'
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

project ('graphics-test')
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
        "%{IncludeDir.glfw}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.tinygltf}",
        "%{IncludeDir.vkbootstrap}",
        "%{IncludeDir.vulkan}",
        "%{IncludeDir.vma}",
        "%{IncludeDir.gtest}"
    })

    links ({
        'assets',
        'core',
        'entities',
        'graphics',
        'math',
        'glfw',
        'spdlog',
        'stb',
        'vk-bootstrap',
        'vma',
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