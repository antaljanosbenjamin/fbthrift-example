# Copyright (c) Facebook, Inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


project(ThriftTransport_CPP2_Example CXX)
cmake_minimum_required(VERSION 3.20)


list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)


find_package(Gflags)
find_package(Folly)
find_package(wangle)
find_package(fizz)
find_package(proxygen)
find_package(FBThrift)
include_directories(AFTER "$ENV{HOME}/.local/include")
include_directories(".")
set(THRIFTCPP2 "FBThrift::thriftcpp2")
set(THRIFT1 ${FBTHRIFT_COMPILER})

set(CMAKE_CXX_STANDARD 17)

include(${FBTHRIFT_INCLUDE_DIR}/thrift/ThriftLibrary.cmake)

add_subdirectory(if)
add_subdirectory(cpp2)
