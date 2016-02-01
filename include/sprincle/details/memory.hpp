#ifndef SPRINCLE_MEMORY_HEADER
#define SPRINCLE_MEMORY_HEADER

#include <map>

using namespace std;

namespace sprincle {
  namespace details {

    ///
    /// memory is a stateful indexer used to store values for beta nodes.
    ///
    template<
      class Key,
      class PrimaryValue,
      class SecondaryValue
    >
    struct memory {

      using key_t = Key;
      using primary_value_t = PrimaryValue;
      using secondary_value_t = SecondaryValue;

      multimap<key_t, primary_value_t> primary_indexer;
      multimap<key_t, secondary_value_t> secondary_indexer;

    };

  }
}

#endif // SPRINCLE_MEMORY_HEADER
