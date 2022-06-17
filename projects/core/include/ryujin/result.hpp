#ifndef result_hpp__
#define result_hpp__

#include "variant.hpp"
#include "utility.hpp"

namespace ryujin
{
    template <typename SuccessType, typename ErrorType>
    class result
    {
    public:
        static result<SuccessType, ErrorType> from_success(const SuccessType& res);
        static result<SuccessType, ErrorType> from_success(SuccessType&& res) noexcept;
        static result<SuccessType, ErrorType> from_error(const ErrorType& err);
        static result<SuccessType, ErrorType> from_error(ErrorType&& err) noexcept;

        explicit result(const SuccessType& res);
        explicit result(const ErrorType& res);
        explicit result(SuccessType&& res) noexcept;
        explicit result(ErrorType&& res) noexcept;

        result(const result&) = delete;
        result(result&& res) noexcept;
        ~result() = default;

        result& operator=(const result&) = delete;
        result& operator=(result<SuccessType, ErrorType>&& other) noexcept;

        operator bool() const noexcept;
        
        SuccessType* operator->() noexcept;
        const SuccessType* operator->() const noexcept;
        SuccessType& operator*() noexcept;
        const SuccessType& operator*() const noexcept;

        SuccessType success() noexcept;
        ErrorType error_code() const noexcept;

    private:
        ryujin::variant<SuccessType, ErrorType> _impl;

        bool _isSuccessful;
        bool _isInitialized;
    };

    template<typename SuccessType, typename ErrorType>
    inline result<SuccessType, ErrorType>::result(result&& res) noexcept
    {
        _impl =  ryujin::move(res._impl);

        _isInitialized = res._isInitialized;
        _isSuccessful = res._isSuccessful;

        res._isInitialized = false;
        res._isSuccessful = false;
    }

    template<typename SuccessType, typename ErrorType>
    inline result<SuccessType, ErrorType>& result<SuccessType, ErrorType>::operator=(result<SuccessType, ErrorType>&& other) noexcept
    {
        _isInitialized = other._isInitialized;
        _isSuccessful = other._isSuccessful;

        _impl =  ryujin::move(other._impl);

        other._isInitialized = false;
        other._isSuccessful = false;

        return *this;
    }

    template<typename SuccessType, typename ErrorType>
    inline result<SuccessType, ErrorType>::operator bool() const noexcept
    {
        return _isInitialized && _isSuccessful;
    }

    template<typename SuccessType, typename ErrorType>
    inline SuccessType* result<SuccessType, ErrorType>::operator->() noexcept
    {
        return ryujin::get_if<SuccessType>(&_impl);
    }

    template<typename SuccessType, typename ErrorType>
    inline const SuccessType* result<SuccessType, ErrorType>::operator->() const noexcept
    {
        return ryujin::get_if<SuccessType>(&_impl);
    }

    template<typename SuccessType, typename ErrorType>
    inline SuccessType& result<SuccessType, ErrorType>::operator*() noexcept
    {
        return ryujin::get<SuccessType>(_impl);
    }

    template<typename SuccessType, typename ErrorType>
    inline const SuccessType& result<SuccessType, ErrorType>::operator*() const noexcept
    {
        return ryujin::get<SuccessType>(_impl);
    }

    template<typename SuccessType, typename ErrorType>
    inline SuccessType result<SuccessType, ErrorType>::success() noexcept
    {
        return ryujin::get<SuccessType>(ryujin::move(_impl));
    }

    template<typename SuccessType, typename ErrorType>
    inline ErrorType result<SuccessType, ErrorType>::error_code() const noexcept
    {
        return ryujin::get<ErrorType>(ryujin::move(_impl));
    }
    
    template<typename SuccessType, typename ErrorType>
    inline result<SuccessType, ErrorType> result<SuccessType, ErrorType>::from_success(const SuccessType& res)
    {
        return result<SuccessType, ErrorType>(res);
    }

    template<typename SuccessType, typename ErrorType>
    inline result<SuccessType, ErrorType> result<SuccessType, ErrorType>::from_success(SuccessType&& res) noexcept
    {
        return result<SuccessType, ErrorType>(ryujin::forward<SuccessType>(res));
    }

    template<typename SuccessType, typename ErrorType>
    inline result<SuccessType, ErrorType> result<SuccessType, ErrorType>::from_error(const ErrorType& err)
    {
        return result<SuccessType, ErrorType>(err);
    }

    template<typename SuccessType, typename ErrorType>
    inline result<SuccessType, ErrorType> result<SuccessType, ErrorType>::from_error(ErrorType&& err) noexcept
    {
        return result<SuccessType, ErrorType>(ryujin::forward<ErrorType>(err));
    }

    template<typename SuccessType, typename ErrorType>
    inline result<SuccessType, ErrorType>::result(const SuccessType& res)
        : _impl(res)
    {
        _isInitialized = true;
        _isSuccessful = true;
    }
    
    template<typename SuccessType, typename ErrorType>
    inline result<SuccessType, ErrorType>::result(const ErrorType& res)
        : _impl(res)
    {
        _isInitialized = true;
        _isSuccessful = false;
    }
    
    template<typename SuccessType, typename ErrorType>
    inline result<SuccessType, ErrorType>::result(SuccessType&& res) noexcept
        : _impl(ryujin::forward<SuccessType>(res))
    {
        _isInitialized = true;
        _isSuccessful = true;
    }
    
    template<typename SuccessType, typename ErrorType>
    inline result<SuccessType, ErrorType>::result(ErrorType&& res) noexcept
        : _impl(ryujin::forward<ErrorType>(res))
    {
        _isInitialized = true;
        _isSuccessful = false;
    }
}

#endif // result_hpp__