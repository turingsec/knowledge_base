#ifndef __TCP_H_
#define __TCP_H_

#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/types.h>
#include <stddef.h>
#include <time.h>
#include <stdbool.h>
#include "../../include/common.h"

#define ETHER_TYPE		0x0800

#define DEFAULT_IF		"eth0"
#define BUF_SIZ			(1024 * 64 + 1024)

#define DEVICE_MTU		1440
#define MAX_PACKET		(DEVICE_MTU + sizeof(struct ethhdr))

#define MAX_TRACK		4096

#define TIMEOUT			60

#define MEGA512			524288

#define LOGON			false

#define TCP_OPTION_ENDOFLIST	0
#define TCP_OPTION_NOP			1
#define TCP_OPTION_MSS			2
#define TCP_OPTION_WSOPT		3
#define TCP_OPTION_SACK_PER		4
#define TCP_OPTION_SACK			5
#define TCP_OPTION_ECHO			6
#define TCP_OPTION_ECHO_REPLY	7
#define TCP_OPTION_TSOPT		8
#define TCP_OPTION_POCP			9
#define TCP_OPTION_POSP			10
#define TCP_OPTION_CC			11
#define TCP_OPTION_CCNEW		12
#define TCP_OPTION_CCECHO		13
#define TCP_OPTION_ACR			14
#define TCP_OPTION_TCO			18
#define TCP_OPTION_MD5			19
#define TCP_OPTION_QUICKSTART	27
#define TCP_OPTION_USERTIMEOUT	28

typedef unsigned int	uint;

struct tcp_segment
{
	struct tcp_segment	*next;
	uint				seq;
	
	char				*head;
	uint				datalen;
	
	struct iphdr		iph;
	struct tcphdr		tcph;
};

struct host_info
{
	__be32				addr;
	__be16				port;
	
	uint				transmited_bytes;
	uint				rcv_bytes;
	
	struct tcp_segment	*unorder_list;
	struct tcp_segment	*transmit_list;
};

/*
	This struct work for track the fake seq/ack of the connection between A and B
*/
struct track_connection
{
	struct track_connection	*next;
	
#define	CLIENT	0
#define SERVER	1
#define MAX_END	2
	struct host_info		ends[MAX_END];
	
#define TCP_UNUSED		0
#define TCP_SYN			1
#define TCP_SYN_ACK		2
#define TCP_ESTABLISH	3
#define TCP_SHUTDOWN	4
	short					state;
	
	time_t					lasttime;
};

struct track_head
{
	struct	track_connection	*next;
	int							len;
};

/*
    96 bit (12 bytes) pseudo header needed for tcp header checksum calculation 
*/
struct pseudo_header
{
    u_int32_t	source_address;
    u_int32_t	dest_address;
    u_int8_t	placeholder;
    u_int8_t	protocol;
    u_int16_t	tcp_length;
};

struct tcp_sack_block
{
	uint	left_edge;
	uint	right_edge;
};

struct tcp_option_sack
{
	unsigned char			count;
	struct tcp_sack_block	blocks[4];	
};

void parse_tcp_option(struct tcp_option_sack *sack, struct tcphdr *tcph);

bool insert_list(struct tcp_segment **head, struct tcp_segment *newone);
void release_list(struct tcp_segment **head);

void tcp_sack_reply(struct host_info *tx_host, struct host_info *rx_host, struct tcp_segment *segment);

void retransmit_frame(int sockfd, struct host_info *tx_host, struct host_info *rx_host, struct tcphdr *tcph);
					
void tcp_send_ack(int sockfd, struct host_info *tx_host, struct host_info *rx_host);

void tcp_keepalive_ack(int sockfd, struct host_info *tx_host, struct host_info *rx_host);

struct tcp_segment* tcp_peek_list(struct tcp_segment **, uint next_seq, bool remove);

void tcp_rcv_unorder(char *head, 
					uint datalen, 
					struct host_info *tx_host, 
					struct host_info *rx_host, 
					struct iphdr *iph, 
					struct tcphdr *tcph, 
					__be32 daddr);

void tcp_forward_finish_update(char *head, 
					uint datalen, 
					struct host_info *host, 
					struct iphdr *iph,
					struct tcphdr *tcph);

void tcp_forward_finish(int sockfd, 
				char *head, 
				uint datalen, 
				struct host_info *tx_host, 
				struct host_info *rx_host, 
				struct iphdr *iph, 
				struct tcphdr *tcph, 
				__be32 daddr,
				bool retransmit);
				
void tcp_forward(int sockfd, 
				char *head, 
				uint datalen, 
				struct host_info *tx_host, 
				struct host_info *rx_host, 
				struct iphdr *iph, 
				struct tcphdr *tcph, 
				__be32 daddr);
				
#endif
