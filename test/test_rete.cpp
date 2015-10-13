//
// Created by david on 10/13/15.
//
#define BOOST_TEST_MODULE ReteTest

#include <boost/test/unit_test.hpp>
#include <sprincle/rete.hpp>
#include <utility>
#include <tuple>
#include <string>
#include <utility>
#include <caf/all.hpp>

using namespace std;
using namespace sprincle;

template <class Handle, class Changeset>
void tester(event_based_actor* self, const Handle& testee, const Changeset& changes) {
  self->link_to(testee);
  // will be invoked if we receive an unexpected response message
  self->on_sync_failure([=] {
    BOOST_FAIL("Actor failed");
    self->quit(exit_reason::user_shutdown);
  });
  self->sync_send(testee, changes).then(
    [=](auto changed) {
      BOOST_CHECK( true );
    }
  );
}

BOOST_AUTO_TEST_SUITE( ReteTestSuite )

BOOST_AUTO_TEST_CASE( TrimmerTestCase_0 ) {


    scoped_actor self;

    using InputType = tuple<int, int, int>;

    changeset<int, int, int> changes(
      set<InputType> { make_tuple(1,2,3) },
      set<InputType> { make_tuple(4,5,6) }
    );

    auto trimmer_actor = spawn(trimmer<tuple<int, int, int>, index_sequence<0, 1>>());
    auto tester_actor = spawn(tester<actor, changeset<int, int, int>>, trimmer_actor, changes); //ERROR decltype cannot resolve address of overloaded function


    self->await_all_other_actors_done();

}

BOOST_AUTO_TEST_SUITE_END( )
