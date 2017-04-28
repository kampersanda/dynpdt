//
// Created by Kampersanda on 2017/02/21.
//

#ifndef DYN_PDT_BONSAI_HPP
#define DYN_PDT_BONSAI_HPP

#include "basics.hpp"
#include "FitVector.hpp"

namespace dyn_pdt {

/*
 * Very simple implementation of m-Bonsai (recursive) described in
 *  - Poyias and Raman, Improved practical compact dynamic tries, SPIRE, 2015.
 * */
class BonsaiPR {
public:
  static std::string name() {
    return "BonsaiPR";
  }

  BonsaiPR(uint64_t num_slots, uint64_t alphabet_size, uint8_t width_1st) {
    num_nodes_ = 1; // root
    num_slots_ = num_slots;
    alphabet_size_ = alphabet_size;
    width_1st_ = width_1st;

    root_id_ = num_slots_ / 2;

    empty_mark_ = alphabet_size + 2; // the maximum quotient value expected + 1
    max_dsp1st_ = (1U << width_1st) - 1;

    prime_ = greater_prime(alphabet_size * num_slots + num_slots - 1);
    multiplier_ = UINT64_MAX / prime_; // TODO: support invertible

    if (num_bits(alphabet_size - 1) < num_bits(empty_mark_)) {
      std::cerr << "#bits required for alphabet_size < #bits allocated practically" << std::endl;
      std::cerr << "The former is " << uint32_t(num_bits(alphabet_size - 1)) << std::endl;
      std::cerr << "The latter is " << uint32_t(num_bits(empty_mark_)) << std::endl;
    }

    slots_ = std::make_unique<FitVector>(num_slots, num_bits(empty_mark_) + width_1st,
                                         empty_mark_ << width_1st);
  }

  ~BonsaiPR() {}

  uint64_t get_root() const {
    return root_id_;
  }

  bool get_child(uint64_t& node_id, uint64_t symbol) const {
    if (alphabet_size_ <= symbol) {
      std::cerr << "ERROR: out-of-range symbol in get_child()" << std::endl;
      exit(1);
    }

    const auto hv = hash_(node_id, symbol);
    if (empty_mark_ <= hv.quo) {
      std::cerr << "ERROR: out-of-range hv.quo in get_child()" << std::endl;
      exit(1);
    }

    for (uint64_t pos = hv.rem, cnt = 0;; pos = next_(pos), ++cnt) {
      if (pos == root_id_) {
        continue;
      }
      const auto quo = get_quo_(pos);
      if (quo == empty_mark_) {
        return false;
      }
      if (quo == hv.quo && get_dsp_(pos) == cnt) { // already registered?
        node_id = pos;
        return true;
      }
    }
  }

  bool add_child(uint64_t& node_id, uint64_t symbol) {
    if (alphabet_size_ <= symbol) {
      std::cerr << "ERROR: out-of-range symbol in add_child()" << std::endl;
      exit(1);
    }

    const auto hv = hash_(node_id, symbol);
    if (empty_mark_ <= hv.quo) {
      std::cerr << "ERROR: out-of-range hv.quo in add_child()" << std::endl;
      exit(1);
    }

    for (uint64_t pos = hv.rem, cnt = 0;; pos = next_(pos), ++cnt) {
      if (pos == root_id_) {
        continue;
      }
      const uint64_t quo = get_quo_(pos);
      if (quo == empty_mark_) {
        update_slot_(pos, hv.quo, cnt);
        node_id = pos;
        ++num_nodes_;
        if (num_nodes_ == num_slots_) {
          std::cerr << "ERROR: num_nodes reaches a limit" << std::endl;
          exit(1);
        }
        return true;
      }
      if (quo == hv.quo && get_dsp_(pos) == cnt) { // already registered?
        node_id = pos;
        return false;
      }
    }
  }

  uint64_t num_slots() const {
    return num_slots_;
  }
  uint64_t num_nodes() const {
    return num_nodes_;
  }

  double average_dsp() const {
    uint64_t num_used_slots = 0, sum_dsp = 0;
    for (uint64_t i = 0; i < num_slots_; ++i) {
      if (get_quo_(i) != empty_mark_) {
        ++num_used_slots;
        sum_dsp += get_dsp_(i);
      }
    }
    return double(sum_dsp) / num_used_slots;
  }

  void show_stat(std::ostream& os) const {
    using std::endl;
    os << "Show statistics of " << name() << endl;
    os << " - num_nodes:\t" << num_nodes() << endl;
    os << " - num_slots:\t" << num_slots() << endl;
    os << " - num_auxs:\t" << aux_map_.size() << endl;
    os << " - load_factor:\t" << static_cast<double>(num_nodes_) / num_slots() << endl;
    os << " - slot_width:\t" << static_cast<uint32_t>(slots_->width()) << endl;
    os << " - slot_memory:\t" << slots_->size_in_bytes() << endl;
    os << " - aux_memory:\t" << estimate_map_memory(aux_map_) << endl;
    os << " - average_dsp:\t" << average_dsp() << endl;
  }

  BonsaiPR(const BonsaiPR&) = delete;
  BonsaiPR& operator=(const BonsaiPR&) = delete;

private:
  struct HashValue {
    uint64_t rem;
    uint64_t quo;
  };

  uint64_t num_nodes_;
  uint64_t num_slots_;
  uint64_t alphabet_size_;
  uint8_t width_1st_;

  uint64_t root_id_;
  uint64_t empty_mark_;
  uint64_t max_dsp1st_; // maximum displacement value in the 1st layer

  uint64_t prime_;
  uint64_t multiplier_;

  std::unique_ptr<FitVector> slots_;
  std::map<uint64_t, uint32_t> aux_map_; // for exceeding displacement values

  // Expecting 0 <= quo <= alp_size + 1
  HashValue hash_(uint64_t node_id, uint64_t symbol) const {
    uint64_t c = symbol * num_slots_ + node_id;
    uint64_t c_rnd = ((c % prime_) * multiplier_) % prime_;
    return {c_rnd % num_slots_, c_rnd / num_slots_};
  }

  uint64_t next_(uint64_t pos) const {
    return ++pos >= num_slots_ ? 0 : pos;
  }

  uint64_t get_quo_(uint64_t pos) const {
    return slots_->get(pos) >> width_1st_;
  }

  uint64_t get_dsp_(uint64_t pos) const {
    const auto dsp = slots_->get(pos) & max_dsp1st_;
    if (dsp < max_dsp1st_) {
      return dsp;
    }
    auto it = aux_map_.find(pos);
    return it == aux_map_.end() ? UINT64_MAX : it->second;
  }

  void update_slot_(uint64_t pos, uint64_t quo, uint64_t dsp) {
    auto val = quo << width_1st_;
    if (dsp < max_dsp1st_) {
      val |= dsp;
    } else {
      val |= max_dsp1st_;
      assert(aux_map_.find(pos) == aux_map_.end());
      aux_map_.insert({pos, dsp});
    }
    slots_->set(pos, val);
  }
};

}

#endif //DYN_PDT_BONSAI_HPP
