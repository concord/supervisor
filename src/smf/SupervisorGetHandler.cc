#include "SupervisorGetHandler.hpp"
#include <proxygen/httpserver/ResponseBuilder.h>
#include <json/writer.h>
#include <proc/readproc.h>

namespace Concord {

/**
 * I suppose you want to process just one specific PID.
 *
 * In that case call openproc with the PROC_FILLSTAT and PROC_PID flags set
 * and then pass a pid_t pointer (pointing at your PID) as the second
 * openproc argument. In that case the readproc function should give you
 * the requested data. Once done, call the closeproc function and that's
 * all.
*/

/**
 * Uses libprocps3 to inspect processes.
 * This package is available through aptitude, just link against -lprocps
 * No, there are no docs, you can inspect the source at /usr/include/proc/...
 */
Json::Value SupervisorGetHandler::inspectProcesses() {
  PROCTAB *proc = openproc(PROC_PID | PROC_FILLMEM | PROC_FILLSTAT
                             | PROC_FILLSTATUS | PROC_FILLWCHAN,
                           processes_.data());
  if(proc == NULL) {
    return Json::Value();
  }
  Json::Value json_info(Json::arrayValue);
  proc_t process_info;
  memset(&process_info, 0, sizeof(process_info));
  while(readproc(proc, &process_info) != NULL) {
    Json::Value entry(Json::objectValue);
    entry["tid"] =               std::to_string(process_info.tid);
    entry["ppid"] =              std::to_string(process_info.ppid);
    entry["state"] =             std::to_string(process_info.state);
    entry["utime"] =             std::to_string(process_info.utime);
    entry["stime"] =             std::to_string(process_info.stime);
    entry["vmem_pages"] =        std::to_string(process_info.size);
    entry["residentmem_pages"] = std::to_string(process_info.resident);
    entry["share_pages"] =       std::to_string(process_info.share);
    json_info.append(entry);
  }
  // CHECK(processes_.length() == proc.length()) << "Didn't get desired pids";

  closeproc(proc);
  return json_info;
}

SupervisorGetHandler::SupervisorGetHandler(
  const std::vector<SubprocessPtr> &processes) {
  for(const auto &process : processes) {
    processes_.push_back(process->pid());
  }
}

void SupervisorGetHandler::onRequest(
  std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
  Json::Value info = Json::Value(Json::objectValue);
  if(info.isNull()) {
    info["error"] = "No processes were inspected";
  } else {
    info["process_info"] = inspectProcesses();
  }
  process_info_ = Json::FastWriter().write(info);
}

void SupervisorGetHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
}

void SupervisorGetHandler::onEOM() noexcept {
  // if(process_info_.asString()) {
  //   // error happened
  //   return;
  // }
  proxygen::ResponseBuilder(downstream_)
    .status(200, "OK")
    .body(folly::IOBuf::copyBuffer(process_info_, 0, 0))
    .sendWithEOM();
}

void SupervisorGetHandler::onUpgrade(
  proxygen::UpgradeProtocol protocol) noexcept {}

void SupervisorGetHandler::requestComplete() noexcept { delete this; }

void SupervisorGetHandler::onError(proxygen::ProxygenError err) noexcept {
  delete this;
}
}
