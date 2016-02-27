#pragma once

#include <folly/Memory.h>
#include <folly/Subprocess.h>
#include <proxygen/httpserver/RequestHandler.h>

#include <json/json.h>

namespace Concord {

class SupervisorGetHandler : public proxygen::RequestHandler {
 public:
  using SubprocessPtr = std::shared_ptr<folly::Subprocess>;

  explicit SupervisorGetHandler(const std::vector<SubprocessPtr> &processes);

  void
  onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

  void onEOM() noexcept override;

  void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;

  void requestComplete() noexcept override;

  void onError(proxygen::ProxygenError err) noexcept override;

 private:
  Json::Value inspectProcesses(); // add const

  std::vector<pid_t> processes_;
  std::string process_info_;
};
}
