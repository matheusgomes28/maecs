#ifndef MAECS_H
#define MAECS_H

#include <gsl/assert>
#include <ankerl/unordered_dense.h>
#include <compile_time/compile_time.hpp>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <span>
#include <typeinfo>
#include <utility>
#include <variant>
#include <vector>

namespace maecs
{
    using EntityId = std::uint64_t;
    using Bitmask = std::uint64_t;
    using ComponentId = decltype(std::declval<std::type_info>().hash_code());
    using ComponentName = decltype(std::declval<std::type_info>().name());

    template <typename... Cs>
    using ComponentType = std::variant<Cs...>;

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

    namespace detail
    {
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
            // Expects(ent_bitmask == entity_bitmask.find(ent_id)->second);
            auto existing_ents_iter = entity_components.find(ent_bitmask);
            // if (existing_ents_iter == end(entity_components))
            // {
            //     entity_components[ent_bitmask] = {};
            //     // auto [inserted_iter, inserted] = entity_components.insert({ent_bitmask, {}});
            //     // Ensures(inserted);
            //     // Ensures(inserted_iter != end(entity_components));
                
            //     // existing_ents_iter = inserted_iter;
            // }

            entity_components[ent_bitmask].push_back({ent_id, component_data});
        }
    }

} // maecs

namespace maecs {
    EntityId generate_entity_id()
    {
        static EntityId curr_id = 0;
        return curr_id++;
    }
    
    template <typename... Cs>
    using Component = std::variant<Cs...>;
    // std::variant<Square, Circle> -> Square has id 0, Circle has id 1

    /* TODO : Let's encode the type returned here*/
    // Shoudl probably move this to "compile_time"
    template <typename... Types>
    struct TupleTypes
    {
    };

    // Disable this if not through our TupleTypes
    template <typename... Types>
    class EntityView;

    template <typename... Cs, typename... Ds>
    class EntityView<TupleTypes<Cs...>, TupleTypes<Ds...>>
    {
    public:
        using variant_type = std::variant<Cs...>;
        EntityView(std::span<std::pair<EntityId, variant_type>> data)
            : _data{data}
        {
        }

        // Do we want to return the reference to the object here?
        template<typename C>
        C component() const
        {
            constexpr auto position = compile_time::IndexOf<C, Ds...>::index();
            return std::get<C>(_data[position].second);
        }

        EntityId id() const
        {
            // Quick bug for now, need to handle case where
            // we don't have any entities here
            return _data[0].first;
        }

    private:
        std::span<std::pair<EntityId, std::variant<Cs...>>> _data;
    };

    // usage: EntityView<Circle, Square>(the_variant<Circle, Square, Position>...);
    
    template <typename... Cs>
    class Registry {
    public:

        // TODO : if we have many components then std::tuple<optional<Cs>...> is gonna be big
        //        if we have std::variant<C1, C2, ..., Cn> has size at most max(sizeof(Cs)...)
        using tuple_t = std::tuple<std::optional<Cs>...>;

        using component_t = std::variant<Cs...>;

        // Whenever we init Registry, we want to iterate over the
        // types of Cs... and store the <name, id> in the _component_ids;
        Registry() : _type_arrays{ &typeid(Cs)... }
        {
            // Cannot be optimzed out
            detail::add_component_id<Cs...>(_component_ids, 0);
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

        // TODO : lets worry about the hard case of unioning things
        //        i.e. entities(0b0101) = entities(0b0101) + entities(0b1101) + entities(0b0111) + entities(0b1111)
        /// @brief Gets all the entities with the specified components
        /// set.
        /// @tparam ...Ds the component classes to get
        /// @return a view into the entities which contain all these components
        template <typename... Ds>
        std::optional<std::vector<EntityView<TupleTypes<Cs...>, TupleTypes<Ds...>>>> newest_get()
        {
            auto constexpr bitmask = bit_mask<Ds...>();
            
            // find the stuff and return it
            auto found = _variant_components.find(bitmask);
            if (found == end(_variant_components))
            {
                return std::nullopt;
            }

            std::vector<EntityView<TupleTypes<Cs...>, TupleTypes<Ds...>>> entity_views;
            std::size_t const num_components = sizeof...(Ds);
            for (std::size_t i = 0; i < found->second.size(); i += num_components)
            {
                std::span<std::pair<EntityId, std::variant<Cs...>>> view{begin(found->second) + i, num_components};
                EntityView<TupleTypes<Cs...>, TupleTypes<Ds...>> entity_view{view};
                entity_views.push_back(entity_view);
            }

            return entity_views;
        }

        template <typename D>
        bool newest_set(EntityId ent_id, D const& component)
        {
            static auto constexpr component_bitmask = bit_mask<D>();
            auto const prev_ent_bitmask = _entity_bitmask[ent_id];
            _entity_bitmask[ent_id] = prev_ent_bitmask | component_bitmask;

            // TODO : If entity is new, assign it to some component
            if (prev_ent_bitmask == 0)
            {
                _variant_components[component_bitmask] = {{ent_id, component}};
                return true;
            }

            if (!(prev_ent_bitmask & component_bitmask))
            {
                // We should probably cache this when the entity is added,
                // i.e. where in the vector the start of the entity is
                auto prev_component_entities = _variant_components.find(prev_ent_bitmask);
                Expects(prev_component_entities != end(_variant_components));

                auto& component_entity_vector = prev_component_entities->second;
                auto found = std::find_if(begin(component_entity_vector), end(component_entity_vector), [=](auto const& ent_component_pair){
                    return ent_component_pair.first == ent_id;
                });
                Expects(found != end(component_entity_vector));

                // get however many other types we need
                auto const prev_comp_count = compile_time::count_bits_set(prev_ent_bitmask);

                // move previous items + the new component
                auto& new_component_vector = _variant_components[prev_ent_bitmask | component_bitmask];
                std::move(found, found + prev_comp_count, back_inserter(new_component_vector));
                auto const component_index = compile_time::bits_to_left(prev_ent_bitmask, component_bitmask);
                
                // element needs to be inserted within the previous components, wherever
                // it's meant to be with relation to bitmask position
                new_component_vector.insert(begin(new_component_vector) + new_component_vector.size() - prev_comp_count + component_index, {ent_id, component});
                
                // Finally, remove the moved elements
                // component_entity_vector.erase(found, found + prev_comp_count);
                // Do we not need to call these elements?
                
                return true;
            }

            // If the entity already has those components

            auto prev_component_entities = _variant_components.find(prev_ent_bitmask);
            Expects(prev_component_entities != end(_variant_components));

            auto& component_entity_vector = prev_component_entities->second;
            auto found = std::find_if(begin(component_entity_vector), end(component_entity_vector), [=](auto const& ent_component_pair){
                return ent_component_pair.first == ent_id;
            });
            Expects(found != end(component_entity_vector));
            auto const component_index = compile_time::bits_to_left(component_bitmask, prev_ent_bitmask);
            auto this_component = std::next(found, component_index);
            Expects(this_component != end(component_entity_vector));
            std::get<D>(this_component->second) = component;
            return true;
        }

        template <typename C>
        C& entity_component(EntityComponent<Cs...>& component_tuple)
        {
            constexpr std::size_t index = compile_time::IndexOf<C, Cs...>::index();
            auto& component_optional = std::get<index>(component_tuple);
            Expects(component_optional);
            return *component_optional;
        }

        template <typename C>
        C const& entity_component(EntityComponent<Cs...> const& component_tuple) const
        {
            constexpr std::size_t index = compile_time::IndexOf<C, Cs...>::index();
            auto& component_optional = std::get<index>(component_tuple);
            Expects(component_optional);
            return *component_optional;
        }

        template <typename Head, typename... Tail>
        static constexpr std::size_t bit_mask()
        {
            if constexpr (sizeof...(Tail) == 0)
            {
                return (1 << compile_time::IndexOf<Head, Cs...>::index());
            }
            else
            {
                return (1 << compile_time::IndexOf<Head, Cs...>::index()) | bit_mask<Tail...>();
            }
        }

    private:
        // This will store the component Ids for each class given as parameter to Registry
        // the value is of type pair<ComponentId, index in variant>
        ankerl::unordered_dense::map<ComponentName, std::pair<ComponentId, std::size_t>> _component_ids;

        /// @brief  TODO
        ankerl::unordered_dense::map<EntityId, std::size_t> _entity_bitmask;

        // This is where we store all the component data
        ankerl::unordered_dense::map<Bitmask, std::vector<std::pair<EntityId, component_t>>> _variant_components;

        // Array to store all the type_ids in order of Components declared in variant
        std::array<std::type_info const*, sizeof...(Cs)> _type_arrays;
    };

}

#endif // MAECS_H
