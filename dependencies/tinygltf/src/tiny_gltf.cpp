#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <json/json.hpp>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#define TINYGLTF_NO_INCLUDE_JSON
#include <tinygltf/tiny_gltf.h>