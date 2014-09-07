#pragma once

#include <memory>

template<class T>
class recursive_vector
{
public:
    // Member types
    typedef T value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;

    class iterator
    {
    public:
        iterator() : array_(nullptr), index_(0) {}
        iterator(pointer array, std::size_t index) : array_(array), index_(index) {}
        bool operator==(const iterator &comparand) const { return array_ == comparand.array_ && index_ == comparand.index_; }
        bool operator!=(const iterator &comparand) const { return !(*this == comparand); }
        iterator operator++(int) { iterator copy(array_, index_); index_++; return copy; }
        iterator &operator++() { index_++; return *this; }
        reference operator*() const { return array_[index_]; }

    private:
        pointer array_;
        std::size_t index_;
    };

    class const_iterator
    {
    public:
        const_iterator() : array_(nullptr), index_(0) {}
        const_iterator(const_pointer array, std::size_t index) : array_(array), index_(index) {}
        bool operator==(const const_iterator &comparand) const { return array_ == comparand.array_ && index_ == comparand.index_; }
        bool operator!=(const const_iterator &comparand) const { return !(*this == comparand); }
        const_iterator operator++(int) { const_iterator copy(array_, index_); index_++; return copy; }
        const_iterator &operator++() { index_++; return *this; }
        const_reference operator*() const { return array_[index_]; }

    private:
        const_pointer array_;
        std::size_t index_;
    };

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // Member functions
    recursive_vector() : container_(nullptr), size_(0), count_(0)
    {

    }

    ~recursive_vector()
    {
        delete[] container_;
    }

    recursive_vector(const recursive_vector &other) : container_(other.size_ > 0 ? new value_type[other.size_] : nullptr), size_(other.size_), count_(other.count_)
    {
        std::copy(other.container_, other.container_ + size_, container_);
    }
    
    recursive_vector(recursive_vector &&other)
    {
        swap(*this, other);
    }
    
    template<class... Args>
    iterator emplace_back(Args&&... args)
    {
        if(count_ + 1 > size_)
        {
            grow();
        }
        
        container_[count_] = value_type(std::forward<Args>(args)...);
        return iterator(container_, count_++);
    }
    
    recursive_vector &operator=(recursive_vector other)
    {
        swap(*this, other);
        return *this;
    }
    
    friend void swap(recursive_vector &first, recursive_vector &second)
    {
        using std::swap;
        
        swap(first.size_, second.size_);
        swap(first.count_, second.count_);
        swap(first.container_, second.container_);
    }

    reference operator[](size_type index)
    {
        return container_[index];
    }

    const_reference operator[](size_type index) const
    {
        return container_[index];
    }

    void clear()
    {
        count_ = 0;
        size_ = 0;
        delete[] container_;
        container_ = nullptr;
    }

    size_type size() const
    {
        return count_;
    }

    // Iterators
    iterator begin()
    {
        return iterator(container_, 0);
    }

    const_iterator cbegin() const
    {
        return const_iterator(container_, 0);
    }

    iterator end()
    {
        return iterator(container_, count_);
    }

    const_iterator cend() const
    {
        return const_iterator(container_, count_);
    }

    // Modifiers
    void push_back(const T &t)
    {
        if(count_ + 1 > size_)
        {
            grow();
        }

        container_[count_++] = t;
    }

private:
    void grow()
    {
        if(size_ == 0)
        {
            container_ = new T[1];
            size_ = 1;
            return;
        }

        size_ *= 2;
        pointer old = container_;
        container_ = new value_type[size_];
        std::copy(old, old + count_, container_);
        delete[] old;
    }

    pointer container_;
    size_type size_;
    size_type count_;
};
