#pragma once

#include <vector>
#include <folly/Synchronized.h>

#include <if/gen-cpp2/StorageService.h>

namespace example {
namespace storage {

class StorageServiceHandler : virtual public StorageServiceSvIf {
 public:
  StorageServiceHandler() = default;

  explicit StorageServiceHandler(int64_t /*currentTime*/)
      : StorageServiceHandler() {}

  explicit StorageServiceHandler(std::function<int64_t()> /*timeFn*/)
      : StorageServiceHandler() {}

  void GetProperty(
      PropertyValue& resp,
      std::unique_ptr<PropertyRequest> req) override;

  void GetPropertyStream(::std::vector<PropertyValue>& resp, std::unique_ptr<PropertyRequest> req) override;
};
} // namespace chatroom
} // namespace example
