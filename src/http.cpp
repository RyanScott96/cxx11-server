#include "http.h"
#include <cstdint>
#include <cstring>
#include <deque>
#include <sstream>
#include <string>
#include <utility>
#include <memory>
#include <unordered_map>

std::string http::method_as_string(http::Method method) {
  switch (method) {
  case Method::GET:
    return "GET";
  case Method::HEAD:
    return "HEAD";
  case Method::POST:
    return "POST";
  case Method::PUT:
    return "PUT";
  case Method::DELETE:
    return "DELETE";
  case Method::CONNECT:
    return "CONNECT";
  case Method::OPTIONS:
    return "OPTIONS";
  case Method::TRACE:
    return "TRACE";
  case Method::PATCH:
    return "PATCH";
  };
  return "ERROR";
};

http::Method http::string_to_method(std::string method) {
  std::unordered_map<std::string, Method> map = {
      {"GET", Method::GET},         {"HEAD", Method::HEAD},
      {"POST", Method::POST},       {"PUT", Method::PUT},
      {"DELETE", Method::DELETE},   {"CONNECT", Method::CONNECT},
      {"OPTIONS", Method::OPTIONS}, {"TRACE", Method::TRACE},
      {"PATCH", Method::PATCH},
  };
  return map.at(method);
};

std::string http::version_as_string(Version version) {
  switch (version) {
  case Version::HTTP1_1:
    return "HTTP/1.1";
  case Version::HTTP2:
    return "HTTP/2";
  case Version::HTTP3:
    return "HTTP/3";
  }
  return "ERROR";
};

http::Version http::string_into_version(std::string version) {
  std::unordered_map<std::string, Version> map = {
      {"HTTP/1.1", Version::HTTP1_1},
      {"HTTP/2", Version::HTTP2},
      {"HTTP/3", Version::HTTP3},
  };
  return map.at(version);
};

http::Request::Request(Method method, std::string target) : method(method), target(target) {
  version = Version::HTTP1_1;
  headers = std::map<std::string, std::string>{};
  data_size = 0;
};

http::Method http::Request::getMethod() {
  return this->method;
};

void http::Request::setMethod(http::Method method) {
  this->method = method;
};

std::string http::Request::getTarget() {
  return this->target;
};

void http::Request::setTarget(std::string target) {
  this->target = target;
};

http::Version http::Request::getVersion() {
  return this->version;
};

const std::map<std::string, std::string>& http::Request::getHeaders() {
  return this->headers;
};

std::string http::Request::getHeader(std::string key) {
  return this->headers.at(key);
};

void http::Request::setHeader(std::string key, std::string value) {
  this->headers.insert(std::make_pair(key, value));
};

size_t http::Request::serialize(std::unique_ptr<unsigned char[]>& out) {
  std::string request_line = http::method_as_string(this->getMethod()) + " "
                             + this->getTarget() + " "
                             + http::version_as_string(this->getVersion()) + "\r\n";
  std::string headers = "";
  for (auto header : this->getHeaders()) {
    headers += (header.first + ": " + header.second + "\r\n");
  }

  std::string text = request_line + headers + "\r\n";
  std::unique_ptr<unsigned char[]> request(new unsigned char[text.size() + 1 + this->data_size]);
  memcpy(request.get(), text.c_str(), text.size());
  memcpy(request.get() + text.size() + 1, this->data.get(), this->data_size);
  out = std::move(request);
  return (size_t) text.size() + 1 + this->data_size;
};

http::Request http::Request::deserialize(std::string bytes) {
  
  // Split message into lines by /r/n
  // Stop when line that is only /r/n
  int start = 0;
  int end = 0;
  std::string delimiter = "\r\n";
  std::deque<std::string> lines{};

  while ((end = bytes.find(delimiter, start)) != std::string::npos &&
         end - start > delimiter.size()) {
    std::string line = bytes.substr(start, end - start);
    lines.push_back(line);
    bytes.erase(0, line.length() + delimiter.length());
  }
  bytes.erase(0, delimiter.length());

  // Parse request line
  std::stringstream line{lines.front()};
  lines.pop_front();
  std::deque<std::string> tokens{};
  std::string token{};
  while (line >> token) {
    tokens.push_back(token);
  }

  http::Version version = http::string_into_version(tokens.at(2));
  http::Method method = http::string_to_method(tokens.at(0));
  std::string uri = tokens.at(1);


  http::Request request(method, uri);

  // Parse headers
  while(!lines.empty()) {
    line = std::stringstream(lines.front());
    lines.pop_front();

    tokens.clear();
    token.clear();
    while (line >> token) {
      tokens.push_back(token);
    }
    request.setHeader(tokens.at(0), tokens.at(1));
  }

  request.data_size = bytes.size() + 1;
  request.data = std::unique_ptr<unsigned char[]>(new unsigned char[request.data_size]);
  memcpy(request.data.get(), bytes.c_str(), request.data_size);
  return std::move(request);
};

http::Response::Response(uint8_t statusCode, std::string statusText) : statusCode(statusCode), statusText(statusText) {
    version = Version::HTTP1_1;
    headers = getDefaultReponseHeaders();
};

http::Version http::Response::getVersion() {
  return this->version;
};

uint8_t http::Response::getStatusCode() {
  return this->statusCode;
};

void http::Response::setStatusCode(uint8_t status) {
  this->statusCode = status;
};

std::string http::Response::getStatusText() {
  return this->statusText;
};

void http::Response::setStatusText(std::string text) {
  this->statusText = text;
};

const std::map<std::string, std::string>& http::Response::getHeaders() {
  return this->headers;
};

std::string http::Response::getHeader(std::string key) {
  return this->headers.at(key);
};

void http::Response::setHeader(std::string key, std::string value) {
  this->headers.insert(std::make_pair(key, value));
};

 std::unique_ptr<unsigned char[]> http::Response::getData() {
  std::unique_ptr<unsigned char[]> copy(new unsigned char[this->data_size]);
  memcpy(copy.get(), this->data.get(), this->data_size);
  return std::move(copy);
}

void http::Response::setData(const unsigned char data[], uint64_t size) {
  this->data_size = size + 1;
  this->data = std::unique_ptr<unsigned char[]>(new unsigned char[size + 1]);
  memcpy(this->data.get(), data, size);
}

size_t  http::Response::serialize(std::unique_ptr<unsigned char[]>& out) {
  std::string head = "";
  std::string version = version_as_string(this->version);
  std::string code = std::to_string(this->statusCode);

  head += version + " " + code + " " + statusText + "\r\n";
  for (auto header : this->headers) {
    head += header.first + " : " + header.second + "\r\n";
  }
  head += "\r\n";

  std::unique_ptr<unsigned char[]> response(new unsigned char[head.size() + this->data_size + 1]);
  memcpy(response.get(), head.c_str(), head.size());
  memcpy(response.get() + head.size(), this->data.get(), this->data_size);
  out = std::move(response);
  return (size_t) head.size() + this->data_size;
};

std::map<std::string, std::string> http::Response::getDefaultReponseHeaders() {
  return {
      std::make_pair("Content-Type", "text/html; charset=UTF-8"),
      std::make_pair("Content-Encoding", "UTF-8"),
      std::make_pair("Accept", "*/*"),
    };
};

