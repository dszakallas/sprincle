#ifndef SPRINCLE_FILTER_NODE_HEADER
#define SPRINCLE_FILTER_NODE_HEADER

#include <utility>

#include <caf/behavior.hpp>
#include <caf/spawn.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/range.hpp>

#include "message_atoms.hpp"
#include "delta.hpp"

#include "details/rete_actor.hpp"

using namespace caf;
using namespace std;

namespace sprincle {

  namespace /* private */ {
    template<class Predicate, class Container>
    decltype(auto) filter_impl(Predicate&& p, Container&& v) noexcept {

      auto good = boost::make_iterator_range(
        boost::filter_iterator<Predicate, decltype(begin(forward<Container>(v)))>
          (forward<Predicate>(p), begin(forward<Container>(v)), end(forward<Container>(v))),
        boost::filter_iterator<Predicate, decltype(begin(forward<Container>(v)))>
          (forward<Predicate>(p), end(forward<Container>(v)), end(forward<Container>(v)))
      );

      return remove_reference_t<Container>(move(good.begin()), move(good.end()));
    };
  }

  template<class Input, class Predicate, class OutputSlot>
  struct filter_node : public details::rete_actor {

    using output_slot_t = OutputSlot;
    using predicate_t = Predicate;
    using input_t = Input;
    using input_delta_t = delta<input_t>;
    using output_delta_t = input_delta_t;

    const predicate_t predicate;

    filter_node(const predicate_t& predicate, const actor& next_actor) : details::rete_actor(next_actor), predicate(predicate) {};

    caf::behavior make_behavior() override {
      return caf::behavior {
        [=](primary_message_t, const input_delta_t& changes) noexcept {

          output_delta_t result(
            filter_impl(predicate, changes.positive),
            filter_impl(predicate, changes.negative)
          );

          if(!result.positive.empty() || !result.negative.empty())
            this->send(next_actor, output_slot_t::value, move(result));

        },
        [=](end_message_t) {
          this->send(next_actor, end_message_t::value);
        },
        others >> [=] {
          //TODO: Print error
        }
      };
    };
  };

  //TODO Generalize later
  template<class Input, class Predicate, class OutputSlot>
  decltype(auto) spawn_filter_node(const Predicate& predicate, const actor& next_actor, OutputSlot) noexcept {
    return spawn<filter_node<Input, Predicate, OutputSlot>>(predicate, next_actor);
  };

}

#endif //SPRINCLE_FILTER_NODE_HEADER
