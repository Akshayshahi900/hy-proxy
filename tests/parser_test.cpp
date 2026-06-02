#include <cassert>
#include <iostream>

#include "parser.h"
#include "request.h"


void test_POST(){
  std::string raw = "POST /login HTTP/1.1\r\n"
                    "Host: localhost\r\n"
                    "Content-Length: 15\r\n"
                    "\r\n"
                    "username=akshay";
  
  HttpRequest req = parseRequest(raw);

  assert(req.method == "POST");
  assert(req.path == "/login");
  assert(req.version == "HTTP/1.1");

  assert(req.headers["Host"] == "localhost");

  assert(req.body == "username=akshay");
}

int main(){
  test_POST();
  
  std::cout << "All tests passed";
}
