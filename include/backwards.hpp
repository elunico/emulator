#ifndef BACKWARDS_HPP
#define BACKWARDS_HPP


#include <string>
#include <string_view>

inline constexpr auto
string_section(std::string const& s, int start, int end) {
  return std::string_view{s.begin() + start, s.begin() + end};
}

#endif