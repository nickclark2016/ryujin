#ifndef task_hpp__
#define task_hpp__

#include "../core/functional.hpp"
#include "../core/optional.hpp"
#include "../core/utility.hpp"

namespace ryujin
{
    class itask
    {
    public:
        virtual void execute() = 0;
    };

    template <typename ...>
    class task;

    template <typename Return, typename ... Args>
    class task<Return(Args...)> : public itask
    {
    public:
        using result = Return;

        explicit task(move_only_function<Return(Args...)>&& fn, Args... args);

        void execute() override;
        
        template <enable_if_t<!is_same_v<Return, void>, bool> = true>
        Return& get_result() noexcept;
    private:

        template <sz ... Is>
        void invoke_from_tuple(index_sequence<Is...>);

        move_only_function<Return(Args...)> _fn;
        tuple<Args...> _args;
        optional<Return> _result = nullopt;
    };
    
    template<typename Return, typename ...Args>
    inline task<Return(Args...)>::task(move_only_function<Return(Args...)>&& fn, Args ...args)
        : _fn(ryujin::forward<move_only_function<Return(Args...)>>(fn)), _args(ryujin::forward<Args>(args)...)
    {
    }
    
    template<typename Return, typename ...Args>
    inline void task<Return(Args...)>::execute()
    {
        constexpr auto seq = index_sequence_for<Args...>();
        invoke_from_tuple(seq);
    }

    template<typename Return, typename ...Args>
    template<enable_if_t<!is_same_v<Return, void>, bool>>
    inline Return& task<Return(Args...)>::get_result() noexcept
    {
        return _result.value();
    }

    template<typename Return, typename ...Args>
    template<sz ...Is>
    inline void task<Return(Args...)>::invoke_from_tuple(index_sequence<Is...>)
    {
        if constexpr (is_same_v<Return, void>)
        {
            _fn(ryujin::get<Is>(_args)...);
        }
        else
        {
            _result = _fn(ryujin::get<Is>(_args)...);
        }
    }
}

#endif // task_hpp__
