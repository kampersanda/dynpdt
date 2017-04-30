#ifndef DYNPDT_LABEL_POOL_PLAIN_HPP
#define DYNPDT_LABEL_POOL_PLAIN_HPP

#include "basics.hpp"

namespace dynpdt {

template <typename _ValueType>
class LabelPool_Plain {
public:
  using ValueType = _ValueType;

  static std::string name() {
    return "LabelPool_Plain";
  }

  LabelPool_Plain(uint64_t size) {
    pools_.resize(size);
  }

  ~LabelPool_Plain() {}

  ValueType* compare_and_get(uint64_t id, CharRange label) {
    uint64_t dummy;
    return compare_and_get(id, label, dummy);
  }

  ValueType* compare_and_get(uint64_t id, CharRange label, uint64_t& num_match) {
    num_match = 0;

    auto ptr = pools_[id].get();
    if (!ptr) {
      return nullptr;
    }

    if (label.begin == label.end) {
      return reinterpret_cast<ValueType*>(ptr);
    }

    while (label.begin + num_match != label.end) {
      if (label.begin[num_match] != ptr[num_match]) {
        return nullptr;
      }
      ++num_match;
    }

    return reinterpret_cast<ValueType*>(ptr + num_match);
  }

  ValueType* append(uint64_t id, CharRange label) {
    if (pools_[id]) {
      std::cerr << "ERROR: already exist" << std::endl;
      exit(1);
    }

    ++num_labels_;

    const auto length = label.length();
    const auto new_alloc = length + sizeof(ValueType);
    sum_bytes_ += new_alloc;

    pools_[id] = make_char_array(new_alloc);
    auto ptr = pools_[id].get();

    std::memcpy(ptr, label.begin, length);
    ptr += length;

    std::memset(ptr, 0, sizeof(ValueType));
    return reinterpret_cast<ValueType*>(ptr);
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
    os << " - ave_length:\t" << static_cast<double>(sum_bytes()) / num_labels() << endl;
  }

  LabelPool_Plain(const LabelPool_Plain&) = delete;
  LabelPool_Plain& operator=(const LabelPool_Plain&) = delete;

private:
  std::vector<CharArray> pools_;
  uint64_t num_labels_ = 0;
  uint64_t sum_bytes_ = 0;
};

} // namespace - dynpdt

#endif // DYNPDT_LABEL_POOL_PLAIN_HPP
