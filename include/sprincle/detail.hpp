//
// Created by david on 10/13/15.
//

#ifndef SPRINCLE_DETAIL_H
#define SPRINCLE_DETAIL_H

#include <tuple>
#include <utility>

using namespace std;

namespace sprincle {

  namespace detail {

    //TODO: Generalize to universal reference?

    template<size_t... Indices, typename Tuple>
    decltype(auto) project(const Tuple& t) {
      return tuple<typename tuple_element<Indices, Tuple>::type...>(get<Indices>(t)...);
    }

  }
}

#endif //SPRINCLE_DETAIL_H
