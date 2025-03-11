#include <arpa/inet.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int client(int argc, char const *argv[]);
int main(int argc, char const *argv[]) { client(argc, argv); }

void *getinaddr(addrinfo *p) {
  void *actualaddr;
  if (p->ai_family == AF_INET) {
    actualaddr = (sockaddr_in *)p->ai_addr;
  } else {
    actualaddr = (sockaddr_in6 *)p->ai_addr;
  }
  return actualaddr;
}

int client(int argc, char const *argv[]) {
  int status, s, err, bytes_read;

  char ipstr[INET6_ADDRSTRLEN];
  char buf[100];

  addrinfo hints;
  addrinfo *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (argc < 3) {
    std::cerr << "Please specify server to connect to and port\n";
    return -1;
  }
  status = getaddrinfo(argv[1], argv[2], &hints, &res);
  if (status != 0) {
    std::cout << "getaddrinfo error" << gai_strerror(status) << "\n";
    return -1;
  }

  addrinfo *p;
  for (p = res; p != NULL; p = p->ai_next) {
    s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (s == -1) {
      continue;
    }
    err = connect(s, p->ai_addr, p->ai_addrlen);
    if (err == -1) {
      close(s);
      continue;
    }
    break;
  }
  if (p == NULL) {
    std::cerr << "Unable to connect\n";
    return 2;
  }
  inet_ntop(p->ai_family, getinaddr(p), ipstr, INET_ADDRSTRLEN);
  std::cout << "Client connected to " << ipstr << "\n";
  /* start connection if got socket fd*/
  freeaddrinfo(res);
  bytes_read = recv(s, buf, sizeof(buf), 0);
  if (bytes_read == -1) {
    std::cerr << "Error receiving\n";
    return 1;
  }
  std::cout << "Client Received: " << std::string(buf) << " bytes\n";
  close(s);
  return 0;
}
