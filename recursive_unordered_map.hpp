#pragma once

template<
    class Key,
    class T,
    class Hash = std::hash<Key>,
    class KeyEqual = std::equal_to<Key>
> class recursive_unordered_map
{
public:
    // Member types
    typedef Key key_type;
    typedef T mapped_type;
    typedef std::pair<Key, T> value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef Hash hasher;
    typedef KeyEqual key_equal;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;

    class iterator
    {
    public:
        iterator() 
            : array_(nullptr), index_(0) 
        {
        }

        iterator(pointer array, std::size_t index, std::size_t end, bool *occupied)
            : array_(array), index_(index), end_(end), occupied_(occupied)
        { 
            advance();
        }

        bool operator==(const iterator &comparand) const 
        { 
            return array_ == comparand.array_ && index_ == comparand.index_;
        }

        bool operator!=(const iterator &comparand) const 
        { 
            return !(*this == comparand);
        }

        iterator operator++(int)
        { 
            iterator copy(array_, index_);
            index_++;
            advance();
            return copy;
        }

        iterator &operator++()
        { 
            index_++;
            advance();
            return *this;
        }

        reference operator*() const 
        {
            return array_[index_];
        }

        pointer operator->() const
        {
            return &array_[index_];
        }

    private:
        void advance()
        {
            if(array_ == nullptr)
            {
                return;
            }

            while(index_ < end_ && !occupied_[index_])
            {
                index_++;
            }
        }

        pointer array_;
        std::size_t index_;
        std::size_t end_;
        bool *occupied_;
    };

    class const_iterator
    {
    public:
        const_iterator() 
            : array_(nullptr), index_(0) 
        {
        }

        const_iterator(const_pointer array, std::size_t index, std::size_t end, const bool *occupied)
            : array_(array), index_(index), end_(end), occupied_(occupied)
        { 
            advance();
        }

        bool operator==(const const_iterator &comparand) const 
        { 
            return array_ == comparand.array_ && index_ == comparand.index_;
        }

        bool operator!=(const const_iterator &comparand) const 
        {
            return !(*this == comparand);
        }

        const_iterator operator++(int) 
        { 
            const_iterator copy(array_, index_);
            index_++;
            advance();
            return copy;
        }

        const_iterator &operator++()
        { 
            index_++;
            advance();
            return *this;
        }

        const_reference operator*() const 
        { 
            return array_[index_];
        }

        const_pointer operator->() const
        {
            return &array_[index_];
        }

    private:
        void advance()
        {
            if(array_ == nullptr)
            {
                return;
            }

            while(index_ < end_ && !occupied_[index_])
            {
                index_++;
            }
        }

        const_pointer array_;
        std::size_t index_;
        std::size_t end_;
        const bool *occupied_;
    };

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // Member functions
    recursive_unordered_map() : container_(nullptr), size_(0), count_(0), max_load_factor_(0.75), occupied_(nullptr)
    {

    }

    ~recursive_unordered_map()
    {
        clear();
    }
    
    recursive_unordered_map(const recursive_unordered_map &other) : container_(other.size_ ? new value_type[other.size_] : nullptr), size_(other.size_), count_(other.count_), max_load_factor_(other.max_load_factor_), occupied_(other.size_ ? new bool[other.size_] : nullptr)
    {
        std::copy(other.container_, other.container_ + size_, container_);
        std::copy(other.occupied_, other.occupied_ + size_, occupied_);
    }
    
    recursive_unordered_map(recursive_unordered_map &&other)
    {
        swap(*this, other);
    }
    
    recursive_unordered_map &operator=(recursive_unordered_map other)
    {
        swap(*this, other);
        return *this;
    }
    
    friend void swap(recursive_unordered_map &first, recursive_unordered_map &second)
    {
        using std::swap;
        
        swap(first.size_, second.size_);
        swap(first.count_, second.count_);
        swap(first.container_, second.container_);
        swap(first.max_load_factor_, second.max_load_factor_);
        swap(first.occupied_, second.occupied_);
        swap(first.hasher_, second.hasher_);
        swap(first.key_equal_, second.key_equal_);
    }

    // Capacity
    bool empty() const
    {
        return count_ == 0;
    }

    std::size_t size() const
    {
        return count_;
    }

    std::size_t max_size() const
    {
        return 0;
    }

    // Iterators
    iterator begin()
    {
        return iterator(container_, 0, size_, occupied_);
    }

    const_iterator begin() const
    {
        return cbegin();
    }

    const_iterator cbegin() const
    {
        return const_iterator(container_, 0, size_, occupied_);
    }

    iterator end()
    {
        return iterator(container_, size_, size_, occupied_);
    }

    const_iterator end() const
    {
        return cend();
    }

    const_iterator cend() const
    {
        return const_iterator(container_, size_, size_, occupied_);
    }

    // Element access
    mapped_type &operator[](const key_type &key)
    {
        auto match = find(key);

        if(match == end())
        {
            insert(key, mapped_type());
            match = find(key);
        }

        return match->second;
    }

    const mapped_type &operator[](const key_type &key) const
    {
        return at(key);
    }

    mapped_type &at(const key_type &key)
    {
        auto match = find(key);

        if(match == end())
        {
            throw std::out_of_range("no element found with specified key");
        }

        return match->second;
    }

    const mapped_type &at(const key_type &key) const
    {
        auto match = find(key);

        if(match == end())
        {
            throw std::out_of_range("no element found with specified key");
        }

        return match->second;
    }

    // Element lookup
    iterator find(const key_type &key)
    {
        if(size_ == 0)
        {
            return end();
        }

        auto index = hash(key);
        size_type end_index = index == 0 ? size_ - 1 : index - 1;
        
        while(occupied_[index] && !key_equal_(container_[index].first, key) && index != end_index)
        {
            index++;

            if(index == size_)
            {
                index = 0;
            }
        }

        if(occupied_[index] && key_equal_(container_[index].first, key))
        {
            return iterator(container_, index, size_, occupied_);
        }

        return end();
    }

    const_iterator find(const key_type &key) const
    {
        if(size_ == 0)
        {
            return end();
        }

        auto index = hash(key);
        size_type end_index = index == 0 ? size_ - 1 : index - 1;
        
        while(occupied_[index] && !key_equal_(container_[index].first, key) && index != end_index)
        {
            index++;

            if(index == size_)
            {
                index = 0;
            }
        }

        if(occupied_[index] && key_equal_(container_[index].first, key))
        {
            return const_iterator(container_, index, size_, occupied_);
        }

        return end();
    }

    size_type count(const key_type &key) const
    {
        return find(key) == end() ? 0 : 1;
    }

    std::pair<iterator, iterator> equal_range(const key_type& key)
    {
        return {find(key), end()};
    }

    std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
    {
        return {find(key), end()};
    }
    
    // Modifiers
    std::pair<iterator, bool> emplace(key_type &&key, mapped_type &&value)
    {
        reserve(count_ + 1);
        
        size_type index = hash(key);
        size_type end = index == 0 ? size_ - 1 : index - 1;
        
        while(occupied_[index] && !key_equal_(container_[index].first, key) && index != end)
        {
            index++;
            
            if(index == size_)
            {
                index = 0;
            }
        }
        
        bool was_occupied = true;
        
        if(!occupied_[index])
        {
            was_occupied = false;
            occupied_[index] = true;
            count_++;
            swap(container_[index].first, key);
            swap(container_[index].second, value);
        }
        
        return std::pair<iterator, bool>(iterator(container_, index, size_, occupied_), !was_occupied);
    }
    
    void emplace_hint();

    void insert(const key_type &key, const mapped_type &value)
    {
        reserve(count_ + 1);

        size_type index = hash(key);
        size_type end = index == 0 ? size_ - 1 : index - 1;

        while(occupied_[index] && !key_equal_(container_[index].first, key) && index != end)
        {
            index++;

            if(index == size_)
            {
                index = 0;
            }
        }

        if(!occupied_[index] || key_equal_(container_[index].first, key))
        {
            occupied_[index] = true;
            container_[index].first = key;
            count_++;
        }

        container_[index].second = value;
    }

    void erase(const key_type &key);

    void clear()
    {
        if(size_ > 0)
        {
            count_ = 0;
            size_ = 0;
            delete[] container_;
            container_ = nullptr;
            delete[] occupied_;
            occupied_ = nullptr;
        }
    }

    // Hash policy
    double load_factor() const
    {
        return count_ / (double)size_;
    }

    double max_load_factor() const
    {
        return max_load_factor_;
    }

    void max_load_factor(double z)
    {
        max_load_factor_ = z;
    }

    void rehash(size_type n)
    {
        if(size_ == 0)
        {
            container_ = new value_type[n];
            occupied_ = new bool[n];
            std::fill(occupied_, occupied_ + n, false);
            size_ = n;
            return;
        }

        if(n <= size_)
        {
            return;
        }

        size_ = n;
        auto new_occupied = new bool[n];
        std::fill(new_occupied, new_occupied + size_, false);
        replace_container(container_, occupied_, count_, new value_type[n], new_occupied);
    }

    void reserve(size_type n)
    {
        if(size_ == 0)
        {
            container_ = new value_type[1];
            occupied_ = new bool[1];
            std::fill(occupied_, occupied_ + 1, false);
            size_ = 1;
            return;
        }
        
        std::size_t required_size = n / max_load_factor();

        if(required_size <= size_)
        {
            return;
        }

        rehash(size_ * 2);
    }

    // Observers
    hasher hash_function() const
    {
        return hasher_;
    }

    key_equal key_eq() const
    {
        return key_equal_;
    }

private:
    size_type hash(const key_type &key) const
    {
        return hasher_(key) % size_;
    }

    bool need_to_grow()
    {
        return size_ == 0 || load_factor() > max_load_factor();
    }

    void replace_container(pointer source_container, bool *source_occupied, size_type source_size, pointer replacement_container, bool *replacement_occupied)
    {
        container_ = replacement_container;
        occupied_ = replacement_occupied;
        count_ = 0;

        int source_index = 0;

        for(std::size_t i = 0; i < source_size; i++)
        {
            while(!source_occupied[source_index])
            {
                source_index++;
            }

            insert(source_container[source_index].first, source_container[source_index].second);
            source_index++;
        }

        delete[] source_container;
        delete[] source_occupied;
    }

    pointer container_;
    std::size_t size_;
    std::size_t count_;
    hasher hasher_;
    key_equal key_equal_;
    double max_load_factor_;
    bool *occupied_;
};

template <class Key, class T, class Hash, class Pred>
void swap(const recursive_unordered_map<Key, T, Hash, Pred>& lhs,
    const recursive_unordered_map<Key, T, Hash, Pred>& rhs)
{
    lhs.swap(rhs);
}

template <class Key, class T, class Hash, class Pred>
bool operator==(const recursive_unordered_map<Key, T, Hash, Pred>& lhs,
    const recursive_unordered_map<Key, T, Hash, Pred>& rhs)
{
    return false;
}

template <class Key, class T, class Hash, class Pred>
bool operator!=(const recursive_unordered_map<Key, T, Hash, Pred>& lhs,
    const recursive_unordered_map<Key, T, Hash, Pred>& rhs)
{
    return !(lhs == rhs);
}