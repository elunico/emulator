#ifndef BACKWARDS_HPP
#define BACKWARDS_HPP

#if __cplusplus <= 202002L
#define INTEGRAL std::integral

#include <string>
#include <string_view>

inline auto
string_section(std::string const& s, int start, int end) {
  return std::string_view{s.begin() + start, s.begin() + end};
}

#else
#define INTEGRAL typename

inline auto
string_section(std::string const& s, int start, int end) {
  return s.substr(start, end);
}

#endif

#endif
