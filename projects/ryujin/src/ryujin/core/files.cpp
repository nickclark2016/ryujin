#include <ryujin/core/files.hpp>

#include <ryujin/core/as.hpp>
#include <ryujin/core/primitives.hpp>

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

#undef NO_ERROR

namespace ryujin
{
    namespace detail
    {
        files::error_code validate_file(const std::string& path)
        {
            namespace fs = std::filesystem;

            const auto fp = fs::absolute(path);
            if (fs::exists(fp))
            {
                const auto status = fs::status(fp);
                const auto perms = status.permissions();
                if ((perms & (fs::perms::owner_read | fs::perms::group_read | fs::perms::owner_read)) != fs::perms::none)
                {
                    return files::error_code::NO_ERROR;
                }
                return files::error_code::FILE_NOT_ACCESSIBLE;
            }
            return files::error_code::FILE_NOT_FOUND;
        }
    }

    result<binary_file, files::error_code> files::load_binary(const std::string& path)
    {
        const auto validation = detail::validate_file(path);
        if (validation != error_code::NO_ERROR)
        {
            spdlog::warn("Failed to load binary file: {} - Error code: {}", path, as<u32>(validation));
            return result<binary_file, error_code>::from_error(validation);
        }

        std::ifstream file(path, std::ios::ate | std::ios::binary);
        const auto size = as<sz>(file.tellg());
        binary_file bin = {
            path,
            make_unique<char[]>(size),
            size
        };

        file.seekg(0);
        file.read(bin.bytes.get(), size);
        file.close();

        spdlog::info("Successfully loaded binary file: {}", path);

        return result<binary_file, error_code>::from_success(std::move(bin));
    }

    result<text_file, files::error_code> files::load_text(const std::string& path)
    {
        const auto validation = detail::validate_file(path);
        if (validation != error_code::NO_ERROR)
        {
            spdlog::warn("Failed to load text file: {} - Error code: {}", path, as<u32>(validation));
            return result<text_file, error_code>::from_error(validation);
        }

        std::ifstream file(path);
        std::ostringstream stringStream;
        stringStream << file.rdbuf();

        spdlog::info("Successfully loaded text file: {}", path);

        return result<text_file, error_code>::from_success({
            path,
            stringStream.str()
        });
    }
}