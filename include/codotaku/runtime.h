#ifndef CODOTAKU_RUNTIME_H
#define CODOTAKU_RUNTIME_H

#include <SDL3/SDL_gpu.h>
#include <functional>
#include <glm/vec2.hpp>
#include <span>
#include <stdexcept>

#include <codotaku/render_pass.h>
#include <codotaku/runtime_info.h>

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

  private:
    void cleanup() noexcept;

    SDL_Window *window_ = nullptr;
    SDL_GPUDevice *device_ = nullptr;
    RuntimeInfo info_;
    bool running_ = false;
};

} // namespace codotaku

#endif
