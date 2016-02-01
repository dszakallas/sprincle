//
// Created by david on 1/23/16.
//

#ifndef SPRINCLE_EXACTLY_HEADER
#define SPRINCLE_EXACTLY_HEADER

#include <tuple>
#include <utility>

#include "compare_tuples.hpp"
#include "equals.hpp"

using namespace std;

namespace sprincle {
  namespace details {

    template<class Input>
    struct exactly {

      using input_t = Input;
      input_t expected_tuple;

      exactly(input_t&& expected_tuple) noexcept : expected_tuple(expected_tuple) {}
      exactly(const input_t& expected_tuple) noexcept : expected_tuple(expected_tuple) {}

      template<class ActualTuple, class Indices = make_index_sequence<tuple_size<input_t>::value>>
      bool operator()(ActualTuple&& actual) const noexcept {
        return compare_tuples(equals(), expected_tuple, forward<ActualTuple>(actual), Indices());
      }
    };

    /*
     * Predicate filter.
     */
    template<class Input>
    decltype(auto) make_exactly(Input&& expected) noexcept {
      return exactly<remove_reference_t<Input>>(forward<Input>(expected));
    }
  }
}

#endif //SPRINCLE_EXACTLY_HEADER
