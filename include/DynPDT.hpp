#ifndef DYNPDT_DYNPDT_HPP
#define DYNPDT_DYNPDT_HPP

#include "SimpleBonsai.hpp"
#include "LabelPool_Plain.hpp"
#include "LabelPool_BitMap.hpp"

namespace dynpdt {

struct Setting {
  uint64_t num_keys = 0;
  double load_factor = 0.0;
  uint64_t fixed_len = 0; // of node labels
  uint8_t width_1st = 0;

  void show_stat(std::ostream& os) const {
    using std::endl;
    os << "Show statistics of Setting" << endl;
    os << " - num_keys:\t" << num_keys << endl;
    os << " - load_factor:\t" << load_factor << endl;
    os << " - fixed_len:\t" << fixed_len << endl;
    os << " - width_1st:\t" << static_cast<uint32_t>(width_1st) << endl;
  }
};

template<typename _LabelPoolType>
class DynPDT {
public:
  using LabelPoolType = _LabelPoolType;
  using ValueType = typename _LabelPoolType::ValueType;

  static constexpr uint8_t kAdjustAlphabet = 3; // heuristic
  static constexpr uint8_t kLabelMax = UINT8_MAX - kAdjustAlphabet;
  static constexpr uint64_t kStepSymbol = UINT8_MAX; // <UINT8_MAX, 0>

  static std::string name() {
    std::ostringstream oss;
    oss << "DynPDT_" << LabelPoolType::name().substr(std::strlen("LabelPool_"));
    return oss.str();
  }

  DynPDT(Setting setting) : setting_(setting) {
    if (!is_power2(setting_.fixed_len)) {
      std::cerr << "ERROR: fixed_len must be a power of 2." << std::endl;
      exit(1);
    }

    trie_ = std::make_unique<SimpleBonsai>(setting_.num_keys / setting_.load_factor,
                                           (setting_.fixed_len << 8) - kAdjustAlphabet,
                                           setting_.width_1st);
    label_pool_ = std::make_unique<LabelPoolType>(trie_->num_slots());
    table_.fill(UINT8_MAX);
  }

  ~DynPDT() {}

  const ValueType* find(const std::string& key) const {
    return find_(key);
  }

  ValueType* update(const std::string& key) {
    return update_(key);
  }

  uint64_t num_keys() const {
    return num_keys_;
  }
  uint64_t num_steps() const {
    return num_steps_;
  }
  uint8_t num_chars() const {
    return num_chars_;
  }

  const SimpleBonsai* get_trie() const {
    return trie_.get();
  }

  void show_stat(std::ostream& os) const {
    using std::endl;
    setting_.show_stat(os);
    os << "Show statistics of " << name() << endl;
    os << " - num_keys:\t" << num_keys() << endl;
    os << " - num_steps:\t" << num_steps() << endl;
    os << " - num_chars:\t" << static_cast<uint32_t>(num_chars()) << endl;
    trie_->show_stat(os);
    label_pool_->show_stat(os);
  }

  DynPDT(const DynPDT&) = delete;
  DynPDT& operator=(const DynPDT&) = delete;

private:
  const Setting setting_;
  uint64_t num_keys_ = 0;
  uint64_t num_steps_ = 0;

  // code table to avoid overflow
  std::array<uint8_t, 256> table_;
  uint8_t num_chars_ = 0;

  std::unique_ptr<SimpleBonsai> trie_;
  std::unique_ptr<LabelPoolType> label_pool_;

  const ValueType* find_(CharRange key) const {
    assert(key.begin != key.end);

    auto node_id = trie_->get_root();

    while (key.begin != key.end) {
      uint64_t num_match = 0;
      auto value_ptr = label_pool_->compare_and_get(node_id, key, num_match);

      if (value_ptr) {
        return value_ptr;
      }

      key.begin += num_match;

      // Follow step nodes
      while (setting_.fixed_len <= num_match) {
        if (!trie_->get_child(node_id, kStepSymbol)) {
          return nullptr;
        }
        num_match -= setting_.fixed_len;
      }

      if (table_[*key.begin] == UINT8_MAX) {
        // Useless character
        return nullptr;
      }

      if (!trie_->get_child(node_id, make_symbol_(*key.begin++, num_match))) {
        return nullptr;
      }
    }

    return label_pool_->compare_and_get(node_id, key);
  }

  ValueType* update_(CharRange key) {
    assert(key.begin != key.end);

    auto node_id = trie_->get_root();

    if (num_keys_ == 0) {
      // First insert
      ++num_keys_;
      return label_pool_->append(node_id, key);
    }

    while (key.begin != key.end) {
      uint64_t num_match = 0;
      auto value_ptr = label_pool_->compare_and_get(node_id, key, num_match);

      if (value_ptr) {
        return value_ptr;
      }

      key.begin += num_match;

      while (setting_.fixed_len <= num_match) {
        if (trie_->add_child(node_id, kStepSymbol)) {
          ++num_steps_;
        }
        num_match -= setting_.fixed_len;
      }

      if (table_[*key.begin] == UINT8_MAX) {
        // Update table
        table_[*key.begin] = num_chars_++;
        if (kLabelMax < num_chars_) {
          std::cerr << "ERROR: kLabelMax < alphabet_count_" << std::endl;
          exit(1);
        }
      }

      if (trie_->add_child(node_id, make_symbol_(*key.begin++, num_match))) {
        ++num_keys_;
        return label_pool_->append(node_id, key);
      }
    }

    auto value_ptr = label_pool_->compare_and_get(node_id, key);
    if (value_ptr) {
      return value_ptr;
    }

    ++num_keys_;
    return label_pool_->append(node_id, key);
  }

  uint64_t make_symbol_(uint8_t label, uint64_t offset) const {
    const auto symbol = static_cast<uint64_t>(table_[label]) | (offset << 8);
    assert(symbol != kStepSymbol);
    return symbol;
  }
};

} // namespace - dynpdt

#endif // DYNPDT_DYNPDT_HPP
