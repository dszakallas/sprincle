//
// Created by david on 1/23/16.
//

#ifndef SPRINCLE_NOT_IN_SEQUENCE_HEADER
#define SPRINCLE_NOT_IN_SEQUENCE_HEADER

#include <utility>

using namespace std;

namespace sprincle {
  namespace details {

    namespace /* private */ {

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
      using type = index_sequence<>;
    };

    template<class First, class Second>
    using not_in_sequence_t = typename not_in_sequence<First, Second>::type;

    template<size_t... First, size_t... Second,
      size_t FirstHead, size_t SecondHead>
    struct not_in_sequence<index_sequence<FirstHead, First...>,
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

    template<size_t... First, size_t FirstHead>
    struct not_in_sequence<index_sequence<FirstHead, First...>, index_sequence<>> {
      using type = index_sequence<FirstHead, First...>;
    };

  }
}

#endif //SPRINCLE_NOT_IN_SEQUENCE_HEADER
