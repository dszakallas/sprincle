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

  using output_t = route_sensor_network::output_t;

  scoped_actor self;
  actor network = spawn<route_sensor_network>(filename, self);
  typename delta<output_t>::changeset_t matches;
  size_t inputs_closed = 0;
  size_t calls = 0;

  auto read_time = measure<>::duration([&]{

    using namespace std::placeholders;

    self->send(network, start::value);

    self->do_receive(
      on<primary, delta<output_t>>() >> [&](primary, const delta<output_t>& output) {
        ++calls;
        for(const auto& negative : output.negative)
          matches.erase(negative);

        matches.insert(begin(output.positive), end(output.positive));
      },
      on<io_end>() >> [&](io_end){
        ++inputs_closed;
      }
    ).until([&] { return inputs_closed == 4; });

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
