#include <codotaku/graphics_pipeline.h>
#include <codotaku/runtime.h>

namespace codotaku
{

GraphicsPipeline::GraphicsPipeline(SDL_GPUDevice *device, const SDL_GPUGraphicsPipelineCreateInfo &info)
    : device_(device), owns_(true)
{
    pipeline_ = SDL_CreateGPUGraphicsPipeline(device, &info);
    if (!pipeline_) {
        throw SDLException("Failed to create graphics pipeline");
    }
}

GraphicsPipeline::~GraphicsPipeline() noexcept
{
    release();
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline &&other) noexcept
    : device_(other.device_), pipeline_(other.pipeline_), owns_(other.owns_)
{
    other.device_ = nullptr;
    other.pipeline_ = nullptr;
    other.owns_ = false;
}

GraphicsPipeline &GraphicsPipeline::operator=(GraphicsPipeline &&other) noexcept
{
    if (this != &other) {
        release();
        device_ = other.device_;
        pipeline_ = other.pipeline_;
        owns_ = other.owns_;
        other.device_ = nullptr;
        other.pipeline_ = nullptr;
        other.owns_ = false;
    }
    return *this;
}

void GraphicsPipeline::release() noexcept
{
    if (owns_ && pipeline_) {
        SDL_ReleaseGPUGraphicsPipeline(device_, pipeline_);
    }
}

void VertexInputBuilder::addBuffer(Uint32 slot, Uint32 pitch, SDL_GPUVertexInputRate inputRate)
{
    buffers_[numBuffers_++] = {
        .slot = slot,
        .pitch = pitch,
        .input_rate = inputRate,
    };
}

void VertexInputBuilder::addAttribute(
    Uint32 bufferSlot,
    SDL_GPUVertexElementFormat format,
    Uint32 offset)
{
    attributes_[numAttributes_] = {
        .location = numAttributes_,
        .buffer_slot = bufferSlot,
        .format = format,
        .offset = offset,
    };
    numAttributes_++;
}

SDL_GPUVertexInputState VertexInputBuilder::build() const noexcept
{
    return {
        .vertex_buffer_descriptions = buffers_,
        .num_vertex_buffers = numBuffers_,
        .vertex_attributes = attributes_,
        .num_vertex_attributes = numAttributes_,
    };
}

void VertexInputBuilder::reset() noexcept
{
    numBuffers_ = 0;
    numAttributes_ = 0;
}

} // namespace codotaku
