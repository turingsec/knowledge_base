#ifndef _COMMON_H_
#define _COMMON_H_

#include <linux/types.h>
#include <sys/types.h>
#include <sys/param.h>
#include <stdbool.h>

char *iptoa(__be32);
char *ethernet_mactoa(unsigned char *);
void arp_force(__be32);
int _find_macaddr_by_ip(char *devname, __be32, unsigned char *);
bool find_macaddr_by_ip(char *devname, __be32, unsigned char *);

unsigned short csum(unsigned short *ptr, int nbytes);

#endif // _COMMON_H_
