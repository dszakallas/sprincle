//
// Created by david on 10/13/15.
//

#ifndef SPRINCLE_DETAIL_H
#define SPRINCLE_DETAIL_H

#include <tuple>
#include <utility>
#include <type_traits>

using namespace std;



/* PUBLIC INTERFACE */
namespace sprincle {

  /* FWD DECLARATIONS */
  namespace {

    template<class T1, class T2>
    bool equals_impl(T1&& t1, T2&& t2) noexcept;

    //Compare elements of a single tuple helpers
    template<class comparator_t, class tuple_t, size_t... I>
    bool compare_elements_detail(comparator_t&& compare, tuple_t&& t, index_sequence<I...>) noexcept;

    template<class comparator_t, class T1, class T2, class... Ts>
    bool compare_elements_impl(comparator_t&& compare, T1&& t1, T2&& t2, Ts&&... ts) noexcept;

    template<class comparator_t, class T1, class T2>
    bool compare_elements_impl(comparator_t&& compare, T1&& t1, T2&& t2) noexcept;

    //Compare two tuples element-by-element helpers
    template<class comparator_t, class left_tuple_t, class right_tuple_t, size_t... I>
    bool compare_tuples_detail(comparator_t&& compare, left_tuple_t&& left_t, right_tuple_t&& right_t, index_sequence<I...>) noexcept;

    template<class comparator_t, class element_pair_t, class... element_pairs_t>
    bool compare_tuples_impl(comparator_t&& compare, element_pair_t&& t, element_pairs_t&&... ts) noexcept;

    template<class comparator_t, class element_pair_t>
    bool compare_tuples_impl(comparator_t&& compare, element_pair_t&& t) noexcept;

  }

  // Thanks to TartanLlama
  // http://stackoverflow.com/questions/33648875/c-construct-a-tuple-from-elements-with-indices-that-are-not-in-a-given-index-s

  namespace {

    template<size_t First, typename Seq>
    struct sequence_cat;

    template<size_t First, size_t... Seq>
    struct sequence_cat<First, index_sequence<Seq...>> {
      using type = index_sequence<First, Seq...>;
    };

    template<size_t First, typename Seq>
    using sequence_cat_t = typename sequence_cat<First, Seq>::type;
  }

  template<class First, class Second>
  struct not_in_sequence {
    using type = std::index_sequence<>;
  };

  template <class First, class Second>
  using not_in_sequence_t = typename not_in_sequence<First, Second>::type;

  template <size_t... First, size_t... Second,
    size_t FirstHead, size_t SecondHead>
  struct not_in_sequence <index_sequence<FirstHead, First...>,
    index_sequence<SecondHead, Second...>> {
    using seq1 = index_sequence<First...>;
    using seq2 = index_sequence<Second...>;

    using type =
    conditional_t<
      (FirstHead == SecondHead),
      not_in_sequence_t<seq1, seq2>,
      sequence_cat_t<
        FirstHead,
        not_in_sequence_t<seq1, sequence_cat_t<SecondHead, seq2>>
      >
    >;
  };

  template <size_t... First, size_t FirstHead>
  struct not_in_sequence <index_sequence<FirstHead, First...>, index_sequence<>> {
    using type = index_sequence<FirstHead, First...>;
  };



  template<size_t... Indices>
  struct project {

    template<class tuple_t>
    using projected_t = tuple<typename tuple_element<Indices, tuple_t>::type...>;

    template<class tuple_t>
    decltype(auto) operator()(const tuple_t& t) const noexcept {
      return tuple<typename tuple_element<Indices, tuple_t>::type...>(get<Indices>(t)...);
    }
  };

  template<size_t... Indices>
  decltype(auto) make_project_helper(index_sequence<Indices...>) noexcept {
    return project<Indices...>();
  }

  template<class Is>
  decltype(auto) make_project() noexcept {
    return make_project_helper(Is());
  }


  /*
   * Match selectors for joins
   */
  template<size_t Primary, size_t Secondary>
  struct match_pair {
    enum {
      primary = Primary,
      secondary = Secondary
    };
  };


  //TODO: Generalize to universal reference
  template<class primary_t, class secondary_t, class... match_pairs>
  decltype(auto) match(const primary_t& primary, const secondary_t& secondary) noexcept {
    return project<(match_pairs::primary)...>()(primary) == project<(match_pairs::secondary)...>()(secondary);
  }


  struct equals {
    template<class T1, class T2>
    bool operator()(T1&& t1, T2&& t2) const noexcept {
      return equals_impl(forward<T1>(t1), forward<T2>(t2));
    }
  };

  struct not_equals {
    template<class T1, class T2>
    bool operator()(T1&& t1, T2&& t2) const noexcept {
      return !equals_impl(forward<T1>(t1), forward<T2>(t2));
    }
  };

  namespace {
    template<class T1, class T2>
    bool equals_impl(T1&& t1, T2&& t2) noexcept {
      return t1 == t2;
    };
  }

  /*
  * Binary filters.
  * Functor
  */
  struct forall_equals {
    template<class tuple_t, class I = make_index_sequence<tuple_size<remove_reference_t<tuple_t>>::value>>
    bool operator()(tuple_t&& t) const noexcept {
      return compare_elements_detail(equals(), forward<tuple_t>(t), I());
    }
  };

  struct exists_not_equal {
    template<class tuple_t, class I = make_index_sequence<tuple_size<remove_reference_t<tuple_t>>::value>>
    bool operator()(tuple_t&& t) const noexcept {
      return compare_elements_detail(not_equals(), forward<tuple_t>(t), I());
    }
  };


  /*
   * Predicate filter.
   * Concepts: TMC, TMA, TCC, TCA, Functor
   */
  template<class tuple_t>
  struct exactly {

    tuple_t expected;

    exactly(tuple_t&& expected) noexcept : expected(expected) {}
    exactly(const tuple_t& expected) noexcept : expected(expected) {}

    template<class actual_tuple_t, class I = make_index_sequence<tuple_size<tuple_t>::value>>
    bool operator()(actual_tuple_t&& actual) const noexcept {
      //static_assert(tuple_size<tuple_t>::value == tuple_size<decltype(actual)>::value, "Size of the compared tuples don't match");
      return compare_tuples_detail(equals(), expected, forward<actual_tuple_t>(actual), I());
    }
  };

  template<class tuple_t>
  decltype(auto) make_exactly(tuple_t&& expected) noexcept {
    return exactly<remove_reference_t<tuple_t>>(forward<tuple_t>(expected));
  }


  namespace {

    //Compare elements of a single tuple helpers
    template<class comparator_t, class tuple_t, size_t... I>
    bool compare_elements_detail(comparator_t&& compare, tuple_t&& t, index_sequence<I...>) noexcept {
      return compare_elements_impl(forward<comparator_t>(compare), get<I>(forward<tuple_t>(t))...);
    }

    template<class comparator_t, class T1, class T2, class... Ts>
    bool compare_elements_impl(comparator_t&& compare, T1&& t1, T2&& t2, Ts&&... ts) noexcept {
      if (!compare(forward<T1>(t1), forward<T2>(t2))) return false;
      return compare_elements_impl(forward<comparator_t>(compare), forward<T1>(t1), forward<Ts>(ts)...);
    }

    template<class comparator_t, class T1, class T2>
    bool compare_elements_impl(comparator_t&& compare, T1&& t1, T2&& t2) noexcept {
      return compare(forward<T1>(t1), forward<T1>(t2));
    }

    //Compare two tuples element-by-element helpers
    //Trust me u dont want to read these...
    template<class comparator_t, class left_tuple_t, class right_tuple_t, size_t... I>
    bool compare_tuples_detail(comparator_t&& compare, left_tuple_t&& left_t, right_tuple_t&& right_t, index_sequence<I...>) noexcept {
      return compare_tuples_impl(forward<comparator_t>(compare),
                                 make_tuple(
                                   get<I>(forward<left_tuple_t>(left_t)),
                                   get<I>(forward<right_tuple_t>(right_t)))...);
    }

    template<class comparator_t, class element_pair_t, class... element_pairs_t>
    bool compare_tuples_impl(comparator_t&& compare, element_pair_t&& t, element_pairs_t&&... ts) noexcept {
      if (!compare(get<0>(forward<element_pair_t>(t)), get<1>(forward<element_pair_t>(t)))) return false;
      return compare_tuples_impl(forward<comparator_t>(compare), forward<element_pairs_t>(ts)...);
    }

    template<class comparator_t, class element_pair_t>
    bool compare_tuples_impl(comparator_t&& compare, element_pair_t&& t) noexcept {
      return compare(get<0>(forward<element_pair_t>(t)), get<1>(forward<element_pair_t>(t)));
    }

  }



}



#endif //SPRINCLE_DETAIL_H
