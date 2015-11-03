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

#include <boost/iterator/filter_iterator.hpp>
#include <boost/range.hpp>

#include <caf/all.hpp>

#include "detail.hpp"

using namespace std;
using namespace caf;

namespace sprincle {

  template<typename tuple_t>
  struct changeset {

    using change_t = tuple_t;
    using changeset_t = vector<change_t>;


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
      positive(forward<changeset_t>(p)),
      negative(forward<changeset_t>(n))
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
        [=](const changeset<tuple_t>& changes) {

          auto insert = [](auto& to, const auto& from) {
            auto i = 0;
            for (const auto& tuple : from) {
              to[i] = project<I...>(tuple);
              ++i;
            }
          };

          using projected_change_t = decltype(project<I...>(declval<typename changeset<tuple_t>::change_t>()));
          using projected_changeset_t = typename changeset<projected_change_t>::changeset_t;

          projected_changeset_t positives(changes.positive.size());
          projected_changeset_t negatives(changes.negative.size());

          insert(positives, changes.positive);
          insert(negatives, changes.negative);

          return changeset<projected_change_t>(move(positives), move(negatives));
        },
        others >> [=] {
          //TODO: Print error
        }
      };

    }

  };

  template<class tuple_t, class predicate_t>
  struct filter {
    static decltype(auto) behavior(event_based_actor* self) {
      return caf::behavior {
        [=](const changeset<tuple_t>& changes) {

          changeset<tuple_t> filtered;

          auto good_positives = boost::make_iterator_range(
            boost::make_filter_iterator<predicate_t>(begin(changes.positive), end(changes.positive)),
            boost::make_filter_iterator<predicate_t>(end(changes.positive), end(changes.positive))
          );

          for(const auto& good_one : good_positives)
            filtered.positive.push_back(good_one);

          auto good_negatives = boost::make_iterator_range(
            boost::make_filter_iterator<predicate_t>(begin(changes.negative), end(changes.negative)),
            boost::make_filter_iterator<predicate_t>(end(changes.negative), end(changes.negative))
          );

          for(const auto& good_one : good_negatives)
            filtered.negative.push_back(good_one);

          return filtered;

        },
        others >> [=] {
          //TODO: Print error
        }
      };
    };

  };



}




#endif //SPRINCLE_RETE_HPP
