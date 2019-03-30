#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()

#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_RAW, INET_ADDRSTRLEN
#include <netinet/ip.h>       // IP_MAXPACKET (which is 65535)
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_ARP = 0x0806
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>

#include <errno.h>            // errno, perror()

#include <netinet/in.h>
#include <net/route.h>


 /**
    * Create socket function
    */
    int create_socket() {

      int sockfd = 0;

      sockfd = socket(AF_INET, SOCK_DGRAM, 0);
      if(sockfd == -1){
        fprintf(stderr, "Could not get socket.\n");
        return -1;
      }

      return sockfd;

    }

    /**
    * Generic ioctrlcall to reduce code size
    */
    int generic_ioctrlcall(int sockfd, u_long *flags, struct ifreq *ifr) {

      if (ioctl(sockfd, (long unsigned int)flags, &ifr) < 0) {
        // fprintf(stderr, "helllllo\n");
        fprintf(stderr, "ioctl: %s\n", (char *)flags);
        fprintf(stderr, "ioctl error: %s\n", strerror(errno));
        return -1;
      }
      return 1;
    }

    /**
    * Set route with metric 100
    */
    int set_route(int sockfd, char *gateway_addr,  struct sockaddr_in *addr) {
      struct rtentry route;
      int err = 0;
      memset(&route, 0, sizeof(route));
      addr = (struct sockaddr_in*) &route.rt_gateway;
      addr->sin_family = AF_INET;
      addr->sin_addr.s_addr = inet_addr(gateway_addr);
      addr = (struct sockaddr_in*) &route.rt_dst;
      addr->sin_family = AF_INET;
      addr->sin_addr.s_addr = inet_addr("0.0.0.0");
      addr = (struct sockaddr_in*) &route.rt_genmask;
      addr->sin_family = AF_INET;
      addr->sin_addr.s_addr = inet_addr("0.0.0.0");
      route.rt_flags = RTF_UP | RTF_GATEWAY;
      route.rt_metric = 100;
      err = ioctl(sockfd, SIOCADDRT, &route);
      if ((err) < 0) {
        // fprintf(stderr, "ioctl: %s\n",  "mahdi MOAHMMADI Error");
        fprintf(stderr, "ioctl error: %s\n", strerror(errno));
        return -1;
      }
      return 1;
    }

    /**
    * Set ip function
    */
    int set_ip(char *iface_name, char *ip_addr, char *gateway_addr)
    {
      if(!iface_name)
        return -1;
      struct ifreq ifr;
      struct sockaddr_in sin;
      int sockfd = create_socket();

      sin.sin_family = AF_INET;

      // Convert IP from numbers and dots to binary notation
      inet_aton(ip_addr,&sin.sin_addr.s_addr);

      /* get interface name */
      strncpy(ifr.ifr_name, iface_name, IFNAMSIZ);

      /* Read interface flags */
      generic_ioctrlcall(sockfd, (u_long *)"SIOCGIFFLAGS", &ifr);
      /*
      * Expected in <net/if.h> according
      * "UNIX Network Programming".
      */
      #ifdef ifr_flags
      # define IRFFLAGS       ifr_flags
      #else   /* Present on kFreeBSD */
      # define IRFFLAGS       ifr_flagshigh
      #endif
      // If interface is down, bring it up
      if (ifr.IRFFLAGS | ~(IFF_UP)) {
        ifr.IRFFLAGS |= IFF_UP;
        generic_ioctrlcall(sockfd, (u_long *)"SIOCSIFFLAGS", &ifr);
      }
      // Set route
      set_route(sockfd, gateway_addr    ,  &sin);
      memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
      // Set interface address
	    //  fprintf(stderr,"hello\n");
      if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0) {
	      //  fprintf(stderr,"hello\n");
        fprintf(stderr, "Cannot set IP address. ");
        perror(ifr.ifr_name);
        fprintf(stderr, "ioctl error: %s\n", strerror(errno));
        return -1;
      }
      #undef IRFFLAGS

      return 0;
    }

int main(int argc, char** argv) {
  char* interface = 0;
  char* ip_addr = 0;
  char* gateway = 0;

  if (argc < 4) {
    printf("Insufficient number of arguments...\n\n");
    printf("Usage: %s <interface> <ip address> <gateway>\n\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  interface = argv[1];
  ip_addr = argv[2];
  gateway = argv[3];
	set_ip(interface, ip_addr, gateway);

  printf("IP Address of %s changed to %s with gateway %s.\n", interface, ip_addr, gateway);

  return EXIT_SUCCESS;
}
