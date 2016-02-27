#include "SupervisorHandlerFactory.hpp"
#include "SupervisorDeleteHandler.hpp"
#include "SupervisorPostHandler.hpp"
#include "SupervisorGetHandler.hpp"
#include "BadRequestHandler.hpp"

namespace Concord {

const std::string SupervisorHandlerFactory::REST_STR = "process";

void SupervisorHandlerFactory::onServerStart(
  folly::EventBase *evb) noexcept {}

void SupervisorHandlerFactory::onServerStop() noexcept {}

proxygen::RequestHandler *SupervisorHandlerFactory::onRequest(
  proxygen::RequestHandler *,
  proxygen::HTTPMessage *message) noexcept {
  /**
   * REST API - using query parameter 'process'
   * ==========================================
   * GET process => Return hardware monitoring of all processes
   * GET process/1 => Return monitoring of a particular process
   * POST process => Create a new process
   * POST process/1 => ERROR
   * PUT process => ERROR
   * PUT process/1 => ERROR
   * DELETE process => ERROR
   * DELETE process/1 => Remove process 1
   */
  if(!message->getMethod()) {
    return new BadRequestHandler("Missing HTTP Method");
  }
  if(!message->hasQueryParam(REST_STR)) {
    return new BadRequestHandler("Ill formed query string");
  }

  using proxygen::HTTPMethod;
  switch(*(message->getMethod())) {
  case HTTPMethod::GET:
    return handleGetRequest(findProcessId(message->getQueryString()));
  case HTTPMethod::POST:
    return new SupervisorPostHandler();
  case HTTPMethod::DELETE:
    return handleDeleteRequest(findProcessId(message->getQueryString()));
  default:
    return new BadRequestHandler("Unsupported HTTP Method");
  }
}

proxygen::RequestHandler *SupervisorHandlerFactory::handleGetRequest(
  const boost::optional<int> &process_id) {
  std::vector<SubprocessPtr> processes;
  if(process_id) {
    auto found = processMap_.find(*process_id);
    if(found == processMap_.end()) {
      return new BadRequestHandler("A process with that ID does not exist");
    }
    processes.push_back(found->second);
  } else {
    for(const auto &p : processMap_) {
      processes.push_back(p.second);
    }
  }
  return new SupervisorGetHandler(processes);
}

proxygen::RequestHandler *SupervisorHandlerFactory::handleDeleteRequest(
  const boost::optional<int> &process_id) {
  if(!process_id) {
    return new BadRequestHandler("Unsupported action - Delete must use a process id");
  }
  auto found = processMap_.find(*process_id);
  if(found == processMap_.end()) {
    return new BadRequestHandler("A process with that ID does not exist");
  }
  processMap_.erase(found);
  return new SupervisorDeleteHandler(found->second);
}

boost::optional<int>
SupervisorHandlerFactory::findProcessId(const std::string &query_string) {
  std::size_t found = query_string.find(SupervisorHandlerFactory::REST_STR);
  // assert(found != std::string::npos);
  std::string procId = query_string.substr(found + REST_STR.length() + 1);
  return procId == "" ? boost::none : boost::optional<int>(std::stoi(procId));
}

}
