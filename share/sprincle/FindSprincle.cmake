#
# Copyright 2016 David Szakallas <david.szakallas[AT]gmail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
# - Neither the name of Eyescale Software GmbH nor the names of its
# contributors may be used to endorse or promote products derived from this
# software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# =============================================================================
#
# Sprincle
#
#==============================================================================
# This module uses the following input variables:
#
# SPRINCLE_ROOT - Path to the Sprincle module
#
# This module defines the following output variables:
#
# SPRINCLE_FOUND - Was Sprincle and all of the specified components found?
#
# SPRINCLE_INCLUDE_DIRS - Where to find the headers
#
# Sprincle is a header only library
#==============================================================================
#

# Assume not found.
set(SPRINCLE_FOUND FALSE)

# Find headers
find_path(SPRINCLE_INCLUDE_DIR sprincle/all.hpp
  HINTS ${Sprincle_ROOT}/include $ENV{Sprincle_ROOT}/include
  ${COMMON_SOURCE_DIR}/sprincle ${CMAKE_SOURCE_DIR}/sprincle
  /usr/local/include
  /usr/include)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sprincle DEFAULT_MSG SPRINCLE_INCLUDE_DIR)

if(SPRINCLE_FOUND)
  set(SPRINCLE_INCLUDE_DIRS ${CPPNETLIB_INCLUDE_DIR})
else()
  set(SPRINCLE_FOUND)
  set(SPRINCLE_INCLUDE_DIR)
  set(SPRINCLE_INCLUDE_DIRS)
endif()
