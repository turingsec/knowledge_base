#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "common.h"

char *ethernet_mactoa(unsigned char *sa_data)
{
	static char buff[256];
	unsigned char *ptr = sa_data;

    memset(buff, 0, sizeof(buff));

	sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X",
		(ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
		(ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));

    return (buff);
}

char *iptoa(__be32 addr)
{
	static char buff[256];
	unsigned char *ptr = (unsigned char *) &addr;
	
    memset(buff, 0, sizeof(buff));
	
	sprintf(buff, "%d.%d.%d.%d",
		(ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
		(ptr[3] & 0377));
	
    return (buff);
}

int _find_macaddr_by_ip(char *devname, __be32 ipaddr, unsigned char *mac)
{
    int                 sock;
    struct arpreq       areq;
    struct sockaddr_in  *sin;

    /* Get an internet domain socket. */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		return -1;
	}

    /* Make the ARP request. */
    memset((void *)&areq, 0, sizeof(areq));
	
    sin = (struct sockaddr_in *) &areq.arp_pa;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = ipaddr;

	sin = (struct sockaddr_in *) &areq.arp_ha;
	sin->sin_family = ARPHRD_ETHER;

	strncpy(areq.arp_dev, devname, 15);

	if (ioctl(sock, SIOCGARP, (caddr_t) &areq) == -1) {
		close(sock);
		perror("-- Error: unable to make ARP request, error");
		return -1;
	}
	
    close(sock);
	
    memcpy(mac, areq.arp_ha.sa_data, ETH_ALEN);
	
	if((mac[0] | mac[1] | mac[2] | mac[3] | mac[4] | mac[5]) == 0){
		return -1;
	}
	
    return 0;
}

bool find_macaddr_by_ip(char *devname, __be32 ipaddr, unsigned char *mac)
{
    int i = 0;
	
    do {
        if(_find_macaddr_by_ip(devname, ipaddr, mac) < 0){
            arp_force(ipaddr);
            continue;
        }
		
		return true;
		
    }while(i++ < 2);

    return false;
}

void arp_force(__be32 ipaddr)
{
    struct sockaddr_in  sin;
	int                 fd;

	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
        return;
	}

	memset(&sin, 0, sizeof(sin));

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = ipaddr;
	sin.sin_port = htons(67);

    sendto(fd, NULL, 0, 0, (struct sockaddr *)&sin, sizeof(sin));

	close(fd);
}

/*
    Generic checksum calculation function
*/
unsigned short csum(unsigned short *ptr, int nbytes)
{
    register long sum;
    unsigned short oddbyte;
    register short answer;
	
    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
	
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }
	
    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer = (short)~sum;
    
    return(answer);
}
