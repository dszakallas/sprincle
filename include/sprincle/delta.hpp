#ifndef SPRINCLE_DELTA_HEADER
#define SPRINCLE_DELTA_HEADER

#include <set>

using namespace std;

namespace sprincle {

  ///
  /// delta is a data structure that holds positive and negative changes that
  /// propagate along the Rete network.
  ///
  template<class Change>
  struct delta {
    using change_t = Change;
    template <class T> using set_t = set<T>;
    using changeset_t = set_t<change_t>;

    changeset_t positive;
    changeset_t negative;

    delta() noexcept : positive(), negative() {}

    template<class Changeset>
    delta(Changeset&& p, Changeset&& n) noexcept :
      positive(forward<Changeset>(p)),
      negative(forward<Changeset>(n))
    {}

  };

}

#endif // SPRINCLE_DELTA_HEADER
