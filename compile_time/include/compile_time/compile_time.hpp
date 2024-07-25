#ifndef COMPILE_TIME_H
#define COMPILE_TIME_H

#include <limits>
#include <type_traits>

namespace compile_time
{   
    // // What happens if Target is not in List types??
    // /// @brief Gets the index of a type in the list of templated types
    // /// @tparam Target should be the type we want to find the index for
    // /// @tparam ListHead first type in the list of template args
    // /// @tparam ...ListTails the remaning type of list args
    // /// @return std::size_t that represents index of Target in {ListHead, ListTains...}
    // template<typename Target, typename ListHead, typename... ListTails>
    // constexpr std::size_t get_type_index()
    // {
    //     if constexpr (std::is_same<Target, ListHead>::value)
    //         return 0;
    //     else
    //         return 1 + get_type_index<Target, ListTails...>();
    // }

    /// @brief Class to workout the position of a type in a parameter pack
    /// @tparam Head the type to find the index for.
    /// @tparam HeadOfTail the current head of the parameter pack.
    /// @tparam ...Ts  the remaining types in the parameter pack.
    template <typename Head, typename HeadOfTail, typename... Ts>
    struct IndexOf
    {
        static constexpr std::size_t index()
        {
            if constexpr (std::is_same_v<Head, HeadOfTail>)
            {
                return 0;
            }
            else
            {
                if constexpr (sizeof...(Ts) != 0)
                {
                    if constexpr (std::is_same_v<Head, HeadOfTail>)
                    {
                        return 0;
                    }
        
                    return 1 + IndexOf<Head, Ts...>::index();
                }
                else
                {
                    static_assert(std::is_same_v<Head, HeadOfTail>, "Type not found in parameter pack");
                }
            }
        }
    };

    /// @brief Counts the number of bits set for the given bitmask
    /// @param bitmask the integer representing the bitset
    /// @return a unsigned integer representing the number of bits set
    constexpr std::size_t count_bits_set(std::size_t bitmask)
    {
        std::size_t sum = 0;
        for (std::size_t i = 0; i < std::numeric_limits<std::size_t>::digits; ++i)
        {
            sum += (bitmask & (1ul << i)) ? 1 : 0;
        }
        return sum;
    }

    // count how many bits b2 has set on left of b1
    /// @brief counts the number of differing bits in *b2* that are to
    /// the left of *b1*
    /// @param b1 bitmask for the base mask (to be compared to)
    /// @param b2 bitmask for the comparrison mask (the differing bits)
    /// @return the number of differing bits in b2 on the left of b1.
    constexpr std::size_t bits_to_left(std::size_t b1, std::size_t b2)
    {
        std::size_t count = 0;
        for (int i = std::numeric_limits<std::size_t>::digits - 1; i >= 0; --i)
        {
            auto const b2_mask = (b2 >> i);
            if (b1 >> i)
            {
                return count;
            }
            count += (b2_mask & 1) ? 1 : 0;
        }
        return count;
    }
}

#endif // COMPILE_TIME_H
