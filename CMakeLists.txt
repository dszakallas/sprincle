cmake_minimum_required(VERSION 3.2)
project(sprincle C CXX)

# silence policy CMP0042 warning by enabling RPATH explicitly
if(APPLE AND NOT DEFINED CMAKE_MACOSX_RPATH)
  set(CMAKE_MACOSX_RPATH true)
endif()

# prohibit in-source builds
if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please use "
                        "./configure to choose a build directory and "
                        "initialize the build configuration.")
endif()

# set module path appropriately
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# set binary output path if not defined by user
if("${EXECUTABLE_OUTPUT_PATH}" STREQUAL "")
  set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin")
endif()
# set library output path if not defined by user, but always set
# library output path to binary output path for Xcode projects
if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
  set(LIBRARY_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}")
elseif("${LIBRARY_OUTPUT_PATH}" STREQUAL "")
  set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib")
endif()

set(CMAKE_BUILD_TYPE Debug)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")


################################################################################
#                    check for biicode build pipeline                          #
################################################################################
IF(BIICODE)

  # define the biicode target and let it do its thing
  ADD_BII_TARGETS()

  # but first specify the compiled/linker options calculated above
  string(REPLACE " " ";" REPLACED_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  target_compile_options(${BII_BLOCK_TARGET} INTERFACE ${REPLACED_CXX_FLAGS})

  include(biicode/boost/setup)

  bii_find_boost(COMPONENTS boost_test REQUIRED)

  RETURN()

ENDIF()

include_directories(include)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(trainbenchmark)
enable_testing()
set(TEST_ARGS --log_level=all)



add_test(NAME DetailTest COMMAND DetailTest ${TEST_ARGS})
add_test(NAME ReteTest COMMAND ReteTest ${TEST_ARGS})
add_test(NAME LoadModelTest COMMAND LoadModelTest ${TEST_ARGS})





