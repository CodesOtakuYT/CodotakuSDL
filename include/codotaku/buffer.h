#ifndef CODOTAKU_BUFFER_H
#define CODOTAKU_BUFFER_H

#include <SDL3/SDL_gpu.h>

namespace codotaku
{

class Buffer
{
  public:
    Buffer() noexcept = default;

    Buffer(SDL_GPUDevice *device, SDL_GPUBuffer *handle, bool owns = true, Uint32 size = 0) noexcept
        : device_(device), handle_(handle), owns_(owns), size_(size)
    {
    }

    ~Buffer() noexcept
    {
        if (owns_ && handle_) {
            SDL_ReleaseGPUBuffer(device_, handle_);
        }
    }

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    Buffer(Buffer &&other) noexcept
        : device_(other.device_), handle_(other.handle_), owns_(other.owns_), size_(other.size_)
    {
        other.device_ = nullptr;
        other.handle_ = nullptr;
        other.owns_ = false;
        other.size_ = 0;
    }

    Buffer &operator=(Buffer &&other) noexcept
    {
        if (this != &other) {
            if (owns_ && handle_) {
                SDL_ReleaseGPUBuffer(device_, handle_);
            }
            device_ = other.device_;
            handle_ = other.handle_;
            owns_ = other.owns_;
            size_ = other.size_;
            other.device_ = nullptr;
            other.handle_ = nullptr;
            other.owns_ = false;
            other.size_ = 0;
        }
        return *this;
    }

    [[nodiscard]] SDL_GPUBuffer *handle() const noexcept
    {
        return handle_;
    }

    [[nodiscard]] Uint32 size() const noexcept
    {
        return size_;
    }

  private:
    SDL_GPUDevice *device_ = nullptr;
    SDL_GPUBuffer *handle_ = nullptr;
    bool owns_ = false;
    Uint32 size_ = 0;
};

} // namespace codotaku

#endif
