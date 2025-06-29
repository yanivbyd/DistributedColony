#include "be_colony.h"
#include <cstddef>
#include <random>
#include <vector>
#include <stdexcept>
#include <cmath>
#include "../shared/utils.h"
using distributedcolony::GetImageResponse;

ColonyBackend& ColonyBackend::instance() {
    static ColonyBackend instance;
    return instance;
}

void ColonyBackend::init(int width, int height) {
    if (grid) {
        delete[] grid;
    }
    grid_width = width;
    grid_height = height;
    grid = new Cell[width * height];
    for (int i = 0; i < width * height; i++) {
        for (int x = 0; x < width; ++x) {
            grid[i].blue = 255;
            grid[i].red = 255;
            grid[i].green = 255;
        }
    }
    Cell &cell = *get_random_cell(grid);
    cell.red = 255;
    cell.blue = 0;
    cell.green = 0;
}

Cell* ColonyBackend::get_random_cell(Cell* matrix) {
    if (!matrix || grid_width == 0 || grid_height == 0) {
        throw std::runtime_error("get_random_cell: matrix is null or dimensions are zero");
    }
    int index = Random::range(0, grid_width * grid_height - 1);
    return &matrix[index];
}

void ColonyBackend::fill_image(GetImageResponse &response, int offsetX, int offsetY, int width, int height) {
    if (!grid || offsetX < 0 || offsetY < 0 || width <= 0 || height <= 0 ||
        offsetX + width > grid_width || offsetY + height > grid_height) {
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
            int idx = (offsetY + y) * grid_width + (offsetX + x);
            const Cell& cell = grid[idx];
            rgb.push_back(cell.red);
            rgb.push_back(cell.green);
            rgb.push_back(cell.blue);
        }
    }
    response.set_rgbbytes(reinterpret_cast<const char*>(rgb.data()), rgb.size());
}

Cell* ColonyBackend::cell_at_pos(int x, int y) {
    if (x < 0 || y < 0 || x >= grid_width || y >= grid_height) return nullptr;
    return &grid[y * grid_width + x];
}

void ColonyBackend::blast(int x, int y, int radius, Color color) {
    Cell blast_color;
    blast_color.red = static_cast<uint8_t>(color.red());
    blast_color.green = static_cast<uint8_t>(color.green());
    blast_color.blue = static_cast<uint8_t>(color.blue());
    blast_color.extra = 0;
    Cell *center = cell_at_pos(x, y);
    if (!center) return;
    center->update_color(blast_color, 1);
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            int nx = x + dx;
            int ny = y + dy;
            Cell *cell = cell_at_pos(nx, ny);
            if (!cell) continue;
            float dist = std::sqrt(dx*dx + dy*dy);
            if (dist == 0 || dist > radius) continue;
            const float adjusted_dist = dist * Random::range(0.998f, 1.002f);
            const float factor = 1.0f - (adjusted_dist / radius) * 0.5f;
            cell->update_color(blast_color, factor);
        }
    }
} 