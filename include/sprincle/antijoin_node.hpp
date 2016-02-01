#ifndef SPRINCLE_ANTIJOIN_NODE_HEADER
#define SPRINCLE_ANTIJOIN_NODE_HEADER

#include <caf/behavior.hpp>
#include <caf/actor.hpp>
#include <caf/spawn.hpp>

#include "details/rete_actor.hpp"
#include "details/project.hpp"
#include "details/memory.hpp"

#include "message_atoms.hpp"
#include "delta.hpp"

using namespace caf;

namespace sprincle {

  template<class Primary, class Secondary, class OutputSlot, class... MatchPairs>
  struct antijoin_node :
    public details::rete_actor,
    public details::memory<
      typename details::project<Primary, (MatchPairs::primary_value)...>::output_t,
      Primary,
      Secondary
    > {

    using key_t = typename details::project<Primary, (MatchPairs::primary_value)...>::output_t;
    using primary_value_t = Primary;
    using secondary_value_t = Secondary;

    using output_slot_t = OutputSlot;

    using match_t = typename details::project<primary_value_t, (MatchPairs::primary_value)...>::output_t;

    using primary_delta_t = delta<primary_value_t>;
    using secondary_delta_t = delta<secondary_value_t>;
    using output_delta_t = delta<primary_value_t>;
    using output_t = typename delta<primary_value_t>::change_t;

    details::project<primary_value_t, (MatchPairs::primary_value)...> primary_match;
    details::project<secondary_value_t, (MatchPairs::secondary_value)...> secondary_match;

    antijoin_node(const actor& next_actor) : details::rete_actor(next_actor) {};

    behavior make_behavior() override {

      return {
        [=](primary_message_t, const primary_delta_t& primaries) noexcept {

          const auto& negatives = primaries.negative;
          const auto& positives = primaries.positive;

          output_delta_t result;

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
            this->send(next_actor, output_slot_t::value, move(result));
          }
        },
        [=](secondary_message_t, const secondary_delta_t& secondaries) noexcept {

          const auto& negatives = secondaries.negative;
          const auto& positives = secondaries.positive;

          output_delta_t result;

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
            this->send(next_actor, output_slot_t::value, move(result));

          }

        },
        [=](end_message_t) {
          send(next_actor, end_message_t::value);
        },
        others >> [=] {
          //TODO: Print error
        }
      };
    }
  };

  template<class Primary, class Secondary, class... MatchPairs, class message_atom>
  decltype(auto) spawn_antijoin_node(const actor& next_actor, message_atom) noexcept {
    return spawn<antijoin_node<Primary, Secondary, message_atom, MatchPairs...>>(next_actor);
  };


}

#endif // SPRINCLE_ANTIJOIN_NODE_HEADER
