/*
 * share_mem.h
 */

#ifndef _SHARE_MEM_H_
#define _SHARE_MEM_H_


#pragma pack(push)
#pragma pack(8)
#define INTERFACE extern "C" __attribute__((visibility("default")))



INTERFACE void write_share_mem(char *mem, void *ords);
INTERFACE void * env_init(int share_memory_key);
INTERFACE void env_fini(void * shm_addr);



#pragma pack(pop)


#endif

