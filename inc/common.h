//////////////////////////////////////////////////////////////////
//
//  Copyright(C), 2013-2016, GEC Tech. Co., Ltd.
//
//  File name: teddy/inc/common.h
//  RFBP stand for "Resume From BreakPoint"
//
//  Author: Vincent Lin (林世霖)  微信公众号：秘籍酷
//
//  Date: 2017-8
//  
//  Description: 通用头文件
//
//  GitHub: github.com/vincent040   Bug Report: 2437231462@qq.com
//
//////////////////////////////////////////////////////////////////

#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <semaphore.h>

#define CONNECTING 0
#define CONNECT_FAIL 1
#define CONNECT_SUCCESS 2


#define SEARCHING 0
#define SEARCH_FAIL 1
#define SEARCH_SUCCESS 2

extern int g_connecting;
extern int g_searching;

extern sem_t s;
extern char *filename;

void http_request(char*buf, int size, char *filepath, char *host, int start);
void arg_parser(char *arg, char **host, char **file);
bool check_response(char *httphead);
long long get_size(char *httphead);
long long get_len(char *httphead);

#endif
