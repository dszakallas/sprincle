//
// Created by david on 10/13/15.
//
#define BOOST_TEST_MODULE ReteTest

#include <utility>
#include <tuple>
#include <set>

#include <caf/all.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range.hpp>

#include <sprincle/rete.hpp>
#include <sprincle/detail.hpp>

using namespace std;
using namespace sprincle;

BOOST_AUTO_TEST_SUITE( ReteTestSuite )

/**********************************************************************************************************/
/* TRIMMER TESTS*/

BOOST_AUTO_TEST_CASE( TrimmerTestCase_0 ) {

    using InputChange = tuple<int, int, int>;

    auto removeLast = sprincle::project<0, 1>();

    using OutputChange = typename decltype(removeLast)::template projected_t<InputChange>;

    delta<InputChange> changesIn(
      //positive
      set<InputChange>{
        make_tuple(1,2,3)
      },
      //negative
      set<InputChange>{
        make_tuple(1,9,3),
        make_tuple(1,6,3)
      }
    );

    scoped_actor self;

    auto trimmer_actor = spawn_map_node<InputChange>(removeLast, self, primary_slot::value);

    self->send(trimmer_actor, primary_slot::value, changesIn);

    // check trimmer_actor's response
    self->receive(
      on<primary_slot, delta<OutputChange>>() >> [&](primary_slot, const delta<OutputChange>& changesOut){
        BOOST_CHECK( changesIn.positive.size() == changesOut.positive.size() );

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
      },
      others >> [=] {
        BOOST_FAIL( "Unexpected message" );
      }
    );
    self->send_exit(trimmer_actor, exit_reason::user_shutdown);


}
/**********************************************************************************************************/
/* FILTER TESTS*/

/*
 * Tests
 */
BOOST_AUTO_TEST_CASE( FilterTestCase_0 ) {

    using Change = tuple<long, long, long>;

    delta<Change> changes(
      //positive
      set<Change>{
        make_tuple(1,2,3)
      },
      //negative
      set<Change>{
        make_tuple(9,9,9),
        make_tuple(1,6,3)
      }
    );

    scoped_actor self;

    auto filter_actor = spawn_filter_node<Change>(forall_equals(), self, primary_slot::value);

    self->send(filter_actor, primary_slot::value, changes);

    self->receive(
      on<primary_slot, delta<Change>>() >> [&](primary_slot, const delta<Change>& changesOut){
        BOOST_CHECK(changesOut.positive.size() == 0);
        BOOST_CHECK(changesOut.negative.size() == 1);
      },
      others >> [=] {
        BOOST_FAIL( "Unexpected message" );
      }
    );

    self->send_exit(filter_actor, exit_reason::user_shutdown);

}

/**
 * Test Predicate Evaluator Node
 */
BOOST_AUTO_TEST_CASE( FilterTestCase_1 ) {

  using Change = tuple<long, long, long>;

  delta<Change> changes(
    //positive
    set<Change>{
      make_tuple(1,2,3)
    },
    //negative
    set<Change>{
      make_tuple(9,9,9),
      make_tuple(1,6,3)
    }
  );

  scoped_actor self;

  auto filter_actor = spawn_filter_node<Change>(exactly(make_tuple(1,2,3)), self, primary_slot::value);

  self->send(filter_actor, primary_slot::value, changes);

  self->receive(
    on<primary_slot, delta<Change>>() >> [&](primary_slot, const delta<Change>& changesOut){
      BOOST_CHECK( changesOut.positive.size() == 1 );
      BOOST_CHECK( changesOut.negative.size() == 0 );
      BOOST_CHECK( changesOut.positive.find(make_tuple(1,2,3)) != end(changesOut.positive) );
    },
    others >> [=] {
      BOOST_FAIL( "Unexpected message" );
    }
  );

  self->send_exit(filter_actor, exit_reason::user_shutdown);

}

BOOST_AUTO_TEST_CASE( JoinTestCase_0 ) {

  using PrimaryChange = tuple<int, long, long>;
  using SecondaryChange = tuple<int, string, string>;

  scoped_actor self;

  auto testee = spawn_join_node<PrimaryChange, SecondaryChange, match_pair<0,0>>(self, primary_slot::value);

  //TODO clean this up later
  using ResultChange = typename join_node<PrimaryChange, SecondaryChange, primary_slot, match_pair<0,0>>::result_tuple_t;

  using Result = delta<ResultChange>;

  delta<PrimaryChange> primary_1(
    set<PrimaryChange>{
      make_tuple(1,1l,1l),
      make_tuple(2,2l,2l)
    },
    set<PrimaryChange>{}
  );

  self->send(testee, primary_slot::value, primary_1);

  delta<SecondaryChange> secondary_1(
    set<SecondaryChange>{
      make_tuple(1,string("England"),string("London")),
      make_tuple(1,string("Hungary"),string("Budapest"))
    },
    set<SecondaryChange>{}
  );

  self->send(testee, secondary_slot::value, secondary_1);

  self->receive(
    on<primary_slot, Result>() >> [&](primary_slot, const Result& result) {
      BOOST_CHECK( (result.positive.size()==2) );
      BOOST_CHECK( !result.negative.size() );

      BOOST_CHECK( (result.positive.find(make_tuple(1,1l,1l,string("England"),string("London"))) != end(result.positive)) );
      BOOST_CHECK( (result.positive.find(make_tuple(1,1l,1l,string("Hungary"),string("Budapest"))) != end(result.positive)) );
    },
    others >> [=] {
      BOOST_FAIL( "Unexpected message" );
    }
  );

  delta<SecondaryChange> secondary_2(
    set<SecondaryChange>{ },
    set<SecondaryChange>{
      make_tuple(1,string("England"),string("London"))
    }
  );

  self->send(testee, secondary_slot::value, secondary_2);

  self->receive(
    on<primary_slot, Result>() >> [&](primary_slot, const Result& result) {
      BOOST_CHECK( !result.positive.size() );
      BOOST_CHECK( (result.negative.size() == 1) );

      //can blow up
      BOOST_CHECK( (result.negative.find(make_tuple(1,1l,1l,string("England"),string("London"))) != end(result.negative)) );
    },
    others >> [=] {
      BOOST_FAIL( "Unexpected message" );
    }
  );
  self->send_exit(testee, exit_reason::user_shutdown);

}

BOOST_AUTO_TEST_CASE( AntijoinTestCase_0 ) {

  using PrimaryChange = tuple<int, long, long>;
  using SecondaryChange = tuple<int, string, string>;

  scoped_actor self;

  auto testee = spawn_antijoin_node<PrimaryChange, SecondaryChange, match_pair<0,0>>(self, primary_slot::value);

  using ResultChange = typename antijoin_node<PrimaryChange, SecondaryChange, primary_slot, match_pair<0,0>>::result_tuple_t;

  using Result = delta<ResultChange>;

  delta<PrimaryChange> primary_1(
    set<PrimaryChange>{
      make_tuple(1,1l,1l),
      make_tuple(2,2l,2l)
    },
    set<PrimaryChange>{}
  );

  self->send(testee, primary_slot::value, primary_1);

  self->receive(
    on<primary_slot, delta<ResultChange>>() >> [=](primary_slot, const Result& result) {
      BOOST_CHECK( (result.positive.size()==2) );
      BOOST_CHECK( !result.negative.size() );

      BOOST_CHECK( (result.positive.find(make_tuple(1,1l,1l)) != end(result.positive)) );
      BOOST_CHECK( (result.positive.find(make_tuple(2,2l,2l)) != end(result.positive)) );
    },
    others >> [=] {
      BOOST_FAIL( "Unexpected message" );
    }
  );

  delta<SecondaryChange> secondary_1(
    set<SecondaryChange>{
      make_tuple(1,string("England"),string("London")),
      make_tuple(1,string("Hungary"),string("Budapest"))
    },
    set<SecondaryChange>{}
  );

  self->send(testee, secondary_slot::value, secondary_1);

  self->receive(
    on<primary_slot, delta<ResultChange>>() >> [=](primary_slot, const Result& result) {
      BOOST_CHECK( !result.positive.size() );
      BOOST_CHECK( (result.negative.size()==1) );

      BOOST_CHECK( (result.negative.find(make_tuple(1,1l,1l)) != end(result.negative)) );
    },
    others >> [=] {
      BOOST_FAIL( "Unexpected message" );
    }
  );

  self->send_exit(testee, exit_reason::user_shutdown);

}

BOOST_AUTO_TEST_SUITE_END( )
