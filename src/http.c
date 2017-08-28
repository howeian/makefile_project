//////////////////////////////////////////////////////////////////
//
//  Copyright(C), 2013-2016, GEC Tech. Co., Ltd.
//
//  File name: teddy/src/http.c
//  RFBP stand for "Resume From BreakPoint"
//
//  Author: Vincent Lin (林世霖)  微信公众号：秘籍酷
//
//  Date: 2017-8
//  
//  Description: HTTP协议处理
//
//  GitHub: github.com/vincent040   Bug Report: 2437231462@qq.com
//
//////////////////////////////////////////////////////////////////

#include "common.h"

void http_request(char*buf, int size, char *filepath, char *host, int start)
{
	assert(buf);

	bzero(buf, size);
	snprintf(buf, size, "GET /%s HTTP/1.1\r\n"
			    "Range: bytes=%d-\r\n"
			    "Host: %s\r\n\r\n", filepath, start, host);
}



long long get_size(char *httphead)
{
	assert(httphead);
	char *delim = "Content-Range: ";

	char *p = strstr(httphead, delim);
	if(p != NULL)
	{
		p += strlen(delim);
		p = strstr(p, "/") + 1;
		return atoll(p);
	}
	return 0LL;
}

long long get_len(char *httphead)
{
	assert(httphead);
	char *delim = "Content-Length: ";

	char *p = strstr(httphead, delim);
	if(p != NULL)
	{
		p += strlen(delim);
		return atoll(p);
	}
	return 0LL;
}






bool check_response(char *httphead)
{
	char *tmp = calloc(1,128);
	memcpy(tmp, httphead, strstr(httphead, "\r\n")-httphead);

	if(strstr(tmp, "20") && !strstr(tmp, "206"))
	{
		fprintf(stderr, "对端服务器不支持断点续传，");
		fprintf(stderr, "本文件将重新下载... ...\n");
		return true;
	}
	else if(strstr(tmp, "206"))
	{
		return true;
	}

	if(strstr(tmp, "30"))
	{
		fprintf(stderr, "重定向错误.\n");
	}
	if(strstr(tmp, "400"))
	{
		fprintf(stderr, "请求无效.\n");
	}
	if(strstr(tmp, "401"))
	{
		fprintf(stderr, "未授权.\n");
	}
	if(strstr(tmp, "403"))
	{
		fprintf(stderr, "禁止访问.\n");
	}
	if(strstr(tmp, "404"))
	{
		fprintf(stderr, "无法找到文件.\n");
	}
	if(strstr(tmp, "405"))
	{
		fprintf(stderr, "资源被禁止.\n");
	}
	if(strstr(tmp, "407"))
	{
		fprintf(stderr, "要求代理身份验证.\n");
	}
	if(strstr(tmp, "410"))
	{
		fprintf(stderr, "永远不可用.\n");
	}
	if(strstr(tmp, "414"))
	{
		fprintf(stderr, "请求URI太长.\n");
	}
	if(strstr(tmp, "50"))
	{
		fprintf(stderr, "服务器错误.\n");
	}

	free(tmp);
	return false;
}






