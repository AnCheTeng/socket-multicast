/* Receiver/client multicast Datagram example. */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function: Receive binary data from socket
// Usage: UDP_receive_binary_data(filename, socket_fd, sockaddr);
int UDP_receive_binary_data(char* filename, int sockfd)
{
    /* Create file where data will be stored */
    FILE *fp;
    fp = fopen(filename,"wb");
    if(NULL == fp)
    {
        printf("Error opening file");
        return 1;
    }

    /* Parameter for recvfrom */
    int bytesReceived = 256;
    char recvBuff[255];
    int addr_len = sizeof(struct sockaddr_in);
    /* Receive data in chunks of 256 bytes */
    while( bytesReceived > 253 )
    {
        memset(recvBuff, 0, sizeof(recvBuff));
        // bytesReceived = recvfrom(sockfd,recvBuff,sizeof(recvBuff),0, (struct sockaddr *)&addr ,&addr_len);
        bytesReceived = read(sockfd, recvBuff, sizeof(recvBuff));
        // printf("byteslength %d \n", bytesReceived);
        fwrite(recvBuff,1,strlen(recvBuff),fp);
    }

    if(bytesReceived < 0)
    {
        printf("\n Read Error \n");
    }

    printf("\nTransmission is OVER!\n");

    return 0;
}


struct sockaddr_in localSock;
struct ip_mreq group;
int sd;
int datalen;
char databuf[1024];

int main(int argc, char *argv[])
{
    /* Wrong usage when argc is not enough */
    if (argc < 2) {
        fprintf(stderr,"usage %s  output_filename\n", argv[0]);
        exit(0);
    }

    char* filename = strtok(argv[1], " ");

    /* Create a datagram socket on which to receive. */
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0)
    {
        perror("Opening datagram socket error");
        exit(1);
    }
    else
        printf("Opening datagram socket....OK.\n");

    /* Enable SO_REUSEADDR to allow multiple instances of this */
    /* application to receive copies of the multicast datagrams. */

    int reuse = 1;
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
    {
        perror("Setting SO_REUSEADDR error");
        close(sd);
        exit(1);
    }
    else
        printf("Setting SO_REUSEADDR...OK.\n");


    /* Bind to the proper port number with the IP address */
    /* specified as INADDR_ANY. */
    memset((char *) &localSock, 0, sizeof(localSock));
    localSock.sin_family = AF_INET;
    localSock.sin_port = htons(4321);
    localSock.sin_addr.s_addr = INADDR_ANY;
    if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
    {
        perror("Binding datagram socket error");
        close(sd);
        exit(1);
    }
    else
        printf("Binding datagram socket...OK.\n");

    /* Join the multicast group 226.1.1.1 on the local 203.106.93.94 */
    /* interface. Note that this IP_ADD_MEMBERSHIP option must be */
    /* called for each local interface over which the multicast */
    /* datagrams are to be received. */
    group.imr_multiaddr.s_addr = inet_addr("226.1.1.1");
    group.imr_interface.s_addr = inet_addr("115.43.223.12");
    if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
    {
        perror("Adding multicast group error");
        close(sd);
        exit(1);
    }
    else
        printf("Adding multicast group...OK.\n");

    /* Read from the socket. */
    datalen = sizeof(databuf);
    UDP_receive_binary_data(filename, sd);

    return 0;
}
