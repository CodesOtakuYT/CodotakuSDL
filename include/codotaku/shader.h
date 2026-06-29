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

    Shader(SDL_GPUDevice *device, SDL_GPUShader *shader, bool owns = true) noexcept
        : device_(device), shader_(shader), owns_(owns)
    {
    }

    ~Shader() noexcept;

    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    Shader(Shader &&other) noexcept;
    Shader &operator=(Shader &&other) noexcept;

    [[nodiscard]] SDL_GPUShader *handle() const noexcept
    {
        return shader_;
    }

  private:
    void release() noexcept;

    SDL_GPUDevice *device_ = nullptr;
    SDL_GPUShader *shader_ = nullptr;
    bool owns_ = false;
};

} // namespace codotaku

#endif
