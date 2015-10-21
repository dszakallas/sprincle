//
// Created by david on 10/21/15.
//
#include <raptor2/raptor2.h>
#define MODEL void

MODEL load_model() {

  raptor_world* world = raptor_new_world();


  raptor_uri* syntax_uri = raptor_new_uri(world, "http://www.dajobe.org/2004/01/turtle/");


  rdf_parser* rdf_parser = raptor_new_parser_for_content(world, syntax_uri, nullptr, nullptr, 0, nullptr);


  //TODO



  raptor_free_parser(rdf_parser);

  raptor_free_uri(syntax_uri);

  raptor_free_world(world);


}

