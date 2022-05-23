#ifndef filesystem_hpp__
#define filesystem_hpp__

#include "string.hpp"
#include "result.hpp"
#include "utility.hpp"

namespace ryujin::filesystem
{
	enum class error_code
	{
		none
	};

	enum class file_type
	{
		none,
		not_found,
		regular,
		directory,
		symlink,
		block,
		character,
		fifo,
		socket,
		unknown,
	};

	enum class copy_options 
	{
		none,
		skip_existing,
		overwrite_existing,
		update_existing,
		recursive,
		copy_symlinks,
		skip_symlinks,
		directories_only,
		create_symlinks,
		create_hard_links
	};

	class path
	{
	public:
#if defined(_RYUJIN_WINDOWS)
		using value_type = wchar_t;
#elif defined(_RYUJIN_LINUX)
		using value_type = char;
#endif
		using string_type = basic_string<value_type>;

		static constexpr wchar_t preferred_separator = L'\\';

		path() = default;
		path(const path&) = default;
		path(path&&) = default;
		~path() = default;
		path& operator=(const path&) = default;
		path& operator=(path&&) noexcept = default;

		path(const string_type& source) : _source(source) {}
		path(const value_type* source) : _source(source) {}

		path(string_type&& source) : _source(ryujin::move(source)) {}

		path& operator/=(const path& rhs);
		path& operator+=(const path& rhs);
		path& operator+=(const string_type& rhs);
		path& operator+=(const value_type* const rhs);
		path& operator+=(const value_type rhs);

		void clear() noexcept;

		path& make_preferred() noexcept;
		path& remove_filename() noexcept;
		path& replace_filename(const path& replacement);
		path& replace_extension(const path& replacement) noexcept;

		[[nodiscard]] const string_type& native() const noexcept;
		[[nodiscard]] const value_type* c_str() const noexcept;

		operator string_type() const
		{
			return _source;
		}

		[[nodiscard]] path root_name() const;
		[[nodiscard]] path root_directory() const;
		[[nodiscard]] path root_path() const;
		[[nodiscard]] path relative_path() const;
		[[nodiscard]] path parent_path() const;
		[[nodiscard]] path filename() const;
		[[nodiscard]] path stem() const;
		[[nodiscard]] path extension() const;
		[[nodiscard]] bool empty() const;
		[[nodiscard]] bool has_root_name() const noexcept;
		[[nodiscard]] bool has_root_directory() const noexcept;
		[[nodiscard]] bool has_root_path() const noexcept;
		[[nodiscard]] bool has_relative_path() const noexcept;
		[[nodiscard]] bool has_parent_path() const noexcept;
		[[nodiscard]] bool has_filename() const noexcept;
		[[nodiscard]] bool has_stem() const noexcept;
		[[nodiscard]] bool has_extension() const noexcept;
		[[nodiscard]] bool is_absolute() const noexcept;
		[[nodiscard]] bool is_relative() const noexcept;

		using iterator = value_type*;
		using const_iterator = const value_type*;

		[[nodiscard]] inline iterator begin();
		[[nodiscard]] inline const_iterator cbegin() const;
		[[nodiscard]] inline iterator end() noexcept;
		[[nodiscard]] inline const_iterator cend() const noexcept;

	private:
		string_type _source;
	};

	class directory_entry
	{

	};

	class directory_iterator
	{

	};

	class recursive_directory_iterator
	{

	};

	class file_status
	{

	};

	// Copies files or directories
	error_code copy(const path& from, const path& to, copy_options options = copy_options::none);
	error_code create_directory(const path& p);

	result<path, error_code> current_path();

	bool exists(const path& p);
	result<sz, error_code> file_size(const path& p);

	result<bool, error_code> remove(const path& p);
	result<sz, error_code> remove_all(const path& p);

	error_code rename(const path& oldp, const path& newp);

	bool is_directory(const path& p);
	bool is_file(const path& p);
	bool is_empty(const path& p);
	bool is_symlink(const path& p);
}

#endif // filesystem_hpp__