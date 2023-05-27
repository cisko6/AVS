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

#include <netinet/in.h>

#define IF_NAME "eth0"
#define ARP_REQUEST 1
#define ARP_REPLY 2

struct eth_hdr
{
    u_int8_t dst_mac[6];
    u_int8_t src_mac[6];
    u_int16_t eth_type;
    char payload[0];
}__attribute__((packed));

struct arp_header
{
    u_int16_t hw_type;
    u_int16_t proto_type;
    u_int8_t hw_len;
    u_int8_t proto_len;
    u_int16_t opcode;
    u_int8_t sender_mac[6];
    struct in_addr sender_ip;
    u_int8_t target_mac[6];
    struct in_addr target_ip;
}__attribute__((packed));

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("USAGE: %s IP_Address\n",argv[0]);
        exit(EXIT_FAILURE);
    }


    int sock = socket(AF_PACKET,SOCK_RAW,htons(ETHERTYPE_ARP));
    if(sock == -1){
        perror("SOCKET");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_ll addr;
    bzero(&addr,sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = if_nametoindex(IF_NAME);
    if(addr.sll_ifindex == 0){
        perror("IFNAMETOINDEX");
        exit(EXIT_FAILURE);
    }
    
    if(bind(sock,(struct sockaddr *)&addr,sizeof(addr)) == -1){
        perror("BIND");
        close(sock);
        exit(EXIT_FAILURE);
    }

    struct eth_hdr hdr;
    memset(hdr.dst_mac,0xff,6);
    hdr.src_mac[0] = 0x08;
    hdr.src_mac[1] = 0x00;
    hdr.src_mac[2] = 0x27;
    hdr.src_mac[3] = 0x2e;
    hdr.src_mac[4] = 0x8b;
    hdr.src_mac[5] = 0x06;
    hdr.eth_type = htons(ETHERTYPE_ARP);
    
    struct arp_header arp;
    bzero(&arp,sizeof(arp));
    arp.hw_type = htons(1);
    arp.proto_type = htons(ETHERTYPE_IP);
    arp.hw_len = 6;
    arp.proto_len = 4;
    arp.opcode = htons(ARP_REQUEST);
    memcpy(arp.sender_mac,hdr.src_mac,6);

    if(inet_aton("10.0.2.15",&arp.sender_ip) == 0){
        printf("ERROR: inet:aton\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    if(inet_aton(argv[1],&arp.target_ip) == 0){
        printf("ERROR: inet:aton\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    int buff_len = sizeof(hdr)+sizeof(arp);
    u_int8_t buff[buff_len];
    memcpy(buff,&hdr,sizeof(hdr));
    memcpy(buff+sizeof(hdr),&arp,sizeof(arp));

    write(sock,buff,buff_len);
    printf("Zapisal na socket\n");

    //RECEIVE
    int read_len;
    for(;;){

    bzero(buff,buff_len);
    read_len = read(sock,buff,buff_len);

    printf("prisiel ramec\n");
    if(read_len < buff_len){
        continue;
    }

    struct eth_hdr *hdr_template;
    hdr_template = (struct eth_hdr *)&buff;
    if(memcmp(hdr_template->dst_mac,hdr.src_mac,6) != 0)
        continue;
    
    if(hdr_template->eth_type != htons(ETHERTYPE_ARP))
        continue;
    
    struct arp_header * arp_template;
    arp_template = (struct arp_header *) hdr_template->payload;

    if(arp_template->opcode != htons(ARP_REPLY))
        continue;

    if(arp_template->sender_ip.s_addr != arp.target_ip.s_addr)
        continue;

    if(arp_template->target_ip.s_addr != arp.sender_ip.s_addr)
        continue;

    if(memcmp(arp_template->target_mac, arp.sender_mac,sizeof(arp.sender_mac) != 0))
        continue;

    
    printf("REPLY received: %hhx:%hhx:%hhx:%hhx:%hhx:%hhx\n",
        arp_template->sender_mac[0],
        arp_template->sender_mac[1],
        arp_template->sender_mac[2],
        arp_template->sender_mac[3],
        arp_template->sender_mac[4],
        arp_template->sender_mac[5]
        );
        break;
    }




    close(sock);
    exit(EXIT_SUCCESS);
}

