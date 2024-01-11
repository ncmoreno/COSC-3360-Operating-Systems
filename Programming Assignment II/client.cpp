#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

struct thread
{
    int data;
    int port;
    const char *hostname;
    int nbits;
    std::string cMSG;
    char decode;
};

// Child Threads
void *mThreads(void *arg)
{
    struct thread *th_ptr = (struct thread*)arg;
    struct sockaddr_in serv_addr;  // Address of the server to connect
    struct hostent *server;        // Host
   
    int sockfd2, m;   // File descriptor and return value m
   
    // Server with same host
    server = gethostbyname(th_ptr->hostname);
    if(server == NULL)
        perror("ERROR, no such host\n");
   
    // Creation of a new socket
    sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd2 < 0)
        perror("ERROR opening socket");
   
    // Server address with same port number
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(th_ptr->port);

    // Connect a new socket from server
    if(connect(sockfd2,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        perror("ERROR connecting");
   
    // Split compressed message by bitsize and store in string binMSG
    std::string binMSG = th_ptr->cMSG.substr(th_ptr->data*th_ptr->nbits, th_ptr->nbits);
   
    // Convert strings to char
    char code[1024];
    int p = binMSG.size();
    for(int i = 0; i < p; i++)
    {
        code[i] = binMSG[i];
    }
    code[p] = '\0';
   
    // Send binary codes to the server
    m = write(sockfd2, &code, sizeof(code));
    if (m < 0)
        perror("ERROR writing to socket");
       
    // Read characters from the server
    char characters[1024];
    m = read(sockfd2, &characters, sizeof(characters));
    th_ptr->decode = *characters;  // Store characters in my struct

    close(sockfd2); // Close sockfd2 file descriptor
   
    return 0;
}

int main(int argc, char *argv[])
{
    //Copied sample code from BB "Socket example program"
    int sockfd, portno, n;        // File descriptor, port number, and return value n
    struct sockaddr_in serv_addr; // Address of the server to connect
    struct hostent *server;       // Host
    char buffer[1024];            // Read characters into this buffer

    // Error
    if (argc < 3)
    {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(1);
    }
   
    // Port number
    portno = atoi(argv[2]);

    // Creation of a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        perror("ERROR opening socket");
   
    // Server Host
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
   
    // Server address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    // Connect a socket from server
    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        perror("ERROR connecting");
   
    // Read the number of bits value from server
    bzero(buffer,1024);
    int nbits;
    n = read(sockfd, &nbits, sizeof(nbits));
    if (n < 0)
        perror("ERROR reading from socket");
   
    // Input compressed message
    std::string text;
    std::getline(std::cin, text);

    int numOfBitsMSG = text.length();
    int numOfBitsCode = nbits;
    int numOfCharMSG = numOfBitsMSG/numOfBitsCode;

    thread *array = new thread[numOfCharMSG];
    pthread_t th[numOfCharMSG];

    // Create Threads
    for(int i = 0; i < numOfCharMSG; i++)
    {
        array[i].data = i;             // Initial position
        array[i].port = portno;        // Same Port Number
        array[i].hostname = argv[1];   // Same Server Hostname
        array[i].nbits = nbits;        // Number of bits value
        array[i].cMSG = text;          // Compressed message
        array[i].decode;               // Store decoded message
        if (pthread_create(&th[i], NULL, &mThreads, &array[i]) != 0)
        {
            perror("Failed to create thread");
        }
    }
   
    // Join threads
    for ( int i = 0; i < numOfCharMSG; i++)
    {
        if (pthread_join(th[i], nullptr) != 0)
        {
            perror("Failed to join thread");
        }
    }
   
    // Print the output
    std::cout << "Decompressed message: ";
    for(int i = 0; i < numOfCharMSG; i++)
    {
        std::cout << array[i].decode;
    }
   
    close(sockfd); // Close sockfd file descriptor
    return 0;
}