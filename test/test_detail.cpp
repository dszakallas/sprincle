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
using namespace sprincle::detail;
using namespace sprincle::filters;

BOOST_AUTO_TEST_SUITE( DetailTestSuite )

BOOST_AUTO_TEST_CASE( ProjectionShouldWork_0 )
{
  auto _0 = 5;
  auto _1 = 6;
  auto _2 = 7;

  auto actual = std::make_tuple(_0, _1, _2);
  auto expected = project<0, 1, 2>(actual);
  BOOST_CHECK( expected == actual );

}

BOOST_AUTO_TEST_CASE( ProjectionShouldWork_1 )
{
  auto _0 = std::string("duck");
  auto _1 = 4;
  auto _2 = 7;

  auto actual = project<0, 1>(std::make_tuple(_0, _1, _2));

  BOOST_CHECK( get<0>(actual) == _0 );
  BOOST_CHECK( get<1>(actual) == _1 );

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

BOOST_AUTO_TEST_SUITE_END()