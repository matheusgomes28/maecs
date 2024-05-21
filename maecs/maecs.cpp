module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

export module maecs;

template <typename C>
void add_component_id_one(std::unordered_map<decltype(std::declval<std::type_info>().name()), std::pair<decltype(std::declval<std::type_info>().hash_code()), std::size_t>>& map, std::size_t index)
{
    std::type_info const& type_info = typeid(C);
    map.insert({type_info.name(), {type_info.hash_code(), index}});
}

template <typename C, typename... Cs>
void add_component_id(std::unordered_map<decltype(std::declval<std::type_info>().name()), std::pair<decltype(std::declval<std::type_info>().hash_code()), std::size_t>>& map, std::size_t index)
{
    add_component_id_one<C>(map, index);

    std::size_t const new_index = index + 1;
    if constexpr(sizeof...(Cs) >= 1) {
        add_component_id<Cs...>(map, new_index);
    }
}

template <typename... Cs>
std::vector<std::uint64_t> transform_ent_ids(std::vector<std::pair<std::uint64_t, std::variant<Cs...>>> const& input)
{
    std::vector<std::uint64_t> result;

    std::transform(begin(input), end(input), std::back_inserter(result), [](auto const& pair){
        return pair.first;
    });
    return result;
}

export namespace maecs {
    using EntityId = std::uint64_t;
    using ComponentId = decltype(std::declval<std::type_info>().hash_code());
    using ComponentNameType = decltype(std::declval<std::type_info>().name());
    
    template <typename... Cs>
    using Component = std::variant<Cs...>;
    // std::variant<Square, Circle> -> Square has id 0, Circle has id 1
    
    template <typename... Cs>
    class Registry {
    public:

        // Whenever we init Registry, we want to iterate over the
        // types of Cs... and store the <name, id> in the _component_ids;
        Registry() : _type_arrays{ &typeid(Cs)... }
        {
            // Cannot be optimzed out
            add_component_id<Cs...>(_component_ids, 0);
        }

        /// This function will get all the entity Ids
        /// that have the coponent classes "Cs..." attached
        /// to them
        std::vector<EntityId> get(std::vector<ComponentId> components)
        {
            if (components.size() < 1) {
                return {};
            }

            std::vector<EntityId> result;
            if (auto const found = _entities.find(components[0]); found == end(_entities))
            {
                return {};
            }
            else 
            {
                result = transform_ent_ids(found->second);
            }

            // std::vector<EntityId> current = 
            for (auto const component_id : components) {
                std::vector<EntityId> next_component;
                std::vector<EntityId> inter_result;

                if (auto const found = _entities.find(component_id); found == end(_entities))
                {
                    return {};
                }
                else 
                {
                    next_component = transform_ent_ids(found->second);
                }

                std::set_intersection(begin(result), end(result), begin(next_component), end(next_component), back_inserter(inter_result));
                result = inter_result;
            }

            return result;
        }

        /// This function returns the registered component ID,
        /// if the component is not valid for this registry, return
        /// std:nullopt
        template <typename C>
        std::optional<ComponentId> id()
        {
            // TODO : probably want a more optimal way
            // to find the idx for the type in the variant
            auto const& type_info = typeid(C);
            
            for (std::size_t i = 0; i < _type_arrays.size(); ++i)
            {
                auto const type_hash = _type_arrays[i]->hash_code();
                if (type_hash == type_info.hash_code()) {
                    return std::make_optional<ComponentId>(type_hash);
                }
            }

            return std::nullopt;
        }
        // registry.id<Circle>()
        // registry.id<std::string>() -> return some error

        // TODO : remove the entity
        // TODO : remove component from entity

        // const auto entities = registry.get({Square, Circle});       
        void set(EntityId ent_id, Component<Cs...> const& component)
        {
            auto const index = component.index();
            auto const component_hash = _type_arrays[index]->hash_code();

            auto found = _entities.find(component_hash);
            if (found != end(_entities)) {
                found->second.push_back({ent_id, component});
                return;
            }

            _entities.insert({component_hash, {{ent_id, component}}});
        }

    private:
        // This will store the component Ids for each class given as parameter to Registry
        // the value is of type pair<ComponentId, index in variant>
        std::unordered_map<ComponentNameType, std::pair<ComponentId, std::size_t>> _component_ids;

        // This will store the entity-component data
        std::unordered_map<ComponentId, std::vector<std::pair<EntityId, Component<Cs...>>>> _entities;

        // Array to store all the type_ids in order of Components declared in variant
        std::array<std::type_info const*, sizeof...(Cs)> _type_arrays;
    };

}


/*
struct Square{
    int top;
    int left;
    int width;
    int height;
};

struct Circle{
    int center_x;
    int center_y;
    int radius;
};


Registry<Square, Circle> registry;

// -> storing each id in the instance of registry {Square = 0, Circle = 1};
// -> calcualting the id for the compo happens at the contructor
*/
