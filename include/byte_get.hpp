#ifndef BYTE_GET_HPP
#define BYTE_GET_HPP

#include <cstdint>
#include <type_traits>

namespace emulator {

template <typename T>
struct ByteGettableSize {
  static const std::size_t size = sizeof(std::remove_reference_t<T>);
};

template <int index, typename T>
concept ByteShiftGetable =
    requires(T t) {
      { t >> t } noexcept -> std::same_as<std::decay_t<T>>;
      { t& t } noexcept -> std::same_as<std::decay_t<T>>;
      { t* t } noexcept -> std::same_as<std::decay_t<T>>;
      requires(std::is_convertible_v<int, std::decay_t<T>>);
      requires(index < ByteGettableSize<T>::size);
    };

template <int index, typename T>
  requires ByteShiftGetable<index, T>
constexpr auto
byte_of(T const& elt) {
  return (elt >> (static_cast<T>(index) * static_cast<T>(8))) &
         static_cast<T>(0xff);
}

}  // namespace emulator

#endif
