#include "SupervisorDeleteHandler.hpp"

namespace Concord {

SupervisorDeleteHandler::SupervisorDeleteHandler(const SubprocessPtr process)
  : process_(process) {}

void SupervisorDeleteHandler::onRequest(
  std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {}

void SupervisorDeleteHandler::onBody(
  std::unique_ptr<folly::IOBuf> body) noexcept {}

void SupervisorDeleteHandler::onEOM() noexcept {
  // ResponseBuilder(downstream_)
  //   .status(200, "OK")
  //   .header("Request-Number",
  //   folly::to<std::string>(stats_->getRequestCount()))
  //   .body(std::move(body_))
  //   .sendWithEOM();
}

void SupervisorDeleteHandler::onUpgrade(
  proxygen::UpgradeProtocol protocol) noexcept {}

void SupervisorDeleteHandler::requestComplete() noexcept { delete this; }

void SupervisorDeleteHandler::onError(proxygen::ProxygenError err) noexcept {
  delete this;
}
}
