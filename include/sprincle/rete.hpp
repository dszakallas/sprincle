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
#include <map>
#include <set>

#include <boost/iterator/filter_iterator.hpp>
#include <boost/range.hpp>

#include <caf/all.hpp>

#include "detail.hpp"

using namespace std;
using namespace caf;

namespace {

  template<class predicate_t, class container_t>
  decltype(auto) filter_impl(predicate_t&& p, container_t&& v) noexcept {

    auto good = boost::make_iterator_range(
      boost::filter_iterator<predicate_t, decltype(begin(forward<container_t>(v)))>
        (forward<predicate_t>(p), begin(forward<container_t>(v)), end(forward<container_t>(v))),
      boost::filter_iterator<predicate_t, decltype(begin(forward<container_t>(v)))>
        (forward<predicate_t>(p), end(forward<container_t>(v)), end(forward<container_t>(v)))
    );

    return remove_reference_t<container_t>(good.begin(), good.end());
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
    using changeset_t = set<change_t>;

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

    multimap<key_t, primary_value_t> primary_indexer;
    multimap<key_t, secondary_value_t> secondary_indexer;

  };

  template<class tuple_t, class map_t>
  struct map {
    static decltype(auto) behavior(event_based_actor* self, const map_t& map) noexcept {
      return caf::behavior {

        [=](const delta<tuple_t>& changes) {


          using projected_change_t = decltype(map(declval<typename delta<tuple_t>::change_t>()));
          using projected_changeset_t = typename delta<projected_change_t>::changeset_t;

          projected_changeset_t positives;
          projected_changeset_t negatives;

          for(auto i = begin(changes.positive); i != end(changes.positive); ++i)
            positives.insert(map(*i));

          for(auto i = begin(changes.negative); i != end(changes.negative); ++i)
            negatives.insert(map(*i));


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

            this->primary_indexer.erase(key);

            auto match_range = this->secondary_indexer.equal_range(key);

            for(auto i = match_range.first; i != match_range.second; ++i)
              result.negative.insert(tuple_cat(negative, secondary_only(i->second)));

          }

          for(const auto& positive: positives) {
            const auto& key = primary_match(positive);

            this->primary_indexer.insert(make_pair(key, positive));

            auto match_range = this->secondary_indexer.equal_range(key);

            for(auto i = match_range.first; i != match_range.second; ++i)
              result.positive.insert(tuple_cat(positive, secondary_only(i->second)));

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

            this->secondary_indexer.erase(key);

            auto match_range = this->primary_indexer.equal_range(key);

            for(auto i = match_range.first; i != match_range.second; ++i)
              result.negative.insert(tuple_cat(i->second, secondary_only(negative)));

          }

          for(const auto& positive: positives) {
            const auto& key = secondary_match(positive);

            this->secondary_indexer.insert(make_pair(key, positive));

            auto match_range = this->primary_indexer.equal_range(key);

            for(auto i = match_range.first; i != match_range.second; ++i)
              result.positive.insert(tuple_cat(i->second, secondary_only(positive)));

          }

          return result;

        },
        others >> [=] {
          //TODO: Print error
        }
      };
    }
  };

  template<class primary_tuple_t, class secondary_tuple_t, class... match_pairs>
  struct antijoin :
    public event_based_actor,
    public memory<
      typename project<(match_pairs::primary)...>::template projected_t<primary_tuple_t>,
      primary_tuple_t,
      secondary_tuple_t
    > {

    using match_t = typename project<(match_pairs::primary)...>::template projected_t<primary_tuple_t>;
    using result_tuple_t = primary_tuple_t;

    caf::behavior make_behavior() override {

      project<(match_pairs::primary)...> primary_match;
      project<(match_pairs::secondary)...> secondary_match;

      return {
        [=](primary_atom, const delta<primary_tuple_t> &primaries) noexcept {

          const auto& negatives = primaries.negative;
          const auto& positives = primaries.positive;

          delta<result_tuple_t> result;

          for(const auto& negative: negatives) {
            const auto& key = primary_match(negative);

            this->primary_indexer.erase(key);

            auto match_range = this->secondary_indexer.equal_range(key);

            //If no match found
            if(match_range.first == end(this->secondary_indexer))
              result.negative.insert(negative);

          }

          for(const auto& positive: positives) {
            const auto& key = primary_match(positive);

            this->primary_indexer.insert(make_pair(key, positive));

            auto match_range = this->secondary_indexer.equal_range(key);

            //If no match found
            if(match_range.first == end(this->secondary_indexer))
              result.positive.insert(positive);

          }

          return result;

        },
        [=](secondary_atom, const delta<secondary_tuple_t> &secondaries) noexcept {

          const auto& negatives = secondaries.negative;
          const auto& positives = secondaries.positive;

          delta<result_tuple_t> result;

          for(const auto& negative: negatives) {
            const auto& key = secondary_match(negative);

            this->secondary_indexer.erase(key);

            auto match_range = this->primary_indexer.equal_range(key);

            // send a positive update with the tuples matching the primary indexer
            for(auto i = match_range.first; i != match_range.second; ++i)
              result.positive.insert(i->second);

          }

          for(const auto& positive: positives) {
            const auto& key = secondary_match(positive);

            this->secondary_indexer.insert(make_pair(key, positive));

            auto match_range = this->primary_indexer.equal_range(key);

            // send a negative update with the tuples matching the primary indexer
            for(auto i = match_range.first; i != match_range.second; ++i)
              result.negative.insert(i->second);

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
