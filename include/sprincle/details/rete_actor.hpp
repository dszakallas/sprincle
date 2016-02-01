//
// Created by david on 1/23/16.
//

#ifndef SPRINCLE_RETE_ACTOR_HEADER
#define SPRINCLE_RETE_ACTOR_HEADER

#include <caf/actor.hpp>
#include <caf/event_based_actor.hpp>

using namespace caf;

namespace sprincle {
  namespace details {
    struct rete_actor : public event_based_actor {

      const actor next_actor;

      rete_actor(const actor& next_actor) : next_actor(next_actor) {
        link_to(next_actor);
      }

      rete_actor(actor&& next_actor) : next_actor(next_actor) {
        link_to(next_actor);
      }
    };

  }
}

#endif //SPRINCLE_RETE_ACTOR_HEADER
