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
    return Json::Value(Json::nullValue);
  }
  Json::Value json_info(Json::arrayValue);
  proc_t process_info;
  memset(&process_info, 0, sizeof(process_info));
  while(readproc(proc, &process_info) != NULL) {
    Json::Value entry(Json::objectValue);
    entry["tid"] = process_info.tid;
    entry["ppid"] = process_info.ppid;
    entry["state"] = process_info.state;
    entry["utime"] = process_info.utime;
    entry["stime"] = process_info.stime;
    entry["vmem_pages"] = std::to_string(process_info.size);
    entry["residentmem_pages"] = std::to_string(process_info.resident);
    entry["share_pages"] = std::to_string(process_info.share);
    json_info.append(entry);
  }
  CHECK(processes_.size() == json_info.size())
    << "Gathered incorrect process info";
  closeproc(proc);
  return json_info;
}

SupervisorGetHandler::SupervisorGetHandler(
  const std::vector<pid_t> &processes)
  : processes_(processes) {}

void SupervisorGetHandler::onRequest(
  std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
  Json::Value process_data(inspectProcesses());
  if(process_data.isNull()) {
    process_info_["error"] = "No processes were inspected";
  } else {
    process_info_["process_info"] = process_data;
  }
}

void SupervisorGetHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
}

void SupervisorGetHandler::onEOM() noexcept {
  const auto serialized_response = Json::FastWriter().write(process_info_);
  const auto http_status = process_info_.isMember("error") ?
                             std::make_tuple(400, "ERROR") :
                             std::make_tuple(200, "OK");

  proxygen::ResponseBuilder(downstream_)
    .status(std::get<0>(http_status), std::get<1>(http_status))
    .body(folly::IOBuf::copyBuffer(serialized_response, 0, 0))
    .sendWithEOM();
}

void SupervisorGetHandler::onUpgrade(
  proxygen::UpgradeProtocol protocol) noexcept {}

void SupervisorGetHandler::requestComplete() noexcept { delete this; }

void SupervisorGetHandler::onError(proxygen::ProxygenError err) noexcept {
  delete this;
}
}
