module;

#include <ankerl/unordered_dense.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

export module maecs;

export namespace maecs
{
    using EntityId = std::uint64_t;
    using Bitmask = std::uint64_t;
    using ComponentId = decltype(std::declval<std::type_info>().hash_code());
    using ComponentName = decltype(std::declval<std::type_info>().name());
} // maecs

/**************************************************
 * Internal type defs to make functions a little
 * less verbose
**************************************************/
/// @brief Type for a map storing {entity_id, entity_bitmask}
using EntityBitmaskMap = ankerl::unordered_dense::map<maecs::EntityId, maecs::Bitmask>;

/// @brief Type for a map storing {component_name, {component_id, variant_position}}
using ComponentIdMap = ankerl::unordered_dense::map<maecs::ComponentName, std::pair<maecs::ComponentId, std::size_t>>;

/// @brief Type for the cache-friendly (although it's not quite) entity-component type
template <typename... Cs>
using EntityComponent = std::tuple<std::optional<Cs>...>;

// What happens if Target is not in List types??
/// @brief Gets the index of a type in the list of templated types
/// @tparam Target should be the type we want to find the index for
/// @tparam ListHead first type in the list of template args
/// @tparam ...ListTails the remaning type of list args
/// @return std::size_t that represents index of Target in {ListHead, ListTains...}
template<typename Target, typename ListHead, typename... ListTails>
constexpr std::size_t get_type_index()
{
    if constexpr (std::is_same<Target, ListHead>::value)
        return 0;
    else
        return 1 + get_type_index<Target, ListTails...>();
}

template <typename C>
void add_component_id_one(ComponentIdMap& map, std::size_t index)
{
    std::type_info const& type_info = typeid(C);
    map.insert({type_info.name(), {type_info.hash_code(), index}});
}

/// @brief This adds an entry to the component name-{id, index} map
/// so users can get which position in the variant a type is, as well as
/// the component ID for looking up into the component-entity map.
template <typename C, typename... Cs>
void add_component_id(ComponentIdMap& map, std::size_t index)
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

inline void update_entity_bitmask(
    std::uint64_t entity_id,
    std::size_t component_index,
    EntityBitmaskMap& entity_bitmasks
)
{
    static_assert(sizeof(std::size_t) == sizeof(std::uint64_t));

    // Call the entity update function here
    auto const bitmask = (0b1 << component_index);

    // TODO : make this into a function (update_bitmask(entity, new_bitmask))
    auto bitmask_found = entity_bitmasks.find(entity_id);
    if (bitmask_found == end(entity_bitmasks))
    {
        entity_bitmasks[entity_id] = bitmask;
    }
    else
    {
        bitmask_found->second |= bitmask;
    }
}

// ankerl::unordered_dense::map<Bitmask, std::vector<std::pair<EntityId, tuple_t>>> _new_entity_components;
template <typename... Cs>
inline void update_entity_component_map(
    maecs::EntityId ent_id,
    maecs::Bitmask ent_bitmask,
    ankerl::unordered_dense::map<maecs::Bitmask, std::vector<std::pair<maecs::EntityId, EntityComponent<Cs...>>>>& entity_components,
    EntityComponent<Cs...> const& component_data
)
{
    auto existing_ents_iter = entity_components.find(entity_components);
    if (existing_ents_iter == end(entity_components))
    {
        auto [inserted_iter, inserted] = entity_components.insert({ent_bitmask, {}});
        // Expects(inserted);
        existing_ents_iter = inserted_iter;
    }

    existing_ents_iter->second.push_back({ent_id, component_data});
}

export namespace maecs {
    EntityId generate_entity_id()
    {
        static EntityId curr_id = 0;
        return curr_id++;
    }
    
    template <typename... Cs>
    using Component = std::variant<Cs...>;
    // std::variant<Square, Circle> -> Square has id 0, Circle has id 1
    
    template <typename... Cs>
    class Registry {
    public:

        using tuple_t = std::tuple<std::optional<Cs>...>;

        // Whenever we init Registry, we want to iterate over the
        // types of Cs... and store the <name, id> in the _component_ids;
        Registry() : _type_arrays{ &typeid(Cs)... }
        {
            // Cannot be optimzed out
            add_component_id<Cs...>(_component_ids, 0);
        }

        /// @brief Gets the reference to the component held by an entity
        /// @tparam C component class to get
        /// @param entity_id id of the entity
        /// @return a valid optional containing the component reference if
        /// the entity has that component, nulopt otherwise
        template <typename C>
        std::optional<C> get(EntityId entity_id) const
        {
            // Get the component ID for this type
            static const auto component_id = typeid(C).hash_code();
            auto const found = _entities.find(component_id);
            if (found != end(_entities))
            {
                // This is returning a vector<{ent_id, variant}>
                // find the entity in the vector
                std::vector<std::pair<EntityId, Component<Cs...>>> const& entity_components = found->second;
                auto const elem = std::find_if(begin(entity_components), end(entity_components), [entity_id](auto const& val){
                    auto const& [id, component] = val;
                    return id == entity_id;
                });
                // TODO : This is a linear search so its inneficient

                // TODO : should this be an assert??
                // elem = iter to pair{id, component}
                if (elem == end(entity_components))
                {
                    return std::nullopt;
                }

                std::variant<Cs...> const& component = elem->second;
                return std::make_optional<C>(std::get<C>(component));
            }

            return std::nullopt;
        }

        /// This function will get all the entity Ids
        /// that have the coponent classes "Cs..." attached
        /// to them
        std::vector<EntityId> get(std::vector<ComponentId> const& components) const
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
        std::optional<ComponentId> id() const
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

            // Updates the entity bitmask with whatever new entity bitmask we get
            update_entity_bitmask(ent_id, index, _entity_bitmask);
        }

        template <typename C>
        void new_set(EntityId ent_id, C const& component)
        {
            
            static_assert(index != std::variant::npos);
            // Expects(index != std::variant_npos);
            _

            auto const component_hash = _type_arrays[index]->has_code();

            // We need to move the current component information out
            // of the previous bitmask
            auto new_bitmask =  (1 << index);
            auto current_tuple_iter = end(_new_entity_components);
            if (auto current_bitmask_iter = _entity_bitmask.find(ent_id); current_bitmask_iter != end(_entity_bitmask))
            {
                // get the tuple here
                current_tuple_iter = _new_entity_components.find(current_bitmask_iter->second);
                // Expects(current_tuple_iter != end(_new_entity_component));
            }
            else
            {
                _entity_bitmask[ent_id] = new_bitmask;
            }

            if (current_tuple_iter != end(_new_entity_components))
            {
                new_bitmask |= current_tuple_iter->first;
                auto entity_data = extract(current_tuple_iter);
                // gonna be difficult to get move semantics right on this
                // without the hardcoded types with std::optional
                std::get<index>(entity_data) = get<index>(component);
                update_entity_component_map(ent_id, new_bitmask, _new_entity_components, entity_data);
                return;
            }
            
            EntityComponent<Cs...> new_entity_component;
            get<index>(new_entity_component) = get<index>(component);
            update_entity_component_map(ent_id, new_bitmask, _new_entity_components, new_entity_component);
            // TODO : Finish this implementation
        }

        template <typename... Ds>
        std::vector<std::pair<maecs::EntityId, EntityComponent<Cs...>>>& new_get()
        {
            auto const bitmask = bit_mask<Ds...>();
            return _new_entity_components[bitmask];
        }

        template <typename Head, typename... Tail>
        constexpr std::size_t bit_mask()
        {
            if constexpr (sizeof...(Tail) == 0)
            {
                return (1 << get_type_index<Head, Cs...>());
            }
            else
            {
                return (1 << get_type_index<Head, Cs...>()) | bit_mask<Tail...>();
            }
        }

    private:
        // This will store the component Ids for each class given as parameter to Registry
        // the value is of type pair<ComponentId, index in variant>
        ankerl::unordered_dense::map<ComponentName, std::pair<ComponentId, std::size_t>> _component_ids;

        // This will store the entity-component data
        ankerl::unordered_dense::map<ComponentId, std::vector<std::pair<EntityId, Component<Cs...>>>> _entities;

        // Store all the current entity -> bitmask values
        ankerl::unordered_dense::map<EntityId, Bitmask> _entity_bitmask;

        // Wanna create the entity mask -> entity data
        // 011 -> {ent_id, {c1, c2}} = ent_id has components in indexes 0, 1 of the Cs... and the data is in vector
        ankerl::unordered_dense::map<Bitmask, std::vector<std::pair<EntityId, tuple_t>>> _new_entity_components;

        // 010 -> {ent_id, {c1}}
        // add c2 to ent_id ?
        // 110 -> {ent_id, {c1, c2}}
        // 010 -> {}

        // Array to store all the type_ids in order of Components declared in variant
        std::array<std::type_info const*, sizeof...(Cs)> _type_arrays;
    };

}
