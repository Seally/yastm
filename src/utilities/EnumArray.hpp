#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <cstddef>

/**
 * @brief An std::array wrapper that can use an enum class as indices. By
 * default, it uses K::Size as the size of the array, unless overridden.
 *
 * Most functions simply call the internal std::array's function. Most
 * functions that can change the size of the internal vector are not exposed.
 *
 * T must be default constructible.
 */
template <
    typename K,
    typename T,
    std::size_t Size = static_cast<std::size_t>(K::Size)>
class EnumArray {
    std::array<T, Size> data_;
    using wrapped_array_ = decltype(data_);

public:
    using value_type = wrapped_array_::value_type;
    using size_type = wrapped_array_::size_type;
    using difference_type = wrapped_array_::difference_type;
    using reference = wrapped_array_::reference;
    using const_reference = wrapped_array_::const_reference;
    using pointer = wrapped_array_::pointer;
    using const_pointer = wrapped_array_::const_pointer;
    using iterator = wrapped_array_::iterator;
    using const_iterator = wrapped_array_::const_iterator;
    using reverse_iterator = wrapped_array_::reverse_iterator;
    using const_reverse_iterator = wrapped_array_::const_reverse_iterator;

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

    template <K Key>
    constexpr reference get() noexcept
    {
        static_assert(
            static_cast<std::size_t>(Key) < Size,
            "array index out of bounds");

        return (*this)[Key];
    }

    template <K Key>
    constexpr const_reference get() const noexcept
    {
        static_assert(
            static_cast<std::size_t>(Key) < Size,
            "array index out of bounds");

        return (*this)[Key];
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

    constexpr void swap(EnumArray& other) noexcept { data_.swap(other.data_); }
    constexpr void fill(const T& value) { data_.fill(value); }

    friend constexpr bool operator==(const EnumArray& lhs, const EnumArray& rhs)
    {
        return lhs.data_ == rhs.data_;
    }

    friend constexpr auto
        operator<=>(const EnumArray& lhs, const EnumArray& rhs)
    {
        return lhs.data_ <=> rhs.data_;
    }
};

namespace std {
    template <
        typename K,
        typename T,
        std::size_t Size = static_cast<std::size_t>(K::Size)>
    void swap(EnumArray<K, T, Size>& lhs, EnumArray<K, T, Size>& rhs) noexcept
    {
        lhs.swap(rhs);
    }
} // namespace std
