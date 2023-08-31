#ifndef BYTE_GET_HPP
#define BYTE_GET_HPP

#if __cplusplus >= 202002L

#include <concepts>

namespace emulator {

template <typename T>
struct ByteGettableSize {
  static const std::size_t size = sizeof(std::remove_reference_t<T>);
};

template <int index, typename T>
concept ByteShiftGetable = requires(T t) {
  { t >> t } noexcept -> std::same_as<std::remove_reference_t<T>>;
  { t& t } noexcept -> std::same_as<std::remove_reference_t<T>>;
  { t* t } noexcept -> std::same_as<std::remove_reference_t<T>>;
  requires(std::is_convertible_v<int, std::remove_reference_t<T>>);
  requires(index < ByteGettableSize<T>::size);
};

}  // namespace emulator

#define BYTE_OF_RETURN_TYPE requires ByteShiftGetable<index, T> constexpr auto

#else

#define BYTE_OF_RETURN_TYPE \
  std::enable_if_t < index<ByteGettableSize<T>::size, T>

#endif

#include <type_traits>

namespace emulator {

template <int index, typename T>
BYTE_OF_RETURN_TYPE
byte_of(T const& elt) {
  return (elt >> (static_cast<T>(index) * static_cast<T>(8))) &
         static_cast<T>(0xff);
}

}  // namespace emulator

#endif
