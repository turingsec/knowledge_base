#ifndef _ROUTE_H_
#define _ROUTE_H_

#include <linux/types.h>
#include <sys/types.h>
#include <sys/param.h>
#include <stdbool.h>

/* local interface info */
struct ifcfg
{
	char *iface;
	unsigned char hwa[6];
	int		if_index;
	struct	in_addr ipa;
	struct	in_addr bcast;
	struct	in_addr nmask;
	uint	ip_subnet;
	u_short mtu;
};

int		get_gateway();
int 	get_local_info(int rsock, struct ifcfg *ifcfg);

#endif