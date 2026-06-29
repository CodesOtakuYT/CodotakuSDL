#ifndef CODOTAKU_RUNTIME_INFO_H
#define CODOTAKU_RUNTIME_INFO_H

#include <SDL3/SDL_gpu.h>
#include <glm/vec2.hpp>
#include <string>

namespace codotaku
{

struct RuntimeInfo
{
    std::string title = "Application";
    glm::ivec2 windowSize = { 1280, 720 };
    bool vsync = true;
    bool resizable = true;
    bool gpuDebug = false;
    SDL_GPUShaderFormat shaderFormats =
        SDL_GPU_SHADERFORMAT_SPIRV |
        SDL_GPU_SHADERFORMAT_DXIL |
        SDL_GPU_SHADERFORMAT_MSL;
};

} // namespace codotaku

#endif
