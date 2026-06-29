#ifndef CODOTAKU_BUFFER_H
#define CODOTAKU_BUFFER_H

#include <SDL3/SDL_gpu.h>

namespace codotaku
{

class Buffer
{
  public:
    Buffer() noexcept = default;

    Buffer(SDL_GPUDevice *device, SDL_GPUBuffer *buffer) noexcept
        : device_(device), buffer_(buffer)
    {
    }

    ~Buffer() noexcept
    {
        if (buffer_) {
            SDL_ReleaseGPUBuffer(device_, buffer_);
        }
    }

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    Buffer(Buffer &&other) noexcept
        : device_(other.device_), buffer_(other.buffer_)
    {
        other.device_ = nullptr;
        other.buffer_ = nullptr;
    }

    Buffer &operator=(Buffer &&other) noexcept
    {
        if (this != &other) {
            if (buffer_) {
                SDL_ReleaseGPUBuffer(device_, buffer_);
            }
            device_ = other.device_;
            buffer_ = other.buffer_;
            other.device_ = nullptr;
            other.buffer_ = nullptr;
        }
        return *this;
    }

    [[nodiscard]] SDL_GPUBuffer *handle() const noexcept
    {
        return buffer_;
    }

  private:
    SDL_GPUDevice *device_ = nullptr;
    SDL_GPUBuffer *buffer_ = nullptr;
};

} // namespace codotaku

#endif
