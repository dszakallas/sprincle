//
// Created by david on 1/23/16.
//

#ifndef SPRINCLE_EQUALS_HEADER
#define SPRINCLE_EQUALS_HEADER

#include <utility>

using namespace std;

namespace sprincle {
  namespace details {

    namespace /* private */ {
      template<class T1, class T2>
      bool equals_impl(T1&& t1, T2&& t2) noexcept {
        return t1 == t2;
      };
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



  }
}

#endif //SPRINCLE_EQUALS_HEADER
