#pragma once
#include <cstdint>
#include <cstddef>

namespace Titan {
using u32 = uint32_t;
using f32 = float;

template<typename T>
struct Span {
    const T* ptr;
    size_t count;
    Span() : ptr(nullptr), count(0) {}
    template<size_t N> constexpr Span(const T(&arr)[N]) : ptr(arr), count(N) {}
    Span(const T* p, size_t c) : ptr(p), count(c) {}
    const T* begin() const { return ptr; }
    const T* end() const { return ptr + count; }
    size_t size() const { return count; }
};
}
