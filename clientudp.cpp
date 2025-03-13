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

  status = getaddrinfo("localhost", PORT, &hints, &res);

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
    break;
  }
  if (iteraddrinfo == NULL) {
    std::cerr << "Unable to find the  addrinfo  \n";
  }
  std::string buf = "get this bwuoy";

  char ipstr[INET6_ADDRSTRLEN];

  inet_ntop(iteraddrinfo->ai_family, get_in_addr(iteraddrinfo->ai_addr), ipstr,
            INET6_ADDRSTRLEN);
  std::cerr << "sending to: " << ipstr << "\n";

  status = sendto(sfd, buf.c_str(), buf.size() + 1, 0, iteraddrinfo->ai_addr,
                  iteraddrinfo->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error sending\n";
    return 1;
  }
  std::cerr << "Sent\n ";
  freeaddrinfo(res);
  close(sfd);
}
