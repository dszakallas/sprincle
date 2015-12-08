
#ifndef SPRINCLE_SWITCH_SENSOR_QUERY_HPP
#define SPRINCLE_SWITCH_SENSOR_QUERY_HPP

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

  namespace switch_sensor {

    struct network {


      using switch_input_t = tuple<unsigned long>;
      using sensor_input_t = tuple<unsigned long, unsigned long>;
      using in_0_t = input_node<switch_input_t, primary>;
      using in_1_t = input_node<sensor_input_t, primary>;
      using trimmer_node_t = map_node<in_1_t::result_tuple_t, project<0>, secondary>;
      using antijoin_node_t = antijoin_node<in_0_t::result_tuple_t, trimmer_node_t::result_tuple_t, primary, match_pair<0,0>>;
      using output_t = antijoin_node_t::result_tuple_t;

      const actor antijoin_node_0;
      const actor trimmer_node_0;

      const actor in_sensor;
      const actor in_switch;

      const size_t input_size = 2;

      network(const actor& production) :
        antijoin_node_0(spawn<antijoin_node_t>(production)),
        trimmer_node_0(spawn<trimmer_node_t>(project<0>(), antijoin_node_0)),
        in_sensor(spawn<in_1_t>(trimmer_node_0)),
        in_switch(spawn<in_0_t>(antijoin_node_0))
      {}

      network(const network&) = delete;
      network(network&&) = delete;
      network& operator=(const network&) = delete;

    };

    using switch_input_t = typename network::switch_input_t;
    using sensor_input_t = typename network::sensor_input_t;
    using output_t = typename network::output_t;

    //callback on triple read
    template<class Actor>
    void on_triple(const Actor& self, const network& network, string&& subject, string&& predicate, string&& object) {

      if( predicate == "http://www.w3.org/1999/02/22-rdf-syntax-ns#type"
        && net::uri::uri(object).fragment() == "Switch"){
          self->send(
            network.in_switch,
            primary::value,
            delta<switch_input_t> {
              set<switch_input_t> {
                make_tuple(
                  stol(net::uri::uri(subject).fragment().substr(1))
                )
              },
              set<switch_input_t> { /* no negatives */ }
            });
        } else if(net::uri::uri(predicate).fragment() == string("sensor")) {
          self->send(
            network.in_sensor,
            primary::value,
            delta<sensor_input_t> {
              set<sensor_input_t> {
                make_tuple(
                  stol(net::uri::uri(subject).fragment().substr(1)),
                  stol(net::uri::uri(object).fragment().substr(1))
                )
              },
              set<sensor_input_t> { /* no negatives */ }
            });
        }
    }

  }

}

#endif
