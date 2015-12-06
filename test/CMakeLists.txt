find_package(CAF 0.14.1 COMPONENTS core io)

if(CAF_FOUND)
    include_directories(${CAF_INCLUDE_DIRS})
endif()

find_package (Boost COMPONENTS system filesystem unit_test_framework)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    set(LD_FLAGS ${LD_FLAGS} ${Boost_SYSTEM_LIBRARY})
endif()

add_definitions (-DBOOST_TEST_DYN_LINK)


add_executable (DetailTest test_detail.cpp)
add_executable (ReteTest test_rete.cpp)

set(TEST_LIBRARIES
  Sprincle
  ${CAF_LIBRARY_CORE}
  ${CAF_LIBRARY_IO}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

target_link_libraries (DetailTest ${TEST_LIBRARIES})
target_link_libraries (ReteTest ${TEST_LIBRARIES})
