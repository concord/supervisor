#pragma once

#include <folly/Memory.h>
#include <folly/Subprocess.h>
#include <proxygen/httpserver/RequestHandler.h>

namespace Concord {

class SupervisorDeleteHandler : public proxygen::RequestHandler {
 public:
  using SubprocessPtr = std::shared_ptr<folly::Subprocess>;

  explicit SupervisorDeleteHandler(const SubprocessPtr process);

  void
  onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

  void onEOM() noexcept override;

  void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;

  void requestComplete() noexcept override;

  void onError(proxygen::ProxygenError err) noexcept override;

 private:
  SubprocessPtr process_;
};

}
