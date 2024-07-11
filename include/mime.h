#pragma once

#include <string>
#include <unordered_map>

class MIMEType {
private:
  std::string type;
  std::string subtype;
public:
  MIMEType(std::string type, std::string subtype);
  std::string toString();
};

class MIME {
private:
  std::unordered_map<std::string, MIMEType> extMIME;
public:
  MIME();
  std::string getMIMEType(std::string resource);
};
