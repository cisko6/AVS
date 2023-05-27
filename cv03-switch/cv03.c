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

#include <sys/select.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>


struct eth_hdr
{
    u_int8_t dst_mac[6];
    u_int8_t src_mac[6];
    u_int16_t eth_type;
    char payload[0];
}__attribute__((packed));

struct int_entry
{
    int socket;
    char *name;
};

struct cam_entry
{
    u_int8_t mac[6];
    struct int_entry *interface;
    struct cam_entry *prev;
    struct cam_entry *next;
};

struct create_cam_items
{
    struct cam_entry *head;
    struct cam_entry *tail;

};

void close_sockets(int sock_count,struct int_entry *interfaces){
    for(int i = 0; i < sock_count; i++){
        close(interfaces[i].socket);
    }
}

struct cam_entry* find_entry(struct cam_entry *head, u_int8_t *mac){
    if(head == NULL)
        return 0;
    
    struct cam_entry *entry;
    entry = head;
    do{
        if((memcmp(entry->mac,mac,6)) == 0)
            return entry;
        
        entry = entry->next;
    }while(entry != NULL);
    return NULL;
}

void create_entry(struct create_cam_items* vstup, u_int8_t *mac, struct int_entry *interface)
{
    if(find_entry(vstup->head,mac) != NULL)
        return;

    printf("ucim sa\n");

    struct cam_entry *entry;
    
    entry = malloc (sizeof(struct cam_entry));
    bzero(entry,sizeof(struct cam_entry));

    memcpy(entry->mac,mac,6);
    entry->interface = interface;

    if(vstup->head == NULL){
        vstup->head = entry;
        vstup->tail = entry;
    }else{
         entry->prev = vstup->tail;
         vstup->tail->next = entry;
         vstup->tail = entry;
    }
}

void destroy_cam(struct cam_entry *head, struct cam_entry *tail){
    struct cam_entry *next_entry;
    next_entry = head;

    while(next_entry != NULL)
    {
        next_entry = head->next;
        free(head);
        head = next_entry;
    }
    head = NULL;
    tail = NULL;
}

void print_cam(struct cam_entry *head)
{
    printf("|-----------------------------|\n");
    printf("|       MAC       | interface |\n");
    struct cam_entry *entry;
    entry = head;
    while(entry != NULL){
        printf("| %hx:%hx:%hx:%hx:%hx:%hx | %s \n",
            entry->mac[0],
            entry->mac[1],
            entry->mac[2],
            entry->mac[3],
            entry->mac[4],
            entry->mac[5],
            entry->interface->name);

        entry = entry->next;
    }

    printf("|-----------------------------|\n");
}

int main(int argc, char* argv[]){
    if(argc < 3){
        printf("USAGE: %s [int1] [int2] ...\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock_count = argc-1;
    struct int_entry interfaces[sock_count];
    struct sockaddr_ll addr;

    int max_socket_id = 0;

    struct cam_entry *head;
    head = NULL;
    struct cam_entry *tail;
    tail = NULL;

    for(int i = 0; i < sock_count; i++){
        interfaces[i].socket = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
        if(interfaces[i].socket == -1){
            perror("SOCKET");
            close_sockets(i,interfaces);
            exit(EXIT_FAILURE);
        }

        //evidencia max id socketu pre sellect
        if(interfaces[i].socket > max_socket_id)
            max_socket_id = interfaces[i].socket;

        interfaces[i].name = argv[i+1];

        bzero(&addr,sizeof(addr));
        addr.sll_family = AF_PACKET;
        addr.sll_ifindex = if_nametoindex(argv[i+1]);
        if(addr.sll_ifindex == 0){
            perror("IFNAMETOINDEX");
            close_sockets(i+1,interfaces);
            exit(EXIT_FAILURE);
        }
    
        if(bind(interfaces[i].socket,(struct sockaddr *)&addr,sizeof(addr)) == -1){
            perror("BIND");
            close_sockets(i+1,interfaces);
            exit(EXIT_FAILURE);
        }
        //nastavujem promiskuitny rezim
        struct ifreq ifr;
        bzero(&ifr,sizeof(ifr));
        strncpy(ifr.ifr_name,interfaces[i].name,strlen(interfaces[i].name));
        if(ioctl(interfaces[i].socket, SIOCGIFFLAGS, &ifr) == -1)
        {
            perror("IOCTL GET:");
            close_sockets(i+1,interfaces);
            exit(EXIT_FAILURE);
        }
        ifr.ifr_flags |= IFF_PROMISC;
        if(ioctl(interfaces[i].socket, SIOCSIFFLAGS, &ifr) == -1)
            {
                perror("IOCTL SET:");
                close_sockets(i+1,interfaces);
                exit(EXIT_FAILURE);
          }
    }




    //pre select musi byt maxsocketid +1
    max_socket_id++;

    u_int8_t buffer[1500];
    int frame_len;
    struct eth_hdr *frame;
    struct cam_entry *cam_entry_pom;

    while (1)
    {
        fd_set fds;
        FD_ZERO(&fds);
        for(int i = 0; i < sock_count;i++)
        {
            FD_SET(interfaces[i].socket,&fds);
        }

        if(select(max_socket_id, &fds, NULL, NULL, NULL) == -1)
        {
            perror("SELECT: ");
            continue;
        }

        for (int i = 0; i < sock_count; i++)
        {
            if(FD_ISSET(interfaces[i].socket,&fds))
            {
                //na sockete na interfaces[i] mi prišiel ramec
                bzero(buffer,1500);
                frame_len = read(interfaces[i].socket,buffer,1500);
                frame = (struct eth_hdr *) &buffer;
                printf("Prišiel rámec cez %s, \nsrc: %hx:%hx:%hx:%hx:%hx:%hx, \ndst: %hx:%hx:%hx:%hx:%hx:%hx\n",
                interfaces[i].name,
                
                frame->src_mac[0],
                frame->src_mac[1],
                frame->src_mac[2],
                frame->src_mac[3],
                frame->src_mac[4],
                frame->src_mac[5],

                frame->dst_mac[0],
                frame->dst_mac[1],
                frame->dst_mac[2],
                frame->dst_mac[3],
                frame->dst_mac[4],
                frame->dst_mac[5]
                );

                //ucenie
                struct create_cam_items vstup;
                vstup.head = head;
                vstup.tail = tail;
                create_entry(&vstup,frame->src_mac, &interfaces[i]);
                head = vstup.head;
                tail = vstup.tail;
                print_cam(head);

                cam_entry_pom = find_entry(head,frame->dst_mac);
                //zaznam existuje=posielam jednym rozhranim
                if(cam_entry_pom != NULL)
                {
                    write(cam_entry_pom->interface->socket, buffer, frame_len);
                    continue;
                }else{
                    //zaznam neexistuje = floodujem
                    for (int j = 0; j < sock_count; j++)
                    {
                        if(i != j)
                            write(interfaces[j].socket, buffer, frame_len);
                        
                    }
                }
            }
        }
    }
    



    
    destroy_cam(head,tail);
    close_sockets(sock_count,interfaces);
    exit(EXIT_SUCCESS);
}

