#ifndef DYNPDT_FIT_VECTOR_HPP
#define DYNPDT_FIT_VECTOR_HPP

#include "basics.hpp"

namespace dynpdt {

class FitVector {
public:
  static constexpr uint64_t kChunkWidth = 64;

  FitVector(uint64_t length, uint8_t width) {
    if (width == 0 || 64 < width) {
      std::cerr << "ERROR: not 0 < width <= 64" << std::endl;
      exit(1);
    }

    length_ = length;
    width_ = width;
    mask_ = (1U << width) - 1;
    chunks_.resize(length_ * width_ / kChunkWidth + 1);
  }

  FitVector(uint64_t length, uint8_t width, uint64_t init) : FitVector(length, width) {
    for (uint64_t i = 0; i < length; ++i) {
      set(i, init);
    }
  }

  ~FitVector() {}

  uint64_t get(uint64_t i) const {
    const auto chunk_pos = i * width_ / kChunkWidth;
    const auto offset = i * width_ % kChunkWidth;
    if (offset + width_ <= kChunkWidth) {
      return (chunks_[chunk_pos] >> offset) & mask_;
    } else {
      return ((chunks_[chunk_pos] >> offset)
              | (chunks_[chunk_pos + 1] << (kChunkWidth - offset))) & mask_;
    }
  }

  void set(uint64_t i, uint64_t val) {
    const auto chunk_pos = i * width_ / kChunkWidth;
    const auto offset = i * width_ % kChunkWidth;
    chunks_[chunk_pos] &= ~(mask_ << offset);
    chunks_[chunk_pos] |= (val & mask_) << offset;
    if (kChunkWidth < offset + width_) {
      chunks_[chunk_pos + 1] &= ~(mask_ >> (kChunkWidth - offset));
      chunks_[chunk_pos + 1] |= (val & mask_) >> (kChunkWidth - offset);
    }
  }

  uint64_t length() const {
    return length_;
  }

  uint8_t width() const {
    return width_;
  }

  // returns output size
  uint64_t size_in_bytes() const {
    uint64_t ret = 0;
    ret += chunks_.size() * sizeof(uint64_t) + sizeof(chunks_.size());
    ret += sizeof(length_);
    ret += sizeof(width_);
    ret += sizeof(mask_);
    return ret;
  }

  FitVector(const FitVector&) = delete;
  FitVector& operator=(const FitVector&) = delete;

private:
  std::vector<uint64_t> chunks_;
  uint64_t length_ = 0;
  uint8_t width_ = 0;
  uint64_t mask_ = 0;
};

} // namespace - dynpdt

#endif // DYNPDT_FIT_VECTOR_HPP
