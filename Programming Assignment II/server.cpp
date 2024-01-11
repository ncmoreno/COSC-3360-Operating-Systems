#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <iostream>
#include <string>
#include <bitset>
#include <cmath>
#include <map>

struct vault
{
    std::string c;
    int dec;
    std::string cMSG;
    std::string code;
    int nbits;
};

// Copied sample code from BB "Fireman"
void fireman(int)
{
   while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[])
{
    // Copied sample code from BB "Socket example program"
    int sockfd, newsockfd, portno, clilen ,n;  // 2 File descriptors, port number, client address, and return value n
    char buffer[256];                          // Read characters into this buffer
    struct sockaddr_in serv_addr, cli_addr;    // Address of the server and client to connect

    // Terminate if no port provided
    if (argc < 2)
    {
        perror("ERROR, no port provided\n");
        exit(1);
    }

    // Creation of a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        perror("ERROR opening socket");
   
    // Port number
    portno = atoi(argv[1]);
   
    // Server Address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
   
    // Binds a socket to an address
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        perror("ERROR on binding");

    // Listen on the socket for connections
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    // Accept a connection request from client
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
    if(newsockfd < 0)
        perror("ERROR on accept");
   
    // My PA1 code
    int nSymbols;
    std::cin >> nSymbols; //Input for the # of symbols in the alphabet
    vault *alpha = new vault[nSymbols];
    pthread_t th[nSymbols];
    std::string text;
    std::getline(std::cin, text);
   
    //Store each characters and decimal values in struct
    for (int i = 0; i < nSymbols; i++)
    {
        std::getline(std::cin, text);
        alpha[i].c = text.substr(0,1);
        alpha[i].dec = stoi(text.substr(2));
    }

    //Finding the greatest Base 10 code
    int gstBase10 = 0;
    for (int i = 0; i < nSymbols; i++)
    {
        if(alpha[i].dec > gstBase10)
        {
            gstBase10 = alpha[i].dec;
        }
    }
    int nbits = ceil(log2(gstBase10+1));  //Calculate the number of bits

    // Send the number of bits value to client
    n = write(newsockfd, &nbits, sizeof(nbits));
    if(n < 0)
        perror("ERROR writing to socket");

    // Binary Representation
    for(int i = 0; i < nSymbols; i++)
    {
        std::bitset<8>a(alpha[i].dec); //Convert decimal numbers to binary code in # of bits
        alpha[i].code = a.to_string(); //Store each binary code in struct
        alpha[i].code = alpha[i].code.substr(8-nbits);  //Reduce the binary code by # of bits        
    }    

    // Map for binary codes and letters
    std::map<std::string, std::string> mapping;
    for(int i = 0; i < nSymbols; i++)
    {
        mapping.insert(std::pair<std::string,std::string>(alpha[i].code,alpha[i].c));
    }
   
    // Singal from fireman
    signal(SIGCHLD, fireman);
   
    // Infinite loop to handle multiple request from client threads
    while(true)
    {
        // Accept a connection request from client threads
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
       
        // Create fork process
        if (fork() == 0)
        {
            if (sockfd < 0)
            {
                perror("Error accepting");
            }
           
            // Read binary code from each client threads
            bzero(buffer, 1024);
            n = read(newsockfd, &buffer, sizeof(buffer));
            if (n < 0)
            {
                perror("Error reading.");
            }
           
            std::string decode;
            decode = mapping[buffer]; // Decode binary codes into characters
         
            //CONVERT STRINGS TO CHAR
            char convert[1024];
            int p = decode.size();
            for(int i = 0; i < p; i++)
            {
                convert[i] = decode[i];
            }
            convert[p] = '\0';
         
            // Send character values to client
            n = write(newsockfd, &convert, sizeof(convert));
            if (n < 0)
                perror("ERROR writing to socket");
               
            _exit(0);
        }
        close(newsockfd); // Close newsockfd file descriptor
    }
    close(sockfd); // Close sockfd file descriptor
   
    return 0;
}