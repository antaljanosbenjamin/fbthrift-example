#include "if/gen-cpp2/StorageServiceAsyncClient.h"
#include <chrono>
#include <folly/Executor.h>
#include <folly/Unit.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/executors/ThreadedExecutor.h>
#include <folly/executors/thread_factory/NamedThreadFactory.h>
#include <folly/init/Init.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <if/gen-cpp2/StorageService.h>
#include <iostream>
#include <ratio>
#include <thread>
#include <thrift/lib/cpp2/async/ClientStreamBridge.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>

#define PRINT 1

#ifdef NDEBUG
#define PRINT 0
#endif

DEFINE_string(host, "::1", "Storage Server host");
DEFINE_int32(port, 7779, "Storage Server port");

using example::storage::PropertyRequest;
using example::storage::PropertyValue;
using example::storage::StorageServiceAsyncClient;

int main(int argc, char *argv[]) {
  FLAGS_logtostderr = true;
  folly::init(&argc, &argv);
  auto threadFactory = std::make_shared<folly::NamedThreadFactory>("io-thread");
  auto ioThreadPool = std::make_shared<folly::IOThreadPoolExecutor>(
      8, std::move(threadFactory));

  // Create a Thrift client.
  std::unique_ptr<StorageServiceAsyncClient> client{nullptr};

  ioThreadPool->getEventBase()->runInEventBaseThreadAndWait([&ioThreadPool,
                                                             &client]() {
    try {
      auto socket = folly::AsyncSocket::UniquePtr(new folly::AsyncSocket(
          ioThreadPool->getEventBase(), FLAGS_host, FLAGS_port));
      auto channel =
          apache::thrift::HeaderClientChannel::newChannel(std::move(socket));
      channel->setTimeout(0);

      client = std::make_unique<StorageServiceAsyncClient>(std::move(channel));
    } catch (const std::exception &) {
    }
  });

  try {
    // Send a chat message via a Thrift request.
    auto getPropertyRequest = example::storage::PropertyRequest();
    getPropertyRequest.name_ref() = "first ";
    getPropertyRequest.count_ref() = 10;
#if PRINT
    LOG(INFO) << "Sending...";
#endif
    constexpr auto message_count = 10000;
    std::atomic<size_t> received_count{0};
    LOG(INFO) << "Sending " << message_count << " messages...";
    for (int i = 0; i < message_count; ++i) {
      getPropertyRequest.name_ref() = "world " + std::to_string(i);
      client->future_GetPropertyStream(getPropertyRequest)
          .via(ioThreadPool->getEventBase())
          .then([&received_count](
                    folly::Try<std::vector<PropertyValue>> &&reply) {
            if (reply.hasException()) {
              std::cout << "FAILED: "
                        << reply.exception().get_exception()->what();
              ++received_count;
              return;
            }
            for (const auto &value : *reply) {
#if PRINT
              LOG(INFO) << "Client received: " << value.get_string_v();
#endif
            }
#if PRINT
            LOG(INFO) << "Server Response Completed";
#endif
            ++received_count;
          });
#if PRINT
      LOG(INFO) << "Single message sent! " << *getPropertyRequest.name_ref();
#endif
    }

    LOG(INFO) << "Sent!";
    while (received_count.load() != message_count) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };
    LOG(INFO) << "Received!";
  } catch (apache::thrift::transport::TTransportException &ex) {
    LOG(ERROR) << "Request failed " << ex.what();
  }

  if (client != nullptr) {
    ioThreadPool->getEventBase()->runInEventBaseThreadAndWait(
        [client = std::move(client)]() mutable { delete client.release(); });
    client = nullptr;
  }
}
