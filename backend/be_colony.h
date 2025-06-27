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
};

class ColonyBackend {
public:    
    void init(int width, int height);
    void fill_image(GetImageResponse &response, int offsetX, int offsetY, int width, int height);

    static ColonyBackend& instance();

private:
    Cell* matrix;
    int matrix_width;
    int matrix_height;

private:
    ColonyBackend() = default;
    Cell* get_random_cell(Cell* matrix);
};

#endif // BE_COLONY_H 