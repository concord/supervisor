#include "SupervisorDeleteHandler.hpp"
#include <proxygen/httpserver/ResponseBuilder.h>

namespace Concord {

SupervisorDeleteHandler::SupervisorDeleteHandler(const SubprocessPtr process)
  : process_(process) {}

void SupervisorDeleteHandler::onRequest(
  std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {}

void SupervisorDeleteHandler::onBody(
  std::unique_ptr<folly::IOBuf> body) noexcept {}

void SupervisorDeleteHandler::onEOM() noexcept {
  using State = folly::ProcessReturnCode::State;
  if(process_->returnCode().state() != State::RUNNING) {
    // If we get here, the process has just exited for some reason, we should
    // do nothing and let the thread who is waiting on this pid to take care
    // of clean up, etc..
    proxygen::ResponseBuilder(downstream_)
      .status(400, "ERROR")
      .sendWithEOM();    
    return;
  }
  process_->kill();
  proxygen::ResponseBuilder(downstream_)
    .status(200, "OK")
    .sendWithEOM();
}

void SupervisorDeleteHandler::onUpgrade(
  proxygen::UpgradeProtocol protocol) noexcept {}

void SupervisorDeleteHandler::requestComplete() noexcept { delete this; }

void SupervisorDeleteHandler::onError(proxygen::ProxygenError err) noexcept {
  delete this;
}
}
