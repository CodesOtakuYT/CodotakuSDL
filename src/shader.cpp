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
    : device_(device)
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
    SDL_ReleaseGPUShader(device_, shader_);
}

} // namespace codotaku
