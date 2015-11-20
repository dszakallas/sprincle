#
# THIS SHOULD BE EXTRACTED TO ANOTHER PROJECT
# LATER
#


find_package(CAF 0.14.1 COMPONENTS core io)
if(CAF_FOUND)
    include_directories(${CAF_INCLUDE_DIRS})
endif()

find_package (Boost COMPONENTS system filesystem unit_test_framework)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    set(LD_FLAGS ${LD_FLAGS} ${Boost_SYSTEM_LIBRARY})
    add_definitions (-DBOOST_TEST_DYN_LINK)
endif()

find_package(CPPNETLIB)
if(CPPNETLIB_FOUND)
    include_directories(${CPPNETLIB_INCLUDE_DIRS})
endif()

find_package(Raptor)
if(RAPTOR_FOUND)
    include_directories(${RAPTOR_INCLUDE_DIRS})
endif()

set(TRAINBENCHMARK_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

configure_file(config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LD_DIRS
        ${LD_DIRS}
        Sprincle
        ${CAF_LIBRARY_CORE}
        ${CAF_LIBRARY_IO}
        ${Boost_NETWORK_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        ${CPPNETLIB_LIBRARIES}
        ${RAPTOR_LIBRARIES}
      )

## Add benchmark executable
##add_executable (LoadModelTest test_load_model.cpp)
add_executable (QueriesTest test_queries.cpp)

## Link benchmark executable to libraries
##target_link_libraries(LoadModelTest ${LD_DIRS})
target_link_libraries(QueriesTest ${LD_DIRS})
