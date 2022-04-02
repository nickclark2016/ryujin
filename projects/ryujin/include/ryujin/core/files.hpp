#ifndef files_hpp__
#define files_hpp__

#include "result.hpp"

#include <cstddef>
#include <memory>
#include <string>

namespace ryujin
{
    struct binary_file
    {
        std::string path;
        std::unique_ptr<char[]> bytes;
        std::size_t length;
    };

    struct text_file
    {
        std::string path;
        std::string contents;
    };

    class files
    {
    public:
        enum class error_code
        {
            NO_ERROR,
            FILE_NOT_FOUND,
            FILE_NOT_ACCESSIBLE
        };

        static result<binary_file, error_code> load_binary(const std::string& path);
        static result<text_file, error_code> load_text(const std::string& path);
    };
}

#endif // files_hpp__
