#pragma once
#include <string>

class Router {
public:
  virtual ~Router() = default;
  virtual std::string getResource(std::string uri) = 0;
};
