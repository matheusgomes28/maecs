import maecs;

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
bool draw(maecs::Registry<Circle, Rect, Position, Colour>const & registry)
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
        auto const entities_with_circle = registry.get({*my_circle_id, *my_colour_id});

        // Rendering loop system for the circle
        for (auto const& entity : entities_with_circle)
        {
            //void DrawCircle(int centerX, int centerY, float radius, Color color);
            std::optional<Circle> draw_circle = registry.get<Circle>(entity);
            std::optional<Colour> draw_colour = registry.get<Colour>(entity);
            // DrawCircle(draw_circle->center_x, draw_circle->center_y, draw_circle->radius, colour_table[draw_colour->id]);
        }

        auto const my_rect_id = registry.id<Rect>();
        auto const entities_with_rect = registry.get({*my_rect_id, *my_colour_id});

        // Rendering loop system for the square
        for (auto const& entity : entities_with_circle)
        {
            //DrawRectangle(int posX, int posY, int width, int height, Color color); 
            std::optional<Rect> draw_rect = registry.get<Rect>(entity);
            std::optional<Colour> draw_colour = registry.get<Colour>(entity);

            DrawRectangle(draw_rect->left, draw_rect->top, draw_rect->width, draw_rect->height, colour_table[draw_colour->id]);
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

    my_registry.new_set(0, my_rect);
    // auto& my_component_tuple = my_registry.new_get<Rect>()[0].second; // vector<pair<ent_id, comp_tuple>>
    // auto& my_component_optional = std::get<1>(my_component_tuple);
    // auto& my_component = *my_component_optional;


    my_registry.new_set(0, my_position);
    auto& my_component_tuple = my_registry.new_get<Position, Rect>()[0].second; // vector<pair<ent_id, comp_tuple>>
    auto& my_rect_component_optional = std::get<1>(my_component_tuple);
    auto& my_pos_component_optional = std::get<2>(my_component_tuple);
    auto& my_rect_component = *my_rect_component_optional;
    auto& my_pos_component = *my_pos_component_optional;


    // TODO : need to generate unique entity ids
    // Entities: 0, 1, 2, 3
    // <entity, {component}> : [<0: {circle, rect}>, <1: {position}>, <2: {}>, <3: {position, rect}>]
    // Tests:
    // - select ents with {circle} -> 0
    // - select ents with {rect} -> 0, 3
    // - select ents with {position} -> 1, 3
    my_registry.set(0, my_rect);
    my_registry.set(0, my_circle);
    my_registry.set(0, Colour{.id= 2});
    my_registry.set(1, my_position);
    my_registry.set(1, Rect{.top = 300, .left = 400, .width = 50, .height = 50});
    my_registry.set(1, Colour{.id= 10});
    my_registry.set(3, my_position);
    my_registry.set(3, my_rect);
    my_registry.set(3, Circle{.center_x = 512, .center_y = 512, .radius = 200});
    my_registry.set(3, Colour{.id= 3});

    auto const entities_with_circle = my_registry.get({*my_circle_id});
    auto const entities_with_rect = my_registry.get({*my_rect_id});
    auto const entities_with_pos = my_registry.get({*my_position_id});


    draw(my_registry);
}
