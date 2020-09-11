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

/****************
Created By: C2C Chwa and C2C Wang
Documentation: 
****************/
// Driver code
int main() {
    int sockfd; //Socket descriptor, like a file-handle
    char buffer[MAXLINE]; //buffer to store message from server
    char *listRequest = "LIST_REQUEST"; //message to send to server
    char streamRequest[100] = "START_STREAM\n";
    char songName[50]; // store the user's requested song to be stream
    struct sockaddr_in     servaddr;  //we don't bind to a socket to send UDP traffic, so we only need to configure server address

    char kidsChoice[50]; // User input from the 3 options
    int totalFrames = 1; // Initialized to 1 for printing purposes
    int totalBytes = 0; // Total number of bytes streamed from the server
    int errr = 0;
    char* listP; 
    char* bufferP;
    FILE *fp;




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

    // Timeout to close the client if the server doesn't respond within 5 seconds
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
        totalFrames = 1; // Set to 1 for printing purposes (1st Frame Received is frame 1)
        totalBytes = 0;


        // Prompt the user for input, then store the input.
        printf("Enter one of the following commands:\n\"1\" = List Songs\n\"2\" = Stream a Song\n\"3\" = exit\n");
        fgets(kidsChoice,10,stdin);
        strtok(kidsChoice, "\n");


        // Validate the input and make sure it's either a 1, 2, or 3. If not, loop until the input is a valid option
        while ( (strcmp(kidsChoice, "1") != 0) && (strcmp(kidsChoice, "2") != 0) && (strcmp(kidsChoice, "3") != 0) ) {
            printf("You did not enter a valid selection. Please enter the whole number 1, 2, or 3.\n");
            printf("Enter one of the following commands:\n\"1\" = List Songs\n\"2\" = Stream a Song\n\"3\" = exit\n");
            strcpy(kidsChoice, "\0");
            fgets(kidsChoice,10,stdin);
            strtok(kidsChoice, "\n");
        }


        // Exit if the user enters option 3
        if (strcmp(kidsChoice, "3") == 0)
            break;


        //Option #1: Request a list of songs from the server. 
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

        //Option #2: Enter a songname to stream.
        if (strcmp(kidsChoice, "2") == 0) {

            //Prompt the user and store their input along with the required information for the server.
            printf("Please enter a song name: ");
            fgets(songName,49,stdin);
            strtok(songName, "\n");
            strcat(streamRequest, songName);
            
            //Send the message to the server
            sendto(sockfd, (const char *)streamRequest, strlen(streamRequest), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            printf("Sending start stream\nWaiting for response\n");
            strcpy(streamRequest, "START_STREAM\n"); //reset

            //Print each frame received from the server along with the size in Bytes until STREAM_DONE message is received.     
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
                } else if (totalFrames == 1) {
                    fp = fopen(songName, "w+"); //open the .mp3 file
                }

                if (errr != 1) {
                    bufferP = buffer;
                    fwrite(bufferP + 12 , 1 , n-12 , fp); //save the mp3 data without the header information to the .mp3 file
                }

                if (!strcmp(buffer, "STREAM_DONE"))
                    break;

                n -= 12; //Subtract out the header information
                printf("Frame # %d Received with %d Bytes\n", totalFrames, n);
                totalBytes += n; //Increments totalBytes by the number of bytes from the frame minus the header info
                ++totalFrames;
            } while (strcmp(buffer, "STREAM_DONE"));

            totalFrames -= 1; // Subtract 1 from the totalFrames since it was initialized to 1
            if(errr != 1) {
                printf("Stream done. Total Frames: %d Total Size : %d bytes\nDone!\n", totalFrames, totalBytes);
                fclose(fp);
            }
        }


    }

    close(sockfd);
    return 0;
}