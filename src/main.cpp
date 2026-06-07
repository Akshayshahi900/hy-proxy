#include<iostream>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<cstring>
#include<vector>
#include<cstdint>
#include<unordered_map>
#include<errno.h>

#include "connection.h"
#include "parser.h"
#include "request.h"
#include "socket.h"
#include "load_balancer.h"

#define MAX_EVENTS 64
#define BUFFER_SIZE 4096
std::unordered_map<int , Connection> connections;
int epoll_fd;
int server_fd;

bool has_full_request(const std::string & buff){
  return buf.find("\r\n\r\n") != std::string::npos;
}
 
void handle_reading_request(Connection& conn){
  char buf[BUFFER_SIZE];

  ssize_t n = recv(conn.client_fd , buf , BUFFER_SIZE , MSG_DONTWAIT);
  
  if(n==0){
    std::cout << "[" << conn.client_fd << "] Client closed \n";
    conn.state = State::CLOSED;
    return;
  }

  if(n < 0){
    
    if(errno == EAGAIN || errno == EWOULDBLOCK){
      return;
    }
    perror("recv");
    conn.state = State::CLOSED;
    return;

  }

  conn.request_buffer.append(buf , n );
  
  std::cout << "[" <<conn.client_fd << "] Recieved " << n << "bytes. Total: " << conn.request_buffer.size() << "\n";
 
  if(has_full_request(conn.request_buffer)){
    std::cout<< "[" << conn.client_fd << "] Got full HTTP request\n";

    HttpRequest req = parseRequest(conn.request_buffer);
    
    std::cout<< "[" << conn.client_fd << "] Method: " << req.method << ", Path: " << req.path << "\n";

    uint16_t backend_port = load_balancer(req);
    
    std::cout << "[" << conn.client_fd << "]  Load balancer chose port: " << backend_port << "\n";

    
    conn.backend_fd = connect_to_backend("127.0.0.1", backend_port);
    
    if(conn.backend_fd < 0){
      std::cout << "[" << conn.client_fd << "] Failed to create a backend socket\n";

      conn.state = State::CLOSED;
      return ;
    }

    std::cout << "[" << conn.client_fd << "] Connecting to backend... \n";

    conn.state = State::CONNECTING_BACKEND;
    
    // add backend_fd to epoll, using EPOLLOUT for connection readiness
    
    epoll_event ev ={};
    ev.events = EPOLLOUT | EPOLLERR;
    ev.data.fd = conn.backend_fd;
    epoll_ctl(epoll_fd , EPOLL_CTL_ADD , conn.backend_fd , &ev);
  }
}

void handle_connecting_backend(Connection& conn){
  int err = 0;
  socklen_t len = sizeof(err);

  int ret = getsockopt(conn.backend_fd , SOL_SOCKET , SO_ERROR , &err , &len);

  if(ret < 0 || err != 0){
    std::cout << "[" << conn.client_fd << "] Backend Connect Failed: " << strerror(err) <<"\n";
    conn.state = State::CLOSED;
    return;
  }
  std::cout <<  "[" << conn.client_fd << "] Backend connected successfully!\n";

  conn.state = State::FORWARDING_REQUEST;
  
  epoll_event ev = {};
  ev.events = EPOLLOUT | EPOLLIN ;
  ev.data.fd = conn.backend_fd;

  epoll_ctl(epoll_fd , EPOLL_MOD , conn.backend_fd , &ev);

}
int main(){
  int server_fd = create_server_socket(8080);
  
  if(server_fd == -1){
    perror("Create Socket");
  }

  std::cout << "Server listening on PORT 8080\n" ;


 // EPOLL INSTANCE
  int epoll_fd = epoll_create1(0);
  
  epoll_event event{};

  event.events = EPOLLIN;
  event.data.fd = server_fd;


  if(epoll_ctl(epoll_fd ,EPOLL_CTL_ADD , server_fd , &event) == -1){
    perror("epoll_ctl add server");
    return -1;
  }

   epoll_event events[MAX_EVENTS];

   //event loop
  while(true){
  //accept
  
    int ready_count = epoll_wait(epoll_fd , events , MAX_EVENTS , -1);

    if(ready_count == -1){
      perror("epoll_wait");
      continue;
    }
    
    for(int i =0 ; i < ready_count ; i++){
      int fd = events[i].data.fd;

      //create new connected
      if(fd == server_fd){
        while(true){
          int client_fd = accept(server_fd  , nullptr , nullptr);

          if(client_fd ==-1 ){
            break;
          }

          connections.emplace( client_fd , Connection{client_fd , "" , ""});

          
          make_non_blocking(client_fd);

          epoll_event client_event{};
          client_event.events = EPOLLIN;
          client_event.data.fd = client_fd;

          epoll_ctl(epoll_fd , EPOLL_CTL_ADD , client_fd , &client_event);

          std::cout << "Client Connected: " << client_fd << '\n';
        }
      }
      else{
        char buffer[BUFFER_SIZE];
        Connection& conn = connections[fd];
        int bytes_read = recv(conn.fd , buffer , sizeof(buffer) , 0);
        
        if(bytes_read <=0 ){
          std::cout << "Client disconnected:" << conn.fd << '\n';
          epoll_ctl(epoll_fd , EPOLL_CTL_DEL , conn.fd , nullptr);
          close(conn.fd);
          connections.erase(conn.fd);
          continue;
        }
        

        std:: string raw(buffer , bytes_read);
        
        conn.read_buffer.append(buffer , bytes_read);

        /*      
        HttpRequest req = parseRequest(conn.read_buffer);

        std::cout << "Method: " << req.method << '\n';

        std::cout << "Path: " << req.path << '\n';

        std::cout << "Version: " << req.version << '\n';

        for (auto & [k , v] :req.headers){
          std::cout << k << " => " << v << '\n';
        }
  
       std::cout<<"Body: \n";

        for(auto s:req.body){
          std::cout << s ;
        }
        */


        uint16_t backend = load_balancer(req);
        
        //std::string response = proxy(raw , backend);
        conn.backend_fd = connect_to_backend(
            "127.0.0.1" , backend
            );
        conn.request_buffer = raw;
        
        conn.state = State::CONNECTING_BACKEND;

        int bytes_send = send(conn.fd , conn.write_buffer.data() ,conn.write_buffer.size() , 0);
  
        if(bytes_send == -1){
          perror("send");
        }

        conn.read_buffer.clear();
        conn.write_buffer.clear();
      }
    }
  }    

  for(auto& [fd , conn]: connections){
    close(fd);
  }

  close(server_fd);
  close(epoll_fd);

  return 0;
}
