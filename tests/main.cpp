import maecs;

#include <benchmark/benchmark.h>


struct Circle
{
    int center_x;
    int center_y;
    int radius;
};


template <typename... Cs>
void create_entity_with_component(maecs::Registry<Cs...>& registry)
{
    auto const entity_id = maecs::generate_entity_id();
    auto const component = Circle{
        .center_x = 10,
        .center_y = 10,
        .radius = 50
    };

    registry.set(entity_id, component);
}


// TODO : Benchmark the entity component add function
static void BenchmarkEntitySetOneComponent(benchmark::State& state) {
    // Create the registry to be reused in each iteration

    // TODO : make the component registration done
    // by default when we initialise the registry
    maecs::Registry<Circle> registry;
    // auto const my_rect_id = registry.id<Circle>();
    // registry
    for (auto _ : state) {
        create_entity_with_component(registry);
    }
}
// Register the function as a benchmark
BENCHMARK(BenchmarkEntitySetOneComponent);
// Run the benchmark
BENCHMARK_MAIN();