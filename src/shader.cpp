#include <codotaku/shader.h>

#include <codotaku/runtime.h>

namespace codotaku
{

Shader::Shader(
    SDL_GPUDevice *device,
    SDL_GPUShaderFormat format,
    std::span<const Uint8> code,
    SDL_GPUShaderStage stage,
    const char *entrypoint)
    : device_(device), owns_(true)
{
    SDL_GPUShaderCreateInfo ci{};
    ci.code = code.data();
    ci.code_size = code.size();
    ci.entrypoint = entrypoint;
    ci.format = format;
    ci.stage = stage;

    shader_ = SDL_CreateGPUShader(device, &ci);
    if (!shader_) {
        throw SDLException("SDL_CreateGPUShader failed");
    }
}

Shader::~Shader() noexcept
{
    release();
}

Shader::Shader(Shader &&other) noexcept
    : device_(other.device_), shader_(other.shader_), owns_(other.owns_)
{
    other.device_ = nullptr;
    other.shader_ = nullptr;
    other.owns_ = false;
}

Shader &Shader::operator=(Shader &&other) noexcept
{
    if (this != &other) {
        release();
        device_ = other.device_;
        shader_ = other.shader_;
        owns_ = other.owns_;
        other.device_ = nullptr;
        other.shader_ = nullptr;
        other.owns_ = false;
    }
    return *this;
}

void Shader::release() noexcept
{
    if (owns_ && shader_) {
        SDL_ReleaseGPUShader(device_, shader_);
    }
}

} // namespace codotaku
