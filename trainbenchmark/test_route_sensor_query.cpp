#define BOOST_TEST_MODULE RouteSensorQueryTestSuite


#include <boost/network/uri.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <caf/all.hpp>
#include <tuple>
#include <string>
#include <utility>
#include <sstream>
#include <iostream>
#include <chrono>
#include <functional>
#include <cerrno>

#include "config.h"
#include "pretty_tuple.hpp"


#include "load_model.hpp"
#include "route_sensor_query.hpp"

using namespace caf;
using namespace std;
using namespace sprincle;
namespace net = boost::network;
namespace fs = boost::filesystem;

BOOST_AUTO_TEST_SUITE( RouteSensorQueryTestSuite )

BOOST_AUTO_TEST_CASE( RailwayModelMinimalHardwired ) {
  using ulong = unsigned long;

  using edge_t = tuple<ulong, ulong>;
  using output_t = tuple<ulong, ulong, ulong, ulong>;

  delta<edge_t> switch_{ set<edge_t> { make_tuple(4l, 3l) }, set<edge_t> {} };
  delta<edge_t> follows { set<edge_t> { make_tuple(1l, 4l) }, set<edge_t> {} };
  delta<edge_t> sensor { set<edge_t> { make_tuple(3l, 2l) }, set<edge_t> {} };

  scoped_actor self;

  route_sensor::network network(self);

  self->send(network.in_switch, primary::value, switch_);
  self->send(network.in_follows, primary::value, follows);
  self->send(network.in_sensor, primary::value, sensor);

  self->send(network.in_switch, io_end::value);
  self->send(network.in_follows, io_end::value);
  self->send(network.in_sensor, io_end::value);
  self->send(network.in_definedBy, io_end::value);

  typename delta<output_t>::changeset_t matches;

  size_t inputs_closed = 0;

  self->do_receive(
    on<primary, delta<output_t>>() >> [&](primary, const delta<output_t>& output) {
      cout << "Got delta" << endl;
      for(const auto& match : output.negative) {
        cout << "N: " << match << endl;
      }
      for(const auto& match : output.positive) {
        cout << "P: " << match << endl;
      }
      matches.erase(begin(output.negative), end(output.negative));
      matches.insert(begin(output.positive), end(output.positive));
    },
    on<io_end>() >> [&](io_end){
      ++inputs_closed;
    },
    after(chrono::seconds(5)) >> [&]{
      self->quit();
      BOOST_FAIL( "Timeout" );
    }
  ).until([&] { return inputs_closed == network.input_size; });

  BOOST_CHECK( matches.size() == 1);

  BOOST_CHECK( matches.find(make_tuple(4l, 3l, 1l, 2l)) != end(matches) );

}
BOOST_AUTO_TEST_CASE( RailwayModelMinimal ) {

  //LOAD turtle
  fs::path root(TRAINBENCHMARK_MODEL_PATH);
  fs::path metamodel_file("railway-minimal-routesensor-inferred.ttl");
  fs::path full_path = root / metamodel_file;

  // Initialize network
  using ulong = unsigned long;
  using edge_t = tuple<ulong, ulong>;
  using output_t = tuple<ulong, ulong, ulong, ulong>;
  scoped_actor self;
  route_sensor::network network(self);

  cout << full_path.string() << endl;


  using namespace std::placeholders;

  try {
    read_turtle(full_path.string(), bind(route_sensor::on_triple<decltype(self)>, cref(self), cref(network), _1, _2, _3));
  } catch(int err) {
    if (err == ENOENT) {
      BOOST_FAIL( "File not found" );
    }
  }

  self->send(network.in_switch, io_end::value);
  self->send(network.in_follows, io_end::value);
  self->send(network.in_sensor, io_end::value);
  self->send(network.in_definedBy, io_end::value);

  typename delta<output_t>::changeset_t matches;

  size_t inputs_closed = 0;

  self->do_receive(
    on<primary, delta<output_t>>() >> [&](primary, const delta<output_t>& output) {
      matches.erase(begin(output.negative), end(output.negative));
      matches.insert(begin(output.positive), end(output.positive));
    },
    on<io_end>() >> [&](io_end){
      ++inputs_closed;
    },
    after(chrono::seconds(5)) >> [&]{
      self->quit();
      BOOST_FAIL( "Timeout" );
    }
  ).until([&] { return inputs_closed == network.input_size; });

  BOOST_CHECK( matches.size() == 1);

  BOOST_CHECK( matches.find(make_tuple(4l, 3l, 1l, 2l)) != end(matches) );

}


BOOST_AUTO_TEST_SUITE_END( )
