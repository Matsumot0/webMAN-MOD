#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netex/net.h>
#include <netdb.h>

#include "include/vsh_exports.h"
#include "include/network.h"



static int db_s = -1;											 // debug socket




/***********************************************************************
* create a udp/tcp connection
* 	char *ip      = string ip adresse
* 	uint16_t port = port
* 	uint8_t pro   = protokoll, udp = 1 or tcp = 2
***********************************************************************/
int32_t create_conn(const char *ip, uint16_t port, uint8_t pro)
{
	int32_t s = 0;
	struct sockaddr_in sin;
	uint32_t _ip = 0;


	memset((char *) &sin, 0, sizeof(struct sockaddr_in));

	if((_ip = inet_addr(ip)) != (uint32_t) -1)
	{
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = _ip;
	}
	else
	{
		struct hostent *hp;

		if((hp = gethostbyname(ip)) == NULL)
			return -1;

		sin.sin_family = hp->h_addrtype;
		memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
	}

	sin.sin_port = htons(port);

	switch(pro)
	{
		case 1: s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); break;
		case 2: s = socket(AF_INET, SOCK_STREAM, 0); break;
	}

	if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		return -1;

	return s;
}

/***********************************************************************
* listen...
***********************************************************************/
int32_t listen_conn(int32_t port, int32_t backlog)
{
	int s = 0;
	struct sockaddr_in sa;


	s = socket(AF_INET, SOCK_STREAM, 0);

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(s, (struct sockaddr *)&sa, sizeof(sa));

	listen(s, backlog);

	return s;
}

/***********************************************************************
* destroy a udp/tcp connection
* 	int *s = socket to close
***********************************************************************/
void close_conn(int32_t *s)
{
	if(*s != -1)
	{
		shutdown(*s, SHUT_RDWR);
		socketclose(*s);
		*s = -1;
	}
}

/***********************************************************************
* debug init
***********************************************************************/
void dbg_init(void)
{
	db_s = create_conn(DB_IP, DB_PORT, 1);
}

/***********************************************************************
* debug finilize
***********************************************************************/
void dbg_fini(void)
{
	close_conn(&db_s);
}

/***********************************************************************
* debug Printf
***********************************************************************/
void dbg_printf(const char* fmt, ...)
{
  char buf[0x200];
  va_list arg;
  va_start(arg, fmt);
  vsnprintf(buf, sizeof(buf), fmt, arg);
  va_end(arg);
  send(db_s, buf, strlen(buf), 0);
}

