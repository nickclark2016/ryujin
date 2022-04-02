#include "png.hpp"

#include <ryujin/core/as.hpp>
#include <ryujin/core/files.hpp>
#include <ryujin/core/span.hpp>

#include <png.h>

#undef APIENTRY
#include <spdlog/spdlog.h>

namespace ryujin::assets
{
    struct reader
    {
        span<std::byte> bytes;
        std::size_t index = 0;
    };

    static void libpng_rw_callback(png_structp pngPtr, png_bytep outBytes, size_t bytesToRead)
    {
        png_voidp ioPtr = png_get_io_ptr(pngPtr);
        if (ioPtr == nullptr)
        {
            spdlog::error("png_get_io_ptr returned nullptr");
            return;
        }
        reader* ptr = reinterpret_cast<reader*>(ioPtr);

        memcpy_s(outBytes, bytesToRead, ptr->bytes.data() + ptr->index, std::min(bytesToRead, ptr->bytes.length()));

        ptr->index += bytesToRead;
    }

    static bool read_rgba_data(png_structp pngPtr, png_infop infoPtr, texture_asset::mip_level& level)
    {
        const size_t bytesPerRow = png_get_rowbytes(pngPtr, infoPtr);
        const auto bytesPerPixel = bytesPerRow / level.width;

        if (bytesPerRow == 3 || bytesPerRow == 6)
        {
            spdlog::error("read_rgba_data: Expected RGBA payload");
            return false;
        }

        std::byte* rowBytes = new std::byte[bytesPerRow];

        level.bytes.resize(bytesPerRow * level.height);


        for (std::size_t row = 0; row < level.height; ++row)
        {
            png_read_row(pngPtr, reinterpret_cast<png_bytep>(rowBytes), nullptr);
            const auto offset = bytesPerRow * (as<std::size_t>(level.height) - 1 - row);
            for (std::uint32_t bt = 0; bt < bytesPerRow; ++bt)
            {
                level.bytes[offset + bt] = rowBytes[bt];
            }
        }

        // 8 bytes per pixel
        // 4, 2 byte components per pixel
        // Stored in big endian, needs flipping
        if (bytesPerPixel == 8)
        {
            for (std::size_t i = 0; i < level.bytes.size(); i += 2)
            {
                auto ptr = level.bytes.data() + i;
                auto sPtr = reinterpret_cast<unsigned short*>(ptr);
                auto swapped = _byteswap_ushort(*sPtr);
                auto swappedPtr = reinterpret_cast<std::byte*>(&swapped);
                level.bytes[i + 0] = swappedPtr[0];
                level.bytes[i + 1] = swappedPtr[1];
            }
        }

        delete[] rowBytes;
        return true;
    }

    texture_asset* load_png(span<std::byte> buffer)
    {
        png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (pngPtr == nullptr)
        {
            spdlog::error("Failed to create png_structp.");
            return nullptr;
        }
        png_infop infoPtr = png_create_info_struct(pngPtr);
        if (infoPtr == nullptr)
        {
            spdlog::error("Failed to create png_infop struct from png_structp.");;
            png_destroy_read_struct(&pngPtr, nullptr, nullptr);
            return nullptr;
        }

        reader r = {
            .bytes = buffer,
            .index = 0
        };
        png_set_read_fn(pngPtr, &r, libpng_rw_callback);

        if (setjmp(png_jmpbuf(pngPtr)))
        {
            spdlog::error("Failed to setjmp to png_jmpbuf for expected PNG image buffer.");
            png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
            return nullptr;
        }

        const bool isPng = !png_sig_cmp(reinterpret_cast<const unsigned char*>(buffer.data()), 0, 8);
        if (!isPng)
        {
            spdlog::error("Buffer does not represent a PNG image.");
            png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
            return nullptr;
        }

        png_read_info(pngPtr, infoPtr);

        std::uint32_t width = png_get_image_width(pngPtr, infoPtr);
        std::uint32_t height = png_get_image_height(pngPtr, infoPtr);
        std::uint32_t bitDepth = png_get_bit_depth(pngPtr, infoPtr);
        std::uint32_t colorType = png_get_color_type(pngPtr, infoPtr);

        spdlog::debug("Loading PNG with dimensions {} x {}", width, height);

        if (colorType == PNG_COLOR_TYPE_PALETTE)
        {
            png_set_palette_to_rgb(pngPtr);
        }
        else if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
        {
            png_set_expand_gray_1_2_4_to_8(pngPtr);
        }
        
        if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
        {
            png_set_tRNS_to_alpha(pngPtr);
        }

        switch (colorType)
        {
        case PNG_COLOR_TYPE_RGB:
        case PNG_COLOR_TYPE_GRAY:
        case PNG_COLOR_TYPE_PALETTE:
        {
            png_set_add_alpha(pngPtr, 0xFFFF, PNG_FILLER_AFTER);
            break;
        }
        default:
            break;
            // do nothing
        }

        switch (colorType)
        {
        case PNG_COLOR_TYPE_GRAY:
        case PNG_COLOR_TYPE_GRAY_ALPHA:
        {
            png_set_gray_to_rgb(pngPtr);
            break;
        }
        default:
            break;
            // do nothing
        }


        png_read_update_info(pngPtr, infoPtr);

        width = png_get_image_width(pngPtr, infoPtr);
        height = png_get_image_height(pngPtr, infoPtr);
        bitDepth = png_get_bit_depth(pngPtr, infoPtr);
        colorType = png_get_color_type(pngPtr, infoPtr);

        texture_asset::mip_level level = {};
        level.width = width;
        level.height = height;

        std::uint32_t componentCount = 0;
        texture_asset::data_type type = {};

        switch (colorType)
        {
        case PNG_COLOR_TYPE_RGBA:
        {
            const bool loaded = read_rgba_data(pngPtr, infoPtr, level);
            if (bitDepth == 8)
            {
                type = texture_asset::data_type::UCHAR;
            }
            else if (bitDepth == 16)
            {
                type = texture_asset::data_type::USHORT;
            }
            else
            {
                spdlog::error("Unhandled PNG bit depth: {}", bitDepth);
                png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
                return nullptr;
            }
            break;
        }
        default:
            spdlog::error("Unhandled PNG color type: {}", colorType);
            png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
            return nullptr;
        }

        png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
        texture_asset* asset = new texture_asset({ { level } }, componentCount, type);

        return asset;
    }
    
    texture_asset* load_png(const std::string& path)
    {
        const auto fileLoadRes = files::load_binary(path);
        if (fileLoadRes)
        {
            auto& payload = fileLoadRes->bytes;
            auto length = fileLoadRes->length;

            span<std::byte> bytes(reinterpret_cast<std::byte*>(payload.get()), length);
            return load_png(bytes);
        }

        return nullptr;
    }
}
