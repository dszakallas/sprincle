//
// Created by david on 1/23/16.
//

#ifndef SPRINCLE_MATCH_PAIR_HEADER
#define SPRINCLE_MATCH_PAIR_HEADER

#include <cstdlib>

using namespace std;

namespace sprincle {
  template<size_t Primary, size_t Secondary> struct match_pair {
    static constexpr size_t primary_value = Primary;
    static constexpr size_t secondary_value = Secondary;
  };
}

#endif //SPRINCLE_MATCH_PAIR_HEADER
