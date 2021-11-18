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


add_executable(
  example_server
  server/StorageService.h
  server/StorageService.cpp
  server/ExampleServer.cpp
)
target_link_libraries(
  example_server

  storage-cpp2
  ${LIBGFLAGS_LIBRARY}
  ${GLOG_LIBRARIES}
)
install(
  TARGETS example_server
  RUNTIME DESTINATION bin/cpp2/
)

add_executable(
  storage_client

  client/StorageClient.cpp
)
target_link_libraries(
  storage_client

  storage-cpp2
)