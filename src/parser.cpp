#include<iostream>
#include<string>
#include<unordered_map>
#include<sstream>
#include "request.h"


HttpRequest parseRequest(const std::string &raw){
  HttpRequest req;

  std::istringstream stream(raw);
  
  //Request Line
  std::string requestLine;
  std::getline(stream , requestLine);

  if(!requestLine.empty() && requestLine.back() == '\r'){
    requestLine.pop_back();
  }
  std::istringstream firstLine(requestLine);

  firstLine >> req.method >>req.path >> req.version;

  // headers
  std::string line;

//TODO Parse for the -d data flag
  while(std::getline(stream , line)){
    if(!line.empty()&& line.back() == '\r'){
      line.pop_back();
    }

    if(line.empty()) break;
  

    size_t pos = line.find(':');

    if(pos == std::string::npos) continue;

    std::string key = line.substr(0 , pos);
    
    std::string value = line.substr(pos+1);

    if(!value.empty() && value.front() == ' '){
      value.erase(0 , 1);
    }

    req.headers[key] = value;
  
  }

  if(req.headers.find("Content-Length") != req.headers.end()){

    int length = std::stoi(req.headers["Content-Length"]);

    if(length > 0){
      req.body.resize(length);
    

    stream.read(&req.body[0] ,length);
  }
  
  }

  return req;
}
/*
int main(){
   
HttpRequest req= parseRequest(raw);
  std::string raw =" POST /api/v1/users HTTP/1.1\r\n"
  "Host: localhost:8080\r\n"
  "User-Agent: curl/8.4.0\r\n"
  "Accept: application/json\r\n"
  "Content-Type: application/json\r\n"
  "Content-Length: 47\r\n"
  "Connection: close\r\n"
  "\r\n"
  "{"username":"alex123","email":"alex@email.com"}";
  
  std::cout << "Method: " << req.method << '\n';

  std::cout << "Path: " << req.path << '\n';

  std::cout << "Version: " << req.version << '\n';

  for (auto & [k , v] :req.headers){
    std::cout << k << " => " << v << '\n';
  }

} */
