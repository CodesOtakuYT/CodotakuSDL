#ifndef CODOTAKU_POLYGON_H
#define CODOTAKU_POLYGON_H

#include <cstdint>

#include <glm/trigonometric.hpp>
#include <glm/vec2.hpp>

namespace codotaku
{

template<typename F, typename G>
void generatePolygon(
    uint32_t sides,
    float size,
    float rotation,
    F &&vertexFn,
    G &&indexFn)
{
    float step = 6.2831855f / static_cast<float>(sides);
    for (uint32_t i = 0; i < sides; ++i) {
        float a = rotation + step * static_cast<float>(i);
        vertexFn(glm::vec2{ glm::cos(a) * size, glm::sin(a) * size }, i);
    }

    for (uint32_t i = 1; i < sides - 1; ++i) {
        indexFn(0);
        indexFn(i);
        indexFn(i + 1);
    }
}

} // namespace codotaku

#endif
