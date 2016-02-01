//
// Created by david on 1/23/16.
//

#ifndef SPRINCLE_MAP_NODE_HEADER
#define SPRINCLE_MAP_NODE_HEADER

#include <caf/behavior.hpp>
#include <caf/actor.hpp>
#include <caf/spawn.hpp>

#include "details/rete_actor.hpp"

#include "message_atoms.hpp"
#include "delta.hpp"

using namespace caf;

namespace sprincle {

  template<class Input, class Map, class OutputSlot>
  struct map_node : public details::rete_actor {
    using map_t = Map;

    using input_t = Input;
    using output_t = decltype(map_t()(declval<Input>())); // symbolically instantiate and call .()
    using input_delta_t = delta<input_t>;
    using output_delta_t = delta<output_t>;
    using output_slot_t = OutputSlot;

    const map_t map;

    map_node(const map_t& map, const actor& next_actor) : details::rete_actor(next_actor), map(map) {};

    behavior make_behavior() override {
      return behavior {

        [=](primary_message_t, const input_delta_t& changes) {

          typename output_delta_t::changeset_t positives;
          for(auto i = begin(changes.positive); i != end(changes.positive); ++i)
            positives.insert(map(*i));

          typename output_delta_t::changeset_t negatives;
          for(auto i = begin(changes.negative); i != end(changes.negative); ++i)
            negatives.insert(map(*i));

          output_delta_t result(move(positives), move(negatives));
          if(!result.positive.empty() || !result.negative.empty())
            this->send(next_actor, output_slot_t::value, move(result));

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

  //TODO Generalize later
  template<class Input, class Map, class OutputSlot>
  decltype(auto) spawn_map_node(const Map& map, const actor& next_actor, OutputSlot) noexcept {
    return spawn<map_node<Input, Map, OutputSlot>>(map, next_actor);
  };


}

#endif //SPRINCLE_MAP_NODE_HEADER
