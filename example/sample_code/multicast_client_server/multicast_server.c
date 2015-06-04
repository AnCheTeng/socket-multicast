/* Send Multicast Datagram code example. */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function: Send binary data to socket
// Usage: UDP_send_binary_data(filename, socket_fd, sockaddr);
int UDP_send_binary_data(char* filename, int sockfd, struct sockaddr_in addr_to)
{
    /* Open the file that we wish to transfer */
    FILE *fp = fopen(filename,"rb");
    if(fp==NULL)
    {
        printf("File opern error");
        return 1;
    }

    /* Read data from file and send it */
    while(1)
    {
        /* First read file in chunks of 256 bytes */
        unsigned char buff[254]= {0};

        /* Read from file(fp) and record read bytes number(nread) */
        int nread = fread(buff,1,254,fp);

        /* If read was success, send data. */
        if(nread > 0)
        {
            sendto(sockfd,buff,strlen(buff),0,(struct sockaddr*)&addr_to,sizeof(addr_to));
        }

        /* Either there was error, or we reached end of file.
         * When reached the end of file, means the transmission is over! */
        if (nread < 254)
        {
            if (feof(fp))
                printf("\nTransmission is OVER!\n");
            if (ferror(fp))
                printf("Error reading\n");
            break;
        }
    }
    return 0;
}


struct in_addr localInterface;
struct sockaddr_in groupSock;
int sd;
char databuf[1024] = "Multicast test message lol!";
int datalen = sizeof(databuf);

int main (int argc, char *argv[ ])
{
    /* Wrong usage when argc is not enough */
    if (argc < 2) {
        fprintf(stderr,"usage %s filename\n", argv[0]);
        exit(0);
    }

    char* filename = strtok(argv[1], " ");

    /* Create a datagram socket on which to send. */
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0)
    {
        perror("Opening datagram socket error");
        exit(1);
    }
    else
        printf("Opening the datagram socket...OK.\n");

    /* Initialize the group sockaddr structure with a */
    /* group address of 225.1.1.1 and port 5555. */
    memset((char *) &groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr("226.1.1.1");
    groupSock.sin_port = htons(4321);


    localInterface.s_addr = inet_addr("115.43.223.12");
    if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
    {
        perror("Setting local interface error");
        exit(1);
    }
    else
        printf("Setting the local interface...OK\n");

    UDP_send_binary_data(filename, sd, groupSock);


    return 0;
}