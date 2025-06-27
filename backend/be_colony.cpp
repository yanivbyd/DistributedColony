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
    if (matrix) {
        delete[] matrix;
    }
    matrix_width = width;
    matrix_height = height;
    matrix = new Cell[width * height];
    for (int i = 0; i < width * height; i++) {
        for (int x = 0; x < width; ++x) {
            matrix[i].blue = 255;
            matrix[i].red = 255;
            matrix[i].green = 255;
        }
    }
    Cell &cell = *get_random_cell(matrix);
    cell.red = 255;
    cell.blue = 0;
    cell.green = 0;
}

Cell* ColonyBackend::get_random_cell(Cell* matrix) {
    if (!matrix || matrix_width == 0 || matrix_height == 0) {
        throw std::runtime_error("get_random_cell: matrix is null or dimensions are zero");
    }
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, matrix_width * matrix_height - 1);
    return &matrix[dis(gen)];
}

void ColonyBackend::fill_image(GetImageResponse &response, int offsetX, int offsetY, int width, int height) {
    if (!matrix || offsetX < 0 || offsetY < 0 || width <= 0 || height <= 0 ||
        offsetX + width > matrix_width || offsetY + height > matrix_height) {
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
            int idx = (offsetY + y) * matrix_width + (offsetX + x);
            const Cell& cell = matrix[idx];
            rgb.push_back(cell.red);
            rgb.push_back(cell.green);
            rgb.push_back(cell.blue);
        }
    }
    response.set_rgbbytes(reinterpret_cast<const char*>(rgb.data()), rgb.size());
}

Cell ColonyBackend::pick_random_color() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 255);
    Cell c;
    c.red = dist(gen);
    c.green = dist(gen);
    c.blue = dist(gen);
    c.extra = 0;
    return c;
}

int ColonyBackend::random_distance(int max_dist) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, max_dist);
    return dist(gen);
}

Cell* ColonyBackend::cell_at_pos(int x, int y) {
    if (x < 0 || y < 0 || x >= matrix_width || y >= matrix_height) return nullptr;
    return &matrix[y * matrix_width + x];
}

void ColonyBackend::blast(int x, int y) {
    Cell color = pick_random_color();
    Cell *center = cell_at_pos(x, y);
    if (!center) return;
    center->apply_color(color);
    int distance = random_distance(12); // distance is between 1 and 50
    for (int dy = -distance; dy <= distance; ++dy) {
        for (int dx = -distance; dx <= distance; ++dx) {
            int nx = x + dx;
            int ny = y + dy;
            Cell *cell = cell_at_pos(nx, ny);
            if (!cell) continue;
            float dist = std::sqrt(dx*dx + dy*dy);
            if (dist == 0 || dist > distance) continue;
            float factor = 1.0f / dist;
            cell->apply_color(color, factor);
        }
    }
} 