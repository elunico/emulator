#ifndef BACKWARDS_HPP
#define BACKWARDS_HPP

#if __cplusplus <= 202002L
#define INTEGRAL std::integral
#else
#define INTEGRAL typename
#endif

#endif
