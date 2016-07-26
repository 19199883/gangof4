
#include "moduleloadlibrarylinux.h"
#include <dlfcn.h>

CModuleLoadLibraryLinux::CModuleLoadLibraryLinux()
{
}

CModuleLoadLibraryLinux::~CModuleLoadLibraryLinux()
{
}

void* CModuleLoadLibraryLinux::Open(std::string path)
{
  return dlopen(path.c_str(), RTLD_NOW);
}

int CModuleLoadLibraryLinux::Close(void *handle)
{
  return (handle != NULL) ? dlclose(handle) : NULL;
}
   
void* CModuleLoadLibraryLinux::Load(void *handle, std::string symbol)
{
  return dlsym(handle, symbol.c_str());
}

std::string CModuleLoadLibraryLinux::getErrorDescription()
{
  return std::string(dlerror());
}
