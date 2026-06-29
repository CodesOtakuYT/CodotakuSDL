#ifndef CODOTAKU_GRAPHICS_PIPELINE_H
#define CODOTAKU_GRAPHICS_PIPELINE_H

#include <SDL3/SDL_gpu.h>
#include <cstddef>

namespace codotaku
{

class GraphicsPipeline
{
  public:
    GraphicsPipeline() noexcept = default;

    GraphicsPipeline(SDL_GPUDevice *device, const SDL_GPUGraphicsPipelineCreateInfo &info);

    ~GraphicsPipeline() noexcept;

    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;

    GraphicsPipeline(GraphicsPipeline &&other) noexcept
        : device_(other.device_), pipeline_(other.pipeline_)
    {
        other.device_ = nullptr;
        other.pipeline_ = nullptr;
    }

    GraphicsPipeline &operator=(GraphicsPipeline &&other) noexcept
    {
        if (this != &other) {
            if (pipeline_) {
                SDL_ReleaseGPUGraphicsPipeline(device_, pipeline_);
            }
            device_ = other.device_;
            pipeline_ = other.pipeline_;
            other.device_ = nullptr;
            other.pipeline_ = nullptr;
        }
        return *this;
    }

    [[nodiscard]] SDL_GPUGraphicsPipeline *handle() const noexcept
    {
        return pipeline_;
    }

  private:
    SDL_GPUDevice *device_ = nullptr;
    SDL_GPUGraphicsPipeline *pipeline_ = nullptr;
};

class VertexInputBuilder
{
  public:
    void addBuffer(Uint32 slot, Uint32 pitch, SDL_GPUVertexInputRate inputRate);

    void addAttribute(
        Uint32 bufferSlot,
        SDL_GPUVertexElementFormat format,
        Uint32 offset);

    [[nodiscard]] SDL_GPUVertexInputState build() const noexcept;

    void reset() noexcept;

  private:
    static constexpr int kMaxBuffers = 4;
    static constexpr int kMaxAttributes = 16;

    SDL_GPUVertexBufferDescription buffers_[kMaxBuffers]{};
    SDL_GPUVertexAttribute attributes_[kMaxAttributes]{};
    Uint32 numBuffers_ = 0;
    Uint32 numAttributes_ = 0;
};

} // namespace codotaku

#endif
