/*
 * udpv6.c
 * A Semi Peer to Peer chat application. 
 *
 * Homework: Redes de Computadores
 * Author: Duarte Farinha, #11093917
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h>
#include <sys/select.h>

static volatile int keepRunning = 1;    // Keep running

#define STDIN 0
#define MAXLINE 1024 
/*
 * Function to print out the error message and exit the program.
 *
 * @param msg - The message to be printed
 */ 
 void error(const char *msg) {
    perror(msg);
    exit(0);
}

// Driver code 
int main(int argc, char *argv[]) { 
    int sockfd;                                         // file descriptor used on socket
    char buffer[MAXLINE];                               // To store messages sent and receive; (messages get deleted after use)
    char *hellos = "ok";                                // just a message to display if the datagram was sucessfully sent.
    //char *helloc = "Hello from client";
    struct sockaddr_in serv_addr, peer_addr, addru;     // serv_addr my ip, peer_addr peer ip, addru ip received on the datagram
    struct hostent *hp;                                 // To store the Ip address of the peer
    fd_set fds;                                         // Socket descriptor set
    int max_fd;

    // Creating socket file descriptor 
    // Socket to bind to ip address 
    if ( (sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0 ) {                 //Here created a socket to be used with udp protocol (SOCK_DGRAM)
        error("socket creation failed"); 
    }
    
    // The gethostbyname() call get the IP address for the hostname via DNS
    hp = gethostbyname(argv[2]);
    if (hp == 0) error("Unknown host");

    
    memset(&serv_addr, 0, sizeof(serv_addr)); 
    memset(&peer_addr, 0, sizeof(peer_addr)); 
    memset(&addru, 0, sizeof(addru));  

    // Filling server information ( )
    serv_addr.sin_family    = AF_INET;                                      // IPv4 
    serv_addr.sin_addr.s_addr = INADDR_ANY;                                 // Used INADDR_ANY to be able to bind the socket to all local interfaces
    serv_addr.sin_port = htons(atoi(argv[1]));                              // The port my server uses

    //Filling a client information that's also not me...
    peer_addr.sin_family = AF_INET;                                         //IPv4
    bcopy((char *)hp->h_addr, (char *)&peer_addr.sin_addr, hp->h_length);   //Ip address of the peer
    peer_addr.sin_port = htons(atoi(argv[3]));                              //Port of the peer opened
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&serv_addr,  
            sizeof(serv_addr)) < 0 ) 
    { 
        error("bind failed");  
    } 
      
    int len, n;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);                                             // Set watch on keyboard input
    max_fd = STDIN;                                                         // First time called, so this is the highest value
    FD_SET(sockfd, &fds);                                                   // Set watch on messages from the server
    max_fd = (sockfd > max_fd)?sockfd:max_fd;                               // Get the highest number
    
    
    unsigned int lenc = sizeof(struct sockaddr_in);
    fd_set tmp_fds;
    
    while (keepRunning){
        tmp_fds = fds;                                                      // We don't want to alter the fds, so we use a copy

        if(select (max_fd + 1, &tmp_fds, NULL, NULL, NULL) <0){
            error("ERROR on Select");
        }

        for(int i = 0 ; i <= max_fd ; i++) {
            if(FD_ISSET(i, &tmp_fds)) {                                      
                bzero(buffer, sizeof(buffer));                              // Clean the buffer for new data
                if (i == STDIN) {                                           // This the keyboard
                    if(fgets(buffer, MAXLINE, stdin) == NULL){
                        close(sockfd);
                        error("Problem reading from the keyboard");
                    }
                    if(strcmp(buffer, "exit\n") == 0){                      // We could add more commands
                        keepRunning = 0;
                        printf("Exiting ...");
                    }
                
                    //
                    // CLIENT part of the chat ( send and receive an ok)
                    //
        
                    sendto(sockfd, (const char *)buffer, strlen(buffer),    // here we send a datagram to the peer containing the message written
                        MSG_CONFIRM, (const struct sockaddr *) &peer_addr,  
                        sizeof(peer_addr));
                    bzero(buffer, sizeof(buffer));                          //empties the buffer
                    printf("Sent.\n"); 
                
                    n = recvfrom(sockfd, (char *)buffer, MAXLINE,           //receives a datagram with a confirmation message
                        MSG_WAITALL, (struct sockaddr *) &peer_addr, 
                        &lenc); 
                    buffer[n] = '\0'; 
                    printf("Server : %s\n", buffer); 
                    bzero(buffer, sizeof(buffer));
                } 

                else 
                {   
                    //
                    // SERVER part of the chat ( receive and send ok)
                    //
                    
                    len = sizeof(addru);  //len is value/resuslt
                    n = recvfrom(sockfd, (char *)buffer, MAXLINE,           //same thing as in the client part but in reverse
                        MSG_WAITALL, ( struct sockaddr *) &addru, 
                        &len);
                    
                    
                    buffer[n] = '\0'; 
                    printf("Client : %s\n", buffer); 
                    sendto(sockfd, (const char *)hellos, strlen(hellos),  
                        MSG_CONFIRM, (const struct sockaddr *) &addru, 
                        len);

                    bzero(buffer, sizeof(buffer));
                    printf("Sent.\n");  
                }
            }                
        }
    }
    close(sockfd);                                                          //closes the socket
    return 0;
}
