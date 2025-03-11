#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT "8000"
#define BACKLOG 10

int server();
int main(int argc, char const *argv[]) {
  // client(argc, argv);
  server();
  return 0;
}

void *getinaddr(addrinfo *p) {
  void *actualaddr;
  if (p->ai_family == AF_INET) {
    actualaddr = (sockaddr_in *)p->ai_addr;
  } else {
    actualaddr = (sockaddr_in6 *)p->ai_addr;
  }
  return actualaddr;
}

int server() {
  int status = 0, s = 0, err = 0;
  char msg[] = "Peter is here";

  // info about running interface/socket
  char ipstr[INET6_ADDRSTRLEN];
  addrinfo *p;

  // client socket addr
  sockaddr_storage their_addr;
  socklen_t addrlen;

  // hint setup
  addrinfo hints;
  addrinfo *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  status = getaddrinfo(NULL, PORT, &hints, &res);
  if (status != 0) {
    std::cout << "getaddrinfo error" << gai_strerror(status) << "\n";
    return -1;
  }

  // iterate over valid addreses
  for (p = res; p != NULL; p = p->ai_next) {
    s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (s == -1) {
      continue;
    }
    err = bind(s, p->ai_addr, p->ai_addrlen);
    /*
    if kernel allowed the socket to be bound exit
    */
    if (errno != -1) {
      break;
    }
  }
  /*
  start listening and accept queued requests
  */
  inet_ntop(p->ai_family, getinaddr(p), ipstr, INET6_ADDRSTRLEN);
  std::cerr << "starting to listen on " << ipstr << "\n";
  freeaddrinfo(res);
  while (1) {
    err = listen(s, BACKLOG); // setup queue
    int clientfd = accept(s, (sockaddr *)&their_addr, &(addrlen));
    size_t len = sizeof(msg), bytes_sent = 0;
    bytes_sent = send(clientfd, msg, len, 0);
    if (bytes_sent == -1) {
      std::cerr << "Unable to send\n";
    }
  }

  return 0;
}
