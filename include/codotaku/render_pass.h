#ifndef CODOTAKU_RENDER_PASS_H
#define CODOTAKU_RENDER_PASS_H

#include <SDL3/SDL_gpu.h>

#include <codotaku/buffer.h>
#include <codotaku/graphics_pipeline.h>

namespace codotaku
{

class RenderPass
{
  public:
    RenderPass() = default;

    RenderPass(SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass) noexcept
        : cmdBuf_(cmdBuf), pass_(pass)
    {
    }

    ~RenderPass() noexcept
    {
        SDL_EndGPURenderPass(pass_);
    }

    RenderPass(RenderPass &&) = delete;
    RenderPass &operator=(RenderPass &&) = delete;
    RenderPass(const RenderPass &) = delete;
    RenderPass &operator=(const RenderPass &) = delete;

    [[nodiscard]] SDL_GPURenderPass *handle() const noexcept
    {
        return pass_;
    }

    void bindPipeline(const GraphicsPipeline &pipeline) noexcept;

    void bindVertexBuffer(const Buffer &buffer, Uint32 slot = 0, Uint32 offset = 0) noexcept;

    void bindVertexBuffers(Uint32 firstSlot, const SDL_GPUBufferBinding *bindings, Uint32 count) noexcept
    {
        SDL_BindGPUVertexBuffers(pass_, firstSlot, bindings, count);
    }

    void bindIndexBuffer(
        const Buffer &buffer,
        Uint32 offset = 0,
        SDL_GPUIndexElementSize elementSize = SDL_GPU_INDEXELEMENTSIZE_32BIT) noexcept;

    void pushVertexUniform(Uint32 slotIndex, const void *data, Uint32 length) noexcept
    {
        SDL_PushGPUVertexUniformData(cmdBuf_, slotIndex, data, length);
    }

    void pushFragmentUniform(Uint32 slotIndex, const void *data, Uint32 length) noexcept
    {
        SDL_PushGPUFragmentUniformData(cmdBuf_, slotIndex, data, length);
    }

    void bindFragmentSampler(Uint32 slot, SDL_GPUTexture *texture, SDL_GPUSampler *sampler) noexcept
    {
        SDL_GPUTextureSamplerBinding binding{ .texture = texture, .sampler = sampler };
        SDL_BindGPUFragmentSamplers(pass_, slot, &binding, 1);
    }

    void bindVertexSampler(Uint32 slot, SDL_GPUTexture *texture, SDL_GPUSampler *sampler) noexcept
    {
        SDL_GPUTextureSamplerBinding binding{ .texture = texture, .sampler = sampler };
        SDL_BindGPUVertexSamplers(pass_, slot, &binding, 1);
    }

    void setViewport(const SDL_GPUViewport &viewport) noexcept
    {
        SDL_SetGPUViewport(pass_, &viewport);
    }

    void setViewport(float x, float y, float w, float h, float minDepth = 0.0f, float maxDepth = 1.0f) noexcept
    {
        SDL_GPUViewport viewport{ x, y, w, h, minDepth, maxDepth };
        SDL_SetGPUViewport(pass_, &viewport);
    }

    void setScissor(const SDL_Rect &rect) noexcept
    {
        SDL_SetGPUScissor(pass_, &rect);
    }

    void draw(Uint32 vertexCount, Uint32 instanceCount, Uint32 firstVertex, Uint32 firstInstance) noexcept
    {
        SDL_DrawGPUPrimitives(pass_, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void drawIndexed(Uint32 indexCount, Uint32 instanceCount, Uint32 firstIndex, Sint32 vertexOffset, Uint32 firstInstance) noexcept
    {
        SDL_DrawGPUIndexedPrimitives(pass_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

  private:
    SDL_GPUCommandBuffer *cmdBuf_ = nullptr;
    SDL_GPURenderPass *pass_ = nullptr;
};

inline void RenderPass::bindPipeline(const GraphicsPipeline &pipeline) noexcept
{
    SDL_BindGPUGraphicsPipeline(pass_, pipeline.handle());
}

inline void RenderPass::bindVertexBuffer(const Buffer &buffer, Uint32 slot, Uint32 offset) noexcept
{
    SDL_GPUBufferBinding binding{ .buffer = buffer.handle(), .offset = offset };
    SDL_BindGPUVertexBuffers(pass_, slot, &binding, 1);
}

inline void RenderPass::bindIndexBuffer(const Buffer &buffer, Uint32 offset, SDL_GPUIndexElementSize elementSize) noexcept
{
    SDL_GPUBufferBinding binding{ .buffer = buffer.handle(), .offset = offset };
    SDL_BindGPUIndexBuffer(pass_, &binding, elementSize);
}

} // namespace codotaku

#endif
