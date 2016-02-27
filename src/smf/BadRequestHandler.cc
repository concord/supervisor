#include "BadRequestHandler.hpp"

namespace Concord {

BadRequestHandler::BadRequestHandler(const std::string &reason)
  : reason_(reason) {}

void BadRequestHandler::onRequest(
  std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {}

void BadRequestHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {}

void BadRequestHandler::onEOM() noexcept {
  proxygen::ResponseBuilder(downstream_).status(400, "ERROR").sendWithEOM();
}

void BadRequestHandler::onUpgrade(proxygen::UpgradeProtocol proto) noexcept {}

void BadRequestHandler::requestComplete() noexcept { delete this; }

void BadRequestHandler::onError(proxygen::ProxygenError err) noexcept {
  delete this;
}

}
