//
// Created by david on 10/22/15.
//

#ifndef SPRINCLE_LOAD_MODEL_H
#define SPRINCLE_LOAD_MODEL_H

#include <vector>
#include <string>
#include <tuple>
using namespace std;

namespace sprincle {
  vector<tuple<string, string, string>> read_turtle(string filename);
  vector<tuple<string, string, string>> read_ntriples(string filename);
}


#endif //SPRINCLE_LOAD_MODEL_H

