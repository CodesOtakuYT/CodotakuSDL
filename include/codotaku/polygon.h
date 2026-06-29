#ifndef CODOTAKU_POLYGON_H
#define CODOTAKU_POLYGON_H

#include <cstdint>
#include <vector>

#include <glm/trigonometric.hpp>
#include <glm/vec2.hpp>

namespace codotaku
{

template<typename F>
std::vector<uint32_t> generatePolygon(
    uint32_t sides,
    float size,
    float rotation,
    F &&vertexFn)
{
    float step = 6.2831855f / static_cast<float>(sides);
    for (uint32_t i = 0; i < sides; ++i) {
        float a = rotation + step * static_cast<float>(i);
        vertexFn(glm::vec2{ glm::cos(a) * size, glm::sin(a) * size }, i);
    }

    std::vector<uint32_t> indices;
    indices.reserve((sides - 2) * 3);
    for (uint32_t i = 1; i < sides - 1; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    return indices;
}

} // namespace codotaku

#endif
