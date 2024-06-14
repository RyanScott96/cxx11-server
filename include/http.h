#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <sys/socket.h>

namespace http {

enum class Method {
  GET,
  HEAD,
  POST,
  PUT,
  DELETE,
  CONNECT,
  OPTIONS,
  TRACE,
  PATCH,
};
std::string method_as_string(Method);
Method string_to_method(std::string);

enum class Version {
  HTTP1_1,
  HTTP2,
  HTTP3,
};
std::string version_as_string(Version version);
Version string_into_version(std::string version);

class Request {
private:
  Method method;
  std::string target;
  Version version;
  std::map<std::string, std::string> headers;
  std::unique_ptr<unsigned char[]> data;
  size_t data_size;
  
public:
  static Request deserialize(std::string);

  Request(Method method, std::string target);

  Method getMethod();

  void setMethod(Method method);

  std::string getTarget();

  void setTarget(std::string target);

  Version getVersion();

  const std::map<std::string, std::string> &getHeaders();

  std::string getHeader(std::string key);

  void setHeader(std::string key, std::string value);

  size_t serialize(std::unique_ptr<unsigned char[]>&);
};

class Response {
private:
  Version version;
  uint8_t statusCode;
  std::string statusText;
  std::map<std::string, std::string> headers;
  std::unique_ptr<unsigned char[]> data;
  size_t data_size;

public:
  Response(uint8_t statusCode, std::string statusText);
 
  Version getVersion();

  uint8_t getStatusCode();

  void setStatusCode(uint8_t status);

  std::string getStatusText();

  void setStatusText(std::string text);

  const std::map<std::string, std::string> &getHeaders();

  std::string getHeader(std::string key);

  void setHeader(std::string key, std::string value);

  std::unique_ptr<unsigned char[]> getData();

  void setData(const unsigned char data[], uint64_t size);

  size_t serialize(std::unique_ptr<unsigned char[]>&);

private:
  std::map<std::string, std::string> getDefaultReponseHeaders();
};
} // namespace http
