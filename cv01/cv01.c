#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> 
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

#define HELLO_SIZE 20
#define IF_NAME "eth0"

struct eth_hdr
{
    u_int8_t dst_mac[6];
    u_int8_t src_mac[6];
    u_int16_t eth_type;
    char payload[0];
}__attribute__((packed));

struct hello_protocol
{
    char hello[HELLO_SIZE];
}__attribute__((packed));


int main()
{
    int sock;
    struct sockaddr_ll addr;
    u_int8_t frame[sizeof(struct eth_hdr)+sizeof(struct hello_protocol)];
    struct eth_hdr *hdr;
    struct hello_protocol *hello;

    sock = socket(AF_PACKET,SOCK_RAW,0);
    if(sock == -1)
    {
        perror("SOCKET");
        exit(EXIT_FAILURE);
    }

    bzero(&addr,sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = if_nametoindex(IF_NAME);
    if(addr.sll_ifindex == 0)
    {
        perror("IFNAMETOINDEX");
        close(sock);
        exit(EXIT_FAILURE);
    }


    if(bind(sock,(struct sockaddr*)&addr,sizeof(addr)) == -1){
        perror("BIND");
        close(sock);
        exit(EXIT_FAILURE);
    }

    
    hdr = (struct eth_hdr*) &frame;
    hdr->dst_mac[0] = 0xaa;
    hdr->dst_mac[1] = 0xbb;
    hdr->dst_mac[2] = 0xcc;
    hdr->dst_mac[3] = 0xee;
    hdr->dst_mac[4] = 0xff;
    hdr->dst_mac[5] = 0x00;

    memcpy(hdr->src_mac,hdr->dst_mac,sizeof(hdr->dst_mac));
    hdr->eth_type = htons(0xabba);

    hello = (struct hello_protocol*) hdr->payload;
    bzero(hello,sizeof(struct hello_protocol));
    memcpy(hello->hello,"Ahoj",4);

    if(write(sock,&frame, sizeof(frame)) == -1)
    {
        perror("WRITE");
        close(sock);
        exit(EXIT_FAILURE);
    }
    close(sock);
    exit(EXIT_SUCCESS);
}
