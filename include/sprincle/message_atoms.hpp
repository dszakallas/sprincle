//
// Created by david on 1/23/16.
//

#ifndef SPRINCLE_MESSAGE_ATOMS_HEADER
#define SPRINCLE_MESSAGE_ATOMS_HEADER

namespace sprincle {
  using primary_message_t = caf::atom_constant<caf::atom("primary")>;
  using secondary_message_t = caf::atom_constant<caf::atom("secondary")>;
  using end_message_t = caf::atom_constant<caf::atom("end")>;
}

#endif //SPRINCLE_MESSAGE_ATOMS_HEADER
