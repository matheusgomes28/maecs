import maecs;

#include <raylib.h>

#include <array>

namespace
{
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
// bool draw(emulator::Cpu& cpu)
// {
//     InitWindow(512, 512, "6502 Graphics");
//     while (!WindowShouldClose())
//     {
//         BeginDrawing();
//         ClearBackground(BLACK);
//         // 0200 - 05FF :
//         for (int i = 0x0200; i < 0x0600; ++i)
//         {
//             auto const colour_id = cpu.mem[i] % 16;
//             auto const& colour   = colour_table[colour_id];

//             // Draw this colour rectangle
//             auto const rel_pos = i - 0x0200;
//             auto const row     = rel_pos / 32;
//             auto const col     = rel_pos % 32;
//             DrawRectangle(col * 16, row * 16, 16, 16, colour);
//         }
//         EndDrawing();

//         // Make sure we only draw at most 30 times per second
//         std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 30));
//     }

//     return true;
// }

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

int main(int argc, char** argv)
{
    // Create a registry for a few classes
    maecs::Registry<Circle, Rect, Position> my_registry;
    auto const my_rect_id = my_registry.id<Rect>();
    auto const my_circle_id = my_registry.id<Circle>();
    auto const my_position_id = my_registry.id<Position>();


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

    // TODO : need to generate unique entity ids
    // Entities: 0, 1, 2, 3
    // <entity, {component}> : [<0: {circle, rect}>, <1: {position}>, <2: {}>, <3: {position, rect}>]
    // Tests:
    // - select ents with {circle} -> 0
    // - select ents with {rect} -> 0, 3
    // - select ents with {position} -> 1, 3
    my_registry.set(0, my_rect);
    my_registry.set(0, my_circle);
    my_registry.set(1, my_position);
    my_registry.set(3, my_position);
    my_registry.set(3, my_rect);

    auto const entities_with_circle = my_registry.get({*my_circle_id});
    auto const entities_with_rect = my_registry.get({*my_rect_id});
    auto const entities_with_pos = my_registry.get({*my_position_id});

    // draw(easy65k);
}
