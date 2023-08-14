#ifndef MEMORY_H
#define MEMORY_H

#include <stdexcept>

#include "bytedefs.hpp"

namespace emulator {

template <typename WordSize, u64 N, typename BusSize = WordSize>
struct memory {
  WordSize* bank;

  explicit memory() { bank = new WordSize[N]; }

  ~memory() { delete[] bank; }

  [[maybe_unused]] void
  initialize() {
    for (int i = 0; i < N; i++) bank[i] = static_cast<WordSize>(0);
  }

  WordSize&
  operator[](BusSize addr) {
    // *metaout << "Getting memory at " << addr << std::endl;
    check_addr(addr);
    return bank[addr];
  }

  void
  check_addr(BusSize addr) const {
    if (addr >= N) throw std::out_of_range("Address of memory is out of range");
  }

  WordSize const&
  operator[](BusSize addr) const {
    check_addr(addr);
    return bank[addr];
  }
};
}  // namespace emulator

#endif
