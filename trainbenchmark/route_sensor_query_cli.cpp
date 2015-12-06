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

#include "config.h"
#include "pretty_tuple.hpp"

#include "measure.hpp"
#include "load_model.hpp"
#include "route_sensor_query.hpp"

namespace po = boost::program_options;

void route_sensor_query(const string& filename) {
  using ulong = unsigned long;

  using edge_t = tuple<ulong, ulong>;
  using output_t = tuple<ulong, ulong, ulong, ulong>;

  scoped_actor self;
  route_sensor::network network(self);
  typename delta<output_t>::changeset_t matches;
  size_t inputs_closed = 0;
  size_t calls = 0;

  auto read = measure<>::duration([&]{

    using namespace std::placeholders;

    read_turtle(filename, bind(route_sensor::on_triple<decltype(self)>, cref(self), cref(network), _1, _2, _3));

    self->send(network.in_switch, io_end::value);
    self->send(network.in_follows, io_end::value);
    self->send(network.in_sensor, io_end::value);
    self->send(network.in_definedBy, io_end::value);

    self->do_receive(
      on<primary, delta<output_t>>() >> [&](primary, const delta<output_t>& output) {
        cout << ++calls << endl;
        matches.erase(begin(output.negative), end(output.negative));
        matches.insert(begin(output.positive), end(output.positive));
      },
      on<io_end>() >> [&](io_end){
        ++inputs_closed;
      },
      after(chrono::seconds(5)) >> [&]{
        self->quit();
      }
    ).until([&] { return inputs_closed == network.input_size; });

  });

  auto check = measure<>::duration([&]{
    cout << "Matches:" << endl;
    cout << "--------" << endl;
    for(const auto& match : matches)
      cout << match << endl;
  });

  cout << "Read: " << read.count() << endl;
  cout << "Check: " << check.count() << endl;

}

int main(int argc, char* argv[]) {

  po::options_description desc("Allowed options");
  desc.add_options()
      ("help", "produce help message")
      ("file", po::value<string>(), "load model file")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
      cout << desc << "\n";
      return 1;
  }

  if (vm.count("file")) {
    route_sensor_query(vm["file"].as<string>());
  } else {
      cout << "Filename required.\n";
  }


}
