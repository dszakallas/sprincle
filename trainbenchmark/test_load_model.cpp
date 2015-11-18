//
// Created by david on 10/22/15.
//

#define BOOST_TEST_MODULE LoadModelTest

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "load_model.h"
#include "config.h"
#include "pretty_tuple.hpp"

#include <tuple>
#include <string>
#include <utility>
#include <sstream>
#include <iostream>

using namespace std;
namespace fs = boost::filesystem;

BOOST_AUTO_TEST_SUITE( LoadModelTestSuite )

BOOST_AUTO_TEST_CASE( ShouldLoadRailwayMetamodel ) {

    fs::path root(TRAINBENCHMARK_ROOT);

    fs::path metamodel_file("/resources/railway-metamodel.ttl");

    fs::path full_path = root / metamodel_file;

    auto triples = sprincle::read_turtle(full_path.string());

    BOOST_CHECK( triples.size() > 0 );

    BOOST_TEST_MESSAGE( "##############");
    BOOST_TEST_MESSAGE( "TUPLES FOUND: ");
    BOOST_TEST_MESSAGE( "##############");

    for(const auto& triple : triples) {
      BOOST_TEST_MESSAGE( tuple_to_str(triple) );
    }

}

BOOST_AUTO_TEST_CASE( ShouldLoadRouteSensorMetamodel ) {

  //TODO EDIT THIS
  fs::path root(TRAINBENCHMARK_ROOT);

  fs::path metamodel_file("/resources/railway_minimal_routesensor_metamodel.ttl");

  fs::path full_path = root / metamodel_file;

  auto triples = sprincle::read_turtle(full_path.string());

  BOOST_CHECK( triples.size() > 0 );

  BOOST_TEST_MESSAGE( "##############");
  BOOST_TEST_MESSAGE( "TUPLES FOUND: ");
  BOOST_TEST_MESSAGE( "##############");

  for(const auto& triple : triples) {
    BOOST_TEST_MESSAGE( tuple_to_str(triple) );
  }

}
BOOST_AUTO_TEST_SUITE_END()
