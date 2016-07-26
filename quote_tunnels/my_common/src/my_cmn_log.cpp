﻿#include "my_cmn_log.h"

namespace
{
// 互斥量的RAII封装
class _MYMutexGuard
{
public:
    _MYMutexGuard(pthread_mutex_t &sync_mutex)
        : sync_mutex_(sync_mutex)
    {
        pthread_mutex_lock(&sync_mutex_);
    }
    ~_MYMutexGuard()
    {
        pthread_mutex_unlock(&sync_mutex_);
    }

private:
    _MYMutexGuard(const _MYMutexGuard&);
    _MYMutexGuard &operator=(const _MYMutexGuard&);

    // 互斥量
    pthread_mutex_t &sync_mutex_;
};
}

using namespace my_cmn;

const char *log_mod_name[LOG_MOD_MAX] =
    {
        "exchange",
        "tunnel",
        "quote",
        "signal",
        "oss"
    };

const char *log_pri_name[LOG_PRI_MAX] =
    {
        "CRITICAL",
        "ERROR",
        "WARN",
        "INFO",
        "DEBUG"
    };

my_log *my_log::_log_inst = NULL;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int my_log::get_proc(void)
{
    _log_pid = getpid();

    /*
     * Get the process name from /proc
     */
    char buff[128];
    FILE *fp;

    snprintf(buff, sizeof(buff), "/proc/%d/status", _log_pid);
    fp = fopen(buff, "r");
    if (fp == NULL)
    {
        return 1;
    }

    fgets(buff, sizeof(buff), fp);

    /* Remove "Name:    " */
    char *p = strstr(buff, "Name:");
    p += sizeof("Name:");

    while (*p == '\t' || *p == 0x20)
    {
        p++;
    }
    char *q = _log_proc;
    while (*p != '\n' && *p != '\0' && *p != '\r')
    {
        *q = *p++;
        q++;
    }
    *q = '\0';

    return 0;
}

my_log * my_log::instance(const char *file, int size, int num)
{
    register int idx;

    if (_log_inst != NULL)
    {
        return _log_inst;
    }

    _MYMutexGuard lock(mutex);
    if (_log_inst != NULL)
    {
        return _log_inst;
    }
    my_log * local_log = new my_log();
    if (local_log == NULL)
    {
        return NULL;
    }
    if (file)
        local_log->set_log_file(file);

    /* Fill the process information */
    // local_log->_log_pid = getpid();
    // strncpy(local_log->_log_proc, argv[0], sizeof(local_log->_log_proc) - 1);
    // local_log->_log_proc[31] = '\0';
    local_log->get_proc();

    if (size == 0)
    {
        local_log->_entry_size = LOG_ENTRY_SIZE;
    }
    else
    {
        local_log->_entry_size = size;
    }

    if (num == 0)
    {
        local_log->_entries_num = LOG_ENTRY_NUM;
    }
    else
    {
        local_log->_entries_num = num;
    }

    /* initialize the _log_file */
    local_log->get_log_file();

    /* initialize the buffer and entries */
    log_entry_t *log_entries = new log_entry_t[local_log->_entries_num];
    if (log_entries == NULL)
    {
        syslog(LOG_ERR, "No enough memory for program %s(%d)\n",
            local_log->_log_proc, local_log->_log_pid);

        goto ERROR;
    }

    local_log->_log_entries = log_entries;

    for (idx = 0; idx < local_log->_entries_num; idx++)
    {
        log_entries[idx].buff = new char[local_log->_entry_size];
        if (log_entries[idx].buff == NULL)
        {
            syslog(LOG_ERR, "No enough memory for program %s(%d)\n",
                local_log->_log_proc, local_log->_log_pid);

            goto ERROR;
        }
    }

    local_log->_next = local_log->_written = log_entries;

    if (pthread_mutex_init(&local_log->_log_mutex, NULL))
    {
        syslog(LOG_ERR,
            "Failed to initialize the mutex for program %s(%d)\n",
            local_log->_log_proc, local_log->_log_pid);

        goto ERROR;
    }

    /* create the thread. */
    idx = pthread_create(&local_log->_log_thd_id, NULL, my_log::_log_worker, local_log);
    if (idx != 0)
    {
        syslog(LOG_ERR,
            "Failed to create log thread for program %s(%d)\n",
            local_log->_log_proc, local_log->_log_pid);

        goto ERROR;
    }
    _log_inst = local_log;
    return _log_inst;

    ERROR:
    if (log_entries != NULL)
    {
        for (idx = 0; idx < _log_inst->_entries_num; idx++)
        {
            delete log_entries[idx].buff;
            log_entries[idx].buff = NULL;
        }

        delete log_entries;
        log_entries = NULL;
        //_log_inst->_log_entries = NULL;
    }
    return NULL;
}

int my_log::set_log_file(const char *file, const char *path)
{
    int len;
    if (path == NULL)
    {
        bzero(_log_path, sizeof(_log_path));
        if (getcwd(_log_path, sizeof(_log_path)) == NULL)
        {
            return 1;
        }
    }
    else
    {
        strncpy(_log_path, path, sizeof(_log_path));
    }
    //syslog(LOG_EMERG, "file: %s, log_path: %s\n", file, _log_path);

    /* Assume the length of file and path is less than MAX_PATH */
    /* assert((strlen(file)+strlen(_log_path)) < sizeof(_log_path)); */
    // strncat(_log_path, file, (sizeof(_log_path) - strlen(_log_path)));
    len = strlen(_log_path);
    snprintf(_log_path + len, sizeof(_log_path) - len, "/%s.log", file);

    FILE *log_file = fopen(_log_path, "a+");
    if (log_file == NULL)
    {
        return 1;
    }

    fseek(log_file, 0, SEEK_END); /* make sure at the end of file. */

    if (_log_file != NULL)
    {
        fclose(_log_file);
    }

    _log_file = log_file;

    return 0;
}

FILE * my_log::get_log_file(void)
{
    if (_log_file == NULL)
    {
        if (set_log_file(_log_proc, NULL) != 0)
        {
            return NULL;
        }
    }

    return _log_file;
}

void my_log::split_log(void)
{
    struct stat st;
    char new_file[256];
    char buf[32];
    struct timespec ts;

    bzero(&st, sizeof(struct stat));
    if (fstat(fileno(_log_file), &st) != 0)
    {
        return;
    }

    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm localtime_result;
    strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", localtime_r(&ts.tv_sec, &localtime_result));
    snprintf(new_file, sizeof(new_file), "%s-%s", _log_path, buf);

    if (st.st_size >= 104857600)
    {
        fclose(_log_file);

        rename(_log_path, new_file);

        _log_file = fopen(_log_path, "a+");
        if (_log_file == NULL)
        {
            syslog(LOG_ERR, "failed to open log file %s\n", _log_path);
        }
    }
}

void * my_log::_log_worker(void *arg)
{
    my_log *log_p = reinterpret_cast<my_log *>(arg);
    char tm_buff[32];
    register int count = 0;
    struct tm localtime_result;

    if (log_p == NULL || log_p->_log_entries == NULL)
    {
        return NULL; /* not prepare, exit */
    }

    log_p->_state = LOG_STATE_RUN;

    while (1)
    {
        if (log_p->_state == LOG_STATE_LASTCHANCE)
        {
            break;
        }

        if (log_p->is_empty())
        {
            usleep(100); /* 10ms */

            continue;
        }

        while ((log_p->_written != log_p->_next) && (count++ < 10000))
        {
            strftime(tm_buff, 32, "%Y-%m-%d %T",
                localtime_r(&log_p->_written->ts.tv_sec, &localtime_result));

            fprintf(log_p->_log_file, "%s.%09lu %s/%d %s %s %s\n",
                tm_buff,
                log_p->_written->ts.tv_nsec,
                log_p->_log_proc, log_p->_log_pid,
                log_mod_name[log_p->_written->mod_id],
                log_pri_name[log_p->_written->priority],
                log_p->_written->buff);

            if (++log_p->_written >= log_p->_log_entries + log_p->_entries_num)
            {
                log_p->_written = log_p->_log_entries;
            }
        }

        fflush(log_p->_log_file); /* flush the data into disk. */
        count = 0; /* Reset the counter. */

        log_p->split_log();

        usleep(100);
    }

    /* Last chance to write the log into syslog. */
    while (log_p->_written != log_p->_next)
    {
        strftime(tm_buff, 32, "%Y-%m-%d %T",
            localtime_r(&log_p->_written->ts.tv_sec, &localtime_result));

        fprintf(log_p->_log_file, "%s.%lu %s/%d %s %s %s\n",
            tm_buff,
            log_p->_written->ts.tv_nsec,
            log_p->_log_proc, log_p->_log_pid,
            log_mod_name[log_p->_written->mod_id],
            log_pri_name[log_p->_written->priority],
            log_p->_written->buff);

        if (++log_p->_written >= log_p->_log_entries + log_p->_entries_num)
        {
            log_p->_written = log_p->_log_entries;
        }
    }

    fflush(log_p->_log_file);

    log_p->_state = LOG_STATE_STOPPED;
    /* Release all the resources here */
    log_p->destroy();

    return NULL;
}

void my_log::destroy(void)
{
    int idx;

    pthread_join(_log_thd_id, NULL);

    if (_log_entries != NULL)
    {
        for (idx = 0; idx < _entries_num; idx++)
        {
            if (_log_entries[idx].buff != NULL)
            {
                delete _log_entries[idx].buff;
                _log_entries[idx].buff = NULL;
            }
        }

        delete _log_entries;
        _log_entries = NULL;
    }

    if (_log_file != NULL)
    {
        fclose(_log_file);
    }

    /* Destroty the thread mutex lock. */
    pthread_mutex_destroy(&_log_mutex);

    /* Release the instance. */
    delete _log_inst;
}

int my_log::log(int mod_id, int priority, const char *format, ...)
{
    va_list valist;
    struct timespec ts;
    log_entry_t *entry;

    if (priority > _level)
    {
        return 0;
    }

    clock_gettime(CLOCK_REALTIME, &ts);

    if (_state >= LOG_STATE_LASTCHANCE)
    {
        return LOG_ERR_HAS_STOPPED;
    }

    if (is_full())
    {
        return LOG_ERR_CACHEFULL;
    }

#ifndef STHREAD
    pthread_mutex_lock(&_log_mutex);
#endif

    entry = _next;

    va_start(valist, format);
    vsnprintf(entry->buff, _entry_size, format, valist);
    va_end(valist);

    entry->ts.tv_sec = ts.tv_sec;
    entry->ts.tv_nsec = ts.tv_nsec;

    entry->mod_id = mod_id;
    entry->priority = priority;
    entry->seq_no++;

    if (++_next >= _log_entries + _entries_num)
    {
        _next = _log_entries;
    }
#ifndef STHREAD
    pthread_mutex_unlock(&_log_mutex);
#endif
    return LOG_SUCCESS;
}

int my_log::shutdown(void)
{
    _state = LOG_STATE_LASTCHANCE;

    return 0;
}

