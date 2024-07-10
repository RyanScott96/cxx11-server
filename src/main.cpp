#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <arpa/inet.h>

#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/ip.h>

#include <unistd.h>

#include "http.h"
#include "router.h"
#include "threadpool.h"

void listenForHTTP(std::uint16_t, std::string);
void handleHTTPConnection(int);
void concurrentPrint(std::string);
class DumbRouter: public Router {
public:
  std::string getResource(std::string uri) override {
    if (uri == "/") {
      return "index.html";
    }
    return uri.substr(1);
  };
  void registerRoute(std::string uri, std::string resource) override {
    return;
  };
  void deregisterRoute(std::string uri) override {
    return;
  };
};

Router* router;

int main(int argc, char **argv) {
  const std::uint16_t port = 8080;
  const std::string address = "127.0.0.1";
  router = new DumbRouter{};
  {
    ThreadPool executor(std::thread::hardware_concurrency() - 1);
    for (size_t i = 0; i < std::thread::hardware_concurrency() - 1; ++i) {
      executor.enqueue([=]() { listenForHTTP(port, address); });
    }
  }
  delete router;
}

void listenForHTTP(const std::uint16_t port, const std::string address) {
  concurrentPrint("Starting Listener Thread!");
  sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr(address.c_str());
  server_address.sin_port = htons(port);

  int error;
  auto socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  const int one = 1;
  setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
  setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(int));

  error = bind(socket_descriptor, (sockaddr *)&server_address,
               sizeof(server_address));
  if (error) {
    concurrentPrint("Error Binding!");
  }
  error = listen(socket_descriptor, std::thread::hardware_concurrency() * 2);
  if (error) {
    concurrentPrint("Error listening!");
  }

  int connection_descriptor;
  while (true) {
    socklen_t address_size = sizeof(server_address);
    connection_descriptor =
        accept(socket_descriptor, (sockaddr *)&server_address, &address_size);
    handleHTTPConnection(connection_descriptor);
    close(connection_descriptor);
  }
}

void handleHTTPConnection(int connection) {
  concurrentPrint("Got a connection!");
  concurrentPrint("Reading Request...");

  std::string bytes;
  bytes.resize(4096);
  size_t request_size = recv(connection, &bytes[0], bytes.size(), 0);
  bytes.resize(request_size);
  http::Request request = http::Request::deserialize(bytes);

  concurrentPrint("Request resource: " + request.getTarget() + "...");

  std::string res_body = "";
  std::string target = request.getTarget();
  
  if (!target.empty()) {
    std::ifstream file_handle(router->getResource(target));
    while(!file_handle.eof()) {
      std::string buf;
      std::getline(file_handle, buf);
      res_body += (buf + '\n');
    }
  }

  http::Response res = http::Response(200, "OK");
  res.setData(reinterpret_cast<const unsigned char *>(res_body.c_str()), res_body.size());
  if (target == "/favicon.ico") {
    res.setHeader("Content-Type", "image/gif");
  }
  res.setHeader("Content-Length", std::to_string(res_body.size() - 1));
  
  std::unique_ptr<unsigned char[]> encoded;
  size_t length = res.serialize(encoded);
  concurrentPrint("Sending Response...");
  std::string garbled(reinterpret_cast<char *>(encoded.get()));
  send(connection, encoded.get(), length, 0);
}

void concurrentPrint(std::string message) {
  static std::mutex print_m;
  {
    std::unique_lock<std::mutex> lock(print_m);
    std::cout << std::this_thread::get_id() << "\t" << message << std::endl;
  }
}
