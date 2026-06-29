#include <codotaku/staging_belt.h>
#include <codotaku/buffer.h>
#include <codotaku/runtime.h>
#include <cstring>

namespace codotaku
{

static size_t alignUp(size_t value, size_t alignment)
{
    if (alignment == 0) return value;
    return (value + alignment - 1) / alignment * alignment;
}

StagingBelt::StagingBelt(SDL_GPUDevice *device, size_t initialCapacity)
    : device_(device), capacity_(initialCapacity)
{
    SDL_GPUTransferBufferCreateInfo info{};
    info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    info.size = static_cast<Uint32>(capacity_);
    transferBuf_ = SDL_CreateGPUTransferBuffer(device_, &info);
    if (!transferBuf_) {
        throw SDLException("StagingBelt: failed to create transfer buffer");
    }
}

StagingBelt::~StagingBelt()
{
    if (transferBuf_) {
        SDL_ReleaseGPUTransferBuffer(device_, transferBuf_);
    }
}

StagingBelt::StagingBelt(StagingBelt &&other) noexcept
    : device_(other.device_),
      transferBuf_(other.transferBuf_),
      capacity_(other.capacity_),
      bufferEntries_(std::move(other.bufferEntries_)),
      textureEntries_(std::move(other.textureEntries_))
{
    other.device_ = nullptr;
    other.transferBuf_ = nullptr;
    other.capacity_ = 0;
}

StagingBelt &StagingBelt::operator=(StagingBelt &&other) noexcept
{
    if (this != &other) {
        if (transferBuf_) {
            SDL_ReleaseGPUTransferBuffer(device_, transferBuf_);
        }
        device_ = other.device_;
        transferBuf_ = other.transferBuf_;
        capacity_ = other.capacity_;
        bufferEntries_ = std::move(other.bufferEntries_);
        textureEntries_ = std::move(other.textureEntries_);
        other.device_ = nullptr;
        other.transferBuf_ = nullptr;
        other.capacity_ = 0;
    }
    return *this;
}

void StagingBelt::upload(const Buffer &dst, size_t dstOffset, std::span<const Uint8> data)
{
    bufferEntries_.push_back({ .data = data, .buffer = dst.handle(), .dstOffset = static_cast<Uint32>(dstOffset), .srcOffset = 0 });
}

void StagingBelt::upload(const Buffer &dst, size_t dstOffset, const void *data, size_t size)
{
    upload(dst, dstOffset, std::span<const Uint8>(static_cast<const Uint8 *>(data), size));
}

void StagingBelt::upload(const TextureUploadInfo &info, std::span<const Uint8> data)
{
    textureEntries_.push_back({ .data = data, .info = info, .srcOffset = 0 });
}

void StagingBelt::upload(const TextureUploadInfo &info, const void *data, size_t size)
{
    upload(info, std::span<const Uint8>(static_cast<const Uint8 *>(data), size));
}

void StagingBelt::flush(SDL_GPUCommandBuffer *cmdBuf)
{
    if (bufferEntries_.empty() && textureEntries_.empty()) return;

    // compute total size and per-entry offsets with alignment
    size_t totalSize = 0;
    for (auto &entry : bufferEntries_) {
        entry.srcOffset = static_cast<Uint32>(totalSize);
        totalSize += entry.data.size();
    }
    for (auto &entry : textureEntries_) {
        auto texelSize = SDL_GPUTextureFormatTexelBlockSize(entry.info.format);
        totalSize = alignUp(totalSize, texelSize);
        entry.srcOffset = static_cast<Uint32>(totalSize);
        totalSize += entry.data.size();
    }

    if (totalSize > capacity_) {
        grow(totalSize);
    }

    // map and write
    auto *mapped = static_cast<Uint8 *>(SDL_MapGPUTransferBuffer(device_, transferBuf_, false));
    for (auto &entry : bufferEntries_) {
        std::memcpy(mapped + entry.srcOffset, entry.data.data(), entry.data.size());
    }
    for (auto &entry : textureEntries_) {
        std::memcpy(mapped + entry.srcOffset, entry.data.data(), entry.data.size());
    }
    SDL_UnmapGPUTransferBuffer(device_, transferBuf_);

    // record copy pass
    auto *copyPass = SDL_BeginGPUCopyPass(cmdBuf);
    for (auto &entry : bufferEntries_) {
        SDL_GPUTransferBufferLocation src{ .transfer_buffer = transferBuf_, .offset = entry.srcOffset };
        SDL_GPUBufferRegion dst{ .buffer = entry.buffer, .offset = entry.dstOffset, .size = static_cast<Uint32>(entry.data.size()) };
        SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);
    }
    for (auto &entry : textureEntries_) {
        auto &info = entry.info;
        auto ppr = info.pixelsPerRow != 0 ? info.pixelsPerRow : info.w;
        auto rpl = info.rowsPerLayer != 0 ? info.rowsPerLayer : info.h;
        SDL_GPUTextureTransferInfo src{
            .transfer_buffer = transferBuf_,
            .offset = entry.srcOffset,
            .pixels_per_row = static_cast<Uint32>(ppr),
            .rows_per_layer = static_cast<Uint32>(rpl),
        };

        SDL_GPUTextureRegion dst{
            .texture = info.texture,
            .mip_level = info.mipLevel,
            .layer = info.layer,
            .x = info.x,
            .y = info.y,
            .z = info.z,
            .w = info.w,
            .h = info.h,
            .d = info.d,
        };

        SDL_UploadToGPUTexture(copyPass, &src, &dst, false);
    }
    SDL_EndGPUCopyPass(copyPass);

    reset();
}

void StagingBelt::flush()
{
    auto *cmdBuf = SDL_AcquireGPUCommandBuffer(device_);
    flush(cmdBuf);
    if (!SDL_SubmitGPUCommandBuffer(cmdBuf)) {
        throw SDLException("StagingBelt: failed to submit command buffer");
    }
    SDL_WaitForGPUIdle(device_);
}

void StagingBelt::reset()
{
    bufferEntries_.clear();
    textureEntries_.clear();
}

void StagingBelt::grow(size_t newSize)
{
    if (transferBuf_) {
        SDL_ReleaseGPUTransferBuffer(device_, transferBuf_);
    }
    SDL_GPUTransferBufferCreateInfo info{};
    info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    info.size = static_cast<Uint32>(newSize);
    transferBuf_ = SDL_CreateGPUTransferBuffer(device_, &info);
    if (!transferBuf_) {
        throw SDLException("StagingBelt: failed to grow transfer buffer");
    }
    capacity_ = newSize;
}

} // namespace codotaku
