#ifndef CODOTAKU_CAMERA2D_H
#define CODOTAKU_CAMERA2D_H

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

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

  private:
    glm::vec2 position_{0.0f};
    float zoom_ = 1.0f;
    float rotation_ = 0.0f;
};

} // namespace codotaku

#endif
