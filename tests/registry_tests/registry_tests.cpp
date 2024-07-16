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

    auto const entity_view = (*maybe_component)[0];
    ASSERT_EQ(entity_view.id(), 0);

    
    auto const& component_data = entity_view.component<Rectangle>();
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

    auto const entity_view = (*maybe_component)[0];
    ASSERT_EQ(entity_view.id(), 0);

    auto const& rectangle_data = entity_view.component<Rectangle>();
    ASSERT_EQ(rectangle_data.width, 10);
    ASSERT_EQ(rectangle_data.height, 10);

    auto const& circle_data = entity_view.component<Circle>();
    ASSERT_EQ(circle_data.center_x, -10);
    ASSERT_EQ(circle_data.center_y, -1);
    ASSERT_EQ(circle_data.radius, 52);

}
