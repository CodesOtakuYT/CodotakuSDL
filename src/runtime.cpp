#include <codotaku/runtime.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_filesystem.h>
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

    if (auto *bp = SDL_GetBasePath()) {
        basePath_ = std::filesystem::path(bp);
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

    auto formats = SDL_GetGPUShaderFormats(device_);
    if (formats & SDL_GPU_SHADERFORMAT_DXIL) {
        shaderFormat_ = SDL_GPU_SHADERFORMAT_DXIL;
        shaderExt_ = "dxil";
    } else if (formats & SDL_GPU_SHADERFORMAT_MSL) {
        shaderFormat_ = SDL_GPU_SHADERFORMAT_MSL;
        shaderExt_ = "msl";
    } else {
        shaderFormat_ = SDL_GPU_SHADERFORMAT_SPIRV;
        shaderExt_ = "spv";
    }
}

Runtime::~Runtime()
{
    cleanup();
}

Runtime::Runtime(Runtime &&other) noexcept
    : basePath_(std::move(other.basePath_)),
      shaderExt_(std::move(other.shaderExt_)),
      window_(other.window_),
      device_(other.device_),
      shaderFormat_(other.shaderFormat_),
      info_(std::move(other.info_)),
      running_(other.running_)
{
    other.window_ = nullptr;
    other.device_ = nullptr;
    other.running_ = false;
}

Runtime &Runtime::operator=(Runtime &&other) noexcept
{
    if (this != &other) {
        cleanup();
        basePath_ = std::move(other.basePath_);
        shaderExt_ = std::move(other.shaderExt_);
        window_ = other.window_;
        device_ = other.device_;
        shaderFormat_ = other.shaderFormat_;
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

SDL_GPUShaderFormat Runtime::shaderFormat() const noexcept
{
    return shaderFormat_;
}

DataBlob Runtime::loadFile(const std::filesystem::path &relativePath) const noexcept
{
    auto fullPath = basePath_.empty() ? relativePath : basePath_ / relativePath;
    size_t size = 0;
    auto *data = (Uint8 *)SDL_LoadFile(fullPath.string().c_str(), &size);
    return DataBlob(data, size);
}

Shader Runtime::loadShader(
    const std::filesystem::path &relativePath,
    SDL_GPUShaderStage stage,
    const char *entrypoint) const
{
    auto path = relativePath;
    path += "." + shaderExt_;
    auto blob = loadFile(path);
    if (!blob) {
        throw SDLException(("Failed to load shader: " + path.string()).c_str());
    }
    return Shader(device_, shaderFormat_, blob.span(), stage, entrypoint);
}

Shader Runtime::loadShader(
    const std::filesystem::path &relativePath,
    const char *entrypoint) const
{
    auto name = relativePath.filename().string();
    SDL_GPUShaderStage stage;
    if (name.contains("_vert") || name.contains(".vert")) {
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    } else if (name.contains("_frag") || name.contains(".frag")) {
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    } else {
        throw SDLException(("Cannot detect shader stage from: " + name).c_str());
    }
    return loadShader(relativePath, stage, entrypoint);
}

Buffer Runtime::createBuffer(SDL_GPUBufferUsageFlags usage, Uint32 size) const
{
    SDL_GPUBufferCreateInfo info{};
    info.usage = usage;
    info.size = size;
    auto *buf = SDL_CreateGPUBuffer(device_, &info);
    if (!buf) {
        throw SDLException("Failed to create GPU buffer");
    }
    return Buffer(device_, buf);
}

Buffer Runtime::createBuffer(
    SDL_GPUBufferUsageFlags usage,
    std::span<const Uint8> data,
    StagingBelt &belt) const
{
    auto buf = createBuffer(usage, static_cast<Uint32>(data.size()));
    belt.upload(buf.handle(), 0, data);
    return buf;
}

Buffer Runtime::createBuffer(
    SDL_GPUBufferUsageFlags usage,
    const void *data, size_t size,
    StagingBelt &belt) const
{
    return createBuffer(usage, std::span<const Uint8>(static_cast<const Uint8 *>(data), size), belt);
}

GraphicsPipeline Runtime::loadPipeline(
    const std::filesystem::path &shaderBasePath,
    const VertexInputBuilder &vertexInput,
    SDL_GPUTextureFormat colorTargetFormat,
    const char *vertEntrypoint,
    const char *fragEntrypoint) const
{
    auto vertPath = shaderBasePath;
    vertPath += ".vert";
    auto vert = loadShader(vertPath, SDL_GPU_SHADERSTAGE_VERTEX, vertEntrypoint);

    auto fragPath = shaderBasePath;
    fragPath += ".frag";
    auto frag = loadShader(fragPath, SDL_GPU_SHADERSTAGE_FRAGMENT, fragEntrypoint);

    return createPipeline(vert, frag, vertexInput, colorTargetFormat);
}

void Runtime::submitOneShot(std::move_only_function<void(SDL_GPUCommandBuffer *)> fn) const
{
    auto *cmdBuf = SDL_AcquireGPUCommandBuffer(device_);
    fn(cmdBuf);
    if (!SDL_SubmitGPUCommandBuffer(cmdBuf)) {
        throw SDLException("submitOneShot: failed to submit command buffer");
    }
    SDL_WaitForGPUIdle(device_);
}

SDL_GPUTextureFormat Runtime::swapchainFormat() const noexcept
{
    return SDL_GetGPUSwapchainTextureFormat(device_, window_);
}

GraphicsPipeline Runtime::createPipeline(const SDL_GPUGraphicsPipelineCreateInfo &info) const
{
    return GraphicsPipeline(device_, info);
}

GraphicsPipeline Runtime::createPipeline(
    const Shader &vertexShader,
    const Shader &fragmentShader,
    const VertexInputBuilder &vertexInput,
    SDL_GPUTextureFormat colorTargetFormat) const
{
    SDL_GPUColorTargetDescription colorTarget{ .format = colorTargetFormat };

    SDL_GPUGraphicsPipelineCreateInfo pi{};
    pi.vertex_shader = vertexShader.handle();
    pi.fragment_shader = fragmentShader.handle();
    pi.vertex_input_state = vertexInput.build();
    pi.target_info.color_target_descriptions = &colorTarget;
    pi.target_info.num_color_targets = 1;

    return GraphicsPipeline(device_, pi);
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
