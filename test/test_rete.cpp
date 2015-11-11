//
// Created by david on 10/13/15.
//
#define BOOST_TEST_MODULE ReteTest

#include <utility>
#include <tuple>
#include <vector>
#include <set>
#include <functional>

#include <caf/all.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range.hpp>

#include <sprincle/rete.hpp>
#include <sprincle/detail.hpp>

#include "pretty_tuple.h"

using namespace std;
using namespace sprincle;

/**
 * The tester function can be used to test actors.
 * It will send the given input to the testee, then runs the given assertion function
 * on the message returned.
 * The signature of the assertion function should be
 * <unspecified>(const Input&, const Output&, const actor&, event_based_actor*)
 *   ^                                              ^              ^
 *   |                                              |              |
 *   *- Arbitrary return type               testee -*      tester -*
 */
template <class Handle, class Input, class Output, class Test>
void tester(event_based_actor* self, const Handle& testee, const Input& input, const Test& test) {
  self->link_to(testee);
  self->on_sync_failure(
    [=] {
      BOOST_FAIL("Unexpected response message");
      self->quit(exit_reason::user_shutdown);
    }
  );
  self->sync_send(testee, input).then([=](const Output& output){
    test(input, output, testee, self);
    self->quit(exit_reason::user_shutdown);

  });

}

BOOST_AUTO_TEST_SUITE( ReteTestSuite )

/**********************************************************************************************************/
/* TRIMMER TESTS*/

BOOST_AUTO_TEST_CASE( TrimmerTestCase_0 ) {

    using InputType = tuple<int, int, int>;

    auto removeLast = sprincle::project<0, 1>();

    using OutputType = decltype(removeLast)::projected_t<InputType>;

    delta<InputType> changes(
      //positive
      vector<InputType>{
        make_tuple(1,2,3)
      },
      //negative
      vector<InputType>{
        make_tuple(1,9,3),
        make_tuple(1,6,3)
      }
    );

    auto assertions = [](const delta<InputType>& changesIn, const delta<OutputType>& changesOut, const actor&, event_based_actor*) {
      BOOST_CHECK(changesIn.positive.size() == changesOut.positive.size());

      auto positives = boost::make_iterator_range(
        boost::make_zip_iterator(boost::make_tuple(std::begin(changesIn.positive), std::begin(changesOut.positive))),
        boost::make_zip_iterator(boost::make_tuple(std::end(changesIn.positive), std::end(changesOut.positive)))
      );

      for(auto&& positive : positives) {
        const auto& in = boost::get<0>(positive);
        const auto& out = boost::get<1>(positive);

        BOOST_CHECK( get<0>(in) == get<0>(out) );
        BOOST_CHECK( get<1>(in) == get<1>(out) );

      }

      BOOST_CHECK(changesIn.negative.size() == changesOut.negative.size());

      auto negatives = boost::make_iterator_range(
        boost::make_zip_iterator(boost::make_tuple(std::begin(changesIn.negative), std::begin(changesOut.negative))),
        boost::make_zip_iterator(boost::make_tuple(std::end(changesIn.negative), std::end(changesOut.negative)))
      );

      for(auto&& negative : negatives) {
        const auto& in = boost::get<0>(negative);
        const auto& out = boost::get<1>(negative);

        BOOST_CHECK( get<0>(in) == get<0>(out) );
        BOOST_CHECK( get<1>(in) == get<1>(out) );

      }
    };

    scoped_actor self;

    auto tester_actor = self->spawn(tester<actor, delta<InputType>, delta<OutputType>, decltype(assertions)>,
                              spawn(sprincle::map<InputType, decltype(removeLast)>::behavior, removeLast),
                              changes, assertions);

    self->await_all_other_actors_done();

}
/**********************************************************************************************************/
/* FILTER TESTS*/


/*
 * Tests
 */
BOOST_AUTO_TEST_CASE( FilterTestCase_0 ) {

    using ChangeType = tuple<long, long, long>;

    delta<ChangeType> changes(
        //positive
        vector<ChangeType>{
        make_tuple(1,2,3)
      },
      //negative
      vector<ChangeType>{
        make_tuple(9,9,9),
        make_tuple(1,6,3)
      }
    );

    auto assertions = [](const delta<ChangeType>&, const delta<ChangeType>& changesOut, const actor&, event_based_actor*){
      BOOST_CHECK(changesOut.positive.size() == 0);
      BOOST_CHECK(changesOut.negative.size() == 1);
    };

    scoped_actor self;

    auto tester_actor = self->spawn(tester<actor, delta<ChangeType>, delta<ChangeType>, decltype(assertions)>,
                                    spawn(filter<ChangeType, sprincle::forall_equals>::behavior, sprincle::forall_equals()),
                                    changes, assertions);

    self->await_all_other_actors_done();

}

/**
 * Test Predicate Evaluator Node
 */
BOOST_AUTO_TEST_CASE( FilterTestCase_1 ) {

  using ChangeType = tuple<long, long, long>;

  delta<ChangeType> changes(
    //positive
    vector<ChangeType>{
      make_tuple(1,2,3)
    },
    //negative
  vector<ChangeType>{
    make_tuple(9,9,9),
    make_tuple(1,6,3)
    }
  );


  auto assertions = [](const delta<ChangeType>&, const delta<ChangeType>& changesOut, const actor&, event_based_actor*){
    BOOST_CHECK( changesOut.positive.size() == 1 );
    BOOST_CHECK( changesOut.negative.size() == 0 );

    BOOST_CHECK( changesOut.positive[0] == make_tuple(1,2,3) );
  };

  scoped_actor self;

  auto tester_actor = self->spawn(tester<actor, delta<ChangeType>, delta<ChangeType>, decltype(assertions)>,
                                  spawn(filter<ChangeType, sprincle::exactly<ChangeType>>::behavior, exactly<ChangeType>(make_tuple(1,2,3))),
                                  changes, assertions);

  self->await_all_other_actors_done();

}




BOOST_AUTO_TEST_CASE( JoinTestCase_0 ) {

  using ChangeTypeP = tuple<int, long, long>;
  using ChangeTypeS = tuple<int, string, string>;

  scoped_actor self;

  auto testee = self->spawn<sprincle::join<ChangeTypeP, ChangeTypeS, match_pair<0,0>>>();

  self->link_to(testee);

  using ResultTuple = typename sprincle::join<ChangeTypeP, ChangeTypeS, match_pair<0,0>>::result_tuple_t;

  using ResultType = delta<ResultTuple>;

  delta<ChangeTypeP> primary_1(
    vector<ChangeTypeP>{
      make_tuple(1,1l,1l),
      make_tuple(2,2l,2l)
    },
    vector<ChangeTypeP>{}
  );

  self->sync_send(testee, primary_atom::value, primary_1).await(
    [=](const ResultType& result) {

      BOOST_TEST_MESSAGE( !result.positive.size() );
      BOOST_TEST_MESSAGE( !result.negative.size() );
    }
  );

  delta<ChangeTypeS> secondary_1(
    vector<ChangeTypeS>{
      make_tuple(1,string("England"),string("London")),
      make_tuple(1,string("Hungary"),string("Budapest"))
    },
    vector<ChangeTypeS>{}
  );

  self->sync_send(testee, secondary_atom::value, secondary_1).await(
    [=](const ResultType& result) {

      BOOST_CHECK( (result.positive.size()==2) );
      BOOST_CHECK( !result.negative.size() );

      //can blow up
      BOOST_CHECK( (result.positive[0] == make_tuple(1,1l,1l,string("England"),string("London"))) );
      BOOST_CHECK( (result.positive[1] == make_tuple(1,1l,1l,string("Hungary"),string("Budapest"))) );
    }
  );

  delta<ChangeTypeS> secondary_2(
    vector<ChangeTypeS>{ },
    vector<ChangeTypeS>{
      make_tuple(1,string("England"),string("London"))
    }
  );

  self->sync_send(testee, secondary_atom::value, secondary_2).await(
    [=](const ResultType& result) {

      BOOST_CHECK( !result.positive.size() );
      BOOST_CHECK( (result.negative.size() == 1) );

      //can blow up
      BOOST_CHECK( (result.negative[0] == make_tuple(1,1l,1l,string("England"),string("London"))) );
    }
  );

  self->send_exit(testee, 0);

}

BOOST_AUTO_TEST_SUITE_END( )