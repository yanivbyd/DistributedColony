#include "be_colony.h"
#include <cstddef>
#include <random>
#include <vector>
#include <stdexcept>
using distributedcolony::GetImageResponse;

ColonyBackend& ColonyBackend::instance() {
    static ColonyBackend instance;
    return instance;
}

void ColonyBackend::init(int width, int height) {
    if (this->matrix) {
        delete[] this->matrix;
    }
    this->matrix_width = width;
    this->matrix_height = height;
    this->matrix = new Cell[width * height];

    Cell &cell = *get_random_cell(this->matrix);
    cell.red = 255;
}

Cell* ColonyBackend::get_random_cell(Cell* matrix) {
    if (!matrix || this->matrix_width == 0 || this->matrix_height == 0) {
        throw std::runtime_error("get_random_cell: matrix is null or dimensions are zero");
    }
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, this->matrix_width * this->matrix_height - 1);
    return &matrix[dis(gen)];
}

void ColonyBackend::fill_image(GetImageResponse &response, int offsetX, int offsetY, int width, int height) {
    if (!this->matrix || offsetX < 0 || offsetY < 0 || width <= 0 || height <= 0 ||
        offsetX + width > this->matrix_width || offsetY + height > this->matrix_height) {
        response.set_status(1);
        return;
    }
    response.set_status(0);
    response.set_width(width);
    response.set_height(height);
    std::vector<uint8_t> rgb;
    rgb.reserve(width * height * 3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (offsetY + y) * this->matrix_width + (offsetX + x);
            const Cell& cell = this->matrix[idx];
            rgb.push_back(cell.red);
            rgb.push_back(cell.green);
            rgb.push_back(cell.blue);
        }
    }
    response.set_rgbbytes(reinterpret_cast<const char*>(rgb.data()), rgb.size());
} 