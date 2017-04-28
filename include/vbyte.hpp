//
// Created by Kampersanda on 2017/04/07.
//

#ifndef DYN_PDT_VBYTE_HPP
#define DYN_PDT_VBYTE_HPP

#include "basics.hpp"

namespace dyn_pdt {

namespace vbyte {

inline uint64_t size(uint64_t val) {
  uint64_t n = 1;
  while (127 < val) {
    ++n;
    val >>= 7;
  }
  return n;
}

inline uint64_t encode(uint8_t* codes, uint64_t val) {
  uint64_t i = 0;
  while (127 < val) {
    codes[i++] = static_cast<uint8_t>((val & 127ULL) | 0x80ULL);
    val >>= 7;
  }
  codes[i++] = static_cast<uint8_t>(val & 127ULL);
  return i;
}

inline uint64_t decode(const uint8_t* codes, uint64_t& val) {
  val = 0;
  uint64_t i = 0, shift = 0;
  while (codes[i] & 0x80ULL) {
    val |= (codes[i++] & 127ULL) << shift;
    shift += 7;
  }
  val |= (codes[i++] & 127ULL) << shift;
  return i;
}

}

}

#endif //DYN_PDT_VBYTE_HPP
