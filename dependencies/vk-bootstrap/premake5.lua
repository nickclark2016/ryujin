project ('vk-bootstrap')
    kind ('StaticLib')
    language ('C++')
    cppdialect ('C++20')

    targetdir (binaries)
    objdir (intermediate)

    files ({
        'include/**.h',
        'src/**.cpp',
    })

    includedirs ({
        'include/',
        "%{IncludeDir.vulkan}"
    })

    filter ({ 'system:windows' })
       systemversion ('latest')
       staticruntime ('Off')

    filter ({})

    filter ({ 'configurations:Debug' })
        optimize ('Off')
        runtime ('Debug')
        symbols ('Full')

    filter ({ 'configurations:Release' })
        optimize ('Speed')
        runtime ('Release')
        symbols ('Off')

    filter ({})