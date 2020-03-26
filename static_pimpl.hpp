//
//  static_pimpl.h
//  static_pimpl
//
//  Created by k1why on 3/26/20.
//  Copyright © 2020 k1why. All rights reserved.
//

#pragma once

#include <new>
#include <cstdint>
#include <type_traits>

namespace pimpl {

constexpr std::uint32_t default_impl_size      = 8;
constexpr std::uint32_t default_impl_alignment = 4;

namespace traits {
    template <bool enable> struct move_control{};
    template <bool enable> struct copy_control{};

    template <> struct move_control<false>
    {
        move_control() = default;
        
        move_control(move_control&&) = delete;
        move_control& operator = (move_control&&) = delete;
    };

    template <> struct copy_control<false>
    {
        copy_control() = default;
        
        copy_control(const copy_control&) = delete;
        copy_control& operator = (const copy_control&) = delete;
    };

    template <class T>
    constexpr bool is_copyable_v = std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T>;

    template <class T>
    constexpr bool is_movable_v = std::is_move_constructible_v<T> || std::is_move_assignable_v<T>;
}

template <class T, uint32_t size = default_impl_size, std::uint32_t alignment = default_impl_alignment>
class static_pimpl //: traits::move_control<traits::is_movable_v<T>>, traits::copy_control<traits::is_copyable_v<T>>
{
public:
    template <class... Args>
    explicit static_pimpl(Args... args) noexcept {
        new (&m_storage) T(std::forward<Args>(args)...);
    }
    
    static_pimpl(const static_pimpl&  rhs) { new (&m_storage) T(*rhs); }
    static_pimpl(      static_pimpl&& rhs) { new (&m_storage) T(std::move(*rhs)); }
    
    ~static_pimpl() noexcept {
        validate<sizeof(T), alignof(T)>();
        get_ptr()->~T();
    }
    
    const T* operator -> () const noexcept { return get_ptr(); }
          T* operator -> ()       noexcept { return get_ptr(); }
    
    const T& operator * () const noexcept { return *get_ptr(); }
          T& operator * ()       noexcept { return *get_ptr(); }
    
private:
    const T* get_ptr() const noexcept { return std::launder(reinterpret_cast<const T*>(&m_storage)); }
          T* get_ptr()       noexcept { return std::launder(reinterpret_cast<      T*>(&m_storage)); }
    
    template <uint32_t actual_size, uint32_t actual_alignment>
    constexpr void validate()
    {
        static_assert(actual_size      == size,      "static_pimpl: size mismatch");
        static_assert(actual_alignment == alignment, "static_pimpl: alignment mismatch");
    }
    
private:
    std::aligned_storage<size, alignment> m_storage;
};
}
