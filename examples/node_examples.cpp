//
// Created by david on 11/6/15.
//
#include <sprincle/detail.hpp>
#include <sprincle/rete.hpp>
#include <caf/actor.hpp>

void examples() {

  /*
   * map nodes are alpha nodes that can transform an input of type
   * InputType to a type OutputType, like you would expect from a function
   * with such a name.
   *
   * you can achieve trimming with it.
   * for this you can use the *project* functor
   * defined in detail.hpp
   * */
  {
    using In = tuple<long, long, long, long>;

    auto trim = sprincle::project<1, 2>();

    auto trimmer = caf::spawn(
      sprincle::map<In, decltype(trim)>::behavior,
      trim
    );

  }

  /*
   * or you can define your custom map functor with an arbitrary
   * transformation, and make it work with that
   */
  {
    //TODO
  }

  /*
   * filter nodes take a predicate, and test the
   * input against it. If the predicate holds, the
   * change is preserved, otherwise dropped.
   *
   * there are some prewritten commonly used predicates:
   *
   *  - forall_equals tests if the changeset is homogenous, ie
   *    all of its items are equal
   */
  {
    using In = tuple<long, long, long, long>;

    auto is_homogenous = forall_equals();

    auto filter_homogenous = caf::spawn(
      filter<In, decltype(is_homogenous)>::behavior,
      isHomogenous);

  }
  /*
   *  - exists_not_equals is exactly the opposite, ie
   *    it tests whether there is an element which is different
   */
  {
    using In = tuple<long, long, long, long>;

    auto is_heterogenous = exists_not_equal();

    auto filter_heterogenous = caf::spawn(
      filter<In, decltype(is_heterogenous)>::behavior,
      is_heterogenous);

  }
  /*
   *  - *exactly* takes a value, and decides if the input
   *    is the same as that given value
   */
  {
    auto ideal = make_tuple(_0, _1, _2);

    /* Instantiates an exactly, deduces type from
     * arguments */
    auto is_ideal = make_exactly(ideal);

    auto filter_ideal = caf::spawn(
      filter<decltype(ideal), decltype(is_ideal)>::behavior,
      is_ideal);

  }



}
