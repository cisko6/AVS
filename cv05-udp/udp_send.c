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

#define DST_ADDR "127.0.0.1"
#define DST_PORT 8888

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

    bzero(buffer,256);
    printf("Zadajte text na poslanie: ");
    scanf("%s",buffer);
            socklen_t addr_len;
        addr_len = sizeof(addr);
    sendto(sock, buffer,strlen(buffer),0,(struct sockaddr *)&addr,addr_len);


    close(sock);
    return EXIT_SUCCESS;
}
