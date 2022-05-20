workspace ('ryujin')
    configurations ({
        'Debug',
        'Release'
    })

    flags ({
        'MultiProcessorCompile'
    })

    filter ({ 'configurations:Debug' })
        runtime ('Debug')
    
    filter ({ 'configurations:Release' })
        runtime ('Release')

    filter ({ 'system:windows' })
        platforms ({ 'x86', 'x64' })
    
    filter ({ 'system:linux' })
        platforms ({ 'x64' })

    filter ({})
    
    root = path.getdirectory(_MAIN_SCRIPT)

    binaries = "%{root}/bin/%{cfg.buildcfg}/%{cfg.system}/%{cfg.architecture}"
    intermediate = "%{root}/bin-int/%{cfg.buildcfg}/%{cfg.system}/%{cfg.architecture}"

    group ('3rd Party Dependencies')
    include ('dependencies/glfw')
    include ('dependencies/googletest')
    include ('dependencies/spdlog')
    include ('dependencies/stb')
    include ('dependencies/tinygltf')
    include ('dependencies/vk-bootstrap')
    include ('dependencies/vulkanmemoryallocator')
    group ('')

    group ('Engine')
    include ('projects/ryujin')
    include ('projects/ryujin-test')
    group ('')

    group ('Sandbox')
    include ('projects/sandbox')
    group ('')

    IncludeDir = {}
    IncludeDir["ryujin"] = "%{root}/projects/ryujin/include"

    IncludeDir["gtest"] = "%{root}/dependencies/googletest/include"
    IncludeDir["glfw"] = "%{root}/dependencies/glfw/include"
    IncludeDir["json"] = "%{root}/dependencies/json/include"
    IncludeDir["spdlog"] = "%{root}/dependencies/spdlog/include"
    IncludeDir["tinygltf"] = "%{root}/dependencies/tinygltf/include"
    IncludeDir["vkbootstrap"] = "%{root}/dependencies/vk-bootstrap/include"
    IncludeDir["vulkan"] = "%{root}/dependencies/vulkan/include"
    IncludeDir["vma"] = "%{root}/dependencies/vulkanmemoryallocator/include"
    IncludeDir["stb"] = "%{root}/dependencies/stb/include"