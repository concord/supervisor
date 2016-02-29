#pragma once

#include <folly/Memory.h>
#include <folly/Subprocess.h>
#include <proxygen/httpserver/RequestHandler.h>

#include <json/json.h>

namespace Concord {

class SupervisorGetHandler : public proxygen::RequestHandler {
 public:
  explicit SupervisorGetHandler(const std::vector<pid_t> &processes);

  void
  onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

  void onEOM() noexcept override;

  void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;

  void requestComplete() noexcept override;

  void onError(proxygen::ProxygenError err) noexcept override;

 private:
  Json::Value inspectProcesses();

  Json::Value process_info_;
  const std::vector<pid_t> processes_;
};
}
