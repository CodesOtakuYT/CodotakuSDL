#ifndef CODOTAKU_CAMERA2D_H
#define CODOTAKU_CAMERA2D_H

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <cmath>

namespace codotaku
{

class Camera2D
{
  public:
    Camera2D() = default;

    void setPosition(glm::vec2 position) noexcept
    {
        position_ = position;
    }

    void setZoom(float zoom) noexcept
    {
        zoom_ = zoom;
    }

    void setRotation(float rotation) noexcept
    {
        rotation_ = rotation;
    }

    [[nodiscard]] glm::vec2 position() const noexcept
    {
        return position_;
    }

    [[nodiscard]] float zoom() const noexcept
    {
        return zoom_;
    }

    [[nodiscard]] float rotation() const noexcept
    {
        return rotation_;
    }

    [[nodiscard]] glm::mat4 viewProjection(glm::ivec2 viewportSize) const noexcept
    {
        float w = static_cast<float>(viewportSize.x);
        float h = static_cast<float>(viewportSize.y);

        glm::mat4 projection = glm::ortho(-w / 2.0f, w / 2.0f, -h / 2.0f, h / 2.0f, -1.0f, 1.0f);

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-position_, 0.0f))
                       * glm::rotate(glm::mat4(1.0f), -rotation_, glm::vec3(0.0f, 0.0f, 1.0f))
                       * glm::scale(glm::mat4(1.0f), glm::vec3(zoom_, zoom_, 1.0f));

        return projection * view;
    }

    [[nodiscard]] glm::vec2 screenToWorld(
        glm::vec2 screenPos, glm::ivec2 viewportSize) const noexcept
    {
        auto inv = glm::inverse(viewProjection(viewportSize));
        auto ndc = glm::vec4(
            2.0f * screenPos.x / static_cast<float>(viewportSize.x) - 1.0f,
            1.0f - 2.0f * screenPos.y / static_cast<float>(viewportSize.y),
            0.0f, 1.0f);
        auto world = inv * ndc;
        return { world.x, world.y };
    }

    [[nodiscard]] glm::vec2 worldToScreen(
        glm::vec2 worldPos, glm::ivec2 viewportSize) const noexcept
    {
        auto clip = viewProjection(viewportSize) * glm::vec4(worldPos, 0.0f, 1.0f);
        return {
            (clip.x + 1.0f) * 0.5f * static_cast<float>(viewportSize.x),
            (1.0f - clip.y) * 0.5f * static_cast<float>(viewportSize.y)
        };
    }

    void zoomAtMouse(glm::vec2 screenPos, glm::ivec2 viewportSize, float wheelDelta) noexcept
    {
        if (wheelDelta == 0.0f) return;
        auto worldBefore = screenToWorld(screenPos, viewportSize);
        zoom_ *= std::pow(1.1f, wheelDelta);
        auto worldAfter = screenToWorld(screenPos, viewportSize);
        position_ += (worldBefore - worldAfter) * zoom_;
    }

  private:
    glm::vec2 position_{0.0f};
    float zoom_ = 1.0f;
    float rotation_ = 0.0f;
};

} // namespace codotaku

#endif
