#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include <net/if.h>

#define DST_ADDR "224.0.0.10"
#define BIND_ADDR "10.0.0.245"
#define DST_PORT 9999
#define IF_NAME "eth0"

int main()
{
    int sock;
    struct sockaddr_in addr;
    char buffer[256];

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock == -1)
    {
        perror("SOCKET: ");
        exit(EXIT_FAILURE);
    }
    
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DST_PORT);
    if(inet_aton(DST_ADDR,&addr.sin_addr) == 0)
    {
        printf("ERROR: inet_aton\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    //JEDINE NOVE - BIND a RECVFROM, scanf preč
    if(bind(sock,(struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        perror("BIND: ");
        close(sock);
        exit(EXIT_FAILURE);
    }

// struktura pre join do mcast skupiny
    struct ip_mreqn allow_multi;
    bzero(&allow_multi,sizeof(allow_multi));
    if(inet_pton(AF_INET, DST_ADDR, &allow_multi.imr_multiaddr) == -1)
    {
        perror("PTON_MULTIADDR: ");
        close(sock);
        exit(EXIT_FAILURE);
    }

    allow_multi.imr_ifindex = if_nametoindex(IF_NAME);
    if(allow_multi.imr_ifindex == 0)
    {
        perror("IF_NAMETOINDEX MULTIADDR: ");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if(setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &allow_multi, sizeof(allow_multi)) == -1)
    {
        perror("SETSOCKOPT: ");
        close(sock);
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        bzero(buffer,256);
        bzero(&addr,sizeof(addr));
        socklen_t addr_len;
        addr_len = sizeof(addr);
        recvfrom(sock, buffer,256,0,(struct sockaddr *)&addr,&addr_len);

        printf("Sprava od: %s:%d | %s\n",
            inet_ntoa(addr.sin_addr),
            ntohs(addr.sin_port),
            buffer);
    }


    close(sock);
    return EXIT_SUCCESS;
}
