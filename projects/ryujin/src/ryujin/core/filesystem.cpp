#include "ryujin/core/filesystem.hpp"

#include <filesystem>

#include "ryujin/core/as.hpp"

namespace ryujin::filesystem
{
	void path::clear() noexcept
	{
		_source.clear();
	}

	path& path::make_preferred() noexcept
	{
		return *this;
	}

	path& path::remove_filename() noexcept
	{
		return *this;
	}

	path& path::replace_filename(const path& replacement)
	{
		return *this;
	}

	path& path::replace_extension(const path& replacement) noexcept
	{
		return *this;
	}

	[[nodiscard]] const path::string_type& path::native() const noexcept
	{
		return _source;
	}

	[[nodiscard]] const path::value_type* path::c_str() const noexcept
	{
		return _source.c_str();
	}

	[[nodiscard]] path path::root_name() const
	{
		return path();
	}

	[[nodiscard]] path path::root_directory() const
	{
		return path();
	}

	[[nodiscard]] path path::root_path() const
	{
		return path();
	}

	[[nodiscard]] path path::relative_path() const
	{
		return path();
	}

	[[nodiscard]] path path::parent_path() const
	{
		return path();
	}

	[[nodiscard]] path path::filename() const
	{
		return path();
	}

	[[nodiscard]] path path::stem() const
	{
		return path();
	}

	[[nodiscard]] path path::extension() const
	{
		return path();
	}

	[[nodiscard]] bool path::empty() const
	{
		return false;
	}

	[[nodiscard]] bool path::has_root_name() const noexcept
	{
		return false;
	}

	[[nodiscard]] bool path::has_root_directory() const noexcept
	{
		return false;
	}

	[[nodiscard]] bool path::has_root_path() const noexcept
	{
		return false;
	}

	[[nodiscard]] bool path::has_relative_path() const noexcept
	{
		return false;
	}

	[[nodiscard]] bool path::has_parent_path() const noexcept
	{
		return false;
	}

	[[nodiscard]] bool path::has_filename() const noexcept
	{
		return false;
	}

	[[nodiscard]] bool path::has_stem() const noexcept
	{
		return false;
	}

	[[nodiscard]] bool path::has_extension() const noexcept
	{
		return false;
	}

	[[nodiscard]] bool path::is_absolute() const noexcept
	{
		return false;
	}

	[[nodiscard]] bool path::is_relative() const noexcept
	{
		return false;
	}

	[[nodiscard]] inline path::iterator path::begin()
	{
		return _source.begin();
	}

	[[nodiscard]] inline path::const_iterator path::cbegin() const
	{
		return _source.cbegin();
	}

	[[nodiscard]] inline path::iterator path::end() noexcept
	{
		return _source.end();
	}

	[[nodiscard]] inline path::const_iterator path::cend() const noexcept
	{
		return _source.cend();
	}

	path& path::operator/=(const path& rhs)
	{
		return *this;
	}

	path& path::operator+=(const path& rhs)
	{
		return *this;
	}

	path& path::operator+=(const string_type& rhs)
	{
		return *this;
	}

	path& path::operator+=(const value_type* const rhs)
	{
		return *this;
	}

	path& path::operator+=(const value_type rhs)
	{
		return *this;
	}

	error_code copy(const path& from, const path& to, copy_options options)
	{
		std::error_code ec;
		std::filesystem::copy(from.c_str(), to.c_str(), as<std::filesystem::copy_options>(options), ec);

		return as<error_code>(ec.value());
	}

	error_code create_directory(const path& p)
	{
		std::error_code ec;
		std::filesystem::create_directory(p.c_str(), ec);

		return as<error_code>(ec.value());
	}

	path current_path()
	{
		std::error_code ec;
		std::filesystem::path p = std::filesystem::current_path(ec);

		/*if (ec.value() != 0)
		{
			return result<path, error_code>::from_error(as<error_code>(ec.value()));
		}*/
		return path(p.c_str());
	}

	bool exists(const path& p)
	{
		return std::filesystem::exists(p.c_str());
	}

	result<sz, error_code> file_size(const path& p)
	{
		std::error_code ec;
		sz size = std::filesystem::file_size(p.c_str(), ec);
		
		if (ec.value() != 0)
		{
			return result<sz, error_code>::from_error(as<error_code>(ec.value()));
		}
		return result<sz, error_code>::from_success(size);
	}

	result<bool, error_code> remove(const path& p)
	{
		std::error_code ec;
		std::filesystem::remove(p.c_str(), ec);

		if (ec.value() != 0)
		{
			return result<bool, error_code>::from_error(as<error_code>(ec.value()));
		}

		return result<bool, error_code>::from_success(true);
	}

	result<sz, error_code> remove_all(const path& p)
	{
		std::error_code ec;
		sz amount = std::filesystem::remove_all(p.c_str(), ec);

		if (ec.value() != 0)
		{
			return result<sz, error_code>::from_error(as<error_code>(ec.value()));
		}

		return result<sz, error_code>::from_success(amount);
	}

	error_code rename(const path& oldp, const path& newp)
	{
		std::error_code ec;
		std::filesystem::rename(oldp.c_str(), newp.c_str(), ec);

		return as<error_code>(ec.value());
	}

	bool is_directory(const path& p)
	{
		return std::filesystem::is_directory(p.c_str());
	}

	bool is_file(const path& p)
	{
		return std::filesystem::is_regular_file(p.c_str());
	}

	bool is_empty(const path& p)
	{
		return std::filesystem::is_empty(p.c_str());
	}

	bool is_symlink(const path& p)
	{
		return std::filesystem::is_symlink(p.c_str());
	}
}
