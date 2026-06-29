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
#include <codotaku/graphics_pipeline.h>
#include <codotaku/render_pass.h>
#include <codotaku/runtime_info.h>
#include <codotaku/shader.h>
#include <codotaku/staging_belt.h>

namespace codotaku
{

template<typename Vertex> class Geometry;

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
    [[nodiscard]] SDL_GPUTextureFormat swapchainFormat() const noexcept;

    [[nodiscard]] DataBlob loadFile(const std::filesystem::path &relativePath) const noexcept;

    [[nodiscard]] Shader loadShader(
        const std::filesystem::path &relativePath,
        SDL_GPUShaderStage stage,
        const char *entrypoint) const;

    [[nodiscard]] Shader loadShader(
        const std::filesystem::path &relativePath,
        const char *entrypoint = "main") const;

    [[nodiscard]] Buffer createBuffer(Uint32 size) const;

    [[nodiscard]] Buffer createBuffer(SDL_GPUBufferUsageFlags usage, Uint32 size) const;

    [[nodiscard]] Buffer createBuffer(
        SDL_GPUBufferUsageFlags usage,
        std::span<const Uint8> data,
        StagingBelt &belt) const;

    [[nodiscard]] Buffer createBuffer(
        SDL_GPUBufferUsageFlags usage,
        const void *data, size_t size,
        StagingBelt &belt) const;

    [[nodiscard]] GraphicsPipeline loadPipeline(
        const std::filesystem::path &shaderBasePath,
        const VertexInputBuilder &vertexInput,
        SDL_GPUTextureFormat colorTargetFormat,
        const char *vertEntrypoint = "VSMain",
        const char *fragEntrypoint = "PSMain") const;

    [[nodiscard]] GraphicsPipeline createPipeline(
        const SDL_GPUGraphicsPipelineCreateInfo &info) const;

    [[nodiscard]] GraphicsPipeline createPipeline(
        const Shader &vertexShader,
        const Shader &fragmentShader,
        const VertexInputBuilder &vertexInput,
        SDL_GPUTextureFormat colorTargetFormat) const;

    [[nodiscard]] StagingBelt createBelt(size_t initialCapacity = 4 * 1024 * 1024) const;

    template<typename Vertex>
    [[nodiscard]] Geometry<Vertex> createGeometry(
        StagingBelt &belt,
        std::span<const Vertex> vertices,
        std::span<const Uint32> indices,
        SDL_GPUIndexElementSize indexElementSize = SDL_GPU_INDEXELEMENTSIZE_32BIT) const;

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
