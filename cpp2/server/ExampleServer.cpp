/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cpp2/server/StorageService.h>
#include <folly/init/Init.h>
#include <folly/portability/GFlags.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp/thrift_config.h>
#include <glog/logging.h>
#include <proxygen/httpserver/HTTPServerOptions.h>
#include <thrift/lib/cpp2/server/ThriftProcessor.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/http2/common/HTTP2RoutingHandler.h>

DEFINE_int32(storage_port, 7779, "Storage Server port");

using apache::thrift::HTTP2RoutingHandler;
using apache::thrift::ThriftServer;
using apache::thrift::ThriftServerAsyncProcessorFactory;
using example::storage::StorageServiceHandler;
using proxygen::HTTPServerOptions;

std::unique_ptr<HTTP2RoutingHandler>
createHTTP2RoutingHandler(std::shared_ptr<ThriftServer> server) {
  auto h2_options = std::make_unique<HTTPServerOptions>();
  h2_options->threads = static_cast<size_t>(server->getNumIOWorkerThreads());
  h2_options->idleTimeout = server->getIdleTimeout();
  h2_options->shutdownOn = {SIGINT, SIGTERM};
  return std::make_unique<HTTP2RoutingHandler>(
      std::move(h2_options), server->getThriftProcessor(), *server);
}

template <typename ServiceHandler>
std::shared_ptr<ThriftServer> newServer(int32_t port) {
  auto handler = std::make_shared<ServiceHandler>();
  auto proc_factory =
      std::make_shared<ThriftServerAsyncProcessorFactory<ServiceHandler>>(
          handler);
  auto server = std::make_shared<ThriftServer>();
  server->setPort(port);
  server->setProcessorFactory(proc_factory);
  server->addRoutingHandler(createHTTP2RoutingHandler(server));

  // server->setupThreadManager();
  return server;
}

int main(int argc, char **argv) {
  FLAGS_logtostderr = 1;
  folly::init(&argc, &argv);


  auto storage_server = newServer<StorageServiceHandler>(FLAGS_storage_port);
  std::shared_ptr<apache::thrift::concurrency::ThreadManager> threadManager(
      apache::thrift::concurrency::PriorityThreadManager::newPriorityThreadManager(8));
  threadManager->setNamePrefix("executor");
  threadManager->start();
  storage_server->setThreadManager(threadManager);
  LOG(INFO) << "Storage Server running on port: " << FLAGS_storage_port;
  storage_server->serve();

  return 0;
}
