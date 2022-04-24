#ifndef files_hpp__
#define files_hpp__

#include "primitives.hpp"
#include "result.hpp"
#include "smart_pointers.hpp"

#include <string>

namespace ryujin
{
    struct binary_file
    {
        std::string path;
        unique_ptr<char[]> bytes;
        sz length;
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
