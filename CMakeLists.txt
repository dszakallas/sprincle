cmake_minimum_required(VERSION 3.2)
project(sprincle C CXX)

###############################################################################
#      HELP                                                                   #
###############################################################################
#
# This CMakeLists.txt file helps defining your block builds
# To learn more visit http://docs.biicode.com/c++.html
#
# To include published cmake scripts:
#   1. INCLUDE(user/block/myrecipe) # include myrecipe.cmake from remote user/block
#   2. Remember to execute bii find
#   Example:
#      INCLUDE(biicode/cmake/tools) # Include tools.cmake file from cmake block from "biicode" user
#      ACTIVATE_CPP11(INTERFACE ${BII_BLOCK_TARGET})
#
# Useful variables:
#   To be modified BEFORE the call to ADD_BII_TARGETS()
#     ${BII_LIB_SRC}  File list to create the library
#
#   To be modified AFTER the call to ADD_BII_TARGETS()
#     ${BII_BLOCK_TARGET}  Interface (no files) target for convenient configuration of all
#                          targets in this block, as the rest of targets always depend on it
#                          has name in the form "user_block_interface"
#     ${BII_LIB_TARGET}  Target library name, usually in the form "user_block". May not exist
#                        if BII_LIB_SRC is empty
#     ${BII_BLOCK_TARGETS} List of all targets defined in this block
#     ${BII_BLOCK_EXES} List of executables targets defined in this block
#     ${BII_exe_name_TARGET}: Executable target (e.g. ${BII_main_TARGET}. You can also use
#                            directly the name of the executable target (e.g. user_block_main)
#
# > EXAMPLE: Add include directories to all targets of this block
#
#    TARGET_INCLUDE_DIRECTORIES(${BII_BLOCK_TARGET} INTERFACE myincludedir)
#
# > EXAMPLE: Link with pthread:
#
#    TARGET_LINK_LIBRARIES(${BII_BLOCK_TARGET} INTERFACE pthread)
#        or link against library:
#    TARGET_LINK_LIBRARIES(${BII_LIB_TARGET} PUBLIC pthread)
#
#    NOTE:  This can be also done adding pthread to ${BII_LIB_DEPS}
#            BEFORE calling ADD_BIICODE_TARGETS()
#
# > EXAMPLE: how to activate C++11
#
#    IF(APPLE)
#         TARGET_COMPILE_OPTIONS(${BII_BLOCK_TARGET} INTERFACE "-std=c++11 -stdlib=libc++")
#    ELSEIF (WIN32 OR UNIX)
#         TARGET_COMPILE_OPTIONS(${BII_BLOCK_TARGET} INTERFACE "-std=c++11")
#    ENDIF(APPLE)
#
# > EXAMPLE: Set properties to target
#
#    SET_TARGET_PROPERTIES(${BII_BLOCK_TARGET} PROPERTIES COMPILE_DEFINITIONS "IOV_MAX=255")
#


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

################################################################################
#                                compiler setup                                #
################################################################################
# check for g++ >= 4.7 or clang++ > = 3.2
if(NOT WIN32 AND NOT CAF_NO_COMPILER_CHECK)
  try_run(ProgramResult
          CompilationSucceeded
          "${CMAKE_CURRENT_BINARY_DIR}"
          "${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_compiler_version.cpp"
          RUN_OUTPUT_VARIABLE CompilerVersion)
  if(NOT CompilationSucceeded OR NOT ProgramResult EQUAL 0)
    message(FATAL_ERROR "Cannot determine compiler version")
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    if(CompilerVersion VERSION_GREATER 4.6)
      message(STATUS "Found g++ version ${CompilerVersion}")
    else()
      message(FATAL_ERROR "g++ >= 4.7 required (found: ${CompilerVersion})")
    endif()
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    if(CompilerVersion VERSION_GREATER 3.1)
      message(STATUS "Found clang++ version ${CompilerVersion}")
    else()
      message(FATAL_ERROR "clang++ >= 3.2 required (found: ${CompilerVersion})")
    endif()
  else()
    message(FATAL_ERROR "Your C++ compiler does not support C++11 "
                        "or is not supported")
  endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


################################################################################
#                    check for biicode build pipeline                          #
################################################################################
IF(BIICODE)

  # define the biicode target and let it do its thing
  ADD_BII_TARGETS()

  # but first specify the compiled/linker options calculated above
  string(REPLACE " " ";" REPLACED_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  target_compile_options(${BII_BLOCK_TARGET} INTERFACE ${REPLACED_CXX_FLAGS})
  
  
  RETURN()
  
ENDIF()
################################################################################
#                           configure dependencies                             #
################################################################################

find_package(CAF 0.14.1 COMPONENTS core io)

if(CAF_FOUND)
    include_directories(${CAF_INCLUDE_DIRS})
    set(LD_DIRS ${LD_DIRS} ${CAF_LIBRARIES})
endif()


find_package(Boost)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    set(LD_DIRS ${LD_DIRS} ${Boost_LIBRARIES})
    set(LD_FLAGS ${LD_FLAGS} ${Boost_SYSTEM_LIBRARY})
endif()

################################################################################
#                                  add targets                                 #
################################################################################


set(SOURCE_FILES main.cpp)
add_executable(sprincle ${SOURCE_FILES})
target_link_libraries (sprincle ${LD_DIRS})





