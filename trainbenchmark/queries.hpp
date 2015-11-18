//
// Created by david on 11/18/15.
//
#include <tuple>
#include <sprincle/rete.hpp>
#include <caf/all.hpp>

using namespace sprincle;
using namespace std;
using namespace caf;

decltype(auto) make_route_sensor_query(const actor& production) noexcept {

  using edge_t = tuple<unsigned long, unsigned long>;
  using in_0_t = input_node<edge_t, primary_slot>;
  using in_1_t = input_node<edge_t, secondary_slot>;
  using join_node_0_t = join_node<edge_t, edge_t, primary_slot, match_pair<0,1>>;
  using join_node_1_t = join_node<join_node_0_t::result_tuple_t, edge_t, primary_slot, match_pair<1,0>>;
  using antijoin_node_0_t = antijoin_node<join_node_1_t::result_tuple_t, edge_t, primary_slot, match_pair<2,0>, match_pair<3,1>>;

  auto antijoin_node_0 = spawn<antijoin_node_0_t>(production);
  auto join_node_1 = spawn<join_node_1_t>(antijoin_node_0);
  auto join_node_0 = spawn<join_node_0_t>(join_node_1);

  auto in_definedBy = spawn<in_1_t>(antijoin_node_0);
  auto in_sensor = spawn<in_1_t>(join_node_1);
  auto in_follows = spawn<in_1_t>(join_node_0);
  auto in_switch = spawn<in_0_t>(join_node_0);

  return make_tuple(in_switch, in_follows, in_sensor, in_definedBy);

}
