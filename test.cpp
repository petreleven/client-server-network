#include <cstdint>
#include <cstring>
#include <iostream>

struct userInfo {
  char *name[4];
  uint32_t id;
};

int main(int argc, char const *argv[]) {
  userInfo uinfo;
  memset(&uinfo, 0, sizeof(userInfo));
  std::cout << uinfo.name[0] << "\n";
  std::cout << uinfo.id << "\n";
}