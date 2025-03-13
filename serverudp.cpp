#include <arpa/inet.h>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "6000"

void *get_in_addr(sockaddr *p) {
  if (p->sa_family == AF_INET) {
    return &(((sockaddr_in *)p)->sin_addr);
  } else {
    return &((sockaddr_in6 *)p)->sin6_addr;
  }
}

int main() {
  int status, sfd = 0;
  addrinfo hints;
  addrinfo *res;
  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // choose ip for me for the host;
  status = getaddrinfo(NULL, PORT, &hints, &res);

  if (status == -1) {
    std::cerr << "Unable to getaddrinfo\n";
    return 2;
  }

  addrinfo *iteraddrinfo;
  for (iteraddrinfo = res; iteraddrinfo != NULL;
       iteraddrinfo = iteraddrinfo->ai_next) {
    sfd = socket(iteraddrinfo->ai_family, iteraddrinfo->ai_socktype,
                 iteraddrinfo->ai_protocol);
    if (sfd == -1) { // unable to create this socket
      continue;
    }
    status = bind(sfd, iteraddrinfo->ai_addr, iteraddrinfo->ai_addrlen);
    if (status == -1) { // unable to bind to this socket
      close(sfd);
      continue;
    }
    break;
  }
  if (iteraddrinfo == NULL) {
    std::cerr << "Unable to find any valid addrinfo or bind \n";
    return 1;
  }
  char buf[100];
  sockaddr_storage theiraddr;
  socklen_t addrlen = sizeof(theiraddr);
  char ipstr[INET6_ADDRSTRLEN];

  inet_ntop(iteraddrinfo->ai_family, get_in_addr(iteraddrinfo->ai_addr), ipstr,
            INET6_ADDRSTRLEN);
  std::cerr << "Starting to listen on: " << ipstr << "\n";

  status = recvfrom(sfd, buf, sizeof(buf) - 1, 0, (sockaddr *)(&theiraddr),
                    &addrlen);
  if (status == -1) {
    std::cerr << "Error receiving\n";
    return 1;
  }

  inet_ntop(iteraddrinfo->ai_family, get_in_addr((sockaddr *)&theiraddr), ipstr,
            INET6_ADDRSTRLEN);
  buf[status] = '\0';
  std::cerr << "UDP Server Received " << buf << "\n";
  freeaddrinfo(res);
  close(sfd);
}
