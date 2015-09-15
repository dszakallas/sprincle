#include <string>
#include <iostream>
#include <memory>

#include "caf/all.hpp"
#include "gtest/gtest.h"


using namespace std;
using namespace caf;

behavior alice(event_based_actor *self) {


  return {

          [=](int i, const actor &buddy) {
            aout(self) << to_string(i) + "|ping -->" << endl;

            if (i >= 10)
              self->quit();
            self->send(buddy, i, self);
          }
  };
}

behavior bob(event_based_actor *self) {

  return {

          [=](int i, const actor &buddy) {
            aout(self) << string("                   <-- pong|") << endl;
            if (i++ >= 10)
              self->quit();
            self->send(buddy, i, self);
          }
  };
}


int main() {
  auto bob_actor = spawn(bob);
  auto alice_actor = spawn(alice);

  scoped_actor self;
  aout(self) << "Starting up..." << endl;
  self->send(alice_actor, 0, bob_actor);

  self->await_all_other_actors_done();

  shutdown();


}