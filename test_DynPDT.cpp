#undef NDEBUG

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <cstring>
#include <fstream>

#include <DynPDT.hpp>

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

template <typename LabelPoolType>
void test(const std::vector<std::string>& keys, const std::vector<std::string>& others) {
  std::cerr << "TEST: " << DynPDT<LabelPoolType>::name() << std::endl;

  Setting setting;
  setting.num_keys = keys.size();
  setting.load_factor = 0.8;
  setting.fixed_len = 32;
  setting.width_1st = 6;

  DynPDT<LabelPoolType> dic(setting);

  for (size_t i = 0; i < keys.size(); ++i) {
    auto ptr = dic.update(keys[i]);
    assert(*ptr == 0);
    *ptr = i + 1;
  }
  assert(dic.num_keys() == keys.size());

  for (size_t i = 0; i < keys.size(); ++i) {
    auto ptr = dic.update(keys[i]);
    assert(ptr);
    assert(*ptr == i + 1);
  }

  for (size_t i = 0; i < keys.size(); ++i) {
    auto ptr = dic.find(keys[i]);
    assert(ptr);
    assert(*ptr == i + 1);
  }

  for (size_t i = 0; i < others.size(); ++i) {
    auto ptr = dic.find(others[i]);
    assert(!ptr);
  }
}

}

int main() {
  const size_t num_keys = 1U << 10;

  std::vector<std::string> keys(num_keys);
  for (size_t i = 0; i < num_keys; ++i) {
    keys[i] = make_key();
  }
  std::sort(std::begin(keys), std::end(keys));
  keys.erase(std::unique(std::begin(keys), std::end(keys)), std::end(keys));

  std::vector<std::string> others;
  others.reserve(num_keys);
  for (size_t i = 0; i < num_keys; ++i) {
    auto key = make_key();
    if (!std::binary_search(std::begin(keys), std::end(keys), key)) {
      others.push_back(key);
    }
  }

  std::shuffle(std::begin(keys), std::end(keys), std::mt19937());

  test<LabelPool_Plain<size_t>>(keys, others);
  test<LabelPool_BitMap<size_t, 0>>(keys, others);
  test<LabelPool_BitMap<size_t, 1>>(keys, others);
  test<LabelPool_BitMap<size_t, 2>>(keys, others);
  test<LabelPool_BitMap<size_t, 3>>(keys, others);

  return 0;
}
