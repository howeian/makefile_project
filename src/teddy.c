#include "common.h"

int g_searching  = SEARCHING;


char *filename;
sem_t s;

void usage(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("Usage: %s URL\n", argv[0]);
		exit(0);
	}
}




void progress(long long nread, long long filesize)
{
	struct winsize ws;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

	int bar_len = ws.ws_col-32;
	bar_len = bar_len > 60 ? 60 : bar_len;

	int rate = filesize/bar_len;
	int cur  = nread/rate;

	char *total = calloc(1, 16);
	if(filesize < 1024)
		snprintf(total, 16, "%llu", filesize);
	else if(filesize >= 1024 && filesize < 1024*1024)
		snprintf(total, 16, "%.1fKB", (float)filesize/1024);
	else if(filesize >= 1024*1024 && filesize < 1024*1024*1024)
		snprintf(total, 16, "%.1fMB", (float)filesize/(1024*1024));


	char *bar = calloc(1, 128);
	if(nread < 1024)
		snprintf(bar, 128, "\r[%llu/%s] [", nread, total);
	else if(nread < 1024*1024)
		snprintf(bar, 128, "\r[%.1fKB/%s] [", (float)nread/1024, total);
	else if(nread < 1024*1024*1024)
		snprintf(bar, 128, "\r[%.1fMB/%s] [", (float)nread/(1024*1024), total);
	free(total);

	int i;
	for(i=0; i<cur; i++)
		snprintf(bar+strlen(bar), 128-strlen(bar)-i, "%s", "#");

	for(i=0; i<bar_len-cur-1; i++)
		snprintf(bar+strlen(bar), 128-strlen(bar)-i, "%s", "-");

	snprintf(bar+strlen(bar), 128-strlen(bar),
			"] [%.1f%%]%c", (float)nread/filesize*100,
			nread==filesize?'\n':' ');
	fprintf(stderr, "%s", bar);
	free(bar);
}



void arg_parser(char *arg, char **host, char **file)
{
	// 从程序的逻辑上分析，本函数的3个参数都绝不可能是NULL
	// 但是暂时无法100%排除BUG的存在，那就使用assert()来进行所谓的断言
	// 如果不小心确实出现了NULL，那么assert将会第一时间告诉你！并立刻退出
	assert(arg);
	assert(host);
	assert(file);

	if(arg[strlen(arg)-1] == '/')
	{
		fprintf(stderr, "非法链接.\n");
		exit(0);
	}

	char *h, *f;
	h = f = arg;

	char *delim1 = "http://";
	char *delim2 = "https://";
	if(strstr(arg, delim1) != NULL)
	{
		h += strlen(delim1);
	}
	else if(strstr(arg, delim2) != NULL)
	{
		h += strlen(delim2);
	}

	f = strstr(h, "/");
	if(f == NULL)
	{
		fprintf(stderr, "非法链接.\n");
		exit(0);
	}
	f += 1;

	*host = calloc(1, 256);
	*file = calloc(1, 2048);

	memcpy(*host, h, f-h-1);
	memcpy(*file, f, strlen(f));
}





void *searching_host(void *arg)
{
	pthread_detach(pthread_self()); // 避免成为僵尸

	fprintf(stderr, "正在寻找主机");
	while(g_searching == SEARCHING)
	{
		fprintf(stderr, ".");
		usleep(500*1000);
	}
	printf("%s", g_searching==SEARCH_SUCCESS ? "[OK]\n": "[FAIL]\n");
	sem_post(&s);

	pthread_exit(NULL);
}






void remote_info(struct hostent *he)
{
	assert(he);

	printf("主机的官方名称：%s\n", he->h_name);

	int i;
	for(i=0; he->h_aliases[i]!=NULL; i++)
	{
		printf("别名[%d]：%s\n", i+1, he->h_aliases[i]);
	}

	printf("IP地址长度：%d\n", he->h_length);

	for(i=0; he->h_addr_list[i]!=NULL; i++)
	{
		printf("IP地址[%d]：%s\n", i+1,
			inet_ntoa(*((struct in_addr **)he->h_addr_list)[i]));
	}
}




int main(int argc, char **argv) // ./teddy   www.xx.com   /xxx/yy/zzz/a.txt
{
	usage(argc, argv);
	
	char *host = NULL;
	char *filepath = NULL;
	arg_parser(argv[1], &host, &filepath);
	
#ifdef DEBUG
	printf("host: %s\n", host);
	printf("filepath: %s\n", filepath);
#endif


	// 显示连接服务器的等待过程... ...
	sem_init(&s, 0, 0);
	pthread_t tid;
	pthread_create(&tid, NULL, searching_host, NULL);

	
	struct hostent *he = gethostbyname(host);
	if(he == NULL && errno == HOST_NOT_FOUND)
	{
		g_searching = SEARCH_FAIL;
		sem_wait(&s);
		fprintf(stderr, "主机名有误，或DNS没配置好.\n");
		exit(0);
	}
	else if(he == NULL)
	{
		g_searching = SEARCH_FAIL;
		sem_wait(&s);
		fprintf(stderr, "非法链接.\n");
		exit(0);
	}
	g_searching = SEARCH_SUCCESS;
	sem_wait(&s);
	
	

#ifdef DEBUG
		remote_info(he);
#endif


	// 取得站点主机IP并发起连接请求
	struct in_addr **addr_list = (struct in_addr **)(he->h_addr_list);

	int fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in srvaddr;
	socklen_t addrlen = sizeof(srvaddr);
	bzero(&srvaddr, addrlen);

	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port   = htons(80);
	srvaddr.sin_addr   = *addr_list[0];

	if(connect(fd, (struct sockaddr *)&srvaddr, addrlen) == -1)
	{
		perror("connect() failed");
		exit(0);
	}

	
	// 准备好本地文件，如果已下载了部分内容，则计算还需下载的字节量
	FILE *fp = NULL;
	long long curlen = 0LL;
	
	// 从带路径的filepath取得最后的不带路径的文件名filename
	if(strstr(filepath, "/"))
		filename = strrchr(filepath, '/')+1;
	else
		filename = filepath;
	
	// 如果这个文件不存在，那么直接创建他就好了
	if(access(filename, F_OK))
	{
		fp = fopen(filename, "w");
	}
	// 如果这个文件存在，那么获取其大小，作为HTTP请求报文的参数
	// 并且以追加的模式打开这个文件，进行续传
	else
	{
		struct stat fileinfo;
		stat(filename, &fileinfo);

		curlen = fileinfo.st_size;
		fp = fopen(filename, "a");
	}
	
	
	if(fp == NULL)
	{
		perror("fopen() failed");
		exit(0);
	}
	
	// 为了防止将来下载的过程中发生异常退出导致数
	// 据无法及时写入磁盘文件，将标准缓冲区设置为不缓冲
	setvbuf(fp, NULL, _IONBF, 0);
	
	
	// 给站点发送HTTP请求报文
	char *sndbuf = calloc(1, 1024);
	http_request(sndbuf, 1024, filepath, host, curlen);   

	int n = send(fd, sndbuf, strlen(sndbuf), 0);
	if(n == -1)
	{
		perror("send() failed");
		exit(0);
	}
	
	
	
#ifdef DEBUG
	printf("Request:\n");
	printf("+++++++++++++++++++++++++++++++\n");
	printf("%s", sndbuf);
	printf("+++++++++++++++++++++++++++++++\n");
	printf("[%d] bytes have been sent.\n\n", n);
#endif
	free(sndbuf);
	
	

	long long total_bytes = curlen;
	
	// 读取站点返回的HTTP响应报文的首部
	char *httphead = calloc(1, 2048);
	n = 0;
	while(1)
	{
		read(fd, httphead+n, 1);
		n++;
		if(strstr(httphead, "\r\n\r\n"))
			break;
	}

	
	long long size; // 要下载的文件的总大小
	long long len ; // 本次要下载的大小
	size = get_size(httphead);
	len  = get_len(httphead);	
	
	
	
	
	// 检查HTTP响应报文
	// 返回：真：服务器正常返回
	//       假：发生错误
	if(!check_response(httphead))
	{
		// 文件已经下载
		if(curlen == size && curlen != 0)
		{
			fprintf(stderr, "该文件已下载.\n");
		}

		// 其他错误
		if(!access(filename, F_OK) && curlen == 0)
			remove(filename);

		exit(0);
	}

	free(httphead);
	
	
	
	// 读取下载的文件内容
	char *recvbuf  = calloc(1, 1024);
	while(1)
	{
		n = recv(fd, recvbuf, 1024, 0);

		if(n == 0)
			break;

		if(n == -1)
		{
			perror("recv() failed");
			exit(0);
		}

		fwrite(recvbuf, n, 1, fp);

		total_bytes += n;
		progress(total_bytes, size);

		if(total_bytes >= size)
			break;
	}

	
	fclose(fp);
	free(recvbuf);
	
	
	return 0;
	
	
}