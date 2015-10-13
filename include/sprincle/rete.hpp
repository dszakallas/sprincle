//
// Created by david on 10/8/15.
//

#ifndef SPRINCLE_RETE_HPP
#define SPRINCLE_RETE_HPP

#include <vector>
#include <set>
#include <tuple>
#include <array>
#include <utility>
#include <algorithm>

#include <caf/all.hpp>

#include "detail.hpp"

using namespace std;
using namespace caf;
using namespace sprincle::detail;

namespace sprincle {

  template<typename... Types>
  struct changeset {
    set<tuple<Types...>> positive;
    set<tuple<Types...>> negative;

    changeset(): positive(), negative() {};

    changeset(changeset&& o):
      positive(forward<decltype(o.positive)>(o.positive)),
      negative(forward<decltype(o.positive)>(o.negative))
    {}

    changeset(const changeset& o):
      positive(o.positive),
      negative(o.positive)
    {}


    changeset(set<tuple<Types...>>&& p, set<tuple<Types...>>&& n) :
      positive(forward<decltype(p)>(p)),
      negative(forward<decltype(p)>(n))
    {}


  };

  template<typename... Types, size_t... I>
  constexpr decltype(auto) trimmer_detail(tuple<Types...>, index_sequence<I...>) {

    return [] (event_based_actor* self) {
      return behavior {
        [=](const changeset<Types...>& changes) {

          using ProjectedTupleType = decltype(project<I...>(declval<tuple<Types...>>()));

          set<ProjectedTupleType> positives;

          for(auto&& positive : changes.positive) {
            positives.insert(project<I...>(positive));
          }

          set<ProjectedTupleType> negatives;

          for(auto&& negative : changes.negative) {
            negatives.insert(project<I...>(negative));
          }

          return changeset<typename tuple_element<I, tuple<Types...>>::type...>(move(negatives), move(positives));


        }
      };
    };
  };

  template<typename Types, typename I>
  constexpr decltype(auto) trimmer() {
    return trimmer_detail(Types(), I());
  };




}




#endif //SPRINCLE_RETE_HPP
