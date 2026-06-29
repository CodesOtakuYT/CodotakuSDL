#ifndef CODOTAKU_DATA_BLOB_H
#define CODOTAKU_DATA_BLOB_H

#include <SDL3/SDL.h>
#include <span>

namespace codotaku
{

class DataBlob
{
  public:
    DataBlob() noexcept = default;

    DataBlob(Uint8 *data, size_t size) noexcept
        : data_(data), size_(size)
    {
    }

    ~DataBlob() noexcept
    {
        SDL_free(data_);
    }

    DataBlob(const DataBlob &) = delete;
    DataBlob &operator=(const DataBlob &) = delete;

    DataBlob(DataBlob &&other) noexcept
        : data_(other.data_), size_(other.size_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    DataBlob &operator=(DataBlob &&other) noexcept
    {
        if (this != &other) {
            SDL_free(data_);
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    [[nodiscard]] const Uint8 *data() const noexcept
    {
        return data_;
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return !data_ || size_ == 0;
    }

    [[nodiscard]] explicit operator bool() const noexcept
    {
        return data_ != nullptr;
    }

    [[nodiscard]] std::span<const Uint8> span() const noexcept
    {
        return { data_, size_ };
    }

  private:
    Uint8 *data_ = nullptr;
    size_t size_ = 0;
};

} // namespace codotaku

#endif
