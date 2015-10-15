//
// Created by david on 10/13/15.
//
#define BOOST_TEST_MODULE ReteTest

#include <utility>
#include <tuple>
#include <utility>
#include <list>

#include <caf/all.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range.hpp>

#include <sprincle/rete.hpp>

using namespace std;
using namespace sprincle;

BOOST_AUTO_TEST_SUITE( ReteTestSuite )


template <class Handle, class ChangesIn, class ChangesOut>
void tester(event_based_actor* self, const Handle& testee, const ChangesIn& changesIn) {
  self->link_to(testee);
  // will be invoked if we receive an unexpected response message
  self->on_sync_failure(
    [=] {
      BOOST_FAIL("Unexpected response message");
      self->quit(exit_reason::user_shutdown);
    }
  );
  // this is where the assertions happen
  self->sync_send(testee, changesIn).then([=](const ChangesOut& changesOut){

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

    // Duplication. Make changeset iterable?

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

    self->quit(exit_reason::user_shutdown);

  });
}

BOOST_AUTO_TEST_CASE( TrimmerTestCase_0 ) {

    scoped_actor self;

    using InputType = tuple<int, int, int>;
    using OutputType = tuple<int, int>;

    changeset<InputType> changes(
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

    auto tester_actor = self->spawn(tester<actor, changeset<InputType>, changeset<OutputType>>,
                              spawn(trimmer<tuple<int, int, int>, 0, 1>::behavior),
                              changes);

    self->await_all_other_actors_done();

}

BOOST_AUTO_TEST_SUITE_END( )