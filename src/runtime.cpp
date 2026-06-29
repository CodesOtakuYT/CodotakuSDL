#include <codotaku/runtime.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

namespace codotaku
{

SDLException::SDLException(const char *message)
    : std::runtime_error(std::string(message) + ": " + SDL_GetError())
{
}

RenderPass FrameContext::beginRenderPass(
    SDL_FColor clearColor,
    SDL_GPULoadOp loadOp,
    SDL_GPUStoreOp storeOp) const noexcept
{
    SDL_GPUColorTargetInfo target{};
    target.texture = swapchainTexture;
    target.clear_color = clearColor;
    target.load_op = loadOp;
    target.store_op = storeOp;
    return RenderPass(SDL_BeginGPURenderPass(cmd, &target, 1, nullptr));
}

RenderPass FrameContext::beginRenderPass(
    std::span<const SDL_GPUColorTargetInfo> colorTargets,
    const SDL_GPUDepthStencilTargetInfo *depthStencil) const noexcept
{
    return RenderPass(SDL_BeginGPURenderPass(
        cmd,
        colorTargets.data(),
        static_cast<Uint32>(colorTargets.size()),
        depthStencil));
}

void FrameContext::setViewport(SDL_GPURenderPass *pass) const noexcept
{
    SDL_GPUViewport viewport{
        0.0f, 0.0f,
        static_cast<float>(swapchainSize.x),
        static_cast<float>(swapchainSize.y),
        0.0f, 1.0f
    };
    SDL_SetGPUViewport(pass, &viewport);
}

Runtime::Runtime(const RuntimeInfo &info)
    : info_(info)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw SDLException("SDL_Init failed");
    }

    SDL_WindowFlags flags = 0;
    if (info_.resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    window_ = SDL_CreateWindow(
        info_.title.c_str(),
        info_.windowSize.x,
        info_.windowSize.y,
        flags);
    if (!window_) {
        SDL_Quit();
        throw SDLException("SDL_CreateWindow failed");
    }

    device_ = SDL_CreateGPUDevice(
        info_.shaderFormats,
        info_.gpuDebug,
        nullptr);
    if (!device_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
        throw SDLException("SDL_CreateGPUDevice failed");
    }

    if (!SDL_ClaimWindowForGPUDevice(device_, window_)) {
        SDL_DestroyGPUDevice(device_);
        device_ = nullptr;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
        throw SDLException("SDL_ClaimWindowForGPUDevice failed");
    }

    if (!info_.vsync) {
        if (!SDL_SetGPUSwapchainParameters(
                device_,
                window_,
                SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                SDL_GPU_PRESENTMODE_IMMEDIATE)) {
            cleanup();
            throw SDLException("SDL_SetGPUSwapchainParameters failed");
        }
    }
}

Runtime::~Runtime()
{
    cleanup();
}

Runtime::Runtime(Runtime &&other) noexcept
    : window_(other.window_), device_(other.device_), info_(std::move(other.info_)), running_(other.running_)
{
    other.window_ = nullptr;
    other.device_ = nullptr;
    other.running_ = false;
}

Runtime &Runtime::operator=(Runtime &&other) noexcept
{
    if (this != &other) {
        cleanup();
        window_ = other.window_;
        device_ = other.device_;
        info_ = std::move(other.info_);
        running_ = other.running_;
        other.window_ = nullptr;
        other.device_ = nullptr;
        other.running_ = false;
    }
    return *this;
}

void Runtime::run(std::move_only_function<void(FrameContext)> frame)
{
    running_ = true;

    while (running_) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running_ = false;
            }
        }
        if (!running_)
            break;

        SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(device_);
        if (!cmd) {
            throw SDLException("SDL_AcquireGPUCommandBuffer failed");
        }

        SDL_GPUTexture *tex = nullptr;
        Uint32 w = 0, h = 0;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window_, &tex, &w, &h)) {
            throw SDLException("SDL_WaitAndAcquireGPUSwapchainTexture failed");
        }

        if (tex) {
            frame(FrameContext{
                .cmd = cmd,
                .swapchainTexture = tex,
                .swapchainSize = { static_cast<int>(w), static_cast<int>(h) } });
        }

        if (!SDL_SubmitGPUCommandBuffer(cmd)) {
            throw SDLException("SDL_SubmitGPUCommandBuffer failed");
        }
    }

    SDL_WaitForGPUIdle(device_);
}

void Runtime::requestQuit() noexcept
{
    running_ = false;
}

bool Runtime::isRunning() const noexcept
{
    return running_;
}

SDL_Window *Runtime::window() const noexcept
{
    return window_;
}

SDL_GPUDevice *Runtime::device() const noexcept
{
    return device_;
}

void Runtime::cleanup() noexcept
{
    SDL_WaitForGPUIdle(device_);
    SDL_ReleaseWindowFromGPUDevice(device_, window_);
    SDL_DestroyGPUDevice(device_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

} // namespace codotaku
