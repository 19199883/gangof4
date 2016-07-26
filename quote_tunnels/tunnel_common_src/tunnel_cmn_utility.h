#if !defined(TUNNEL_CMN_UTILITY_H_)
#define TUNNEL_CMN_UTILITY_H_

#include <pthread.h>
#include <string>
#include <vector>
#include <set>
#include <float.h>

#include "my_cmn_log.h"
#include "qtm.h"

#define DLL_PUBLIC __attribute__ ((visibility ("default")))

class MYMutexGuard
{
public:
    MYMutexGuard(pthread_mutex_t &sync_mutex) : sync_mutex_(sync_mutex)
    {
        pthread_mutex_lock(&sync_mutex_);
    }
    ~MYMutexGuard()
    {
        pthread_mutex_unlock(&sync_mutex_);
    }

private:
    MYMutexGuard(const MYMutexGuard&);
    MYMutexGuard &operator=(const MYMutexGuard&);

    // 互斥量
    pthread_mutex_t &sync_mutex_;
};

typedef std::pair<std::string, unsigned short> IPAndPortNum;
IPAndPortNum ParseIPAndPortNum(const std::string &addr_cfg);

typedef std::pair<std::string, std::string> IPAndPortStr;
IPAndPortStr ParseIPAndPortStr(const std::string &addr_cfg);

void TunnelUpdateState(const char *name, int s);

#endif  //
