#pragma once

namespace detail {
    // https://en.cppreference.com/w/cpp/utility/variant/visit
    template<typename... Ts> struct overloaded: Ts... { using Ts::operator()...; };
    template<typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    // https://habr.com/ru/post/106294/#comment_3341606
    /// This template functions should be used when we perform cast from one pointer type to another
    /// It's safer than using reiterpret_cast
    ///
    /// It doesn't allow to do such things like:
    /// int i = 10;
    /// A *a = pointer_cast<A*>(i);
    /// Only pointer could be used in this function.

    template<typename result, typename source>
    result pointer_cast(source *v)
    {
        return static_cast<result>(static_cast<void*>(v));
    }

    template<typename result, typename source>
    result pointer_cast(const source *v)
    {
        return static_cast<result>(static_cast<const void*>(v));
    }
};