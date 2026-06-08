#include <iostream>
#include <cstdint>
#include<vector>
#include "backend.h"

std::vector<Backend> backends = {
  {"127.0.0.1",3001},
  {"127.0.0.1",3002},
  {"127.0.0.1",3003},
  {"127.0.0.1",3004},
  {"127.0.0.1",3005}
};
size_t current = 0;

Backend load_balancer(){
  Backend backend = backends[current];

  current = (current +1) % backends.size();

  return backend;
}
