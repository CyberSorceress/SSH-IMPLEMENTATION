#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

char* caesar_encrypt(char* message, int shift) {
    int i;
    char *encrypted_message = (char *)malloc(strlen(message) + 1);
    if (!encrypted_message)
        error("Memory allocation failed");

    for (i = 0; i < strlen(message); i++) {
        if (message[i] >= 'A' && message[i] <= 'Z')
            encrypted_message[i] = 'A' + (message[i] - 'A' + shift) % 26;
        else if (message[i] >= 'a' && message[i] <= 'z')
            encrypted_message[i] = 'a' + (message[i] - 'a' + shift) % 26;
        else
            encrypted_message[i] = message[i];
    }
    encrypted_message[i] = '\0';
    return encrypted_message;
}

char* caesar_decrypt(char* encrypted_message, int shift) {
    return caesar_encrypt(encrypted_message, -shift);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Port number not provided\n");
        exit(EXIT_FAILURE);
    }

    int sockfd, newsockfd, portno, n;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int shift = 3; // Caesar cipher shift

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    listen(sockfd,5);
    socklen_t clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
        error("ERROR on accept");

    printf("Chat session established.\n");

    // Start chat session
    while(1) {
        bzero(buffer, 256);
        n = read(newsockfd, buffer, 255);
        if (n < 0) 
            error("ERROR reading from socket");
        
        // Decrypt received message
        char *decrypted_message = caesar_decrypt(buffer, shift);
        printf("Client: %s\n", decrypted_message);
        free(decrypted_message);

        printf("Server: ");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);

        // Encrypt message before sending
        char *encrypted_message = caesar_encrypt(buffer, shift);
        n = write(newsockfd, encrypted_message, strlen(encrypted_message));
        free(encrypted_message);

        if (n < 0) 
            error("ERROR writing to socket");

        // Check for termination message
        if (strncmp(buffer, "bye", 3) == 0)
            break;
    }

    close(newsockfd);
    close(sockfd);
    return 0; 
}
