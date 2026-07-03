/**
 * This file provides the functionality of concatenating string literals at compile time.
 *
 * This awesome implementation was authored by the user "cristi1990an" and was shared on
 * https://www.reddit.com/r/cpp/comments/15z14hh/compile_time_string_literal_concatenation_or_how/
 *
 * A few modifications have been made to the original code of https://godbolt.org/z/Gdfnsf8Pa
 *   - removed string concatenation with delimiter
 *   - changed namespaces
 */

namespace sys_sage {
    template <std::size_t N>
    struct string_literal {
        consteval string_literal(const char (&arr)[N]) noexcept
        {
            std::ranges::copy_n(arr, N, buffer);
        }

        template <std::size_t LhsSize, std::size_t RhsSize>
        requires (LhsSize + RhsSize - 1 == N)
        consteval string_literal(const string_literal<LhsSize>& lhs, const string_literal<RhsSize>& rhs) noexcept
        {
            auto it = std::ranges::copy(lhs.buffer | std::views::take(LhsSize - 1), buffer).out;
            std::ranges::copy(rhs.buffer, it);
        }

        char buffer[N] {};
        std::size_t size = N;
    };

    template <std::size_t LhsSize, std::size_t RhsSize>
    string_literal(string_literal<LhsSize>, string_literal<RhsSize>) -> string_literal<LhsSize + RhsSize - 1>;

    template <string_literal String>
    consteval auto get_static_buffer() noexcept -> const char (&)[String.size]
    {
        return String.buffer;
    }

    template <string_literal Lhs, string_literal Rhs>
    consteval auto static_concat() noexcept -> const char (&)[Lhs.size + Rhs.size - 1]
    {
        return get_static_buffer<string_literal{Lhs, Rhs}>();
    }
    
    template <string_literal Lhs, string_literal Rhs, string_literal ... Others>
    requires (sizeof...(Others) != 0)
    consteval decltype(auto) static_concat() noexcept
    {
        return static_concat<string_literal{Lhs, Rhs}, Others...>();
    }
}
