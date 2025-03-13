#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory_resource>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "8000"
#define BACKLOG 10

void delete_from_pfds(pollfd *&pfds, int i, int &pfdcount) {
  pfds[i] = pfds[pfdcount];
  pfdcount--;
}

void *get_in_addr(sockaddr *sa) {
  void *sin;
  if (sa->sa_family == AF_INET) {
    sin = &((sockaddr_in *)(sa))->sin_addr;
  } else {
    sin = &((sockaddr_in6 *)(sa))->sin6_addr;
  }
  return sin;
}

void add_fd_to_poll(pollfd *&pfds, int &pfdcount, int &maxfds, int newfd) {
  if (pfdcount == maxfds) { // need realloc
    pfds = (pollfd *)realloc(pfds, sizeof(pollfd) * maxfds * 2);
    maxfds *= 2;
  }
  pfds[pfdcount].fd = newfd;
  pfds[pfdcount].events = POLLIN;
  pfdcount++;
};

int getlisterner() {
  int status = 0, sfd;

  addrinfo hints;
  addrinfo *res;
  addrinfo *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;     // IPV4 or IPV6;
  hints.ai_socktype = SOCK_STREAM; // TCP
  hints.ai_flags = AI_PASSIVE;     // find host name

  status = getaddrinfo(NULL, PORT, &hints, &res);
  if (status == -1) {
    std::cerr << "Error getting addrinfo\n";
    return -1;
  }

  for (p = res; p != nullptr; p = p->ai_next) {
    sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sfd == -1) { // continue till kernel sets up socket
      continue;
    }
    status = bind(sfd, p->ai_addr, p->ai_addrlen);
    if (status == -1) { // continue till able to bind
      close(sfd);
      continue;
    }
    break;
  }

  if (p == nullptr) {
    std::cerr << "Unable to bind \n";
    return -1;
  }
  status = listen(sfd, BACKLOG);
  if (status == -1) {
    std::cout << "Unable to listen\n";
    return -1;
  }
  char ipstr[INET6_ADDRSTRLEN];
  inet_ntop(p->ai_family, get_in_addr(p->ai_addr), ipstr, INET6_ADDRSTRLEN);
  std::cerr << "Will listen @" << ipstr << ":" << PORT << "\n";
  freeaddrinfo(res);
  return sfd;
}

int main(int argc, char const *argv[]) {

  int listener, newfd, bytesread = 0;
  char buf[256], ipstr[INET6_ADDRSTRLEN];

  int maxfds = 5;
  int pfdcount = 0;
  pollfd *pfds = (pollfd *)malloc(sizeof(pollfd) * maxfds);

  listener = getlisterner();
  if (listener == -1) {
    return -1;
  }

  pfds[0].fd = listener;
  pfds[0].events = POLLIN;
  pfdcount++;

  while (1) {
    int pollcount = poll(pfds, pfdcount, -1);
    if (pollcount == -1) {
      std::cerr << "Unable to setup poll\n";
      return -1;
    }
    for (int i = 0; i < pfdcount; i++) {
      if (pfds[i].revents & (POLLIN | POLLHUP)) { // a fd is ready to read

        if (pfds[i].fd == listener) { // if its a listener socket
          sockaddr_storage theiraddr;
          socklen_t slen = sizeof(theiraddr);

          newfd = accept(pfds[i].fd, (sockaddr *)(&theiraddr), &slen);
          if (newfd == -1) {
            std::cerr << "Error accepting client\n";
            continue;
          }
          inet_ntop(theiraddr.ss_family, get_in_addr((sockaddr *)&theiraddr),
                    ipstr, INET6_ADDRSTRLEN);
          std::cerr << "connection from client: " << ipstr
                    << " socket:" << newfd << "\n";
          add_fd_to_poll(pfds, pfdcount, maxfds, newfd);
        } else { // this is a clientfd
          bytesread = recv(pfds[i].fd, buf, sizeof(buf) - 1, 0);
          if (bytesread <= 0) {
            if (bytesread == 0) {
              std::cerr << "Connection closed \n";
            } else {
              std::cerr << "Error recv\n";
            }
            close(pfds[i].fd); // remove from kernel sockets
            delete_from_pfds(pfds, i, pfdcount);
            continue;
          }
          buf[bytesread] = '\0';
          std::cout << "Received: " << buf << "\n";
        }
      }
    }
  }
}
