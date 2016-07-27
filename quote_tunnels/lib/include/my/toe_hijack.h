#ifndef __TOE_HIJACK_H_
#define __TOE_HIJACK_H_

#ifdef __cplusplus
extern "C" {
#endif	

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "toe_app.h"

int toe_hijack_init(unsigned char *src_ip_toe0, unsigned char *src_ip_toe1,
					unsigned char *dst_ip_list0, unsigned char *dst_ip_list1);

#ifdef __cplusplus
}
#endif

#endif