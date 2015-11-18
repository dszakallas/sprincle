//
// Created by david on 11/11/15.
//

#ifndef SPRINCLE_PRETTY_TUPLE_H
#define SPRINCLE_PRETTY_TUPLE_H

#include <tuple>
#include <utility>
#include <iostream>
#include <sstream>

using namespace std;

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
template<class... Args>
decltype(auto) tuple_to_str(const std::tuple<Args...>& t) {
  stringstream str;
  str << t;
  str.flush();
  return str.str();
}

#endif //SPRINCLE_PRETTY_TUPLE_H
