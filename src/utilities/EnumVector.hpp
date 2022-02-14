#pragma once

#include <initializer_list>
#include <utility>
#include <vector>

/**
 * @brief An std::vector wrapper that can use an enum class as indices. By
 * default, it uses K::Size as the size of the vector, unless overridden.
 *
 * Most functions simply call the internal std::vector's function. Most
 * functions that can change the size of the internal vector are not exposed.
 *
 * T must be default constructible.
 */
template <
    typename K,
    typename T,
    std::size_t Size = static_cast<std::size_t>(K::Size),
    typename Allocator = std::allocator<T>>
class EnumVector {
    std::vector<T, Allocator> data_;
    using wrapped_vector_ = decltype(data_);

public:
    using value_type = wrapped_vector_::value_type;
    using allocator_type = wrapped_vector_::allocator_type;
    using size_type = wrapped_vector_::size_type;
    using difference_type = wrapped_vector_::difference_type;
    using reference = wrapped_vector_::reference;
    using const_reference = wrapped_vector_::const_reference;
    using pointer = wrapped_vector_::pointer;
    using const_pointer = wrapped_vector_::const_pointer;
    using iterator = wrapped_vector_::iterator;
    using const_iterator = wrapped_vector_::const_iterator;
    using reverse_iterator = wrapped_vector_::reverse_iterator;
    using const_reverse_iterator = wrapped_vector_::const_reverse_iterator;

    constexpr EnumVector() noexcept(noexcept(Allocator()))
        : data_(Size)
    {}
    constexpr explicit EnumVector(const Allocator& alloc) noexcept
        : data_(Size, alloc)
    {}
    constexpr EnumVector(const EnumVector& other) = default;
    constexpr EnumVector(const EnumVector& other, const Allocator& alloc)
        : data_(other.data_, alloc)
    {}
    constexpr EnumVector(EnumVector&& other) noexcept = default;
    constexpr EnumVector(EnumVector&& other, const Allocator& alloc) noexcept
        : data_(std::move(other.data_), alloc)
    {}

    constexpr ~EnumVector() {}

    constexpr EnumVector& operator=(const EnumVector&) = default;
    constexpr EnumVector& operator=(EnumVector&&) noexcept = default;

    constexpr allocator_type get_allocator() const noexcept
    {
        return data_.get_allocator();
    }

    constexpr reference at(const K key)
    {
        return data_.at(static_cast<size_type>(key));
    }
    constexpr const_reference at(const K key) const
    {
        return data_.at(static_cast<size_type>(key));
    }

    constexpr reference operator[](const K key)
    {
        return data_[static_cast<size_type>(key)];
    }
    constexpr const_reference operator[](const K key) const
    {
        return data_[static_cast<size_type>(key)];
    }

    constexpr reference front() { return data_.front(); }
    constexpr const_reference front() const { return data_.front(); }

    constexpr reference back() { return data_.back(); }
    constexpr const_reference back() const { return data_.back(); }

    constexpr T* data() noexcept { return data_.data(); }
    constexpr const T* data() const noexcept { return data_.data(); }

    constexpr iterator begin() noexcept { return data_.begin(); }
    constexpr const_iterator begin() const noexcept { return data_.begin(); }
    constexpr const_iterator cbegin() const noexcept { return data_.cbegin(); }

    constexpr iterator end() noexcept { return data_.end(); }
    constexpr const_iterator end() const noexcept { return data_.end(); }
    constexpr const_iterator cend() const noexcept { return data_.cend(); }

    constexpr reverse_iterator rbegin() noexcept { return data_.rbegin(); }
    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return data_.rbegin();
    }
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return data_.crbegin();
    }

    constexpr reverse_iterator rend() noexcept { return data_.rend(); }
    constexpr const_reverse_iterator rend() const noexcept
    {
        return data_.rend();
    }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return data_.crend();
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return data_.empty();
    }

    constexpr size_type size() const noexcept { return data_.size(); }
    constexpr size_type max_size() const noexcept { return data_.max_size(); }
    constexpr size_type capacity() const noexcept { return data_.capacity(); }

    constexpr void swap(EnumVector& other) noexcept { data_.swap(other.data); }
};
