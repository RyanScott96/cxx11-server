#include "mime.h"
#include <iostream>

MIMEType::MIMEType(std::string type, std::string subtype) {
  this->type = type;
  this->subtype = subtype;
}

std::string MIMEType::toString() {
  return this->type + "/" + this->subtype;
}

MIME::MIME() {
  this->extMIME = std::unordered_map<std::string, MIMEType>{
      {".js", MIMEType{"text", "javascript"}},
      {".css", MIMEType{"text", "css"}},
      {".html", MIMEType{"text", "html"}},
      {".ico", MIMEType{"image","x-icon"}},
  };
}

std::string MIME::getMIMEType(std::string resource) {
  std::string ext = resource.substr(resource.find_last_of("."));
  MIMEType type = this->extMIME.at(ext);
  return type.toString(); 
}
