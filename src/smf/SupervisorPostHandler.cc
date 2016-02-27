#include "SupervisorPostHandler.hpp"

namespace Concord {

SupervisorPostHandler::SupervisorPostHandler() {}

void SupervisorPostHandler::onRequest(
  std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {}

void SupervisorPostHandler::onBody(
  std::unique_ptr<folly::IOBuf> body) noexcept {}

void SupervisorPostHandler::onEOM() noexcept {
  // ResponseBuilder(downstream_)
  //   .status(200, "OK")
  //   .header("Request-Number",
  //   folly::to<std::string>(stats_->getRequestCount()))
  //   .body(std::move(body_))
  //   .sendWithEOM();
}

void SupervisorPostHandler::onUpgrade(
  proxygen::UpgradeProtocol protocol) noexcept {}

void SupervisorPostHandler::requestComplete() noexcept { delete this; }

void SupervisorPostHandler::onError(proxygen::ProxygenError err) noexcept {
  delete this;
}
}
