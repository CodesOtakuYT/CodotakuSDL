#ifndef CODOTAKU_SHADER_H
#define CODOTAKU_SHADER_H

#include <SDL3/SDL_gpu.h>
#include <span>

namespace codotaku
{

class Shader
{
  public:
    Shader() noexcept = default;

    Shader(
        SDL_GPUDevice *device,
        SDL_GPUShaderFormat format,
        std::span<const Uint8> code,
        SDL_GPUShaderStage stage,
        const char *entrypoint = "main");

    ~Shader() noexcept;

    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    Shader(Shader &&other) noexcept
        : device_(other.device_), shader_(other.shader_)
    {
        other.device_ = nullptr;
        other.shader_ = nullptr;
    }

    Shader &operator=(Shader &&other) noexcept
    {
        if (this != &other) {
            SDL_ReleaseGPUShader(device_, shader_);
            device_ = other.device_;
            shader_ = other.shader_;
            other.device_ = nullptr;
            other.shader_ = nullptr;
        }
        return *this;
    }

    [[nodiscard]] SDL_GPUShader *handle() const noexcept
    {
        return shader_;
    }

  private:
    SDL_GPUDevice *device_ = nullptr;
    SDL_GPUShader *shader_ = nullptr;
};

} // namespace codotaku

#endif
