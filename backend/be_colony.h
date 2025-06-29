#ifndef BE_COLONY_H
#define BE_COLONY_H

#include <cstdint>
#include "../build/be_api/colony.pb.h"
using distributedcolony::GetImageResponse;

struct Cell {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint16_t extra;
    
    static uint8_t average(uint8_t a, uint8_t b) {
        return static_cast<uint8_t>((a + b) / 2);
    }
    
    void update_color(const Cell& color, float factor) {
        red = average(red, static_cast<uint8_t>(color.red * factor));
        green = average(green, static_cast<uint8_t>(color.green * factor));
        blue = average(blue, static_cast<uint8_t>(color.blue * factor));
    }
};

class ColonyBackend {
public:    
    void init(int width, int height);
    void blast(int x, int y, int radius);
    void fill_image(GetImageResponse &response, int offsetX, int offsetY, int width, int height);
    
    // Get grid dimensions
    int get_width() const { return grid_width; }
    int get_height() const { return grid_height; }

    static ColonyBackend& instance();

private:
    Cell* grid;
    int grid_width;
    int grid_height;

    ColonyBackend() = default;
    Cell* get_random_cell(Cell* matrix);
    Cell pick_random_color();
    Cell* cell_at_pos(int x, int y);
};

#endif // BE_COLONY_H 