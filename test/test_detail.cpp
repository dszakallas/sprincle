//
// Created by david on 10/13/15.
//

#define BOOST_TEST_MODULE DetailTest

#include <boost/test/unit_test.hpp>
#include <sprincle/detail.hpp>

#include <tuple>
#include <string>
#include <utility>

using namespace std;
using namespace sprincle;

BOOST_AUTO_TEST_SUITE( DetailTestSuite )

BOOST_AUTO_TEST_CASE( ProjectionShouldWork_0 )
{

  auto _0 = 5;
  auto _1 = 6;
  auto _2 = 7;

  auto input = std::make_tuple(_0, _1, _2);

  auto preserve = make_project<make_index_sequence<tuple_size<decltype(input)>::value>>();

  auto output = preserve(input);

  BOOST_CHECK( output == input );

}

BOOST_AUTO_TEST_CASE( ProjectionShouldWork_1 )
{
  auto _0 = std::string("duck");
  auto _1 = 4;
  auto _2 = 7;

  auto trim = project<1>();

  auto actual = trim(std::make_tuple(_0, _1, _2));

  BOOST_CHECK( get<0>(actual) == _1 );

}

BOOST_AUTO_TEST_CASE( ElementsEquals_0 )
{
  auto _0 = 9;
  auto _1 = 4;
  auto _2 = 7;

  auto compare = forall_equals();

  auto equals = compare(std::make_tuple(_0, _1, _2));

  BOOST_CHECK( ! equals );

}

BOOST_AUTO_TEST_CASE( ElementsEquals_1 )
{
  auto _0 = 0;
  auto _1 = 0;
  auto _2 = 0;

  auto compare = forall_equals();


  auto equals = compare(std::make_tuple(_0, _1, _2));

  BOOST_CHECK( equals );

}

BOOST_AUTO_TEST_CASE( ElementsNotEqual_0 )
{
  auto _0 = 0;
  auto _1 = 0;
  auto _2 = 0;

  auto compare = exists_not_equal();

  auto not_equals = compare(std::make_tuple(_0, _1, _2));

  BOOST_CHECK( ! not_equals );

}

BOOST_AUTO_TEST_CASE( ElementsNotEqual_1 )
{
  auto _0 = 0;
  auto _1 = 2;
  auto _2 = 3;

  auto compare = exists_not_equal();

  auto not_equals = compare(std::make_tuple(_0, _1, _2));

  BOOST_CHECK( not_equals );

}

BOOST_AUTO_TEST_CASE( Exactly_1 )
{
  auto _0 = 0;
  auto _1 = 2;
  auto _2 = 3;

  auto compare = exactly<tuple<int, int, int>>(
    std::make_tuple(_0, _1, _2));


  auto the_same = compare(std::make_tuple(_0, _1, _2));

  BOOST_CHECK( the_same );

}

BOOST_AUTO_TEST_CASE( Exactly_2 )
{
  auto _0 = 0;
  auto _1 = 2;
  auto _2 = 3;

  auto compare = make_exactly(std::make_tuple(_0, _1, _2));

  auto the_same = compare(std::make_tuple(_0, _1, _2));

  BOOST_CHECK( the_same );

}
BOOST_AUTO_TEST_SUITE_END()