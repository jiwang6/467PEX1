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
#include <sys/time.h>

#define PORT     4240
#define MAXLINE 1024


// Driver code
int main() {
    int sockfd; //Socket descriptor, like a file-handle
    char buffer[MAXLINE]; //buffer to store message from server
    char *listRequest = "LIST_REQUEST"; //message to send to server
    char streamRequest[100] = "START_STREAM\n";
    char songName[50];
    struct sockaddr_in     servaddr;  //we don't bind to a socket to send UDP traffic, so we only need to configure server address

    char kidsChoice[50]; // what the user wants, don't worry about it
    int totalFrames = 1;
    int totalBytes = 0;
    int errr = 0;
    char* listP;




    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Filling server information
    servaddr.sin_family = AF_INET; //IPv4
    servaddr.sin_port = htons(PORT); // port, converted to network byte order (prevents little/big endian confusion between hosts)
    servaddr.sin_addr.s_addr = INADDR_ANY; //localhost
    
    int n, len = sizeof(servaddr);


    struct timeval timeout;      
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;   
            if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char * ) &timeout, sizeof(timeout)) < 0) {
                perror("setsockopt failed");
                exit(EXIT_FAILURE);
            }

    for (;;) {
        strcpy(kidsChoice, "\0");
        strcpy(buffer, "\0");
        errr = 0;
        totalFrames = 1;
        totalBytes = 0;

        printf("Enter one of the following commands:\n\"1\" = List Songs\n\"2\" = Stream a Song\n\"3\" = exit\n");
        fgets(kidsChoice,10,stdin);
        strtok(kidsChoice, "\n");

        while ( (strcmp(kidsChoice, "1") != 0) && (strcmp(kidsChoice, "2") != 0) && (strcmp(kidsChoice, "3") != 0) ) {
            printf("You did not enter a valid selection. Please enter the whole number 1, 2, or 3.\n");
            printf("Enter one of the following commands:\n\"1\" = List Songs\n\"2\" = Stream a Song\n\"3\" = exit\n");
            strcpy(kidsChoice, "\0");
            fgets(kidsChoice,10,stdin);
            strtok(kidsChoice, "\n");
        }

        if (strcmp(kidsChoice, "3") == 0)
            break;

        //Sending message to server

        if (strcmp(kidsChoice, "1") == 0) {
            printf("Requesting a list of songs\n");
            sendto(sockfd, (const char *)listRequest, strlen(listRequest), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));

            if(( n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len)) < 0) {
                perror("ERROR receiving response from server");
                printf("Errno: %d. ",errno);
            } else {
                buffer[n] = '\0'; //terminate message
                listP = buffer + 11; 
                printf("Songs Available:\n%s\n", listP);
            }
        }

        if (strcmp(kidsChoice, "2") == 0) {
            printf("Please enter a song name: ");
            
            fgets(songName,49,stdin);
            strtok(songName, "\n");
            strcat(streamRequest, songName);
            
            sendto(sockfd, (const char *)streamRequest, strlen(streamRequest), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            printf("Sending start stream\nWaiting for response\n");
            strcpy(streamRequest, "START_STREAM\n"); //reset

            // PRINT EACH FRAME IT RECEIVES            
            do {
                if(( n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len)) < 0) { // receive bytes here
                    printf("Socket timed out.");
                    errr = 1;
                    break;
                }
                
                buffer[n] = '\0'; //terminate message

                if (!strcmp(buffer,"COMMAND_ERROR")) { // when requesting a song not listed
                    printf("Command error received from server. Cleaning up...\nDone!\n");
                    errr = 1;
                    break;
                }

                buffer[n] = '\0'; //terminate message
                
                if (!strcmp(buffer, "STREAM_DONE"))
                {
                    break;
                }

                n -= 12;
                printf("Frame # %d Received with %d Bytes\n", totalFrames, n);
                totalBytes += n;
                ++totalFrames;
            } while (strcmp(buffer, "STREAM_DONE"));

            totalFrames -= 1;
            if(errr == 0)
                printf("Stream done. Total Frames: %d Total Size : %d bytes\nDone!\n", totalFrames, totalBytes);
        }

        /*
        // Receive message from client
        if(( n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len))<0)
        {
            perror("ERROR");
            printf("Errno: %d. ",errno);
            exit(EXIT_FAILURE);
        }
        buffer[n] = '\0'; //terminate message
        

        // Super weird why I had to do it like this but this works... not sure im kinda annoyed
        if (!strcmp(buffer,"COMMAND_ERROR")) { // when requesting a song not listed
            printf("Server : %s\n", buffer);
            printf("Command error received from server. Cleaning up...\nDone!\n");
        }
        else {
            printf("Server : %s\n", buffer);
        }
        */

        // Setting the timeout (using sys/time on the included header file)
        // I really don't know if this works... not sure how to test it since this has to do


    

        //TODO: saving to mp3

    }

    close(sockfd);
    return 0;
}