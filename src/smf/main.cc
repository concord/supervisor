#include <gflags/gflags.h>
#include <proxygen/httpserver/HTTPServer.h>
#include "SupervisorHandlerFactory.hpp"

using namespace proxygen;
using folly::SocketAddress;
using Protocol = HTTPServer::Protocol;

DEFINE_int32(http_port, 11000, "Port to listen on with HTTP protocol");
DEFINE_int32(spdy_port, 11001, "Port to listen on with SPDY protocol");
DEFINE_int32(h2_port, 11002, "Port to listen on with HTTP/2 protocol");
DEFINE_string(ip, "localhost", "IP/Hostname to bind to");

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  std::vector<HTTPServer::IPConfig> IPs = {
    {SocketAddress(FLAGS_ip, FLAGS_http_port, true), Protocol::HTTP},
    {SocketAddress(FLAGS_ip, FLAGS_spdy_port, true), Protocol::SPDY},
    {SocketAddress(FLAGS_ip, FLAGS_h2_port, true), Protocol::HTTP2},
  };

  HTTPServerOptions options;
  options.threads = 1;
  options.idleTimeout = std::chrono::milliseconds(60000);
  options.shutdownOn = {SIGINT, SIGTERM};
  options.enableContentCompression = true;
  options.handlerFactories = RequestHandlerChain()
    .addThen<Concord::SupervisorHandlerFactory>()
    .build();

  HTTPServer server(std::move(options));
  server.bind(IPs);

  // Start HTTPServer mainloop in a separate thread
  std::thread([&]() { server.start(); }).join();
  return 0;
}
