//
// Created by david on 10/13/15.
//
#define BOOST_TEST_MODULE ReteTest

#include <utility>
#include <tuple>
#include <set>
#include <string>

#include <caf/all.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range.hpp>

#include <sprincle/details/project.hpp>
#include <sprincle/details/exactly.hpp>
#include <sprincle/map_node.hpp>
#include <sprincle/filter_node.hpp>
#include <sprincle/join_node.hpp>
#include <sprincle/antijoin_node.hpp>
#include <sprincle/match_pair.hpp>
#include <sprincle/message_atoms.hpp>

using namespace std;
using namespace sprincle;

BOOST_AUTO_TEST_SUITE( ReteTestSuite )

BOOST_AUTO_TEST_CASE( TrimmerTestCase_0 ) {

  using input_change_t = tuple<int, int, int>;

  using remove_last_t = details::project<input_change_t, 0, 1>;

  remove_last_t remove_last;

  using output_change_t = remove_last_t::output_t;

  delta<input_change_t> changesIn(
    //positive
    set<input_change_t>{
      make_tuple(1,2,3)
    },
    //negative
    set<input_change_t>{
      make_tuple(1,9,3),
      make_tuple(1,6,3)
    }
  );

  scoped_actor self;

  auto trimmer_actor = spawn_map_node<input_change_t, remove_last_t>(remove_last, self, primary_message_t::value);

  self->link_to(trimmer_actor);

  self->send(trimmer_actor, primary_message_t::value, changesIn);

  // check trimmer_actor's response
  self->receive(
    on<primary_message_t, delta<output_change_t>>() >> [&](primary_message_t, const delta<output_change_t>& changesOut){
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
}

BOOST_AUTO_TEST_CASE( FilterTestCase_0 ) {

  using change_t = tuple<long, long, long>;

  delta<change_t> changes(
    //positive
    set<change_t>{
      make_tuple(1,2,3)
    },
    //negative
    set<change_t>{
      make_tuple(9,9,9),
      make_tuple(1,6,3)
    }
  );

  scoped_actor self;

  auto filter_actor = spawn_filter_node<change_t>(details::make_exactly(make_tuple(1,2,3)), self, primary_message_t::value);

  self->link_to(filter_actor);

  self->send(filter_actor, primary_message_t::value, changes);

  self->receive(
    on<primary_message_t, delta<change_t>>() >> [&](primary_message_t, const delta<change_t>& changesOut){
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

  using primary_change_t = tuple<int, long, long>;
  using secondary_change_t = tuple<int, string, string>;

  scoped_actor self;

  auto testee = spawn_join_node<primary_change_t, secondary_change_t, match_pair<0,0>>(self, primary_message_t::value);
  self->link_to(testee);


  //TODO clean this up later
  using result_change_t = typename join_node<primary_change_t, secondary_change_t, primary_message_t, match_pair<0,0>>::output_t;

  using result_t = delta<result_change_t>;

  delta<primary_change_t> primary_1(
    set<primary_change_t>{
      make_tuple(1,1l,1l),
      make_tuple(2,2l,2l)
    },
    set<primary_change_t>{}
  );

  self->send(testee, primary_message_t::value, primary_1);

  delta<secondary_change_t> secondary_1(
    set<secondary_change_t>{
      make_tuple(1,string("England"),string("London")),
      make_tuple(1,string("Hungary"),string("Budapest"))
    },
    set<secondary_change_t>{}
  );

  self->send(testee, secondary_message_t::value, secondary_1);

  self->receive(
    on<primary_message_t, result_t>() >> [&](primary_message_t, const result_t& result_t) {
      BOOST_CHECK( (result_t.positive.size()==2) );
      BOOST_CHECK( !result_t.negative.size() );

      BOOST_CHECK( (result_t.positive.find(make_tuple(1,1l,1l,string("England"),string("London"))) != end(result_t.positive)) );
      BOOST_CHECK( (result_t.positive.find(make_tuple(1,1l,1l,string("Hungary"),string("Budapest"))) != end(result_t.positive)) );
    },
    others >> [=] {
      BOOST_FAIL( "Unexpected message" );
    }
  );

  delta<secondary_change_t> secondary_2(
    set<secondary_change_t>{ },
    set<secondary_change_t>{
      make_tuple(1,string("England"),string("London"))
    }
  );

  self->send(testee, secondary_message_t::value, secondary_2);

  self->receive(
    on<primary_message_t, result_t>() >> [&](primary_message_t, const result_t& result_t) {
      BOOST_CHECK( !result_t.positive.size() );
      BOOST_CHECK( (result_t.negative.size() == 1) );

      //can blow up
      BOOST_CHECK( (result_t.negative.find(make_tuple(1,1l,1l,string("England"),string("London"))) != end(result_t.negative)) );
    },
    others >> [=] {
      BOOST_FAIL( "Unexpected message" );
    }
  );

}

BOOST_AUTO_TEST_CASE( AntijoinTestCase_0 ) {

  using primary_change_t = tuple<int, long, long>;
  using secondary_change_t = tuple<int, string, string>;

  scoped_actor self;

  auto testee = spawn_antijoin_node<primary_change_t, secondary_change_t, match_pair<0,0>>(self, primary_message_t::value);

  using result_change_t = typename antijoin_node<primary_change_t, secondary_change_t, primary_message_t, match_pair<0,0>>::output_t;

  using result_t = delta<result_change_t>;

  delta<primary_change_t> primary_1(
    set<primary_change_t>{
      make_tuple(1,1l,1l),
      make_tuple(2,2l,2l)
    },
    set<primary_change_t>{}
  );

  self->link_to(testee);

  self->send(testee, primary_message_t::value, primary_1);

  self->receive(
    on<primary_message_t, delta<result_change_t>>() >> [=](primary_message_t, const result_t& result_t) {
      BOOST_CHECK( (result_t.positive.size()==2) );
      BOOST_CHECK( !result_t.negative.size() );

      BOOST_CHECK( (result_t.positive.find(make_tuple(1,1l,1l)) != end(result_t.positive)) );
      BOOST_CHECK( (result_t.positive.find(make_tuple(2,2l,2l)) != end(result_t.positive)) );
    },
    others >> [=] {
      BOOST_FAIL( "Unexpected message" );
    }
  );

  delta<secondary_change_t> secondary_1(
    set<secondary_change_t>{
      make_tuple(1,string("England"),string("London")),
      make_tuple(1,string("Hungary"),string("Budapest"))
    },
    set<secondary_change_t>{}
  );

  self->send(testee, secondary_message_t::value, secondary_1);

  self->receive(
    on<primary_message_t, delta<result_change_t>>() >> [=](primary_message_t, const result_t& result_t) {
      BOOST_CHECK( !result_t.positive.size() );
      BOOST_CHECK( (result_t.negative.size()==1) );

      BOOST_CHECK( (result_t.negative.find(make_tuple(1,1l,1l)) != end(result_t.negative)) );
    },
    others >> [=] {
      BOOST_FAIL( "Unexpected message" );
    }
  );

}

BOOST_AUTO_TEST_SUITE_END( )
