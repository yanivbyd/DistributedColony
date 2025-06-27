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
    
    void apply_color(const Cell& color, float factor = 1.0f) {
        red = static_cast<uint8_t>(color.red * factor);
        green = static_cast<uint8_t>(color.green * factor);
        blue = static_cast<uint8_t>(color.blue * factor);
    }
};

class ColonyBackend {
public:    
    void init(int width, int height);
    void blast(int x, int y);
    void fill_image(GetImageResponse &response, int offsetX, int offsetY, int width, int height);

    static ColonyBackend& instance();

private:
    Cell* matrix;
    int matrix_width;
    int matrix_height;

    ColonyBackend() = default;
    Cell* get_random_cell(Cell* matrix);
    Cell pick_random_color();
    int random_distance(int max_dist);
    Cell* cell_at_pos(int x, int y);
};

#endif // BE_COLONY_H 