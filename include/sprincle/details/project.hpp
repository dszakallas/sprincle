//
// Created by david on 1/23/16.
//

#ifndef SPRINCLE_PROJECT_HEADER
#define SPRINCLE_PROJECT_HEADER

#include <tuple>
#include <utility>

using namespace std;

namespace sprincle {
  namespace details {

    template<class InputTuple, size_t... Indices>
    struct project {
      using input_t = InputTuple;
      using output_t = tuple<typename tuple_element<Indices, InputTuple>::type...>;

      template<class _InputType>
      decltype(auto) operator()(_InputType&& t) const noexcept {
        return tuple<typename tuple_element<Indices, remove_reference_t<_InputType>>::type...>(get<Indices>(forward<_InputType>(t))...);
      }
    };

    namespace /* private */ {
      template<class InputTuple, size_t... Indices>
      decltype(auto) make_project_helper(index_sequence<Indices...>) noexcept {
        return project<InputTuple, Indices...>();
      }
    }

    template<class InputTuple, class Is>
    decltype(auto) make_project() noexcept {
      return make_project_helper<InputTuple>(Is());
    }
  }
}

#endif //SPRINCLE_PROJECT_HEADER
