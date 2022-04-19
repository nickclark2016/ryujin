#ifndef events_hpp__
#define events_hpp__

#include "../core/primitives.hpp"

#include <cstddef>
#include <functional>
#include <unordered_map>

namespace ryujin
{
    namespace detail
    {
        struct struct_identifier_utility
        {
            static sz id;

            template <typename T>
            static sz fetch_identifier()
            {
                static sz typeId = id++;
                return typeId;
            }
        };
    }

    class event_manager
    {
    public:
        template <typename T, typename ... Args>
        void emit(Args&& ... args) const noexcept;

        template <typename T>
        void subscribe(const std::function<void(const T&)> cb);

    private:
        using event_callback = std::function<void(const void*)>;

        std::unordered_map<sz, std::vector<event_callback>> _callbacks;
    };
    
    template <typename T, typename ... Args>
    inline void event_manager::emit(Args&& ... args) const noexcept
    {
        static const auto id = detail::struct_identifier_utility::fetch_identifier<T>();
        const auto& it = _callbacks.find(id);
        
        if (it != _callbacks.end())
        {
            const T e(std::forward<Args>(args)...);
            const std::vector<event_callback>& cbs = it->second;
            for (const auto& cb : cbs)
            {
                cb(&e);
            }
        }
    }
    
    template<typename T>
    inline void event_manager::subscribe(const std::function<void(const T&)> cb)
    {
        static const auto id = detail::struct_identifier_utility::fetch_identifier<T>();
        std::vector<event_callback>& cbs = _callbacks[id];
        cbs.push_back([=](const void* e) {
                const T* casted_e = reinterpret_cast<const T*>(e);
                cb(*casted_e);
            });
    }
}

#endif // events_hpp__