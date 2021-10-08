#include <dlfcn.h>

#include <stdint.h>
#include <stdio.h>

typedef int32_t (*add_t)(int32_t val1, int32_t val2);

int main() {
  void* lib = dlopen("./libadd.so", RTLD_LAZY);
  add_t add = (add_t)dlsym( lib, "add" );

  int32_t data = add(1, 2);
  printf("%d\n", data);
  
  dlclose(lib);
}

