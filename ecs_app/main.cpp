#include <maecs/maecs.hpp>

#include <raylib.h>

#include <array>
#include <chrono>
#include <thread>
#include <variant>

namespace
{
    struct Position
    {
        int x;
        int y;
    };

    struct Rect
    {
        int top;
        int left;
        int width;
        int height;
    };

    struct Circle
    {
        int center_x;
        int center_y;
        int radius;
    };

    struct Colour
    {
        // refer to the colour pallete index
        std::size_t id;
    };

    struct Triangle
    {
        std::size_t a;
        std::size_t b;
        std::size_t c;
    };

    static constexpr std::array<Color, 16> colour_table{{
        {0, 0, 0, 255}, // Black
        {255, 255, 255, 255}, // White
        {136, 0, 0, 255}, // Dark Red
        {170, 255, 238, 255}, // Light Cyan
        {204, 68, 204, 255}, // Violet
        {0, 204, 85, 255}, // Light Green
        {0, 0, 170, 255}, // Blue
        {255, 241, 224, 255}, {221, 136, 85, 255}, // Light Brown
        {102, 68, 0, 255}, // Dark Brown
        {255, 119, 119, 255}, // Light Red
        {51, 51, 51, 255}, // Dark Grey
        {119, 119, 119, 255}, // Light Grey
        {170, 255, 102, 255}, // Light Lime
        {0, 136, 255, 255}, // Light Blue
        {187, 187, 187, 255} // Grey
    }};
}


// TODO mutex to protect memory
bool draw(maecs::Registry<Circle, Rect, Position, Colour>& registry)
{
    InitWindow(512, 512, "6502 Graphics");
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        // 0200 - 05FF :

        // We're going to get all the entities with circles
        // and draw these circles on the screen
        auto const my_colour_id = registry.id<Colour>();
        auto const my_circle_id = registry.id<Circle>();
        // registry.get is deprecated
        
        // TODO : newest get should support some sort of const correctness
        // auto const entities_with_circle = registry.newest_get<Circle, Colour>();

        auto entities_with_circle = registry.newest_get<Circle, Colour>();

        //void DrawCircle(int centerX, int centerY, float radius, Color color);
        
        auto const& drawable_circle_entities = registry.newest_get<Circle, Position, Colour>();
        for (auto const& entity_view : *drawable_circle_entities)
        {
            // Firstly get the 
            // auto const& circle_component = registry.entity_component<Circle>(component_tuple);
            // auto const& position_component = registry.entity_component<Position>(component_tuple);
            // auto const& colour_component = registry.entity_component<Colour>(component_tuple);
            // DrawCircle(circle_component.center_x, circle_component.center_y, circle_component.radius, colour_table[colour_component.id]);
        }

        EndDrawing();

        // Make sure we only draw at most 30 times per second
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 30));
    }

    return true;
}

int main(/*int argc, char** argv*/)
{

    // auto const type_index = get_type_index<Rect, Rect, Circle, Triangle, Position>();

    // Create a registry for a few classes
    maecs::Registry<Circle, Rect, Position, Colour> my_registry;
    auto const my_rect_id = my_registry.id<Rect>();
    auto const my_circle_id = my_registry.id<Circle>();
    auto const my_position_id = my_registry.id<Position>();


    auto const my_id0 = my_registry.bit_mask<Circle>();
    auto const my_id1 = my_registry.bit_mask<Rect>();
    auto const my_id2 = my_registry.bit_mask<Position>();
    auto const my_id3 = my_registry.bit_mask<Colour>();


    // std::variant<Circle, Rect> 
    auto const my_composite_id0 = my_registry.bit_mask<Colour, Rect>();
    auto const my_composite_id1 = my_registry.bit_mask<Circle, Position, Rect>();

    auto const size_of_entity = sizeof(std::tuple<std::optional<Circle>, std::optional<Rect>, std::optional<Position>, std::optional<Colour>>);


    // Create a dummy Rectangle component for an entity
    Rect const my_rect{
        .top = 10,
        .left = 10,
        .width = 10,
        .height = 10,
    };

    Circle const my_circle{
        .center_x = 50,
        .center_y = 50,
        .radius = 20,
    };

    Position const my_position{
        .x = 0,
        .y = 42,
    };

    // TODO : We need some tests to get the stuff here

    draw(my_registry);
}
