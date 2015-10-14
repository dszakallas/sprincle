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

  template<typename Tuple>
  struct changeset {

    using coll_type = vector<Tuple>;

    coll_type positive;
    coll_type negative;

    changeset(): positive(), negative() {};

    changeset(changeset&& o):
      positive(forward<decltype(o.positive)>(o.positive)),
      negative(forward<decltype(o.negative)>(o.negative))
    {}

    changeset(const changeset& o):
      positive(o.positive),
      negative(o.negative)
    {}

    changeset(coll_type&& p, coll_type&& n) :
      positive(forward<decltype(p)>(p)),
      negative(forward<decltype(p)>(n))
    {}

    changeset(const coll_type& p, const coll_type& n) :
      positive(p),
      negative(n)
    {}

  };

  template<typename Tuple, size_t... I>
  struct trimmer {
    static decltype(auto) behavior(event_based_actor* self) {
      return caf::behavior {
        [=](const changeset<Tuple> &changes) {

          auto insert = [](auto&& to, const auto& from) {
            auto i = 0;
            for (auto&& tuple : from) {
              to[i] = project<I...>(tuple);
              ++i;
            }
          };

          using projected_type = decltype(project<I...>(declval<Tuple>()));

          typename changeset<projected_type>::coll_type positives(changes.positive.size());
          typename changeset<projected_type>::coll_type negatives(changes.negative.size());

          insert(positives, changes.positive);
          insert(negatives, changes.negative);

          return changeset<tuple<typename tuple_element<I, Tuple>::type...>>(move(positives), move(negatives));
        },
        others >> [=] {
          cerr << "unexpected: " << to_string(self->current_message()) << endl;
        }
      };

    }

  };

}




#endif //SPRINCLE_RETE_HPP
