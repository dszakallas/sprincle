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

#set(TEST_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
#configure_file(test_config.h.in test_config.h @ONLY)
#include_directories(${CMAKE_CURRENT_BINARY_DIR})


set(LD_DIRS
        ${LD_DIRS}
        Sprincle
        ${CAF_LIBRARY_CORE}
        ${CAF_LIBRARY_IO}
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY})

#target_link_libraries (DetailTest ${LD_DIRS})
#target_link_libraries (ReteTest ${LD_DIRS})
#target_link_libraries (LoadModelTest ${LD_DIRS})