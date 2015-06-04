#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
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



int server_side(char* IP_Address, char* filename)
{
	struct in_addr localInterface;
	struct sockaddr_in groupSock;
	int sd;

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


    localInterface.s_addr = inet_addr(IP_Address);
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




int client_side(char* IP_Address, char* filename)
{

	struct sockaddr_in localSock;
	struct ip_mreq group;
	int sd;

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

    /* Join the multicast group 226.1.1.1 on the localhost */
    /* interface. Note that this IP_ADD_MEMBERSHIP option must be */
    /* called for each local interface over which the multicast */
    /* datagrams are to be received. */
    group.imr_multiaddr.s_addr = inet_addr("226.1.1.1");
    group.imr_interface.s_addr = inet_addr(IP_Address);
    if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
    {
        perror("Adding multicast group error");
        close(sd);
        exit(1);
    }
    else
        printf("Adding multicast group...OK.\n");

    /* Read from the socket. */
    UDP_receive_binary_data(filename, sd);

    return 0;
}



int main (int argc, char *argv[ ])
{

    /* Wrong usage when argc is not enough */
    if (argc < 3) {
        fprintf(stderr,"usage %s server/client filename\n", argv[0]);
        exit(0);
    }

    char* socket_type = strtok(argv[1], " ");
    // socket_type = tolower(*socket_type);

    char* filename = strtok(argv[2], " ");

    struct hostent *server;
    struct in_addr temp;
    server = gethostbyname("localhost");
    bcopy((char *)server->h_addr, (char *)&temp.s_addr, server->h_length);
    char* IP_address = inet_ntoa(temp);


    if (strncmp(socket_type,"server",strlen("server"))==0)
    	server_side(IP_address, filename);
    else if (strncmp(socket_type,"client",strlen("client"))==0)
    	client_side(IP_address, filename);
    else
    	printf("Please enter server or client!");

}