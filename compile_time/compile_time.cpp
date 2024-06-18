module;
#include <type_traits>


export module compile_time;


export namespace compile_time
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
}
