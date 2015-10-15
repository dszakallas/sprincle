//
// Created by david on 10/8/15.
//

#ifndef SPRINCLE_RETE_HPP
#define SPRINCLE_RETE_HPP

#include <vector>
#include <tuple>
#include <array>
#include <utility>
#include <algorithm>
#include <iostream>

#include <caf/all.hpp>

#include "detail.hpp"

using namespace std;
using namespace caf;
using namespace sprincle::detail;

namespace sprincle {

  template<typename tuple_t>
  struct changeset {

    using changeset_t = vector<tuple_t>;

    changeset_t positive;
    changeset_t negative;

    changeset(): positive(), negative() {};

    changeset(changeset&& o):
      positive(forward<decltype(o.positive)>(o.positive)),
      negative(forward<decltype(o.negative)>(o.negative))
    {}

    changeset(const changeset& o):
      positive(o.positive),
      negative(o.negative)
    {}

    changeset(changeset_t&& p, changeset_t&& n) :
      positive(forward<decltype(p)>(p)),
      negative(forward<decltype(p)>(n))
    {}

    changeset(const changeset_t& p, const changeset_t& n) :
      positive(p),
      negative(n)
    {}

  };

  template<typename tuple_t, size_t... I>
  struct trimmer {
    static decltype(auto) behavior(event_based_actor* self) {
      return caf::behavior {
        [=](const changeset<tuple_t> &changes) {

          auto insert = [](auto&& to, const auto& from) {
            auto i = 0;
            for (auto&& tuple : from) {
              to[i] = project<I...>(tuple);
              ++i;
            }
          };

          using projected_t = decltype(project<I...>(declval<tuple_t>()));
          using projected_changeset_t = typename changeset<projected_t>::changeset_t;

          projected_changeset_t positives(changes.positive.size());
          projected_changeset_t negatives(changes.negative.size());

          insert(positives, changes.positive);
          insert(negatives, changes.negative);

          return changeset<tuple<typename tuple_element<I, tuple_t>::type...>>(move(positives), move(negatives));
        },
        others >> [=] {
          //TODO: Print error
        }
      };

    }

  };

  template<typename tuple_t>
  struct checker {
    static decltype(auto) behavior() {
      return caf::behavior {
        [=](const tuple_t& changes) {

        }
      };
    };

  };



}




#endif //SPRINCLE_RETE_HPP
