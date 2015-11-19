//
// Created by david on 10/22/15.
//

#ifndef SPRINCLE_LOAD_MODEL_HPP
#define SPRINCLE_LOAD_MODEL_HPP

#include <raptor2/raptor2.h>
#include <cstdio>
#include <vector>
#include <string>
#include <tuple>
#include <iostream>
#include <sstream>
#include <cerrno>
using namespace std;

namespace sprincle {

  template<class callback_t>
  void read_turtle(const string& filename, const callback_t& callback) {

    raptor_world *world = raptor_new_world();
    // Whyyyy, whyyyy use unsigned char* for strings? >@
    raptor_uri *syntax_uri = raptor_new_uri(world, (const unsigned char*)("http://www.dajobe.org/2004/01/turtle/"));
    raptor_parser *rdf_parser = raptor_new_parser_for_content(world, syntax_uri, nullptr, nullptr, 0, nullptr);

    // let me shed a tear for this --------------------\
    //                                                 V
    raptor_parser_set_statement_handler(rdf_parser, &const_cast<callback_t&>(callback), [](void *callback, raptor_statement *triple) {

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

      string subject(read(triple->subject));
      string predicate(read(triple->predicate));
      string object(read(triple->object));

      (*(const callback_t*)callback)(move(subject), move(predicate), move(object));
    });

    /* We have to create a base URI from the filename to make the parser work */
    unsigned char *filename_uri_str = raptor_uri_filename_to_uri_string(filename.c_str());
    raptor_uri *base_uri = raptor_new_uri(world, filename_uri_str);

    FILE *file = fopen(filename.c_str(), "r");

    if(!file) {
      raptor_free_uri(base_uri);
      raptor_free_memory(filename_uri_str);
      raptor_free_parser(rdf_parser);
      raptor_free_uri(syntax_uri);
      raptor_free_world(world);
      throw ENOENT;
    }

    raptor_parser_parse_file_stream(rdf_parser, file, filename.c_str(), base_uri);

    fclose(file);
    raptor_free_uri(base_uri);
    raptor_free_memory(filename_uri_str);
    raptor_free_parser(rdf_parser);
    raptor_free_uri(syntax_uri);
    raptor_free_world(world);
  }

  namespace {

    string raptor_log_level_to_str(raptor_log_level log_level) {

      string str { "" };

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

    string pretty_raptor_log_message(raptor_log_message* log_message) {
      stringstream stream;

      if (log_message->level != RAPTOR_LOG_LEVEL_NONE) {
        stream << raptor_log_level_to_str(log_message->level) << ": ";
      }
      stream << log_message->text;
      stream.flush();
      return stream.str();
    }
  }
}


#endif //SPRINCLE_LOAD_MODEL_HPP
