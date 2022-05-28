#ifndef tree_hpp__
#define tree_hpp__

#include "allocator.hpp"
#include "functional.hpp"
#include "iterator.hpp"
#include "primitives.hpp"

#include <new>

namespace ryujin
{
    template <typename T, template <typename> typename Allocator = allocator, typename LessComparator = less<T>, typename EqualityComparator = equal_to<T>, bool AllowDuplicates = true>
    class rb_tree
    {
        enum class node_color
        {
            BLACK,
            RED,
        };

        struct node
        {
            T value;
            node_color color = node_color::BLACK;
            node* parent = nullptr;
            node* left = nullptr;
            node* right = nullptr;

            node(const T& value);
            node(T&& value);

            node* successor() const noexcept;
            node* minimum_descendant() const noexcept;
        };

        struct in_order_iterator
        {
            using difference_type = ptr_diff;
            using value_type = remove_cv_t<T>;
            using pointer = T*;
            using reference = T&;
            using iterator_category = forward_iterator_tag;
            using iterator_concept = forward_iterator_tag;

            node* n;

            bool operator==(const in_order_iterator& rhs) const noexcept;
            bool operator!=(const in_order_iterator& rhs) const noexcept;

            T& operator*() noexcept;
            const T& operator*() const noexcept;
            T* operator->() const noexcept;

            // prefix
            in_order_iterator operator++() noexcept;

            // postfix
            in_order_iterator operator++(int) noexcept;
        };

    public:

        using iterator = in_order_iterator;
        using const_iterator = in_order_iterator;

        rb_tree() = default;
        rb_tree(const rb_tree& tree);
        rb_tree(rb_tree&& tree) noexcept;
        ~rb_tree();

        rb_tree& operator=(const rb_tree& rhs);
        rb_tree& operator=(rb_tree&& rhs) noexcept;

        iterator find(const T& v) const noexcept;

        iterator erase(iterator it);
        iterator insert(const T& v);

        iterator begin() noexcept;
        const_iterator begin() const noexcept;
        const_iterator cbegin() const noexcept;

        iterator end() noexcept;
        const_iterator end() const noexcept;
        const_iterator cend() const noexcept;

        bool empty() const noexcept;
        sz size() const noexcept;

    private:
        node* _root = nullptr;
        sz _size = 0;

        Allocator<node> _nodeAllocator = {};
        LessComparator _less = {};
        EqualityComparator _equals = {};

        void _rotate_left(node* n);
        void _rotate_right(node* n);
        void _delete_node(node* n);
        void _rebalance_after_insert(node* n);
        void _rebalance_after_delete(node* n);
        void _transplant_node(node* u, node* v);
        void _erase_helper(iterator it);
        node* _minimum_helper(node* n);

        node* _copy_helper(node* n);
    };

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::node::node(const T& value)
        : value(value), left(nullptr), right(nullptr), parent(nullptr)
    {
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::node::node(T&& value)
        : value(ryujin::forward<T>(value)), left(nullptr), right(nullptr), parent(nullptr)
    {
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::node* rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::node::successor() const noexcept
    {
        if (right != nullptr)
        {
            return right->minimum_descendant();
        }

        node* p = parent;
        const node* n = this;
        while (p != nullptr && n == p->right)
        {
            n = p;
            p = p->parent;
        }
        return p;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::node* rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::node::minimum_descendant() const noexcept
    {
        node* it = const_cast<node*>(this);
        while (it->left != nullptr)
        {
            it = it->left;
        }
        return it;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::rb_tree(const rb_tree& tree)
        : _size(tree._size)
    {
        _root = _copy_helper(tree._root);
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::rb_tree(rb_tree&& tree) noexcept
        : _root(ryujin::move(tree._root))
    {
        tree._root = nullptr;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::~rb_tree()
    {
        _delete_node(_root);
        _size = 0;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>& rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::operator=(const rb_tree& rhs)
    {
        if (&rhs == this)
        {
            return *this;
        }

        _delete_node(_root);
        _root = _copy_helper(rhs._root);

        _size = rhs._size;

        return *this;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>& rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::operator=(rb_tree&& rhs) noexcept
    {
        if (&rhs == this)
        {
            return *this;
        }

        _root = rhs._root;
        _size = rhs._size;

        rhs._root = nullptr;
        rhs._size = 0;

        return *this;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline typename rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::iterator rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::find(const T& v) const noexcept
    {
        node* it = _root;
        while (it != nullptr)
        {
            if (_less(v, it->value))
            {
                it = it->left;
            }
            else
            {
                if (_equals(v, it->value))
                {
                    return in_order_iterator{ .n = it };
                }
                it = it->right;
            }
        }
        return end();
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline typename rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::iterator rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::erase(iterator it)
    {
        if (it == end())
        {
            return end();
        }
        auto next = it;
        ++next;

        _erase_helper(it);

        return next;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline typename rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::iterator rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::insert(const T& v)
    {
        node* parent = nullptr;
        node* x = _root;

        while (x != nullptr)
        {
            parent = x;
            if (_less(v, x->value))
            {
                x = x->left;
            }
            else
            {
                if constexpr (!AllowDuplicates)
                {
                    if (_equals(v, x->value))
                    {
                        return end();
                    }
                }
                x = x->right;
            }
        }

        node* n = _nodeAllocator.allocate(1);
        ::new(n) node(v);

        n->parent = parent;
        n->color = node_color::RED;

        if (parent == nullptr)
        {
            _root = n;
        }
        else if (_less(v, parent->value))
        {
            parent->left = n;
        }
        else
        {
            parent->right = n;
        }

        ++_size;

        if (n->parent == nullptr)
        {
            n->color = node_color::BLACK;
            return in_order_iterator{ .n = n };
        }

        if (n->parent->parent == nullptr)
        {
            return in_order_iterator{ .n = n };
        }

        _rebalance_after_insert(n);

        return in_order_iterator{ .n = n };
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline typename rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::iterator rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::begin() noexcept
    {
        if (_size == 0)
        {
            return end();
        }

        node* it = _root->minimum_descendant();

        return in_order_iterator{ .n = it };
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline typename rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::const_iterator rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::begin() const noexcept
    {
        if (_size == 0)
        {
            return end();
        }

        node* it = _root->minimum_descendant();

        return in_order_iterator{ .n = it };
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline typename rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::const_iterator rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::cbegin() const noexcept
    {
        if (_size == 0)
        {
            return end();
        }

        node* it = _root->minimum_descendant();

        return in_order_iterator{ .n = it };
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline typename rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::iterator rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::end() noexcept
    {
        return in_order_iterator{ .n = nullptr };
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline typename rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::const_iterator rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::end() const noexcept
    {
        return in_order_iterator{ .n = nullptr };
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline typename rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::const_iterator rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::cend() const noexcept
    {
        return in_order_iterator{ .n = nullptr };
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline bool ryujin::rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::empty() const noexcept
    {
        return _size == 0;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline sz rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::size() const noexcept
    {
        return _size;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline void rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::_rotate_left(node* n)
    {
        node* y = n->right;
        n->right = y->left;
        
        if (y->left != nullptr)
        {
            y->left->parent = n;
        }

        y->parent = n->parent;

        if (n->parent == nullptr)
        {
            _root = y;
        }
        else if (n == n->parent->left)
        {
            n->parent->left = y;
        }
        else
        {
            n->parent->right = y;
        }

        y->left = n;
        n->parent = y;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline void rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::_rotate_right(node* n)
    {
        node* y = n->left;
        n->left = y->right;

        if (y->right != nullptr)
        {
            y->left->parent = n;
        }

        y->parent = n->parent;

        if (n->parent == nullptr)
        {
            _root = y;
        }
        else if (n == n->parent->right)
        {
            n->parent->right = y;
        }
        else
        {
            n->parent->left = y;
        }
        y->right = n;
        n->parent = y;
    }
    
    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline void rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::_delete_node(node* n)
    {
        if (n == nullptr)
        {
            return;
        }

        _delete_node(n->left);
        _delete_node(n->right);
        n->~node();
        _nodeAllocator.deallocate(n, 1);
    }
    
    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline void rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::_rebalance_after_insert(node* n)
    {
        node* p = n;
        while (p->parent->color == node_color::RED)
        {
            // if the parent is the right child of it's parent
            if (p->parent == p->parent->parent->right)
            {
                node* u = p->parent->parent->left;
                if (u && u->color == node_color::RED)
                {
                    u->color = node_color::BLACK;
                    p->parent->color = node_color::BLACK;
                    p->parent->parent->color = node_color::RED;
                    p = p->parent->parent; // reset P to it's grandparent
                }
                else
                {
                    if (p == p->parent->left)
                    {
                        p = p->parent;
                        _rotate_right(p);
                    }
                    p->parent->color = node_color::BLACK;
                    p->parent->parent->color = node_color::RED;
                    _rotate_left(p->parent->parent);
                }
            }
            else
            {
                node* u = p->parent->parent->right;
                if (u && u->color == node_color::RED)
                {
                    u->color = node_color::BLACK;
                    p->parent->color = node_color::BLACK;
                    p->parent->parent->color = node_color::RED;
                    p = p->parent->parent;
                }
                else
                {
                    if (p == p->parent->right)
                    {
                        p = p->parent;
                        _rotate_left(p);
                    }
                    p->parent->color = node_color::BLACK;
                    p->parent->parent->color = node_color::RED;
                    _rotate_right(p->parent->parent);
                }
            }

            if (p == _root)
            {
                break;
            }
        }
        _root->color = node_color::BLACK;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline void rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::_rebalance_after_delete(node* n)
    {
        node* p = n;
        while (p != _root && p->color == node_color::BLACK)
        {
            if (p == p->parent->left)
            {
                node* s = p->parent->right;
                if (s->color == node_color::RED)
                {
                    s->color = node_color::BLACK;
                    p->parent->color = node_color::RED;
                    _rotate_left(p->parent);
                    s = p->parent->right;
                }

                if (s->left->color == node_color::BLACK && s->right->color == node_color::BLACK)
                {
                    s->color = node_color::RED;
                    p = p->parent;
                }
                else
                {
                    if (s->right->color == node_color::BLACK)
                    {
                        s->left->color = node_color::BLACK;
                        s->color = node_color::RED;
                        _rotate_right(s);
                        s = p->parent->right;
                    }

                    s->color = p->parent->color;
                    p->parent->color = node_color::BLACK;
                    s->right->color = node_color::BLACK;
                    _rotate_left(p->parent);
                    p = _root;
                }
            }
            else
            {
                node* s = p->parent->left;
                if (s->color == node_color::RED)
                {
                    s->color = node_color::BLACK;
                    p->parent->color = node_color::RED;
                    _rotate_right(p->parent);
                    s = p->parent->left;
                }

                if (s->right->color == node_color::BLACK && s->right->color == node_color::BLACK)
                {
                    s->color = node_color::RED;
                    p = p->parent;
                }
                else
                {
                    if (s->left->color == 0)
                    {
                        s->right->color = node_color::BLACK;
                        s->color = node_color::RED;
                        _rotate_left(s);
                        s = p->parent->left;
                    }

                    s->color = p->parent->color;
                    p->parent->color = node_color::BLACK;
                    s->left->color = node_color::BLACK;
                    _rotate_right(p->parent);
                    p = _root;
                }
            }
        }

        p->color = node_color::BLACK;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline void rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::_transplant_node(node* u, node* v)
    {
        if (u->parent == nullptr)
        {
            _root = v;
        }
        else if (u == u->parent->left)
        {
            u->parent->left = v;
        }
        else
        {
            u->parent->right = v;
        }
        v->parent = u->parent;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline void rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::_erase_helper(iterator it)
    {
        node* x = nullptr, y = nullptr;
        node* toDelete = it.n;

        if (it == end())
        {
            return;
        }

        y = toDelete;

        node_color originalColor = y->color;
        if (toDelete->left == nullptr)
        {
            x = toDelete->right;
            _transplant_node(toDelete, toDelete->right);
        }
        else if (toDelete->right == nullptr)
        {
            x = toDelete->left;
            _transplant_node(toDelete, toDelete->left);
        }
        else
        {
            y = _minimum_helper(toDelete->right); // focus around the minimum in the right subtree
            originalColor = y->color;
            x = y->right;
            if (y->parent == toDelete)
            {
                x->parent = y;
            }
            else
            {
                _transplant_node(y, y->right);
                y->right = toDelete->right;
                y->right->parent = y;
            }

            _transplant_node(toDelete, y);
            y->left = toDelete->left;
            y->left->parent = y;
            y->color = toDelete->color;
        }
        _nodeAllocator.deallocate(toDelete, 1);
        if (originalColor == node_color::BLACK)
        {
            _rebalance_after_delete(x);
        }
        --_size;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::node* rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::_minimum_helper(node* n)
    {
        return n->minimum_descendant();
    }
    
    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::node* rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::_copy_helper(node* n)
    {
        if (n == nullptr)
        {
            return nullptr;
        }

        node* p = _nodeAllocator.allocate(1);
        ::new(p) node(n->value);

        node* left = _copy_helper(n->left);
        node* right = _copy_helper(n->right);
        p->left = left;
        p->right = right;
        
        if (left != nullptr)
        {
            left->parent = p;
        }

        if (right != nullptr)
        {
            right->parent = p;
        }

        return p;
    }
    
    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline bool rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::in_order_iterator::operator==(const in_order_iterator& rhs) const noexcept
    {
        return n == rhs.n;
    }
    
    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline bool rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::in_order_iterator::operator!=(const in_order_iterator& rhs) const noexcept
    {
        return n != rhs.n;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline T& rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::in_order_iterator::operator*() noexcept
    {
        return n->value;
    }
    
    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline const T& rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::in_order_iterator::operator*() const noexcept
    {
        return n->value;
    }
    
    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline T* rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::in_order_iterator::operator->() const noexcept
    {
        return &n->value;
    }
    
    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::in_order_iterator rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::in_order_iterator::operator++() noexcept
    {
        n = n->successor();
        return *this;
    }

    template <typename T, template <typename> typename Allocator, typename LessComparator, typename EqualityComparator, bool AllowDuplicates>
    inline rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::in_order_iterator rb_tree<T, Allocator, LessComparator, EqualityComparator, AllowDuplicates>::in_order_iterator::operator++(int) noexcept
    {
        in_order_iterator it = *this;
        n = n->successor();
        return it;
    }
}

#endif // tree_hpp__
