//
// Created by Kampersanda on 2017/04/02.
//

#ifndef DYN_PDT_LABELPOOL_BITMAP_HPP
#define DYN_PDT_LABELPOOL_BITMAP_HPP

#include "basics.hpp"
#include "bit_tools.hpp"
#include "vbyte.hpp"

namespace dyn_pdt {

/*
 * Compact label management in a manner similar to sparsetable at
 *  - https://github.com/sparsehash/sparsehash.
 *
 * Accessing a label is supported in O(kGroupSize) time using the skipping approach described in
 *  - Askitis and Zobel, Cache-conscious collision resolution in string hash tables, SPIRE, 2005.
 * */
template<typename _ValueType, int GroupTypeId>
class LabelPool_BitMap {
public:
  using ValueType = _ValueType;

  using GroupTypes = std::tuple<uint8_t, uint16_t, uint32_t, uint64_t>;
  using GroupType = typename std::tuple_element<GroupTypeId, GroupTypes>::type;

  static constexpr uint64_t kGroupSize = sizeof(GroupType) * 8;

  static std::string name() {
    std::ostringstream oss;
    oss << "LabelPool_BitMap" << kGroupSize;
    return oss.str();
  }

  LabelPool_BitMap(uint64_t size) {
    pools_.resize(size / kGroupSize + 1);
    bitmap_.resize(size / kGroupSize + 1, 0);
  }

  ~LabelPool_BitMap() {}

  ValueType* compare_and_get(uint64_t id, CharRange label) {
    uint64_t dummy;
    return compare_and_get(id, label, dummy);
  }

  ValueType* compare_and_get(uint64_t id, CharRange label, uint64_t& num_match) {
    const auto group_id = id / kGroupSize;
    const auto offset = id % kGroupSize;

    if (!bit_tools::get_bit(bitmap_[group_id], offset)) {
      num_match = 0;
      return nullptr;
    }

    auto ptr = pools_[group_id].get();
    const auto loc = bit_tools::popcount(bitmap_[group_id], offset);

    uint64_t len = 0;
    for (uint64_t i = 0; i < loc; ++i) {
      ptr += vbyte::decode(ptr, len);
      ptr += len + sizeof(ValueType);
    }
    ptr += vbyte::decode(ptr, len);

    if (label.begin == label.end) {
      return reinterpret_cast<ValueType*>(ptr);
    }

    for (num_match = 0; num_match < len; ++num_match) {
      if (ptr[num_match] != label.begin[num_match]) {
        return nullptr;
      }
    }

    if (label.begin[num_match]) {
      return nullptr;
    }

    ++num_match;
    return reinterpret_cast<ValueType*>(ptr + len);
  }

  ValueType* append(uint64_t id, CharRange label) {
    const auto group_id = id / kGroupSize;
    const auto offset = id % kGroupSize;

    if (bit_tools::get_bit(bitmap_[group_id], offset)) {
      std::cerr << "ERROR: already exist" << std::endl;
      exit(1);
    }

    ++num_labels_;
    bit_tools::set_bit(bitmap_[group_id], offset);

    if (!pools_[group_id]) {
      const auto label_len = (label.begin == label.end) ? 0 : label.length() - 1;
      const auto new_alloc = vbyte::size(label_len) + label_len + sizeof(ValueType);
      sum_bytes_ += new_alloc;

      pools_[group_id] = make_char_array(new_alloc);
      auto ptr = pools_[group_id].get();

      ptr += vbyte::encode(ptr, label_len);
      std::memcpy(ptr, label.begin, label_len);
      ptr += label_len;
      std::memset(ptr, 0, sizeof(ValueType));

      return reinterpret_cast<ValueType*>(ptr);
    }

    uint64_t front_len = 0, back_len = 0;
    {
      const auto num_labels = bit_tools::popcount(bitmap_[group_id]) - 1;
      const auto loc = bit_tools::popcount(bitmap_[group_id], offset);
      auto ptr = pools_[group_id].get();

      for (uint64_t i = 0; i < num_labels; ++i) {
        uint64_t len = 0;
        len += vbyte::decode(ptr, len) + sizeof(ValueType);
        if (i < loc) {
          front_len += len;
        } else {
          back_len += len;
        }
        ptr += len;
      }
    }

    const auto label_len = (label.begin == label.end) ? 0 : label.length() - 1;
    const auto new_alloc = vbyte::size(label_len) + label_len + sizeof(ValueType);
    sum_bytes_ += new_alloc;

    auto new_pool = make_char_array(front_len + back_len + new_alloc);

    auto orig_ptr = pools_[group_id].get();
    auto new_ptr = new_pool.get();

    std::memcpy(new_ptr, orig_ptr, front_len);
    orig_ptr += front_len;
    new_ptr += front_len;

    new_ptr += vbyte::encode(new_ptr, label_len);
    std::memcpy(new_ptr, label.begin, label_len);
    new_ptr += label_len;

    std::memset(new_ptr, 0, sizeof(ValueType));
    auto ret = reinterpret_cast<ValueType*>(new_ptr);
    new_ptr += sizeof(ValueType);

    std::memcpy(new_ptr, orig_ptr, back_len);
    pools_[group_id] = std::move(new_pool);

    return ret;
  }

  uint64_t num_ptrs() const {
    return pools_.size();
  }

  uint64_t num_labels() const {
    return num_labels_;
  }

  uint64_t sum_bytes() const {
    return sum_bytes_;
  }

  void show_stat(std::ostream& os) const {
    using std::endl;
    os << "Show statistics of " << name() << endl;
    os << " - num_ptrs:\t" << num_ptrs() << endl;
    os << " - num_labels:\t" << num_labels() << endl;
    os << " - sum_bytes:\t" << sum_bytes() << endl;
    os << " - ave_length:\t" << static_cast<double>(sum_bytes()) / num_ptrs() << endl;
    os << " - rate_vbyte_counts:" << endl;

    auto vbyte_counts = count_vbytes_();
    for (size_t i = 0; i < vbyte_counts.size(); ++i) {
      if (vbyte_counts[i] == 0) {
        break;
      }
      os << "   - " << i + 1 << "B:\t"
         << static_cast<double>(vbyte_counts[i]) / num_labels() << endl;
    }
  }

  LabelPool_BitMap(const LabelPool_BitMap&) = delete;
  LabelPool_BitMap& operator=(const LabelPool_BitMap&) = delete;

private:
  std::vector<CharArray> pools_;
  std::vector<GroupType> bitmap_;
  uint64_t num_labels_ = 0;
  uint64_t sum_bytes_ = 0;

  std::array<uint64_t, 8> count_vbytes_() const {
    std::array<uint64_t, 8> counts;
    counts.fill(0);

    for (uint64_t group_id = 0; group_id < pools_.size(); ++group_id) {
      auto ptr = pools_[group_id].get();
      const auto num_labels = bit_tools::popcount(bitmap_[group_id]);

      for (uint64_t i = 0; i < num_labels; ++i) {
        uint64_t len = 0;
        const auto vbyte_size = vbyte::decode(ptr, len);
        ++counts[vbyte_size - 1];
        ptr += vbyte_size + len + sizeof(ValueType);
      }
    }
    return counts;
  };
};

}

#endif //DYN_PDT_LABELPOOL_BITMAP_HPP
