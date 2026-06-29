#ifndef CODOTAKU_GEOMETRY_H
#define CODOTAKU_GEOMETRY_H

#include <cstddef>
#include <span>

#include <SDL3/SDL_gpu.h>

#include <codotaku/runtime.h>

namespace codotaku
{

template<typename Vertex>
class Geometry
{
  public:
    Geometry() = default;

    Geometry(Geometry &&) = default;
    Geometry &operator=(Geometry &&) = default;

    Geometry(
        const Runtime &runtime,
        StagingBelt &belt,
        std::span<const Vertex> vertices,
        std::span<const Uint32> indices,
        SDL_GPUIndexElementSize indexElementSize = SDL_GPU_INDEXELEMENTSIZE_32BIT)
        : vertexCount_(static_cast<Uint32>(vertices.size()))
        , indexCount_(static_cast<Uint32>(indices.size()))
        , indexElementSize_(indexElementSize)
    {
        auto vertSize = static_cast<Uint32>(vertices.size_bytes());
        auto idxSize = static_cast<Uint32>(indices.size_bytes());
        buffer_ = runtime.createBuffer(vertSize + idxSize);
        belt.upload(buffer_, 0, vertices.data(), vertSize);
        belt.upload(buffer_, vertSize, indices.data(), idxSize);
    }

    void bind(RenderPass &pass) const
    {
        auto vertSize = vertexCount_ * sizeof(Vertex);
        pass.bindVertexBuffer(buffer_);
        pass.bindIndexBuffer(buffer_, vertSize, indexElementSize_);
    }

    void drawIndexed(
        RenderPass &pass,
        Uint32 instanceCount = 1,
        Sint32 vertexOffset = 0,
        Uint32 firstInstance = 0) const
    {
        bind(pass);
        pass.drawIndexed(indexCount_, instanceCount, 0, vertexOffset, firstInstance);
    }

    [[nodiscard]] const Buffer &buffer() const noexcept { return buffer_; }
    [[nodiscard]] Uint32 vertexCount() const noexcept { return vertexCount_; }
    [[nodiscard]] Uint32 indexCount() const noexcept { return indexCount_; }

  private:
    Buffer buffer_;
    Uint32 vertexCount_ = 0;
    Uint32 indexCount_ = 0;
    SDL_GPUIndexElementSize indexElementSize_ = SDL_GPU_INDEXELEMENTSIZE_32BIT;
};

template<typename Vertex>
Geometry<Vertex> Runtime::createGeometry(
    StagingBelt &belt,
    std::span<const Vertex> vertices,
    std::span<const Uint32> indices,
    SDL_GPUIndexElementSize indexElementSize) const
{
    return Geometry<Vertex>(*this, belt, vertices, indices, indexElementSize);
}

} // namespace codotaku

#endif
