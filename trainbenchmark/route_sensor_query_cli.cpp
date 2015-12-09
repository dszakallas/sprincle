#include <boost/network/uri.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <caf/all.hpp>
#include <tuple>
#include <string>
#include <utility>
#include <sstream>
#include <iostream>
#include <chrono>
#include <functional>
#include <cerrno>
#include <list>
#include <thread>

#include "config.h"
#include "pretty_tuple.hpp"

#include "measure.hpp"
#include "load_model.hpp"
#include "route_sensor_query.hpp"

namespace po = boost::program_options;

//callback on triple read
void parse_triple(
  list<delta<route_sensor::edge_t>>& in_switch,
  list<delta<route_sensor::edge_t>>& in_sensor,
  list<delta<route_sensor::edge_t>>& in_follows,
  list<delta<route_sensor::edge_t>>& in_definedBy,
  string&& subject, string&& predicate, string&& object) {

  if(net::uri::uri(predicate).fragment() == string("switch")) {
    in_switch.push_back(
      delta<route_sensor::edge_t> {
        set<route_sensor::edge_t> {
          make_tuple(
            stol(net::uri::uri(subject).fragment().substr(1)),
            stol(net::uri::uri(object).fragment().substr(1))
          ),
        },
        set<route_sensor::edge_t>{}
      });
  } else if(net::uri::uri(predicate).fragment() == string("follows")) {
    in_follows.push_back(
      delta<route_sensor::edge_t> {
        set<route_sensor::edge_t> {
          make_tuple(
            stol(net::uri::uri(subject).fragment().substr(1)),
            stol(net::uri::uri(object).fragment().substr(1))
          )
        },
        set<route_sensor::edge_t>{}
      });
  } else if(net::uri::uri(predicate).fragment() == string("sensor")) {
    in_sensor.push_back(
      delta<route_sensor::edge_t> {
        set<route_sensor::edge_t> {
          make_tuple(
            stol(net::uri::uri(subject).fragment().substr(1)),
            stol(net::uri::uri(object).fragment().substr(1))
          )
        },
        set<route_sensor::edge_t>{}
      });
  } else if(net::uri::uri(predicate).fragment() == string("definedBy")) {
    in_definedBy.push_back(
      delta<route_sensor::edge_t> {
        set<route_sensor::edge_t> {
          make_tuple(
            stol(net::uri::uri(subject).fragment().substr(1)),
            stol(net::uri::uri(object).fragment().substr(1))
          )
        },
        set<route_sensor::edge_t>{}
      });
  }
}

void route_sensor_query(const string& filename) {

  using edge_t = route_sensor::edge_t;
  using output_t = route_sensor::output_t;

  scoped_actor self;
  route_sensor::network network(self);
  list<delta<edge_t>> in_switch;
  list<delta<edge_t>> in_sensor;
  list<delta<edge_t>> in_follows;
  list<delta<edge_t>> in_definedBy;
  typename delta<output_t>::changeset_t matches;
  size_t inputs_closed = 0;
  size_t calls = 0;

  using namespace std::placeholders;

  read_turtle(filename, bind(parse_triple,
    ref(in_switch), ref(in_sensor),
    ref(in_follows), ref(in_definedBy), _1, _2, _3));

  auto read_time = measure<>::duration([&]{

    thread a([&]{
      for(const auto& e : in_switch) {
        self->send(network.in_switch, primary::value, e);
      }
      self->send(network.in_switch, io_end::value);
    });
    thread b([&]{
      for(const auto& e : in_sensor) {
        self->send(network.in_sensor, primary::value, e);
      }
      self->send(network.in_sensor, io_end::value);
    });
    thread c([&]{
      for(const auto& e : in_follows) {
        self->send(network.in_follows, primary::value, e);
      }
      self->send(network.in_follows, io_end::value);
    });
    thread d([&]{
      for(const auto& e : in_definedBy) {
        self->send(network.in_definedBy, primary::value, e);
      }
      self->send(network.in_definedBy, io_end::value);
    });

    self->do_receive(
      on<primary, delta<output_t>>() >> [&](primary, const delta<output_t>& output) {
        for(const auto& negative : output.negative)
          matches.erase(negative);

        matches.insert(begin(output.positive), end(output.positive));
      },
      on<io_end>() >> [&](io_end){
        ++inputs_closed;
      }
      // },
      // after(chrono::seconds(5)) >> [&]{
      //   self->quit();
      // }
    ).until([&] { return inputs_closed == network.input_size; });

    a.join();
    b.join();
    c.join();
    d.join();



  });


  auto check_time = measure<>::duration([&]{
    matches.size();
  });

  cout << read_time.count() << endl;
  cout << check_time.count() << endl;
  cout << matches.size() << endl;

}

int main(int argc, char* argv[]) {

  po::options_description desc("Allowed options");
  desc.add_options()
      ("help", "produce help message")
      ("file", po::value<string>(), "load model file")
  ;

  po::positional_options_description pd;
  pd.add("file", 1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(pd).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
      cout << desc << "\n";
      return 1;
  }

  if (vm.count("file")) {
    route_sensor_query(vm["file"].as<string>());
  }
}
