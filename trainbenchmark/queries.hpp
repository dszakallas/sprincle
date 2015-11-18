//
// Created by david on 11/18/15.
//
#include <tuple>
#include <sprincle/rete.hpp>
#include <caf/all.hpp>

using namespace sprincle;
using namespace std;
using namespace caf;

namespace sprincle {

  struct route_sensor_network {

    using edge_t = tuple<unsigned long, unsigned long>;
    using in_0_t = input_node<edge_t, primary_slot>;
    using in_1_t = input_node<edge_t, secondary_slot>;
    using join_node_0_t = join_node<edge_t, edge_t, primary_slot, match_pair<0,1>>;
    using join_node_1_t = join_node<join_node_0_t::result_tuple_t, edge_t, primary_slot, match_pair<1,0>>;
    using antijoin_node_0_t = antijoin_node<join_node_1_t::result_tuple_t, edge_t, primary_slot, match_pair<2,0>, match_pair<3,1>>;

    const actor antijoin_node_0;
    const actor join_node_1;
    const actor join_node_0;

    const actor in_definedBy;
    const actor in_sensor;
    const actor in_follows;
    const actor in_switch;

    route_sensor_network(const actor& production) :
      antijoin_node_0(spawn<antijoin_node_0_t>(production)),
      join_node_1(spawn<join_node_1_t>(antijoin_node_0)),
      join_node_0(spawn<join_node_0_t>(join_node_1)),
      in_definedBy(spawn<in_1_t>(antijoin_node_0)),
      in_sensor(spawn<in_1_t>(join_node_1)),
      in_follows(spawn<in_1_t>(join_node_0)),
      in_switch(spawn<in_0_t>(join_node_0))
    {}

  };
}
