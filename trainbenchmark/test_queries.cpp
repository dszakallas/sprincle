#define BOOST_TEST_MODULE QueriesTest



#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <caf/all.hpp>
#include <tuple>
#include <string>
#include <utility>
#include <sstream>
#include <iostream>
#include <chrono>


#include "config.h"
#include "pretty_tuple.hpp"


#include "load_model.h"
#include "queries.hpp"

using namespace caf;
using namespace std;
using namespace sprincle;
namespace fs = boost::filesystem;

BOOST_AUTO_TEST_SUITE( QueriesTestSuite )

BOOST_AUTO_TEST_CASE( RailwayModelMinimal ) {
  using ulong = unsigned long;

  using edge_t = tuple<ulong, ulong>;
  using output_t = tuple<ulong, ulong, ulong, ulong>;

  delta<edge_t> switch_{ set<edge_t> { make_tuple(4l, 3l) }, set<edge_t> {} };
  delta<edge_t> follows { set<edge_t> { make_tuple(1l, 4l) }, set<edge_t> {} };
  delta<edge_t> sensor { set<edge_t> { make_tuple(3l, 2l) }, set<edge_t> {} };

  scoped_actor self;

  route_sensor_network network(self);

  self->send(network.in_switch, switch_);
  self->send(network.in_follows, follows);
  self->send(network.in_sensor, sensor);

  self->receive(
    on<primary_slot, delta<output_t>>() >> [](primary_slot, const delta<output_t>& output) {
      BOOST_CHECK ( output.positive.find(make_tuple(4l, 3l, 1l, 2l)) != end(output.positive) );
    },
    after(chrono::seconds(5)) >> []{
      BOOST_FAIL( "Timeout" );
    }
  );

}

BOOST_AUTO_TEST_SUITE_END( )
