#include <codotaku/graphics_pipeline.h>
#include <codotaku/runtime.h>

namespace codotaku
{

GraphicsPipeline::GraphicsPipeline(SDL_GPUDevice *device, const SDL_GPUGraphicsPipelineCreateInfo &info)
{
    pipeline_ = SDL_CreateGPUGraphicsPipeline(device, &info);
    if (!pipeline_) {
        throw SDLException("Failed to create graphics pipeline");
    }
    device_ = device;
}

GraphicsPipeline::~GraphicsPipeline() noexcept
{
    if (pipeline_) {
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
