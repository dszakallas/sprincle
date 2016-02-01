//
// Created by david on 1/23/16.
//

#ifndef SPRINCLE_COMPARE_TUPLES_HEADER
#define SPRINCLE_COMPARE_TUPLES_HEADER

#include <tuple>
#include <utility>

using namespace std;

namespace sprincle {
  namespace details {

    namespace /* private */ {

      template<class Comparator, class T1, class T2, class... Ts>
      bool compare_elements_recurse(Comparator&& compare, T1&& t1, T2&& t2, Ts&&... ts) noexcept;

      template<class Comparator, class T1, class T2>
      bool compare_elements_recurse(Comparator&& compare, T1&& t1, T2&& t2) noexcept;

      template<class Comparator, class Pair, class... Pairs>
      bool compare_tuples_recurse(Comparator&& compare, Pair&& t, Pairs&&... ts) noexcept;

      template<class Comparator, class Pair>
      bool compare_tuples_recurse(Comparator&& compare, Pair&& t) noexcept;

      template<class Comparator, class T1, class T2, class... Ts>
      bool compare_elements_recurse(Comparator&& compare, T1&& t1, T2&& t2, Ts&&... ts) noexcept {
        if (!compare(forward<T1>(t1), forward<T2>(t2))) return false;
        return compare_elements_recurse(forward<Comparator>(compare), forward<T1>(t1), forward<Ts>(ts)...);
      }

      template<class Comparator, class T1, class T2>
      bool compare_elements_recurse(Comparator&& compare, T1&& t1, T2&& t2) noexcept {
        return compare(forward<T1>(t1), forward<T1>(t2));
      }

      template<class Comparator, class Pair, class... Pairs>
      bool compare_tuples_recurse(Comparator&& compare, Pair&& t, Pairs&&... ts) noexcept {
        if (!compare(get<0>(forward<Pair>(t)), get<1>(forward<Pair>(t)))) return false;
        return compare_tuples_recurse(forward<Comparator>(compare), forward<Pairs>(ts)...);
      }

      template<class Comparator, class Pair>
      bool compare_tuples_recurse(Comparator&& compare, Pair&& t) noexcept {
        return compare(get<0>(forward<Pair>(t)), get<1>(forward<Pair>(t)));
      }

    }

    //Compare elements of a single tuple piecewise
    template<class Comparator, class Tuple, size_t... I>
    bool compare_tuple_elements(Comparator&& compare, Tuple&& t, index_sequence<I...>) noexcept {
      return compare_elements_recurse(forward<Comparator>(compare), get<I>(forward<Tuple>(t))...);
    }

    //Compare two tuples element-by-element
    template<class Comparator, class left_tuple_t, class right_tuple_t, size_t... I>
    bool compare_tuples(Comparator&& compare, left_tuple_t&& left_t, right_tuple_t&& right_t, index_sequence<I...>) noexcept {
      return compare_tuples_recurse(forward<Comparator>(compare),
                                    make_tuple(
                                      get<I>(forward<left_tuple_t>(left_t)),
                                      get<I>(forward<right_tuple_t>(right_t)))...);
    }


  }
}

#endif //SPRINCLE_COMPARE_TUPLES_HEADER
