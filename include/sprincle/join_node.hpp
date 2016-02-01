#ifndef SPRINCLE_JOIN_NODE_HEADER
#define SPRINCLE_JOIN_NODE_HEADER

#include <caf/behavior.hpp>
#include <caf/actor.hpp>
#include <caf/spawn.hpp>

#include "details/project.hpp"
#include "details/rete_actor.hpp"
#include "details/memory.hpp"
#include "details/not_in_sequence.hpp"

#include "message_atoms.hpp"
#include "delta.hpp"

using namespace caf;

namespace sprincle {

  //TODO variadic OutputSlots
  template<class Primary, class Secondary, class OutputSlot, class... MatchPairs>
  struct join_node :
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

    using secondary_only_seq_t = details::not_in_sequence_t<
      make_index_sequence<tuple_size<secondary_value_t>::value>,
      index_sequence<(MatchPairs::secondary_value)...>
    >;

    using primary_match_t = details::project<primary_value_t, (MatchPairs::primary_value)...>;
    using secondary_match_t = details::project<secondary_value_t, (MatchPairs::secondary_value)...>;
    using secondary_only_t = decltype(details::make_project<secondary_value_t, secondary_only_seq_t>());

    using primary_delta_t = delta<primary_value_t>;
    using secondary_delta_t = delta<secondary_value_t>;
    using output_delta_t = delta<decltype(tuple_cat(declval<primary_value_t>(), declval<typename secondary_only_t::output_t>()))>;

    using output_t = typename output_delta_t::change_t;

    primary_match_t primary_match;
    primary_match_t secondary_match;
    secondary_only_t secondary_only;

    join_node(const actor& next_actor) : details::rete_actor(next_actor) {};

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
            this->send(next_actor, OutputSlot::value, move(result));

        },
        // TODO fix code duplication
        [=](secondary_message_t, const delta<Secondary>& secondary_delta) noexcept {

          const auto& negatives = secondary_delta.negative;
          const auto& positives = secondary_delta.positive;


          output_delta_t result;

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
            this->send(next_actor, OutputSlot::value, move(result));

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

  template<class Primary, class Secondary, class... MatchPairs, class OutputSlot>
  decltype(auto) spawn_join_node(const actor& next_actor, OutputSlot) noexcept {
    return spawn<join_node<Primary, Secondary, OutputSlot, MatchPairs...>>(next_actor);
  }

}

#endif //SPRINCLE_JOIN_NODE_HEADER
