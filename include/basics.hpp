//
// Created by Kampersanda on 2017/02/21.
//

#ifndef DYN_PDT_BASICS_HPP
#define DYN_PDT_BASICS_HPP

#include <array>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <stdlib.h>
#include <stdint.h>
#include <cmath>
#include <memory>
#include <sstream>
#include <cstring>

namespace dyn_pdt {

using CharArray = std::unique_ptr<uint8_t[]>;

inline CharArray make_char_array(uint64_t length) {
  return CharArray(new uint8_t[length]);
}

struct CharRange {
  const uint8_t* begin = nullptr;
  const uint8_t* end = nullptr;

  CharRange() {}
  CharRange(const std::string& str)
    : begin(reinterpret_cast<const uint8_t*>(str.c_str())),
      end(begin + str.length() + 1) {}

  bool operator==(const CharRange& rhs) const {
    if (length() != rhs.length()) {
      return false;
    }
    return std::equal(begin, end, rhs.begin);
  }

  uint64_t length() const {
    return static_cast<uint64_t>(end - begin);
  }
};

inline bool is_power2(uint64_t n) {
  if (n == 0) {
    return false;
  }
  return !(n & (n - 1));
}

inline uint8_t num_bits(uint64_t n) {
  uint8_t ret = 0;
  do {
    n >>= 1;
    ++ret;
  } while (n);
  return ret;
}

inline bool is_prime(uint64_t n) {
  if (n == 2) {
    return true;
  }

  if (n % 2 == 0 || n <= 1) {
    return false;
  }

  const auto max = static_cast<uint64_t>(std::sqrt(n));
  for (uint64_t i = 3; i <= max; i += 2) {
    if (n % i == 0) {
      return false;
    }
  }

  return true;
}

inline uint64_t greater_prime(uint64_t n) {
  assert(n != 0);

  auto ret = n + 1;
  if (ret % 2 == 0 && ret != 2) {
    ret += 1;
  }
  while (!is_prime(ret)) {
    ret += 2;
  }
  return ret;
}

template<typename T, typename V>
inline uint64_t estimate_map_memory(const std::map<T, V>& map) {
  static_assert(std::is_pod<T>::value, "T must be POD");
  static_assert(std::is_pod<V>::value, "V must be POD");

  uint64_t ret = 0;
#if (defined(__GNUC__) || defined(__GNUG__)) && !(defined(__clang__) || defined(__INTEL_COMPILER))
  // http://d.hatena.ne.jp/ny23/20111129/p1
  ret = sizeof(std::_Rb_tree_node<typename std::map<T, V>::value_type>) * map.size();
#endif
  return ret;
};

}


#endif //DYN_PDT_BASICS_HPP
