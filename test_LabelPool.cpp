#undef NDEBUG

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <cstring>
#include <fstream>

#include "include/LabelPool_BitMap.hpp"
#include "include/LabelPool_Plain.hpp"

using namespace dynpdt;

namespace {

std::string make_key(size_t max_length = 1000) {
  static std::random_device rnd;

  std::string key;
  size_t length = (rnd() % max_length);

  for (size_t i = 0; i < length; ++i) {
    key += 'A' + (rnd() % 26);
  }
  return key;
}

template <typename LablePoolType>
void test(const std::vector<CharRange>& ranges, std::vector<uint64_t>& ids) {
  std::cerr << "TEST: " << LablePoolType::name() << std::endl;

  LablePoolType pool(ranges.size());

  const auto size = static_cast<uint64_t>(ranges.size() * 0.8);
  for (size_t i = 0; i < size; ++i) {
    auto ptr = pool.append(ids[i], ranges[i]);
    *ptr = i;
  }

  for (size_t i = 0; i < size; ++i) {
    uint64_t num_match = 0;
    auto ptr = pool.compare_and_get(ids[i], ranges[i], num_match);
    assert(ptr);
    auto value = *ptr;
    assert(value == i);
    assert(ranges[i].length() == num_match);
  }
}

}

int main() {
  const size_t num_keys = 1U << 10;

  std::vector<std::string> keys(num_keys);
  for (size_t i = 0; i < num_keys; ++i) {
    keys[i] = make_key();
  }

  std::vector<CharRange> ranges;
  for (size_t i = 0; i < num_keys; ++i) {
    ranges.emplace_back(keys[i]);
    if ((i % 200) == 0) {
      // set empty string
      ranges[i].end = ranges[i].begin;
    }
  }

  std::vector<uint64_t> ids;
  for (uint64_t i = 0; i < keys.size(); ++i) {
    ids.push_back(i);
  }
  std::shuffle(ids.begin(), ids.end(), std::mt19937());

  test<LabelPool_Plain<size_t>>(ranges, ids);
  test<LabelPool_BitMap<size_t, 0>>(ranges, ids);
  test<LabelPool_BitMap<size_t, 1>>(ranges, ids);
  test<LabelPool_BitMap<size_t, 2>>(ranges, ids);
  test<LabelPool_BitMap<size_t, 3>>(ranges, ids);

  return 0;
}
