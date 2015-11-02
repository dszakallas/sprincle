//
// Created by david on 10/21/15.
//
#include <raptor2/raptor2.h>
#include <cstdio>
#include <vector>
#include <string>
#include <tuple>
#include <iostream>
#include <sstream>
#include <sprincle/load_model.h>

using namespace std;
using namespace sprincle;

/* FWD DECLARATIONS */
namespace {
  vector<tuple<string, string, string>> read_turtle_impl(string filename);
  string raptor_log_level_to_str(raptor_log_level log_level);
  string pretty_raptor_log_message(raptor_log_message* log_message);

}


/* PUBLIC INTERFACE */
namespace sprincle {

  vector<tuple<string, string, string>> read_turtle(string filename) {
    return read_turtle_impl(filename);
  };

}

/* IMPLEMENTATION */
namespace {

  vector<tuple<string, string, string>> read_turtle_impl(string filename) {
    raptor_world *world = raptor_new_world(); //1

    // Whyyyy, whyyyy use unsigned char* for strings? >@
    raptor_uri *syntax_uri = raptor_new_uri(world, (const unsigned char*)("http://www.dajobe.org/2004/01/turtle/")); //2

    raptor_parser *rdf_parser = raptor_new_parser_for_content(world, syntax_uri, nullptr, nullptr, 0, nullptr); //3

    vector<tuple<string, string, string>> triples;

    raptor_parser_set_statement_handler(rdf_parser, &triples, [](void *user_data, raptor_statement *triple) {

      auto read = [](raptor_term *term) {
        auto type = term->type;
        if (type == RAPTOR_TERM_TYPE_URI) {
          return string((const char*)raptor_uri_as_string(term->value.uri));
        } else if (type == RAPTOR_TERM_TYPE_LITERAL) {
          return string((const char*)term->value.literal.string, term->value.literal.string_len);
        } else if (type == RAPTOR_TERM_TYPE_BLANK) {
          return string((const char*)term->value.blank.string, term->value.blank.string_len);
        } else {
          //TODO Then what?
          return string("");
        }
      };

      string subject(read(triple->object));
      string predicate(read(triple->predicate));
      string object(read(triple->object));

      ((vector<tuple<string, string, string>>*)user_data) -> push_back(make_tuple(subject, predicate, object));

    });

    raptor_world_set_log_handler(world, nullptr, [](void*, raptor_log_message* message){
      cout << pretty_raptor_log_message(message) << endl;
    });


    /* We have to create a base URI from the filename to make the parser work */
    unsigned char *filename_uri_str = raptor_uri_filename_to_uri_string(filename.c_str()); //4
    raptor_uri *base_uri = raptor_new_uri(world, filename_uri_str); //5

    FILE *file = fopen(filename.c_str(), "r"); //6

    //
    raptor_parser_parse_file_stream(rdf_parser, file, filename.c_str(), base_uri);

    fclose(file); //6
    raptor_free_uri(base_uri); //5
    raptor_free_memory(filename_uri_str); //4
    raptor_free_parser(rdf_parser); //3
    raptor_free_uri(syntax_uri); //2
    raptor_free_world(world); //1

    return triples;
  }

  string pretty_raptor_log_message(raptor_log_message* log_message) {
    stringstream stream;

    if (log_message->level != RAPTOR_LOG_LEVEL_NONE) {
      stream << raptor_log_level_to_str(log_message->level) << ": ";
    }
    stream << log_message->text;
    stream.flush();
    return stream.str();
  }

  string raptor_log_level_to_str(raptor_log_level log_level) {

    std:string str { "" };

    switch (log_level) {
      case RAPTOR_LOG_LEVEL_NONE:                 break;
      case RAPTOR_LOG_LEVEL_TRACE: str = "TRACE"; break;
      case RAPTOR_LOG_LEVEL_DEBUG: str = "DEBUG"; break;
      case RAPTOR_LOG_LEVEL_INFO:  str = "INFO";  break;
      case RAPTOR_LOG_LEVEL_WARN:  str = "WARN";  break;
      case RAPTOR_LOG_LEVEL_ERROR: str = "ERROR"; break;
      case RAPTOR_LOG_LEVEL_FATAL: str = "FATAL"; break;
    }

    return str;
  }

}


