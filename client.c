#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <poll.h>

#define MAX_QUEUE 5
#define MEM_INC_SIZE 8
#define BUF_SIZE 256
#define MSG_END_CONNECTION_SYMBOL '#' 

int is_client_conn_ended(char *msg){ // ne nado
    for(int j = 0; j < strlen(msg); j++)
        if(msg[j] == MSG_END_CONNECTION_SYMBOL)
            return 1;
    return 0;
}

int main(int argc, char * argv[]){
    int client_socket, port, i, events, isExit;
    ssize_t n_read;
    char buf[BUF_SIZE], msg[BUF_SIZE];
    struct sockaddr_in adr; 
    struct pollfd fds[2]; 

    if(argc < 3){
        fprintf(stderr,"ERROR! Необходимо указать номер порта и никнейм в параметрах\n");
        return 1;
    }



//Create an AF_INET stream socket to receive incoming connections

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1){
        perror("ERROR! Socket wasn't created\n"); 
        exit(1);
    }


//Connect the socket

    memset(&adr, 0, sizeof(adr));
    port = atoi(argv[1]);
    adr.sin_family = AF_INET;
    adr.sin_port = htons(port);
    adr.sin_addr.s_addr = INADDR_ANY;
    if(connect(client_socket, (struct sockaddr *) &adr, sizeof(adr)) == -1){
        perror("ERROR! Can't connect the socket\n"); 
        close(client_socket);
        exit(1);
    }


//Set up the pollfd structure

    fds[0].fd = 0;
    fds[0].events = POLLIN | POLLERR | POLLPRI;
    fds[0].revents = 0;

    fds[1].fd = client_socket;
    fds[1].events = POLLIN | POLLERR | POLLPRI;
    fds[1].revents = 0;

//Send to server the nickname

    write(client_socket, argv[2], strlen(argv[2]));
    isExit = 0;

    while(!isExit){
        events = poll(fds, 2, 100);
        if(events == -1){
            perror("The problems with poll()"); 
            exit(1);
        }
        if(events == 0)
           continue;

        if(fds[0].revents){
            int count = read(0, buf, BUF_SIZE);
            buf[count] = '\0';
            write(client_socket, buf, strlen(buf)-1);
            fds[0].revents = 0;
        }
        if(fds[1].revents){
            int count = read(client_socket, buf, BUF_SIZE);
            if (count == 0) break;
            buf[count] = '\0';
            printf("%s\n", buf);
            fds[1].revents = 0;
        }
    }

    shutdown(client_socket, 2);
    close(client_socket);
    return 100;
}

