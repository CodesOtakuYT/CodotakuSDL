#ifndef CODOTAKU_RENDER_PASS_H
#define CODOTAKU_RENDER_PASS_H

#include <SDL3/SDL_gpu.h>

namespace codotaku
{

class RenderPass
{
  public:
    RenderPass() = default;

    explicit RenderPass(SDL_GPURenderPass *pass) noexcept
        : pass_(pass)
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

    void bindPipeline(SDL_GPUGraphicsPipeline *pipeline) noexcept
    {
        SDL_BindGPUGraphicsPipeline(pass_, pipeline);
    }

    void bindVertexBuffer(SDL_GPUBuffer *buffer, Uint32 slot = 0, Uint32 offset = 0) noexcept
    {
        SDL_GPUBufferBinding binding{ .buffer = buffer, .offset = offset };
        SDL_BindGPUVertexBuffers(pass_, slot, &binding, 1);
    }

    void bindIndexBuffer(SDL_GPUBuffer *buffer, SDL_GPUIndexElementSize elementSize, Uint32 offset = 0) noexcept
    {
        SDL_GPUBufferBinding binding{ .buffer = buffer, .offset = offset };
        SDL_BindGPUIndexBuffer(pass_, &binding, elementSize);
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
    SDL_GPURenderPass *pass_ = nullptr;
};

} // namespace codotaku

#endif
