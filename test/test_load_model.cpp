//
// Created by david on 10/22/15.
//

#define BOOST_TEST_MODULE LoadModelTest

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <sprincle/load_model.h>

#include <tuple>
#include <string>
#include <utility>
#include <iostream>

using namespace std;
namespace fs = boost::filesystem;

// pretty-print a tuple (from http://stackoverflow.com/a/6245777/273767)
template<class Ch, class Tr, class Tuple, std::size_t... Is>
void print_tuple_impl(std::basic_ostream<Ch,Tr>& os,
                      const Tuple & t,
                      std::index_sequence<Is...>)
{
  using swallow = int[]; // guaranties left to right order
  (void)swallow{0, (void(os << (Is == 0? "" : ", ") << std::get<Is>(t)), 0)...};
}

template<class Ch, class Tr, class... Args>
decltype(auto) operator<<(std::basic_ostream<Ch, Tr>& os,
                          const std::tuple<Args...>& t)
{
  os << "<";
  print_tuple_impl(os, t, make_index_sequence<sizeof...(Args)>{});
  return os << ">";
}

BOOST_AUTO_TEST_SUITE( LoadModelTestSuite )

BOOST_AUTO_TEST_CASE( ShouldLoadRailwayMetamodel ) {

    //TODO EDIT THIS
    fs::path root("/home/david/Projects/sprincle");

    fs::path railway_metamodel_file("test/resources/railway-metamodel.ttl");

    fs::path full_path = root / railway_metamodel_file;

    auto triples = sprincle::read_turtle(full_path.string());

    for(const auto& triple : triples) {
      cout << triple << endl;
    }

}
BOOST_AUTO_TEST_SUITE_END()

