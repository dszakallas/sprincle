#ifndef SPRINCLE_MEASURE_HPP
#define SPRINCLE_MEASURE_HPP

#include <chrono>

template<class time_t = std::chrono::nanoseconds>
struct measure
{
  template<class F, class ...Args>
  static auto duration(F&& func, Args&&... args)
  {
      auto start = std::chrono::high_resolution_clock::now();
      std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
      return std::chrono::duration_cast<time_t>(std::chrono::high_resolution_clock::now()-start);
  }
};

#endif
