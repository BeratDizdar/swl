#include "swl.h"
#include "dlfcn.h"

void *swl_LoadLibrary(const char *name) {
    return dlopen(name, RTLD_LAZY);
}

void *swl_GetFunction(void *lib, const char *symbol) {
    return dlsym(lib, symbol);
}

void swl_FreeLibrary(void *lib) {
    dlclose(lib);
}