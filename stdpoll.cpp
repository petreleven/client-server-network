#include <iostream>
#include <poll.h>
#include <sys/poll.h>

int main(int argc, char const *argv[]) {
  int status;
  pollfd fds[1];
  pollfd stdoutfd;
  stdoutfd.fd = 0;
  stdoutfd.events = POLLIN;

  status = poll(fds, 1, 10000); // kernel to listen for 10s
  if (status == -1) {
    std::cerr << "Error trying to setup poll\n";
    return -1;
  } else if (status == 0) {
    std::cerr << "Timed out!!!\n";
    return -1;
  }
  if (fds[0].revents & POLLIN) {
    std::cout << "Pollin occured \n";
  } else {
    std::cout << "Unexpected event";
  }

  return 0;
}
