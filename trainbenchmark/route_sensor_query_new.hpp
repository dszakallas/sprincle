
#ifndef SPRINCLE_ROUTE_SENSOR_QUERY_HPP
#define SPRINCLE_ROUTE_SENSOR_QUERY_HPP

#include <tuple>
#include <boost/network/uri.hpp>
#include <sprincle/rete.hpp>
#include <caf/all.hpp>

#include <iostream>

using namespace sprincle;
using namespace std;
using namespace caf;
namespace net = boost::network;

namespace sprincle {

  using start = caf::atom_constant<caf::atom("start")>;

  struct route_sensor_network : public event_based_actor {

    using edge_t = tuple<unsigned long, unsigned long>;
    using in_0_t = buffered_input_node<edge_t, 10000, primary>;
    using in_1_t = buffered_input_node<edge_t, 10000, secondary>;
    using join_node_0_t = join_node<edge_t, edge_t, primary, match_pair<0,1>>;
    using join_node_1_t = join_node<join_node_0_t::result_tuple_t, edge_t, primary, match_pair<1,0>>;
    using antijoin_node_0_t = antijoin_node<join_node_1_t::result_tuple_t, edge_t, primary, match_pair<2,0>, match_pair<3,1>>;
    using output_t = antijoin_node_0_t::result_tuple_t;

    const string filename;

    const actor antijoin_node_0;
    const actor join_node_1;
    const actor join_node_0;

    const actor in_definedBy;
    const actor in_sensor;
    const actor in_follows;
    const actor in_switch;

    enum {
      input_size = 4
    };

    route_sensor_network(const string& filename, const actor& production) :
      filename(filename),
      antijoin_node_0(spawn<antijoin_node_0_t>(production)),
      join_node_1(spawn<join_node_1_t>(antijoin_node_0)),
      join_node_0(spawn<join_node_0_t>(join_node_1)),
      in_definedBy(spawn<in_1_t>(antijoin_node_0)),
      in_sensor(spawn<in_1_t>(join_node_1)),
      in_follows(spawn<in_1_t>(join_node_0)),
      in_switch(spawn<in_0_t>(join_node_0))
    {
      this->link_to(production);
    }

    route_sensor_network(const route_sensor_network&) = delete;
    route_sensor_network(route_sensor_network&&) = delete;
    route_sensor_network& operator=(const route_sensor_network&) = delete;

    caf::behavior make_behavior() override {
      return caf::behavior {
        [&](start){
          using namespace std::placeholders;
          read_turtle(filename, bind(&route_sensor_network::on_triple, this, _1, _2, _3));

          send(in_switch, io_end::value);
          send(in_follows, io_end::value);
          send(in_sensor, io_end::value);
          send(in_definedBy, io_end::value);
        }
      };
    }

    void on_triple(string&& subject, string&& predicate, string&& object) {

      if(net::uri::uri(predicate).fragment() == string("switch")) {
        send(
          in_switch,
          primary::value,
          edge_t(
            stol(net::uri::uri(subject).fragment().substr(1)),
            stol(net::uri::uri(object).fragment().substr(1))
          ));
      } else if(net::uri::uri(predicate).fragment() == string("follows")) {
        send(
          in_follows,
          primary::value,
          edge_t(
            stol(net::uri::uri(subject).fragment().substr(1)),
            stol(net::uri::uri(object).fragment().substr(1))
          ));
      } else if(net::uri::uri(predicate).fragment() == string("sensor")) {
        send(
          in_sensor,
          primary::value,
          edge_t(
            stol(net::uri::uri(subject).fragment().substr(1)),
            stol(net::uri::uri(object).fragment().substr(1))
          ));
      } else if(net::uri::uri(predicate).fragment() == string("definedBy")) {
        send(
          in_definedBy,
          primary::value,
          edge_t(
            stol(net::uri::uri(subject).fragment().substr(1)),
            stol(net::uri::uri(object).fragment().substr(1))
          ));
      }
    }
  };
}

#endif
