#ifndef CODOTAKU_STAGING_BELT_H
#define CODOTAKU_STAGING_BELT_H

#include <SDL3/SDL_gpu.h>
#include <cstddef>
#include <span>
#include <vector>

namespace codotaku
{

class Buffer;

struct TextureUploadInfo
{
    SDL_GPUTexture *texture;
    SDL_GPUTextureFormat format;
    Uint32 mipLevel = 0;
    Uint32 layer = 0;
    Uint32 x = 0, y = 0, z = 0;
    Uint32 w, h, d = 1;
    Uint32 pixelsPerRow = 0;
    Uint32 rowsPerLayer = 0;
};

class StagingBelt
{
  public:
    StagingBelt(SDL_GPUDevice *device, size_t initialCapacity = 4 * 1024 * 1024);
    ~StagingBelt();

    StagingBelt(const StagingBelt &) = delete;
    StagingBelt &operator=(const StagingBelt &) = delete;

    StagingBelt(StagingBelt &&) noexcept;
    StagingBelt &operator=(StagingBelt &&) noexcept;

    void upload(const Buffer &dst, size_t dstOffset, std::span<const Uint8> data);
    void upload(const Buffer &dst, size_t dstOffset, const void *data, size_t size);
    void upload(const TextureUploadInfo &info, std::span<const Uint8> data);
    void upload(const TextureUploadInfo &info, const void *data, size_t size);

    void flush(SDL_GPUCommandBuffer *cmdBuf);
    void flush();

    void reset();

  private:
    struct BufferUpload
    {
        std::span<const Uint8> data;
        SDL_GPUBuffer *buffer;
        Uint32 dstOffset;
        Uint32 srcOffset;
    };

    struct TextureUpload
    {
        std::span<const Uint8> data;
        TextureUploadInfo info;
        Uint32 srcOffset;
    };

    void grow(size_t newSize);

    SDL_GPUDevice *device_;
    SDL_GPUTransferBuffer *transferBuf_ = nullptr;
    size_t capacity_;
    std::vector<BufferUpload> bufferEntries_;
    std::vector<TextureUpload> textureEntries_;
};

} // namespace codotaku

#endif
