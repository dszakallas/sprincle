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
#include <map>

#include <boost/iterator/filter_iterator.hpp>
#include <boost/range.hpp>

#include <caf/all.hpp>

#include "detail.hpp"

using namespace std;
using namespace caf;

namespace {

  template<class predicate_t, class vector_t>
  decltype(auto) filter_impl(predicate_t&& p, vector_t&& v) noexcept {

    auto good = boost::make_iterator_range(
      boost::filter_iterator<predicate_t, decltype(begin(forward<vector_t>(v)))>
        (forward<predicate_t>(p), begin(forward<vector_t>(v)), end(forward<vector_t>(v))),
      boost::filter_iterator<predicate_t, decltype(begin(forward<vector_t>(v)))>
        (forward<predicate_t>(p), end(forward<vector_t>(v)), end(forward<vector_t>(v)))
    );

    return remove_reference_t<vector_t>(good.begin(), good.end());
  };

}

namespace sprincle {

  /*
   * delta class. Container.
   * Implemented concepts are: TMC, TMA, TCC, TCA, DC
   */
  template<typename tuple_t>
  struct delta {

    using change_t = tuple_t;
    using changeset_t = vector<change_t>;

    changeset_t positive;
    changeset_t negative;

    delta() noexcept : positive(), negative() {}

    template<typename other_changeset_t>
    delta(other_changeset_t&& p, other_changeset_t&& n) noexcept :
      positive(forward<other_changeset_t>(p)),
      negative(forward<other_changeset_t>(n))
    {}

  };

  template<
    class key_t,
    class primary_value_t,
    class secondary_value_t
  >
  struct memory {

    multimap<key_t, primary_value_t> primary_store;
    multimap<key_t, secondary_value_t> secondary_store;

  };

  template<class tuple_t, class map_t>
  struct map {
    static decltype(auto) behavior(event_based_actor* self, const map_t& map) noexcept {
      return caf::behavior {

        [=](const delta<tuple_t>& changes) {


          using projected_change_t = decltype(map(declval<typename delta<tuple_t>::change_t>()));
          using projected_changeset_t = typename delta<projected_change_t>::changeset_t;

          projected_changeset_t positives(changes.positive.size());
          projected_changeset_t negatives(changes.negative.size());

          transform(begin(changes.positive), end(changes.positive), begin(positives), map);
          transform(begin(changes.negative), end(changes.negative), begin(negatives), map);


          return delta<projected_change_t>(move(positives), move(negatives));
        },
        others >> [=] {
          //TODO: Print error
        }
      };
    }
  };


  /*
   * Can be an Equality Node, Inequality Node and Predicate Evaluator
   */
  template<class tuple_t, class predicate_t>
  struct filter {
    static decltype(auto) behavior(event_based_actor* self, const predicate_t& predicate) noexcept {
      return caf::behavior {
        [=](const delta<tuple_t>& changes) noexcept {

          return delta<tuple_t>(
            filter_impl(predicate, changes.positive),
            filter_impl(predicate, changes.negative)
          );

        },
        others >> [=] {
          //TODO: Print error
        }
      };
    };

  };

  using primary_atom = caf::atom_constant<caf::atom("primary")>;
  using secondary_atom = caf::atom_constant<caf::atom("secondary")>;



  template<class primary_tuple_t, class secondary_tuple_t, class... match_pairs>
  struct join :
    public event_based_actor,
    public memory<
      typename project<(match_pairs::primary)...>::template projected_t<primary_tuple_t>,
      primary_tuple_t,
      secondary_tuple_t
    > {

    using primary_only_seq_t = not_in_sequence_t<
        make_index_sequence<tuple_size<primary_tuple_t>::value>,
        index_sequence<(match_pairs::primary)...>
      >;
    using secondary_only_seq_t = not_in_sequence_t<
      make_index_sequence<tuple_size<secondary_tuple_t>::value>,
      index_sequence<(match_pairs::secondary)...>
    >;

    // this syntax is nuts
    using match_t = typename project<(match_pairs::primary)...>::template projected_t<primary_tuple_t>;


    using primary_only_t = typename decltype(make_project<primary_only_seq_t>())::template projected_t<primary_tuple_t>;
    using secondary_only_t = typename decltype(make_project<secondary_only_seq_t>())::template projected_t<secondary_tuple_t>;
    using result_tuple_t = decltype(tuple_cat(declval<primary_tuple_t>(), declval<secondary_only_t>()));

    caf::behavior make_behavior() override {
      auto primary_only = make_project<primary_only_seq_t>();
      auto secondary_only = make_project<secondary_only_seq_t>();

      project<(match_pairs::primary)...> primary_match;
      project<(match_pairs::secondary)...> secondary_match;

      return {
        [=](primary_atom, const delta<primary_tuple_t>& primaries) noexcept {

          const auto& negatives = primaries.negative;
          const auto& positives = primaries.positive;

          delta<result_tuple_t> result;

          for(const auto& negative: negatives) {
            const auto& key = primary_match(negative);

            this->primary_store.erase(key);

            auto match_range = this->secondary_store.equal_range(key);

            for(auto i = match_range.first; i != match_range.second; ++i)
              result.negative.push_back(tuple_cat(negative, secondary_only(i->second)));

          }

          for(const auto& positive: positives) {
            const auto& key = primary_match(positive);

            this->primary_store.insert(make_pair(primary_match(positive), positive));

            auto match_range = this->secondary_store.equal_range(key);

            for(auto i = match_range.first; i != match_range.second; ++i)
              result.positive.push_back(tuple_cat(positive, secondary_only(i->second)));

          }

          return result;


        },
        // TODO fix code duplication
        [=](secondary_atom, const delta<secondary_tuple_t>& secondary_delta) noexcept {

          const auto& negatives = secondary_delta.negative;
          const auto& positives = secondary_delta.positive;


          delta<result_tuple_t> result;

          for(const auto& negative: negatives) {
            const auto& key = secondary_match(negative);

            this->secondary_store.erase(key);

            auto match_range = this->primary_store.equal_range(key);

            for(auto i = match_range.first; i != match_range.second; ++i)
              result.negative.push_back(tuple_cat(i->second, secondary_only(negative)));

          }

          for(const auto& positive: positives) {
            const auto& key = secondary_match(positive);

            this->secondary_store.insert(make_pair(secondary_match(positive), positive));

            auto match_range = this->primary_store.equal_range(key);

            for(auto i = match_range.first; i != match_range.second; ++i)
              result.positive.push_back(tuple_cat(i->second, secondary_only(positive)));

          }

          return result;

        },
        others >> [=] {
          //TODO: Print error
        }
      };
    }
  };

}




#endif //SPRINCLE_RETE_HPP
