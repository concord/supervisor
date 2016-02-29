#pragma once

#include <vector>
#include <string>
#include <map>
#include <folly/Memory.h>
#include <folly/Subprocess.h>
#include <folly/Synchronized.h>
#include <proxygen/httpserver/RequestHandler.h>

namespace Concord {

/**
 * HTTP POST will be used to start a subprocess
 * ============================================
 * This class expects a POST message with a JSON body containing three fields:
 * 1. 'executable' ==> Base64 encoded binary for an executable to exec post fork
 * 2. 'argv' ==> Array of strings containing program name followed by cmd args
 * 3. 'env' ==> Optional array of strings that will be forwarded to the
 *              child process when exec is called
 *
 * NOTE: In the future consider using HTTP multipart/form-data or thrift
 *       instead of JSON.
 */
class SupervisorPostHandler : public proxygen::RequestHandler {
 public:
  using SubprocessPtr = std::shared_ptr<folly::Subprocess>;
  using Options = folly::Subprocess::Options;

  explicit SupervisorPostHandler(
    folly::Synchronized<std::map<pid_t, SubprocessPtr>> &processMap);

  void
  onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

  void onEOM() noexcept override;

  void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;

  void requestComplete() noexcept override;

  void onError(proxygen::ProxygenError err) noexcept override;

 private:
  // modifies stderr_file_ and stdout_file_
  Options supervisorProcessOptions();

  std::string stderr_file_;
  std::string stdout_file_;

  std::unique_ptr<folly::IOBuf> body_;
  std::shared_ptr<folly::Subprocess> subprocess_;

  folly::Synchronized<std::map<pid_t, SubprocessPtr>> &processMap_;
};
}
