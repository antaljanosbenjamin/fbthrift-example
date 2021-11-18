#include "cpp2/server/StorageService.h"

#include <iostream>

#include <glog/logging.h>

#define PRINT 1

#ifdef NDEBUG
#define PRINT 0
#endif

namespace example {
namespace storage {

void StorageServiceHandler::GetProperty(
      PropertyValue& resp,
      std::unique_ptr<PropertyRequest> req) {
#if PRINT
    LOG(INFO) << "Request received " << req->get_name() << std::endl;
#endif
    resp.set_string_v("Property name " + req->get_name());

#if PRINT
    LOG(INFO) << "Sending reply " << resp.get_string_v() << std::endl;
#endif
}

void StorageServiceHandler::GetPropertyStream(
      std::vector<PropertyValue>& resp,
      std::unique_ptr<PropertyRequest> req) {
    constexpr auto kDefaultMessageCount = 1;
    const auto expected_message_count = [&]() -> int64_t{
        if (const auto count_ptr = req->get_count(); count_ptr != nullptr) {
            return *count_ptr;
        }
        return  kDefaultMessageCount;
    }();
#if PRINT
    LOG(INFO) << "Request received " << req->get_name() << ", sending " << expected_message_count << " messages" << std::endl;
#endif
    resp.reserve(expected_message_count);
    for (auto i{0}; i < expected_message_count; ++i) {
      auto& last_resp = resp.emplace_back();
      last_resp.set_string_v("Property name " + req->get_name() + " #" + std::to_string(i));

#if PRINT
      LOG(INFO) << "\"Sending\" reply " << last_resp.get_string_v() << std::endl;
#endif
    }
}

} // namespace chatroom
} // namespace example
