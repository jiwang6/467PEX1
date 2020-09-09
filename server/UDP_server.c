//Code obtained and modified from https://www.geeksforgeeks.org/udp-server-client-implementation-c/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#define PORT 4240
#define MAXLINE 1024
int main()
{
    int socketfd;  //Socket descriptor, like a file-handle
    struct sockaddr_in srvaddr, cliaddr; //Stores IP address, address family (ipv4), and port
    char buffer[MAXLINE]; //buffer to store message from client
    char *hello = "response or whatever"; //message to respond to client

    // Creating socket file descriptor
    // AF_INET = "domain" = IPv4
    // SOCK_DGRAM = "type" = UDP, unreliable
    // protocol=0, specifies UDP within SOCK_DGRAM type.
    if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    srvaddr.sin_family = AF_INET;           // IPv4
    srvaddr.sin_addr.s_addr = INADDR_ANY;   // All available interfaces
    srvaddr.sin_port = htons( PORT );       // port, converted to network byte order (prevents little/big endian confusion between hosts)



    // Forcefully attaching socket to the port 8080
    // Bind expects a sockaddr, we created a sockaddr_in.  (struct sockaddr *) casts pointer type to sockaddr.
    if (bind(socketfd, (const struct sockaddr *)&srvaddr, sizeof(srvaddr))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Receive message from client
    int len=sizeof(cliaddr), n; //must initialize len to size of buffer

    for(;;){ 
        printf("Waiting for connection...\n");

        if((n = recvfrom(socketfd, (char *)buffer, MAXLINE, 0, ( struct sockaddr *) &cliaddr, &len))<0)
        {
            perror("ERROR");
            printf("Errno: %d. ",errno);
            exit(EXIT_FAILURE);
        }
        buffer[n] = '\0';  //terminate message
        printf("Client : %s\n", buffer);

        // Respond to client
        sendto(socketfd, (const char *)hello, strlen(hello), 0, (const struct sockaddr *) &cliaddr, len);
        printf("Hello message sent.\n");

    }
    close(socketfd); // Close socket
    return 0;
}