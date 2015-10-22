//
// Created by david on 10/21/15.
//
#include <raptor2/raptor2.h>
#include <cstdio>
#include <vector>
#include <string>
#include <tuple>
#include <iostream>
#include <sprincle/load_model.h>

using namespace std;
using namespace sprincle;

namespace sprincle {

  vector<tuple<string, string, string>> read_turtle(string filename) {

    raptor_world *world = raptor_new_world();

    raptor_uri *syntax_uri = raptor_new_uri(world, (const unsigned char*)("http://www.dajobe.org/2004/01/turtle/"));

    raptor_parser *rdf_parser = raptor_new_parser_for_content(world, syntax_uri, nullptr, nullptr, 0, nullptr);

    vector<tuple<string, string, string>> triples;

    raptor_parser_set_statement_handler(rdf_parser, &triples, [](void *user_data_, raptor_statement *triple) {
      //vector<tuple<string, string, string>>* user_data = dynamic_cast<vector<tuple<string, string, string>>*>(user_data_);

      cout << "handler called" << endl;

      auto read = [](raptor_term *term) {
        auto type = term->type;
        if (type == RAPTOR_TERM_TYPE_URI) {
          return string((const char*)raptor_uri_as_string(term->value.uri));
        } else if (type == RAPTOR_TERM_TYPE_LITERAL) {
          return string((const char*)term->value.literal.string, term->value.literal.string_len);
        } else if (type == RAPTOR_TERM_TYPE_BLANK) {
          return string((const char*)term->value.blank.string, term->value.blank.string_len);
        } else {
          return string("");
        }
      };

      cout << "start parsing" << endl;

      string subject(read(triple->object));
      string predicate(read(triple->predicate));
      string object(read(triple->object));

      ((vector<tuple<string, string, string>>*)user_data_)->push_back(
        make_tuple(subject, predicate, object));

    });

    FILE *file = fopen(filename.c_str(), "rb");

    raptor_parser_parse_file_stream(rdf_parser, file, filename.c_str(), nullptr);

    fclose(file);
    raptor_free_parser(rdf_parser);
    raptor_free_uri(syntax_uri);
    raptor_free_world(world);

    return triples;

  }
}

