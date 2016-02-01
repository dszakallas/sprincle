#ifndef SPRINCLE_INPUT_NODE_HEADER
#define SPRINCLE_INPUT_NODE_HEADER

#include <caf/behavior.hpp>
#include <caf/spawn.hpp>

#include "details/rete_actor.hpp"

#include "message_atoms.hpp"
#include "delta.hpp"

using namespace caf;

namespace sprincle {
  template<class InputTuple, class OutputSlot>
  struct input_node :
    public details::rete_actor
    {

    using input_delta_t = delta<InputTuple>;
    using output_delta_t = input_delta_t;
    using output_slot_t = OutputSlot;

    input_node(const actor& next_actor) : details::rete_actor(next_actor) {}

    input_node(actor&& next_actor) : details::rete_actor(next_actor) {}

    behavior make_behavior() override {
      return behavior {
        [=](primary_message_t, const input_delta_t& result) {

          if(!result.positive.empty() || !result.negative.empty())
            send(next_actor, output_slot_t::value, result);
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

  template<class InputTuple, class OutputSlot>
  decltype(auto) spawn_input_node(const actor& next_actor, OutputSlot) noexcept {
    return spawn<input_node<InputTuple, OutputSlot>>(next_actor);
  }
}

#endif //SPRINCLE_INPUT_NODE_HEADER
