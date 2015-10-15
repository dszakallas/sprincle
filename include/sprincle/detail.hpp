//
// Created by david on 10/13/15.
//

#ifndef SPRINCLE_DETAIL_H
#define SPRINCLE_DETAIL_H

#include <tuple>
#include <utility>

using namespace std;

namespace sprincle {

  namespace detail {

    struct equals {
      template<class T1, class T2>
      bool operator()(const T1 &t1, const T2 &t2) const {
        return t1 == t2;
      }
    };

    struct not_equals {
      template<class T1, class T2>
      bool operator()(const T1 &t1, const T2 &t2) const {
        return t1 != t2;
      }
    };


    template<size_t... Indices, class tuple_t>
    decltype(auto) project(const tuple_t& t) {
     return tuple<typename tuple_element<Indices, tuple_t>::type...>(get<Indices>(t)...);
    }

    template<class comparator_t, class T1, class T2>
    bool _compare_tuple_impl(const comparator_t& compare, const T1& t1, const T2& t2) {
      return compare(t1,t2);
    };

    //Helper, recursively compares to elements with
    //the comparator.
    template<class comparator_t, class T1, class T2, class... Ts>
    bool _compare_tuple_impl(const comparator_t& compare, const T1& t1, const T2& t2, const Ts&... ts) {
      if (!compare(t1, t2)) return false;
      return _compare_tuple_impl(compare, t1, ts...);
    }

    //Helper, expands the indexes into a parameter pack
    template<class comparator_t, class tuple_t, size_t... I>
    bool _compare_tuple_detail(const comparator_t& compare, const tuple_t& t, index_sequence<I...>) {
      return _compare_tuple_impl(compare, get<I>(t)...);
    }

  }

  namespace filters {

    /*
     * Binary filters.
     */
    struct forall_equals {
      template<class tuple_t, class I = make_index_sequence<tuple_size<tuple_t>::value>>
      bool operator()(const tuple_t &t) const {
        return detail::_compare_tuple_detail(detail::equals(), t, I());
      }
    };

    struct exists_not_equal {
      template<class tuple_t, class I = make_index_sequence<tuple_size<tuple_t>::value>>
      bool operator()(const tuple_t &t) const {
        return detail::_compare_tuple_detail(detail::not_equals(), t, I());
      }
    };

  }
}

#endif //SPRINCLE_DETAIL_H
