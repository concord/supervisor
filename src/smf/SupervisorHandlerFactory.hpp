#pragma once

#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/Subprocess.h>
#include <folly/Synchronized.h>

#include <boost/optional.hpp>

namespace Concord {

class SupervisorHandlerFactory : public proxygen::RequestHandlerFactory {
 public:
  using SubprocessPtr = std::shared_ptr<folly::Subprocess>;

  static const std::string REST_STR;

  void onServerStart(folly::EventBase *evb) noexcept override;
  void onServerStop() noexcept override;
  proxygen::RequestHandler *
  onRequest(proxygen::RequestHandler *,
            proxygen::HTTPMessage *) noexcept override;

 private:
  static boost::optional<int> findProcessId(const std::string &query_string);
  proxygen::RequestHandler *
  handleGetRequest(const boost::optional<int> &process_id);
  proxygen::RequestHandler *
  handleDeleteRequest(const boost::optional<int> &process_id);

  folly::Synchronized<std::map<int, SubprocessPtr>> processMap_;
};
}
