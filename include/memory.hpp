#ifndef MEMORY_H
#define MEMORY_H

#include "bytedefs.hpp"
#include <stdexcept>

namespace emulator {

template <typename WordSize, u64 N, typename BusSize = WordSize> struct memory {
  WordSize bank[N];

  void initialize() {
    for (int i = 0; i < N; i++)
      bank[i] = static_cast<WordSize>(0);
  }

  WordSize &operator[](BusSize addr) {
    // *metaout << "Getting memory at " << addr << std::endl;
    check_addr(addr);
    return bank[addr];
  }

  void check_addr(BusSize addr) {
    if (addr >= N)
      throw std::invalid_argument("Address of memory is out of range");
  }

  WordSize const &operator[](BusSize addr) const {
    if (addr >= N)
      throw std::invalid_argument("Address of memory is out of range");
    return bank[addr];
  }
};
} // namespace emulator

#endif
