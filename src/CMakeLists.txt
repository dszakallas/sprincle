
find_package(CAF 0.14.1 COMPONENTS core io)

if(CAF_FOUND)
    include_directories(${CAF_INCLUDE_DIRS})
endif()

add_library(
        Sprincle
        rete.cpp
)
target_link_libraries (
        Sprincle
        ${CAF_LIBRARY_CORE}
        ${CAF_LIBRARY_IO}
        ${RAPTOR_LIBRARIES}
)
