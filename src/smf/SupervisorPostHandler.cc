#include "SupervisorPostHandler.hpp"
#include "utils/aws_utils.hpp"
#include <proxygen/httpserver/ResponseBuilder.h>
#include <cstdio>
#include <cstdlib>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <json/reader.h>

namespace {
std::string decode64(const std::string &val) {
  using namespace boost::archive::iterators;
  using It =
    transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
  return boost::algorithm::trim_right_copy_if(
    std::string(It(std::begin(val)), It(std::end(val))),
    [](char c) { return c == '\0'; });
}
}

namespace Concord {

folly::Subprocess::Options SupervisorPostHandler::supervisorProcessOptions() {
  // Call to mkstemp will overwrite bytes in stderr_path and stdout_path
  // buffers with actual file name.
  const auto current_dir = boost::filesystem::current_path().string().c_str();
  char stderr_path[256] = {};
  char stdout_path[256] = {};
  strcpy(stderr_path, current_dir);
  strcpy(stdout_path, current_dir);
  strcat(stderr_path, "-XXXXXX");
  strcat(stdout_path, "-XXXXXX");

  Options procopts;
  procopts.parentDeathSignal(9);
  procopts.stdout(mkstemp(stdout_path));
  procopts.stderr(mkstemp(stderr_path));
  stderr_file_ = std::string(stderr_path);
  stdout_file_ = std::string(stdout_path);
  return procopts;
}

SupervisorPostHandler::SupervisorPostHandler(
  folly::Synchronized<std::map<pid_t, SubprocessPtr>> &processMap)
  : processMap_(processMap) {}

void SupervisorPostHandler::onRequest(
  std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {}

void SupervisorPostHandler::onBody(
  std::unique_ptr<folly::IOBuf> body) noexcept {
  if(body_) {
    body_->prependChain(std::move(body));
  } else {
    body_ = std::move(body);
  }
}

void SupervisorPostHandler::onEOM() noexcept {
  /** Fail if no body exists */
  if(!body_) {
    proxygen::ResponseBuilder(downstream_).status(400, "ERROR").sendWithEOM();
    return;
  }

  /** Deserialize data -- Coalesce all data from IOBuffer's into std::string */
  std::string data_body;
  data_body.reserve(body_->length()); // multiply by # of IOBuf links
  try {
    for(const auto &bytes : body_->coalesce()) {
      data_body += bytes;
    }
  } catch(std::bad_alloc &e) {
    LOG(ERROR) << "Failed when coalesceing folly::IOBuf'fers: " << e.what();
    proxygen::ResponseBuilder(downstream_).status(400, "ERROR").sendWithEOM();
    return;
  }

  /** Parse data */
  Json::Value json_body(Json::nullValue);
  Json::Reader().parse(data_body, json_body);
  const auto fill =
    [](std::vector<std::string> &toFill, const Json::Value &arr_data) {
      if(arr_data.isArray()) {
        toFill.reserve(arr_data.size());
        std::transform(arr_data.begin(), arr_data.end(), toFill.begin(),
                       [](const Json::Value &elem) {
                         return elem.isString() ? elem.asString() : "";
                       });
      }
    };
  const auto executable = decode64(json_body["executable"].asString());
  std::vector<std::string> argv, env;
  fill(argv, json_body["argv"]);
  fill(env, json_body["env"]);

  /** Start subprocess */
  try {
    subprocess_ = std::make_shared<folly::Subprocess>(
      argv, supervisorProcessOptions(), executable.c_str(), &env);
  } catch(folly::SubprocessSpawnError &e) {
    LOG(ERROR) << "Could not start subprocess: " << e.what();
    proxygen::ResponseBuilder(downstream_).status(400, "ERROR").sendWithEOM();
    return;
  }

  /** Atomic state update && success response */
  processMap_->operator[](subprocess_->pid()) = subprocess_;
  proxygen::ResponseBuilder(downstream_).status(200, "OK").sendWithEOM();
}

/**
 * Invoked when request processing has been completed and nothing more
 * needs to be done. This may be a good place to log some stats and
 * clean up resources. This is distinct from onEOM() because it is
 * invoked after the response is fully sent. Once this callback has been
 * received, `downstream_` should be considered invalid.
 */
void SupervisorPostHandler::requestComplete() noexcept {
  if(subprocess_) {
    const auto process_id = subprocess_->pid();
    LOG(INFO) << "Waiting on subprocess pid_t: " << process_id;
    try {
      subprocess_->wait();
    } catch(folly::SubprocessError &e) {
      LOG(ERROR) << "Subprocess exited with exception: " << e.what();
    } catch(std::logic_error &e) {
      LOG(ERROR) << "Subprocess is already running: " << e.what();
    } catch(std::exception &e) {
      LOG(ERROR) << "Error when attempting to wait on subprocess: " << e.what();
    }
    LOG(INFO) << "Process has exited with status: "
	      << subprocess_->returnCode().str();
    sendLogsToS3(stderr_file_, stdout_file_);
    processMap_->erase(process_id);
  }
  delete this;
}

/**
 * Request failed. Maybe because of read/write error on socket or client
 * not being able to send request in time.
 *
 * NOTE: Can be invoked at any time (except for before onRequest).
 *
 * No more callbacks will be invoked after this. You should clean up after
 * yourself.
 */
void SupervisorPostHandler::onError(proxygen::ProxygenError err) noexcept {
  delete this;
}

void SupervisorPostHandler::onUpgrade(
  proxygen::UpgradeProtocol protocol) noexcept {}
}
