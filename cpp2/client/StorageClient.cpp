#include "if/gen-cpp2/StorageServiceAsyncClient.h"
#include <chrono>
#include <folly/Executor.h>
#include <folly/Unit.h>
#include <folly/init/Init.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <if/gen-cpp2/StorageService.h>
#include <thread>
#include <folly/executors/ThreadedExecutor.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <iostream>

#define PRINT 1

#ifdef NDEBUG
#define PRINT 0
#endif

DEFINE_string(host, "::1", "Storage Server host");
DEFINE_int32(port, 7779, "Storage Server port");

using example::storage::StorageServiceAsyncClient;
using example::storage::PropertyRequest;
using example::storage::PropertyValue;

int main(int argc, char* argv[]) {
  FLAGS_logtostderr = true;
  folly::init(&argc, &argv);

  // Create an EventBase.
  folly::EventBase eventBase;

  // Create a Thrift client.
  auto socket = folly::AsyncSocket::UniquePtr(
      new folly::AsyncSocket(&eventBase, FLAGS_host, FLAGS_port));
  auto channel =
      apache::thrift::RocketClientChannel::newChannel(std::move(socket));
  auto client =
      std::make_unique<StorageServiceAsyncClient>(std::move(channel));

  try {
    // Send a chat message via a Thrift request.
    auto getPropertyRequest = example::storage::PropertyRequest();
    getPropertyRequest.name_ref() = "world ";
    getPropertyRequest.count_ref() = 5;
    std::vector<PropertyValue> sync_reply;
    LOG(INFO) << "Sending...";
    client->sync_GetPropertyStream(sync_reply, getPropertyRequest);

    for(const auto& value: sync_reply)
    {
#if PRINT
        LOG(INFO) << "Client received: " << value.get_string_v();
#endif
    }

    LOG(INFO) << "Sending...";
    client->future_GetPropertyStream(getPropertyRequest).then([](folly::Try<std::vector<PropertyValue>>&& reply){
        if (reply.hasException()) {
            std::cout << "FAILED: " << reply.exception().get_exception()->what();
            return;
        }
        for(const auto& value: *reply)
        {
#if PRINT
            LOG(INFO) << "Client received: " << value.get_string_v();
#endif
            //++*received_message_count_;
        }
#if PRINT
            LOG(INFO) << "Server Response Completed";
#endif
    });
    eventBase.loop();
  } catch (apache::thrift::transport::TTransportException& ex) {
    LOG(ERROR) << "Request failed " << ex.what();
  }
}
