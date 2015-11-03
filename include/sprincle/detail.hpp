//
// Created by david on 10/13/15.
//

#ifndef SPRINCLE_DETAIL_H
#define SPRINCLE_DETAIL_H

#include <tuple>
#include <utility>
#include <type_traits>

using namespace std;

/* FWD DECLARATIONS */
namespace {

  template<class T1, class T2>
  bool equals_impl(T1&& t1, T2&& t2);

  template<class comparator_t, class T1, class T2>
  bool compare_tuple_impl(comparator_t&& compare, T1&& t1, T2&& t2);

  template<class comparator_t, class T1, class T2, class... Ts>
  bool compare_tuple_impl(comparator_t&& compare, T1&& t1, T2&& t2, Ts&&... ts);

  //Helper, expands the indexes into a parameter pack
  template<class comparator_t, class tuple_t, size_t... I>
  bool compare_tuple_detail(comparator_t&& compare, tuple_t&& t, index_sequence<I...>);

}

/* PUBLIC INTERFACE */
namespace sprincle {

  template<size_t... Indices, class tuple_t>
  decltype(auto) project(const tuple_t& t) {
    return tuple<typename tuple_element<Indices, tuple_t>::type...>(get<Indices>(t)...);
  }

  struct equals {
    template<class T1, class T2>
    bool operator()(T1&& t1, T2&& t2) const {
      return equals_impl(forward<T1>(t1), forward<T2>(t2));
    }
  };

  struct not_equals {
    template<class T1, class T2>
    bool operator()(T1&& t1, T2&& t2) const {
      return !equals_impl(forward<T1>(t1), forward<T2>(t2));
    }
  };

  /*
  * Binary filters.
  */
  struct forall_equals {
    template<class tuple_t, class I = make_index_sequence<tuple_size<remove_reference_t<tuple_t>>::value>>
    bool operator()(tuple_t&& t) const {
      return compare_tuple_detail(equals(), forward<tuple_t>(t), I());
    }
  };

  struct exists_not_equal {
    template<class tuple_t, class I = make_index_sequence<tuple_size<remove_reference_t<tuple_t>>::value>>
    bool operator()(tuple_t&& t) const {
      return compare_tuple_detail(not_equals(), forward<tuple_t>(t), I());
    }
  };

}

namespace {

  template<class T1, class T2>
  bool equals_impl(T1&& t1, T2&& t2) {
    return t1 == t2;
  };

  template<class comparator_t, class T1, class T2>
  bool compare_tuple_impl(comparator_t&& compare, T1&& t1, T2&& t2) {
    return compare(forward<T1>(t1), forward<T1>(t2));
  };

  //Helper, recursively compares to elements with
  //the comparator.
  template<class comparator_t, class T1, class T2, class... Ts>
  bool compare_tuple_impl(comparator_t&& compare, T1&& t1, T2&& t2, Ts&&... ts) {
    if (!compare(forward<T1>(t1), forward<T2>(t2))) return false;
    return compare_tuple_impl(forward<comparator_t>(compare), forward<T1>(t1), forward<Ts>(ts)...);
  }

  //Helper, expands the indexes into a parameter pack
  template<class comparator_t, class tuple_t, size_t... I>
  bool compare_tuple_detail(comparator_t&& compare, tuple_t&& t, index_sequence<I...>) {
    return compare_tuple_impl(forward<comparator_t>(compare), get<I>(forward<tuple_t>(t))...);
  }

}

#endif //SPRINCLE_DETAIL_H
