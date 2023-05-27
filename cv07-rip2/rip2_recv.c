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

#define DST_ADDR "224.0.0.9"
#define DST_PORT 520
#define IF_NAME "eth0"

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

struct rip_route_entry
{
    u_int16_t af;
    u_int16_t route_tag;
    struct in_addr ip_address;
    struct in_addr netmask;
    struct in_addr next_hop;
    u_int32_t metric;
    char next_route[0];
}__attribute__((packed));

struct rip_hdr
{
    u_int8_t command;
    u_int8_t version;
    u_int16_t unused;
    struct rip_route_entry route[0];
}__attribute__((packed));

int main()
{
    int sock;
    struct sockaddr_in addr;
    int buff_size = (sizeof(struct rip_hdr))+(25*sizeof(struct rip_route_entry));
    char buffer[buff_size];

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
        perror(RED "BIND: " RESET);
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
        bzero(buffer,buff_size);
        bzero(&addr,sizeof(addr));
        socklen_t addr_len;
        addr_len = sizeof(addr);
        int recv_size;
        struct rip_hdr* rip;
        recv_size = recvfrom(sock, buffer,buff_size,0,(struct sockaddr *)&addr,&addr_len);
        rip = (struct rip_hdr *) buffer;

        //command musi byt 2
        if(rip->command != 2)
        continue;

        //verzia musi byt 2
        if(rip->version != 2)
        continue;

        //spracoval som rip hlavičku
        recv_size -= sizeof(struct rip_hdr);
        printf(GRN "\nRIPv2"RESET" správa od "BLU"%s"RESET":"YEL"%d\n"RESET,inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        //rip musi ossahovat aspon jednu rip entry
        if(recv_size < sizeof(struct rip_route_entry))
        continue;

        //vsetky rip entries boli prijate v celku
        if(recv_size % sizeof(struct rip_route_entry) != 0)
        continue;

        struct rip_route_entry* entry;
        entry = rip->route;
        while (recv_size > 0)
        {
            if(entry->af != htons(2))
            continue;

            printf(RED" IPv4 entry:\n");
            printf(CYN"  Prefix:   "RESET"%15s\n",inet_ntoa(entry->ip_address));
            printf(MAG"  Netmask:  "RESET"%15s\n",inet_ntoa(entry->netmask));
            printf(GRN"  Next hop: "RESET"%15s\n",inet_ntoa(entry->next_hop));
            printf(YEL"  Metric:   "RESET"%15d\n\n",ntohl(entry->metric));

            recv_size -= sizeof(struct rip_route_entry);
            entry = (struct rip_route_entry *) entry->next_route;
        }
    }


    close(sock);
    return EXIT_SUCCESS;
}
