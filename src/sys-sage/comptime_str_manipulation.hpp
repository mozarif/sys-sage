/*
 * This file provides the functionality for manipulating string literals at compile time.
 * In particular, it enables compile time string literal concatenation and compile time substring literal extraction for type name reflection.
 *
 * The former functionality was originally authored by user "cristi1990an" and shared on
 * https://www.reddit.com/r/cpp/comments/15z14hh/compile_time_string_literal_concatenation_or_how/
 * and
 * https://godbolt.org/z/Gdfnsf8Pa
 *
 * We have made slight modifications to the code (such as removing the delimiter and changing the namespace) and further extended it with the latter functionality.
 */

namespace sys_sage {
    /*
     * @private
     *
     * @brief Represents a string at compile time and is meant to be used in template metaprogramming.
     *        The string may result from copying a string literal, concatenating two strings or extracting a substring.
     */
    template <std::size_t N>
    struct CompStr {
        consteval CompStr(const char (&literal)[N]) noexcept
        {
            std::ranges::copy(literal, buffer);
        }
    
        template <std::size_t N1>
        consteval CompStr(const CompStr<N1> &compStr, std::size_t begin) noexcept
        {
            std::ranges::copy(compStr.buffer | std::ranges::views::drop(begin) | std::ranges::views::take(N - 1), buffer);
            buffer[N - 1] = '\0';
        }
    
        template <std::size_t N1, std::size_t N2> requires (N == N1 + N2 - 1)
        consteval CompStr(const CompStr<N1> &compStr1, const CompStr<N2> &compStr2) noexcept
        {
            std::ranges::copy(compStr2.buffer, std::ranges::copy(compStr1.buffer | std::ranges::views::take(N1 - 1), buffer).out);
        }
    
        char buffer[N];
    };

    /*
     * @private
     *
     * @brief deduction hint for the compiler
     */
    template <std::size_t N1, std::size_t N2>
    CompStr(const CompStr<N1> &, const CompStr<N2> &) -> CompStr<N1 + N2 - 1>;

    /*
     * @private
     *
     * @brief Effectively casts an internal buffer to a string literal.
     */
    template <CompStr compStr>
    consteval auto CompStrToLiteral() -> const char (&)[std::size(compStr.buffer)]
    {
        return compStr.buffer;
    }

    /*
     * @brief Returns the concatenation of two string literals at compile time.
     *        It forms the base case of the recursive string literal concatenation.
     */
    template <CompStr compStr1, CompStr compStr2>
    consteval decltype(auto) CompStrCat() noexcept
    {
        return CompStrToLiteral<CompStr(compStr1, compStr2)>();
    }

    /*
     * @brief Recursively concatenates string literals at compile time.
     */
    template <CompStr compStr1, CompStr compStr2, CompStr... compStrs> requires (sizeof...(compStrs) > 0)
    consteval decltype(auto) CompStrCat() noexcept
    {
        return CompStrCat<CompStr(compStr1, compStr2), compStrs...>();
    }

    /*
     * @private
     *
     * @brief Returns it's own function signature which contains a string representation for the given type `T`.
     */
    template <typename T>
    consteval auto &PrettyFunction()
    {
        return __PRETTY_FUNCTION__;
    }

    /*
     * @private
     *
     * @brief Returns a substring literal starting from `begin` and ending at `end`.
     */
    template <CompStr funcName, std::size_t begin, std::size_t end>
    consteval decltype(auto) CompStrExtract() noexcept
    {
        return CompStrToLiteral<CompStr<end - begin + 1>(funcName, begin)>();
    }

    /*
     * @private
     *
     * @brief Returns a substring literal by removing a prefix that ends with `prefix` and removing a suffix that starts with `suffix`.
     */
    template <CompStr funcName, CompStr prefix, CompStr suffix>
    consteval decltype(auto) CompStrExtract() noexcept
    {
        constexpr std::string_view strv ( funcName.buffer );
        return CompStrExtract<
            funcName,
            strv.find(prefix.buffer) + (std::size(prefix.buffer) - 1),
            strv.rfind(suffix.buffer)
        >();
    }

    /*
     * @brief Returns the string representation of the given type `T` at compile time.
     *        WARNING: The output is compiler-specific.
     *        There are NO GUARANTEES that the output is the same across different compilers or across versions of the same compiler.
     */
    template <typename T, CompStr prefix = "= ", CompStr suffix = "]">
    consteval decltype(auto) CompStrExtract() noexcept
    {
        return CompStrExtract<
            CompStr( PrettyFunction<T>() ),
            prefix,
            suffix
        >();
    }
}