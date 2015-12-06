#ifndef SPRINCLE_MEASURE_HPP
#define SPRINCLE_MEASURE_HPP

#include <iostream>
#include <chrono>

template<typename TimeT = std::chrono::nanoseconds>
struct measure
{
  template<typename F, typename ...Args>
  static auto duration(F&& func, Args&&... args)
  {
      auto start = std::chrono::high_resolution_clock::now();
      std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
      return std::chrono::duration_cast<TimeT>(std::chrono::high_resolution_clock::now()-start);
  }
};

#endif
