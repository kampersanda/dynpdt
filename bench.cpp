#include <chrono>
#include <cassert>
#include <fstream>
#include <iostream>

#include <DynPDT.hpp>

using namespace dynpdt;

namespace {

class StopWatch {
public:
  enum Times {
    SEC, MILLI, MICRO
  };

  StopWatch() : tp_(std::chrono::high_resolution_clock::now()) {}
  ~StopWatch() {}

  double operator()(Times type) const {
    auto tp = std::chrono::high_resolution_clock::now() - tp_;
    switch (type) {
      case Times::SEC:
        return std::chrono::duration<double>(tp).count();
      case Times::MILLI:
        return std::chrono::duration<double, std::milli>(tp).count();
      case Times::MICRO:
        return std::chrono::duration<double, std::micro>(tp).count();
    }
    return 0.0;
  }

  StopWatch(const StopWatch&) = delete;
  StopWatch& operator=(const StopWatch&) = delete;

private:
  std::chrono::high_resolution_clock::time_point tp_;
};

class KeyReader {
public:
  KeyReader(const char* file_name) {
    std::ios::sync_with_stdio(false);
    buf_.reserve(1 << 10);
    ifs_.open(file_name);
  }
  ~KeyReader() {}

  bool is_ready() const {
    return !ifs_.fail();
  }

  const std::string& next() {
    std::getline(ifs_, buf_);
    return buf_;
  }

  KeyReader(const KeyReader&) = delete;
  KeyReader& operator=(const KeyReader&) = delete;

private:
  std::ifstream ifs_;
  std::string buf_;
};

std::vector<std::string> read_keys(const char* file_name) {
  std::ifstream ifs(file_name);
  if (!ifs) {
    std::cerr << "ERROR : failed to open " << file_name << std::endl;
    return {};
  }

  std::string line;
  std::vector<std::string> keys;

  while (std::getline(ifs, line)) {
    if (line.empty()) {
      continue;
    }
    keys.push_back(line);
  }

  return keys;
}

template <typename LabelPoolType>
void run_insert(DynPDT<LabelPoolType>& dic, KeyReader& reader) {
  size_t num_keys = 0;
  const auto limit = static_cast<size_t>(dic.get_trie()->num_slots() * 0.98);
  StopWatch sw;

  while (true) {
    const auto& key = reader.next();
    if (key.empty()) {
      break;
    }
    *dic.update(key) = 1;
    ++num_keys;

    if (limit < dic.get_trie()->num_nodes()) {
      std::cerr << "*** Load Factor exceeds 0.98 ***" << std::endl;
      exit(1);
    }
  }

  auto us = sw(StopWatch::MICRO);

  std::cout << "Bench: run_insert" << std::endl;
  std::cout << " - num_keys:\t" << num_keys << std::endl;
  std::cout << " - insert time:\t" << us / num_keys << " us/key" << std::endl;
}

template <typename LabelPoolType>
void run_search(const DynPDT<LabelPoolType>& dic, const std::vector<std::string>& keys) {
  size_t ok = 0, ng = 0;
  StopWatch sw;

  for (const auto& key : keys) {
    auto ptr = dic.find(key);
    if (ptr != nullptr && *ptr == 1) {
      ++ok;
    } else {
      ++ng;
    }
  }

  auto us = sw(StopWatch::MICRO);

  std::cout << "Bench: run_search" << std::endl;
  std::cout << " - num_keys:\t" << keys.size() << std::endl;
  std::cout << " - ok:\t" << ok << std::endl;
  std::cout << " - ng:\t" << ng << std::endl;
  std::cout << " - search time:\t" << us / keys.size() << " us/key" << std::endl;
}

template <typename LabelPoolType>
int bench(const char* argv[]) {
  auto key_name = argv[2];
  auto query_name = argv[3];

  Setting setting;
  setting.num_keys = static_cast<uint64_t>(std::atoll(argv[4]));
  setting.load_factor = std::atof(argv[5]);
  setting.fixed_len = static_cast<uint64_t>(std::atoi(argv[6]));
  setting.width_1st = static_cast<uint8_t>(std::atoi(argv[7]));

  DynPDT<LabelPoolType> dic(setting);
  {
    KeyReader reader(key_name);
    if (!reader.is_ready()) {
      std::cerr << "ERROR: failed to open " << key_name << std::endl;
      return 1;
    }
    run_insert(dic, reader);
  }

  if (std::strcmp(query_name, "=") == 0) {
    query_name = key_name;
  }

  if (std::strcmp(query_name, "-") != 0) {
    auto keys = read_keys(query_name);
    run_search(dic, keys);
  }

  dic.show_stat(std::cout);
  return 0;
}

} // namespace

int main(int argc, const char* argv[]) {
  std::ostringstream usage;
  usage << argv[0] << " <dic_type> <key> <query> <#keys> <LF> <len> <w1>";

  if (argc != 8) {
    std::cerr << usage.str() << std::endl;
    return 1;
  }

  switch (*argv[1]) {
    case '1':
      return bench<LabelPool_Plain<int>>(argv);
    case '2':
      return bench<LabelPool_BitMap<int, 0>>(argv);
    case '3':
      return bench<LabelPool_BitMap<int, 1>>(argv);
    case '4':
      return bench<LabelPool_BitMap<int, 2>>(argv);
    case '5':
      return bench<LabelPool_BitMap<int, 3>>(argv);
    default:
      break;
  }

  std::cerr << usage.str() << std::endl;
  return 1;
}
