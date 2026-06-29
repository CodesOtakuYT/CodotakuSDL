#ifndef CODOTAKU_RUNTIME_H
#define CODOTAKU_RUNTIME_H

#include <SDL3/SDL_gpu.h>
#include <filesystem>
#include <functional>
#include <glm/vec2.hpp>
#include <span>
#include <stdexcept>
#include <string>

#include <codotaku/buffer.h>
#include <codotaku/data_blob.h>
#include <codotaku/render_pass.h>
#include <codotaku/runtime_info.h>
#include <codotaku/shader.h>
#include <codotaku/staging_belt.h>

namespace codotaku
{

struct SDLException : std::runtime_error
{
    explicit SDLException(const char *message);
};

struct FrameContext
{
    SDL_GPUCommandBuffer *cmd;
    SDL_GPUTexture *swapchainTexture;
    glm::ivec2 swapchainSize;

    [[nodiscard]] RenderPass beginRenderPass(
        SDL_FColor clearColor = { 0.0f, 0.0f, 0.0f, 1.0f },
        SDL_GPULoadOp loadOp = SDL_GPU_LOADOP_CLEAR,
        SDL_GPUStoreOp storeOp = SDL_GPU_STOREOP_STORE) const noexcept;

    [[nodiscard]] RenderPass beginRenderPass(
        std::span<const SDL_GPUColorTargetInfo> colorTargets,
        const SDL_GPUDepthStencilTargetInfo *depthStencil = nullptr) const noexcept;

    void setViewport(SDL_GPURenderPass *pass) const noexcept;
};

class Runtime
{
  public:
    explicit Runtime(const RuntimeInfo &info);
    ~Runtime();

    Runtime(const Runtime &) = delete;
    Runtime &operator=(const Runtime &) = delete;

    Runtime(Runtime &&) noexcept;
    Runtime &operator=(Runtime &&) noexcept;

    void run(std::move_only_function<void(FrameContext)> frame);

    void requestQuit() noexcept;
    bool isRunning() const noexcept;

    [[nodiscard]] SDL_Window *window() const noexcept;
    [[nodiscard]] SDL_GPUDevice *device() const noexcept;
    [[nodiscard]] SDL_GPUShaderFormat shaderFormat() const noexcept;

    [[nodiscard]] DataBlob loadFile(const std::filesystem::path &relativePath) const noexcept;

    [[nodiscard]] Shader loadShader(
        const std::filesystem::path &relativePath,
        SDL_GPUShaderStage stage,
        const char *entrypoint) const;

    [[nodiscard]] Buffer createBuffer(SDL_GPUBufferUsageFlags usage, Uint32 size) const;

    [[nodiscard]] Buffer createBuffer(
        SDL_GPUBufferUsageFlags usage,
        std::span<const Uint8> data,
        StagingBelt &belt) const;

    void submitOneShot(std::move_only_function<void(SDL_GPUCommandBuffer *)> fn) const;

  private:
    void cleanup() noexcept;

    std::filesystem::path basePath_;
    std::string shaderExt_ = "spv";
    SDL_Window *window_ = nullptr;
    SDL_GPUDevice *device_ = nullptr;
    SDL_GPUShaderFormat shaderFormat_ = SDL_GPU_SHADERFORMAT_SPIRV;
    RuntimeInfo info_;
    bool running_ = false;
};

} // namespace codotaku

#endif
