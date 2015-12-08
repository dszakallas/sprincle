#
# THIS SHOULD BE EXTRACTED TO ANOTHER PROJECT
# LATER
#


find_package(CAF COMPONENTS core io)
if(CAF_FOUND)
    include_directories(${CAF_INCLUDE_DIRS})
endif()

find_package (Boost COMPONENTS system filesystem unit_test_framework program_options)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    set(LD_FLAGS ${LD_FLAGS} ${Boost_SYSTEM_LIBRARY})
    add_definitions (-DBOOST_TEST_DYN_LINK)
endif()

find_package(CPPNETLIB COMPONENTS uri)
if(CPPNETLIB_FOUND)
    include_directories(${CPPNETLIB_INCLUDE_DIRS})
endif()

find_package(Raptor)
if(RAPTOR_FOUND)
    include_directories(${RAPTOR_INCLUDE_DIRS})
endif()

set(TRAINBENCHMARK_MODEL_PATH "/home/david/Projects/trainbenchmark/models" CACHE PATH "Trainbenchmark model path")

configure_file(config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


## Add benchmark executable
##add_executable (LoadModelTest test_load_model.cpp)
#add_executable (RouteSensorQueryTest test_route_sensor_query.cpp)
add_executable (RouteSensorQueryCLI route_sensor_query_cli.cpp)

## Link benchmark executable to libraries
##target_link_libraries(LoadModelTest ${LD_DIRS})
# target_link_libraries(RouteSensorQueryTest
#   Sprincle
#   ${CAF_LIBRARY_CORE}
#   ${CAF_LIBRARY_IO}
#   ${Boost_NETWORK_LIBRARY}
#   ${Boost_FILESYSTEM_LIBRARY}
#   ${Boost_SYSTEM_LIBRARY}
#   ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
#   ${CPPNETLIB_LIBRARIES}
#   ${RAPTOR_LIBRARIES})
target_link_libraries(RouteSensorQueryCLI
  Sprincle
  ${CAF_LIBRARY_CORE}
  ${CAF_LIBRARY_IO}
  ${Boost_NETWORK_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${CPPNETLIB_LIBRARIES}
  ${RAPTOR_LIBRARIES})

  configure_file(measure_queries.py measure_queries.py COPYONLY)
  install(FILES measure_queries.py DESTINATION bin)
