#ifndef DYN_PDT_BITMANAGER_HPP
#define DYN_PDT_BITMANAGER_HPP

#include <popcntintrin.h>

#include "basics.hpp"
#include "config.hpp"

namespace dyn_pdt {

namespace bit_tools {

class PopcountTable16Builder {
public:
  constexpr PopcountTable16Builder() : table_() {
    for (int i = 0; i < 256; i++) {
      table_[i] = popcount_(static_cast<uint8_t>(i));
    }
    for (int i = 1; i < 256; i++) {
      for (int j = 0; j < 256; j++) {
        table_[i * 256 + j] = table_[i] + table_[j];
      }
    }
  }
  constexpr uint8_t operator[](uint16_t x) const {
    return table_[x];
  }
private:
  uint8_t table_[65536];

  constexpr uint8_t popcount_(uint8_t x) {
    uint8_t c = 0;
    while (x) {
      if (x & 1U) ++c;
      x >>= 1;
    }
    return c;
  }
};

constexpr auto kPopcountTable16 = PopcountTable16Builder();

/*
 * Gets a bit
 * */
inline bool get_bit(uint8_t x, uint64_t i) {
  assert(i < 8);
  return (x & (1U << i)) != 0;
}
inline bool get_bit(uint16_t x, uint64_t i) {
  assert(i < 16);
  return (x & (1U << i)) != 0;
}
inline bool get_bit(uint32_t x, uint64_t i) {
  assert(i < 32);
  return (x & (1U << i)) != 0;
}
inline bool get_bit(uint64_t x, uint64_t i) {
  assert(i < 64);
  return (x & (1ULL << i)) != 0;
}

/*
 * Sets a bit
 * */
inline void set_bit(uint8_t& x, uint64_t i) {
  assert(i < 8);
  x |= (1U << i);
}
inline void set_bit(uint16_t& x, uint64_t i) {
  assert(i < 16);
  x |= (1U << i);
}
inline void set_bit(uint32_t& x, uint64_t i) {
  assert(i < 32);
  x |= (1U << i);
}
inline void set_bit(uint64_t& x, uint64_t i) {
  assert(i < 64);
  x |= (1ULL << i);
}

/*
 * Popcount
 * */
inline uint64_t popcount(uint8_t x) {
  return kPopcountTable16[x];
}
inline uint64_t popcount(uint16_t x) {
  return kPopcountTable16[x];
}
inline uint64_t popcount(uint32_t x) {
#ifdef DYNPDT_USE_POPCNT
  return _mm_popcnt_u32(x);
#else
  return kPopcountTable16[x & UINT16_MAX] + kPopcountTable16[(x >> 16) & UINT16_MAX];
#endif
}
inline uint64_t popcount(uint64_t x) {
#ifdef DYNPDT_USE_POPCNT
  return _mm_popcnt_u64(x);
#else
  return kPopcountTable16[x & UINT16_MAX] + kPopcountTable16[(x >> 16) & UINT16_MAX]
         + kPopcountTable16[(x >> 32) & UINT16_MAX] + kPopcountTable16[(x >> 48) & UINT16_MAX];
#endif
}

/*
 * Masked Popcount
 * */
inline uint64_t popcount(uint8_t x, uint64_t i) {
  assert(i < 8);
  return kPopcountTable16[x & ((1U << i) - 1)];
}
inline uint64_t popcount(uint16_t x, uint64_t i) {
  assert(i < 16);
  return kPopcountTable16[x & ((1U << i) - 1)];
}
inline uint64_t popcount(uint32_t x, uint64_t i) {
  assert(i < 32);
  uint32_t masked = x & ((1U << i) - 1);
  return popcount(masked);
}
inline uint64_t popcount(uint64_t x, uint64_t i) {
  assert(i < 64);
  uint64_t masked = x & ((1ULL << i) - 1);
  return popcount(masked);
}

}

}

#endif //DYN_PDT_BITMANAGER_HPP
