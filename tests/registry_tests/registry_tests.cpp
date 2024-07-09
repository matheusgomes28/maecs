import maecs;

// TODO : Turn this into a proper C++ module
#include <gtest/gtest.h>


//////////////////////////////////////////////////
// Component Classes 
//////////////////////////////////////////////////
struct Rectangle
{
    int width;
    int height;
};

struct Circle
{
    int center_x;
    int center_y;
    int radius;
};

struct Position
{
    int x;
    int y;
};


TEST(RegistryTests, SetsComponent)
{
    maecs::Registry<Rectangle, Circle, Position> registry;

    registry.newest_set(0, Rectangle{
        .width = 10,
        .height = 10,
    });

    auto maybe_component = registry.newest_get<Rectangle>();    
    ASSERT_TRUE(maybe_component);
    ASSERT_EQ(maybe_component->size(), 1);

    auto const& [ent_id, component_v] = (*maybe_component)[0];
    ASSERT_EQ(ent_id, 0);

    auto const& component_data = std::get<Rectangle>(component_v);
    ASSERT_EQ(component_data.width, 10);
    ASSERT_EQ(component_data.height, 10);
}

TEST(RegistryTests, SetsMultipleComponents)
{
    maecs::Registry<Rectangle, Circle, Position> registry;

    registry.newest_set(0, Rectangle{
        .width = 10,
        .height = 10,
    });
    // At this stage the bitmask is only RECTANGLE

    Circle const rect_component{
        .center_x = -10,
        .center_y = -1,
        .radius = 52,
    };
    registry.newest_set(0, rect_component);
    // Now the bitmask = RECTANGE | CIRCLE

    auto maybe_component = registry.newest_get<Rectangle, Circle>();    
    ASSERT_TRUE(maybe_component);
    ASSERT_EQ(maybe_component->size(), 1);

    auto const& [ent_id, component_v] = (*maybe_component)[0];
    ASSERT_EQ(ent_id, 0);

    auto const& component_data = std::get<Rectangle>(component_v);
    ASSERT_EQ(component_data.width, 10);
    ASSERT_EQ(component_data.height, 10);

}
