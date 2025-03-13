#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT "8000"
#define BACKLOG 10

int server();
int main(int argc, char const *argv[]) {
  // client(argc, argv);
  server();
  return 0;
}

void *getinaddr(sockaddr *p) {
  void *actualaddr;
  if (p->sa_family == AF_INET) {
    actualaddr = &(((sockaddr_in *)p)->sin_addr);
  } else {
    actualaddr = &(((sockaddr_in6 *)p)->sin6_addr);
  }
  return actualaddr;
}

void sigchild_handler(int s) {
  int cachederrno = errno;
  while (waitpid(s, NULL, WNOHANG) > 0) {
    std::cout << "Cleaning procees\n";
  };
  errno = cachederrno;
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
  // clean zombie
  struct sigaction sa;

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
    if (err != -1) {
      close(s);
      break;
    }
  }
  if (p == NULL) {
    return 2;
  }
  sa.sa_handler = sigchild_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    std::cerr << "sigaction\n";
  }
  /*
  start listening and accept queued requests
  */
  inet_ntop(p->ai_family, getinaddr(p->ai_addr), ipstr, INET6_ADDRSTRLEN);
  std::cerr << "starting to listen on " << ipstr << "\n";
  freeaddrinfo(res);
  err = listen(s, BACKLOG); // setup queue
  if (err == -1) {
    std::cerr << "Error listening\n";
    return -1;
  }
  while (1) {
    int clientfd = accept(s, (sockaddr *)&their_addr, &(addrlen));
    sockaddr clientsockaddr;
    socklen_t cliendaddrlen;
    memset(&clientsockaddr, 0, sizeof(clientsockaddr));
    err = getpeername(clientfd, &clientsockaddr, &cliendaddrlen);
    if (err == -1) {
      std::cout << "Error getting peername\n";
    }
    inet_ntop(p->ai_family, getinaddr(&clientsockaddr), ipstr,
              INET6_ADDRSTRLEN);
    std::cerr << "client is on  " << ipstr << "\n";
    if (fork() == 0) { // if child remove reference to socket
      close(s);
      std::cout << "pid is " << getpid() << "\n";
      size_t len = sizeof(msg), bytes_sent = 0;
      bytes_sent = send(clientfd, msg, len, 0);
      if (bytes_sent == -1) {
        std::cerr << "Unable to send\n";
      }
      return 0;
    }
    close(clientfd); // no need for parent to keep ref to client socket
  }

  return 0;
}
