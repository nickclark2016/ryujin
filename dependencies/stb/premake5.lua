project ('stb')
    kind ('StaticLib')
    language ('C++')

    targetdir (binaries)
    objdir (intermediate)

    files ({
        'include/**.h',
        'src/**.cpp',
    })

    includedirs ({
        'include/',
        "%{IncludeDir.json}",
        "%{IncludeDir.stb}"
    })

    filter ({ 'system:windows' })
        systemversion ('latest')
        staticruntime ('Off')
        cppdialect ('C++20')
        defines({
            '_CRT_SECURE_NO_WARNINGS'
        })

    filter ({ 'system:linux' })
        cppdialect ('C++2a')

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