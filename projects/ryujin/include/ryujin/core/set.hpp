#ifndef set_hpp__
#define set_hpp__

#include "allocator.hpp"
#include "primitives.hpp"
#include "tree.hpp"

namespace ryujin
{
    template <typename T, template <typename> typename Allocator = allocator, typename Less = less<T>, typename Equals = equal_to<T>>
    class set
    {
    public:
        using iterator = typename rb_tree<T, Allocator, Less, Equals, false>::iterator;
        using const_iterator = typename rb_tree<T, Allocator, Less, Equals, false>::const_iterator;
        
        bool empty() const noexcept;
        sz size() const noexcept;
        
        iterator erase(iterator it);
        iterator insert(const T& t);
        iterator insert(T&& t);

        iterator find(const T& t) const noexcept;

        iterator begin() noexcept;
        const_iterator begin() const noexcept;
        const_iterator cbegin() const noexcept;

        iterator end() noexcept;
        const_iterator end() const noexcept;
        const_iterator cend() const noexcept;

    private:
        rb_tree<T, Allocator, Less, Equals, false> _storage;
    };
    
    template <typename T, template <typename> typename Allocator, typename Less, typename Equals>
    inline bool set<T, Allocator, Less, Equals>::empty() const noexcept
    {
        return _storage.empty();
    }

    template <typename T, template <typename> typename Allocator, typename Less, typename Equals>
    inline sz set<T, Allocator, Less, Equals>::size() const noexcept
    {
        return _storage.size();
    }
    
    template<typename T, template <typename> typename Allocator, typename Less, typename Equals>
    inline set<T, Allocator, Less, Equals>::iterator set<T, Allocator, Less, Equals>::erase(iterator it)
    {
        return _storage.erase(it);
    }
    
    template<typename T, template <typename> typename Allocator, typename Less, typename Equals>
    inline set<T, Allocator, Less, Equals>::iterator set<T, Allocator, Less, Equals>::insert(const T& t)
    {
        return _storage.insert(t);
    }
    
    template<typename T, template <typename> typename Allocator, typename Less, typename Equals>
    inline set<T, Allocator, Less, Equals>::iterator set<T, Allocator, Less, Equals>::insert(T&& t)
    {
        return _storage.insert(ryujin::forward<T>(t));
    }
    
    template<typename T, template <typename> typename Allocator, typename Less, typename Equals>
    inline set<T, Allocator, Less, Equals>::iterator set<T, Allocator, Less, Equals>::find(const T& t) const noexcept
    {
        return _storage.find(t);
    }
    
    template<typename T, template <typename> typename Allocator, typename Less, typename Equals>
    inline set<T, Allocator, Less, Equals>::iterator set<T, Allocator, Less, Equals>::begin() noexcept
    {
        return _storage.begin();
    }
    
    template<typename T, template <typename> typename Allocator, typename Less, typename Equals>
    inline set<T, Allocator, Less, Equals>::const_iterator set<T, Allocator, Less, Equals>::begin() const noexcept
    {
        return _storage.begin();
    }
    
    template<typename T, template <typename> typename Allocator, typename Less, typename Equals>
    inline set<T, Allocator, Less, Equals>::const_iterator set<T, Allocator, Less, Equals>::cbegin() const noexcept
    {
        return _storage.cbegin();
    }

    template<typename T, template <typename> typename Allocator, typename Less, typename Equals>
    inline set<T, Allocator, Less, Equals>::iterator set<T, Allocator, Less, Equals>::end() noexcept
    {
        return _storage.end();
    }

    template<typename T, template <typename> typename Allocator, typename Less, typename Equals>
    inline set<T, Allocator, Less, Equals>::const_iterator set<T, Allocator, Less, Equals>::end() const noexcept
    {
        return _storage.end();
    }

    template<typename T, template <typename> typename Allocator, typename Less, typename Equals>
    inline set<T, Allocator, Less, Equals>::const_iterator set<T, Allocator, Less, Equals>::cend() const noexcept
    {
        return _storage.cend();
    }
}

#endif // set_hpp__
