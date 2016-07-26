#include "../my_exchange.h"
#include "loadlibraryproxy.h"
#include "makertype.h"

#ifdef LINUX
	#include <limits.h>
	#include <unistd.h>
	#include <dlfcn.h>

#elif defined WINDOWS
	#include <windows.h>
	#include <direct.h>
#else /* NIX */
	#include <limits.h>
	#define _PSTAT64
	#include <sys/pstat.h>
	#include <sys/types.h>
	#include <unistd.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>




CLoadLibraryProxy* CLoadLibraryProxy::pLoadLibraryProxy = NULL;
unsigned long CLoadLibraryProxy::CLoadLibraryProxyCounter  = 0;

CLoadLibraryProxy::CLoadLibraryProxy()
{
	this->pmoduleloader = NULL;
  dl_list.clear();
}

CLoadLibraryProxy::~CLoadLibraryProxy()
{
  if(pmoduleloader)
  {
     delete pmoduleloader;
     pmoduleloader = NULL;
  }
}

CLoadLibraryProxy* CLoadLibraryProxy::CreateLoadLibraryProxy()
  {
    if(!CLoadLibraryProxy::pLoadLibraryProxy)
    {
       if((pLoadLibraryProxy = new(CLoadLibraryProxy)))
       {
          CLoadLibraryProxyCounter ++;
          return pLoadLibraryProxy;
       }
       else
       {
          return NULL;
       }
     }
     else
     {
       CLoadLibraryProxyCounter++;
       return pLoadLibraryProxy;
     }
  }

unsigned long CLoadLibraryProxy::DeleteLoadLibraryProxy()
{
  if(!(CLoadLibraryProxyCounter--))
  {
    CLoadLibraryProxyCounter = 0;
    delete this;
  }
  return CLoadLibraryProxyCounter;
}

bool CLoadLibraryProxy::IsCreatedLoadLibraryProxy()
{
   if(CLoadLibraryProxyCounter)
     return true;
   else
     return false;
}

void CLoadLibraryProxy::setModuleLoadLibrary( CModuleLoadLibrary * moduleloader)
{
  pmoduleloader = moduleloader;
}

CModuleLoadLibrary *CLoadLibraryProxy::getModuleLoadLibrary()
{
  return pmoduleloader;
}

void CLoadLibraryProxy::setBasePathLibrary(std:: string basepathlibrary)
{
  basepath = basepathlibrary;
}

std::string CLoadLibraryProxy::getBasePathLibrary()
{
  return basepath;
}

void *CLoadLibraryProxy::findObject(std::string sobjectname, std::string methodname)
{
  
#ifdef WINDOWS
  std::string   path             = basepath + getPathSeparator() +  sobjectname + ".dll";
#else
  std::string   path             = basepath + getPathSeparator() +  sobjectname + ".so";
#endif

  void         *mkr              = NULL;


  ObjectInfo    lObjectInfo;
  void *phandle                  = NULL;
  std::map<std::string, ObjectInfo>::iterator it = dl_list.find(sobjectname);
  if (it == dl_list.end())
  {
	  phandle = pmoduleloader->Open(path);	  
	  if(phandle == NULL) 
	  {
#ifdef LINUX
		std::string err = dlerror();
		fprintf(stderr, "%s\n", dlerror());
#endif
		return NULL;
	  }
	  else
	  {
		  lObjectInfo.handle = phandle;
		  lObjectInfo.name  = sobjectname;
		  dl_list.insert(std::pair<std::string,ObjectInfo>(sobjectname,lObjectInfo) );
	  }
  }
  else
  {
	  phandle = it->second.handle;
  }    
    
  mkr = pmoduleloader->Load(phandle, methodname);
  if(mkr == NULL)
  {
#ifdef LINUX
	std::string err = dlerror();
	fprintf(stderr, "%s\n", dlerror());
#endif
	return NULL;
  }
 
  return mkr;
}

const char CLoadLibraryProxy::getPathSeparator()
{
#ifdef WINDOWS
  return '\\';
#else
  return '/';
#endif
}

const char CLoadLibraryProxy::getNameSpaceSeparator()
{
  return '_';
}

void CLoadLibraryProxy::deleteObject(std::string sobjectname)
{  
	std::map<std::string, ObjectInfo>::iterator dl_list_iterator = dl_list.find(sobjectname);
  
	if(dl_list_iterator != dl_list.end())
    {
		pmoduleloader->Close(dl_list_iterator->second.handle);
        dl_list.erase(dl_list_iterator);      
    }  
}

std::string CLoadLibraryProxy::getexedir()
{
  std::string path = getexepath();
#if defined (WINDOWS)
  const char c = '\\';
#else
  const char c = '/';
#endif
  std::string::iterator it;

  for ( it = path.end()-1 ; it != path.begin(); it-- )
  {
    if((*it) == c) break;
  }

  path.erase(it, path.end());
  return path;
}

const char* CLoadLibraryProxy::get_directory( char* path )
{
#if defined (WINDOWS)
  const char *c = "\\";
#else
  const char* c = "/";
#endif
  getcwd( path, 260 );
  strcat( path, c );
  return path;
}

std::string CLoadLibraryProxy::getexepath()
{
#ifdef LINUX

 char result[ PATH_MAX ];
 ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
 return std::string( result, (count > 0) ? count : 0 );

#elif defined WINDOWS

 char result[ MAX_PATH ];
 return std::string( result, GetModuleFileName( NULL, result, MAX_PATH ) );

#else /* HP-UX */

  char result[ PATH_MAX ];
  struct pst_status ps;

  if (pstat_getproc( &ps, sizeof( ps ), 0, getpid() ) < 0)
    return std::string();

  if (pstat_getpathname( result, PATH_MAX, &ps.pst_fid_text ) < 0)
    return std::string();

  return std::string( result );

#endif
}



