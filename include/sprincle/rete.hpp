//
// Created by david on 10/8/15.
//

#ifndef SPRINCLE_RETE_HPP
#define SPRINCLE_RETE_HPP

#include <tuple>
#include <utility>
#include <map>
#include <set>

#include <boost/iterator/filter_iterator.hpp>
#include <boost/range.hpp>

#include <caf/all.hpp>

#include "detail.hpp"

using namespace std;
using namespace caf;



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

  struct rete_node : public event_based_actor {

    const actor next_actor;

    rete_node(const actor& next_actor) : next_actor(next_actor) {
      this->link_to(next_actor);
    }

    rete_node(actor&& next_actor) : next_actor(next_actor) {
      this->link_to(next_actor);
    }
  };


  using primary = caf::atom_constant<caf::atom("primary")>;
  using secondary = caf::atom_constant<caf::atom("secondary")>;
  using io_end = caf::atom_constant<caf::atom("io_end")>;

  template<class tuple_t, class message_slot>
  struct input_node : public rete_node {

    using result_tuple_t = tuple_t;

    input_node(const actor& next_actor) : rete_node(next_actor) {}

    input_node(actor&& next_actor) : rete_node(next_actor) {}

    caf::behavior make_behavior() override {
      return caf::behavior {
        [=](primary, const delta<tuple_t>& result) {

          if(!result.positive.empty() || !result.negative.empty())
            this->send(next_actor, message_slot::value, result);
        },
        [=](io_end) {
          this->send(next_actor, io_end::value);
        },
        others >> [=] {
          //TODO: Print error
        }
      };
    }
  };

  template<class tuple_t, class message_slot>
  decltype(auto) spawn_input_node(const actor& next_actor, message_slot) noexcept {
    return spawn<input_node<tuple_t, message_slot>>(next_actor);
  };



  template<class tuple_t, class map_t, class message_slot>
  struct map_node : public rete_node {

    // just look at this type, xD
    using result_tuple_t = decltype(declval<map_t>()(declval<typename delta<tuple_t>::change_t>()));

    const map_t map;

    map_node(const map_t& map, const actor& next_actor) : rete_node(next_actor), map(map) {};

    caf::behavior make_behavior() override {
      return caf::behavior {

        [=](primary, const delta<tuple_t>& changes) {

          using projected_change_t = result_tuple_t;
          using projected_changeset_t = typename delta<projected_change_t>::changeset_t;

          projected_changeset_t positives;
          projected_changeset_t negatives;

          for(auto i = begin(changes.positive); i != end(changes.positive); ++i)
            positives.insert(map(*i));

          for(auto i = begin(changes.negative); i != end(changes.negative); ++i)
            negatives.insert(map(*i));

          delta<projected_change_t> result(move(positives), move(negatives));

          if(!result.positive.empty() || !result.negative.empty())
            this->send(next_actor, message_slot::value, move(result));

        },
        [=](io_end) {
          this->send(next_actor, io_end::value);
        },
        others >> [=] {
          //TODO: Print error
        }
      };
    };
  };

  //TODO Generalize later
  template<class tuple_t, class map_t, class message_slot>
  decltype(auto) spawn_map_node(const map_t& map, const actor& next_actor, message_slot) noexcept {
    return spawn<map_node<tuple_t, map_t, message_slot>>(map, next_actor);
  };

  namespace {

    template<class predicate_t, class container_t>
    decltype(auto) filter_impl(predicate_t&& p, container_t&& v) noexcept {

      auto good = boost::make_iterator_range(
        boost::filter_iterator<predicate_t, decltype(begin(forward<container_t>(v)))>
          (forward<predicate_t>(p), begin(forward<container_t>(v)), end(forward<container_t>(v))),
        boost::filter_iterator<predicate_t, decltype(begin(forward<container_t>(v)))>
          (forward<predicate_t>(p), end(forward<container_t>(v)), end(forward<container_t>(v)))
      );

      return remove_reference_t<container_t>(move(good.begin()), move(good.end()));
    };

  }

  /*
   * Can be an Equality Node, Inequality Node and Predicate Evaluator
   */
  template<class tuple_t, class predicate_t, class message_slot>
  struct filter_node : public rete_node {

    using result_tuple_t = tuple_t;

    const predicate_t predicate;

    filter_node(const predicate_t& predicate, const actor& next_actor) : rete_node(next_actor), predicate(predicate) {};

    caf::behavior make_behavior() override {
      return caf::behavior {
        [=](primary, const delta<tuple_t>& changes) noexcept {

          delta<tuple_t> result(
            filter_impl(predicate, changes.positive),
            filter_impl(predicate, changes.negative)
          );

          if(!result.positive.empty() || !result.negative.empty())
            this->send(next_actor, message_slot::value, move(result));

        },
        [=](io_end) {
          this->send(next_actor, io_end::value);
        },
        others >> [=] {
          //TODO: Print error
        }
      };
    };
  };

  //TODO Generalize later
  template<class tuple_t, class predicate_t, class message_slot = primary>
  decltype(auto) spawn_filter_node(const predicate_t& predicate, const actor& next_actor, message_slot) noexcept {
    return spawn<filter_node<tuple_t, predicate_t, message_slot>>(predicate, next_actor);
  };

  //TODO variadic message_atoms
  template<class primary_tuple_t, class secondary_tuple_t, class message_atom, class... match_pairs>
  struct join_node :
    public rete_node,
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

    join_node(const actor& next_actor) : rete_node(next_actor) {};

    caf::behavior make_behavior() override {
      auto primary_only = make_project<primary_only_seq_t>();
      auto secondary_only = make_project<secondary_only_seq_t>();

      project<(match_pairs::primary)...> primary_match;
      project<(match_pairs::secondary)...> secondary_match;

      return {
        [=](primary, const delta<primary_tuple_t>& primaries) noexcept {


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

          if(!result.positive.empty() || !result.negative.empty())
            this->send(next_actor, message_atom::value, move(result));

        },
        // TODO fix code duplication
        [=](secondary, const delta<secondary_tuple_t>& secondary_delta) noexcept {

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

          if(!result.positive.empty() || !result.negative.empty())
            this->send(next_actor, message_atom::value, move(result));

        },
        [=](io_end) {
          this->send(next_actor, io_end::value);
        },
        others >> [=] {
          //TODO: Print error
        }
      };
    }
  };

  template<class primary_tuple_t, class secondary_tuple_t, class... match_pairs, class message_atom>
  decltype(auto) spawn_join_node(const actor& next_actor, message_atom) noexcept {
    return spawn<join_node<primary_tuple_t, secondary_tuple_t, message_atom, match_pairs...>>(next_actor);
  };


  template<class primary_tuple_t, class secondary_tuple_t, class message_atom, class... match_pairs>
  struct antijoin_node :
    public rete_node,
    public memory<
      typename project<(match_pairs::primary)...>::template projected_t<primary_tuple_t>,
      primary_tuple_t,
      secondary_tuple_t
    > {

    using match_t = typename project<(match_pairs::primary)...>::template projected_t<primary_tuple_t>;
    using result_tuple_t = primary_tuple_t;

    antijoin_node(const actor& next_actor) : rete_node(next_actor) {};

    caf::behavior make_behavior() override {

      project<(match_pairs::primary)...> primary_match;
      project<(match_pairs::secondary)...> secondary_match;

      return {
        [=](primary, const delta<primary_tuple_t> &primaries) noexcept {

          const auto& negatives = primaries.negative;
          const auto& positives = primaries.positive;

          delta<result_tuple_t> result;

          for(const auto& negative: negatives) {
            const auto& key = primary_match(negative);

            this->primary_indexer.erase(key);

            auto match_range = this->secondary_indexer.equal_range(key);

            //If no match found
            if(match_range.first == match_range.second)
              result.negative.insert(negative);

          }

          for(const auto& positive: positives) {
            const auto& key = primary_match(positive);

            this->primary_indexer.insert(make_pair(key, positive));

            auto match_range = this->secondary_indexer.equal_range(key);

            //If no match found
            if(match_range.first == match_range.second)
              result.positive.insert(positive);

          }

          if(!result.positive.empty() || !result.negative.empty()) {
            this->send(next_actor, message_atom::value, move(result));
          }


        },
        [=](secondary, const delta<secondary_tuple_t> &secondaries) noexcept {

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
          if(!result.positive.empty() || !result.negative.empty()) {
            this->send(next_actor, message_atom::value, move(result));

          }

        },
        [=](io_end) {
          this->send(next_actor, io_end::value);
        },
        others >> [=] {
          //TODO: Print error
        }
      };
    }
  };

  template<class primary_tuple_t, class secondary_tuple_t, class... match_pairs, class message_atom>
  decltype(auto) spawn_antijoin_node(const actor& next_actor, message_atom) noexcept {
    return spawn<antijoin_node<primary_tuple_t, secondary_tuple_t, message_atom, match_pairs...>>(next_actor);
  };

}



#endif //SPRINCLE_RETE_HPP
