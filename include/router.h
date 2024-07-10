#pragma once
#include <string>

class Router {
public:
  virtual ~Router() = default;
  virtual std::string getResource(std::string uri) = 0;
  virtual void registerRoute(std::string uri, std::string resource) = 0;
  virtual void deregisterRoute(std::string uri) = 0; 
};
