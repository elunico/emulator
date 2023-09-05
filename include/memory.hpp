#ifndef MEMORY_H
#define MEMORY_H

#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "backwards.hpp"
#include "bytedefs.hpp"

namespace emulator {

template <std::integral WordSize,
          u64 ByteCount,
          std::integral BusSize = WordSize>
struct page {
  u64 size = ByteCount;

  WordSize* bank;
  explicit page() { bank = new WordSize[ByteCount]; }
  ~page() { delete[] bank; }
  [[maybe_unused]] void initialize() {
    for (int i = 0; i < ByteCount; i++)
      bank[i] = static_cast<WordSize>(0);
  }
  WordSize& operator[](BusSize addr) {
    // *metaout << "Getting memory at " << addr << std::endl;
    check_addr(addr);
    return bank[addr];
  }

  WordSize const& at(BusSize addr) const {
    check_addr(addr);
    return bank[addr];
  }

#ifdef NO_BOUNDS_CHECK_MEM
  inline void check_addr(BusSize addr) const noexcept {}
#else
  void check_addr(BusSize addr) const {
    if (addr >= ByteCount)
      throw std::out_of_range("Address of memory is out of range");
  }
#endif
};

template <std::integral WordSize,
          u64 PageCount,
          u64 PageSize = 4096,
          std::integral BusSize = WordSize>
struct memory {
  u64 pageCount = PageCount;
  u64 pageSize = PageSize;

#ifdef UNSAFE_READ
  mutable
#endif
      std::unordered_map<std::size_t, page<WordSize, PageSize, BusSize>>
          pages;

  static auto get_location(BusSize addr)
      -> std::pair<std::size_t, std::size_t> {
    if (PageSize == 0) {
      return std::make_pair(0, 0);
    }
    auto page = (addr / PageSize);
    auto offset = addr % PageSize;
    return std::make_pair(page, offset);
  }

  WordSize& operator[](BusSize addr) {
    // *metaout << "Getting memory at " << addr << std::endl;
    check_addr(addr);
    auto [page, offset] = get_location(addr);
    allocated[page] = true;
    return pages[page][offset];
  }

#ifdef NO_BOUNDS_CHECK_MEM
  inline void check_addr(BusSize addr) const noexcept {}
#else
  void check_addr(BusSize addr) const {
    auto [p, o] = get_location(addr);

    if (p >= PageCount || o >= PageSize)
      throw std::out_of_range("Address of memory is out of range");
  }
#endif

  WordSize const& operator[](BusSize addr) const {
    check_addr(addr);
    auto [page, offset] = get_location(addr);
#ifdef UNSAFE_READ
    return pages[page][offset];
#else
    if (!allocated[page])
      throw std::runtime_error("readonly access of uninitialized memory");
    return pages.at(page).at(offset);
#endif
  }

 private:
  mutable std::unordered_map<std::size_t, bool> allocated;
};
}  // namespace emulator

#endif
