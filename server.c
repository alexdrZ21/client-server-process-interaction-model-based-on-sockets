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

typedef struct private_nicks *prvt_lst;
typedef struct private_nicks{
    char *name;
    prvt_lst nxt;
}prvt_nicks;

typedef struct Node *list;
typedef struct Node{
    char *nick;
    prvt_lst privates;
    int fd;
    int online;
    list next;
}node;

//Checking client in list

list in_list(list lst, char *s){
    while(lst != NULL){
        if (strcmp(lst->nick, s) == 0) return lst;
        else lst = lst->next;
    }
    return NULL;
}


//Add new client to the list

void add_client_to_list(list last, char *buf, int client_fd){
    list tmp = (list)malloc(sizeof(node));
    tmp->nick = buf;
    tmp->fd = client_fd;
    tmp->online = 1;
    tmp->next = NULL;
    tmp->privates = NULL;
    last->next = tmp;
}


//Size of current list

int sizeof_list(list lst){
    int len = 0;
    while(lst != NULL){
        lst = lst->next;
        len++;
    }
    return len;
}


//Search client in the list           

list search_list(list lst, int n){
    int i = 1;
    while(lst != NULL){
        if (i == n) return lst;
        lst = lst->next;
        i++;
    }
}



//Print online users

void print_users(list lst, int fd){
    char s[256];
    strcpy(s, "### Users online: ");
    while(lst != NULL){
        if (lst->online){
            strcat(s, lst->nick);
            strcat(s, " ");
        }
        lst = lst->next;
    }
    write(fd, s, strlen(s));
}


//User diconnected

void disconnected(list lst, int fd, char *buf){
    close(fd);
    fd = -1;
    lst->online = 0;
    strcpy(buf, "### ");
    strcat(buf,  lst->nick);
    strcat(buf, " disconnected from server");  
}

//checking client in list of private messages

int in_list_private(prvt_lst from, char *to_nick){
   while(from != NULL){
        if (strcmp(from->name, to_nick) == 0) return 1;
        else from = from->nxt;
    }
    return 0;
}

//Sending private messages

void private(list from, list to, char *s){
    if(from->privates == NULL){
        from->privates = malloc(sizeof(prvt_nicks));
        from->privates->name = to->nick;
        from->privates->nxt = NULL;
    }else{
        if (in_list_private(from->privates, to->nick) == 0){
            prvt_lst q = from->privates;
            while(q->nxt != NULL) q = q->nxt;
            q->nxt =  malloc(sizeof(prvt_nicks));;
            q = q->nxt;
            q->name = to->nick;;
            q->nxt = NULL;\
        }
    }
    char buffer[BUF_SIZE];
    strcpy(buffer, "");
    strcat(buffer, from->nick);
    strcat(buffer, ":");
    strcat(buffer, s);
    write(to->fd, buffer, strlen(buffer));
}

//Which clients receive private messages

void print_privates(list lst) {
    prvt_lst q = lst->privates;
    if (q == NULL) {
        write(lst->fd, "### You haven't sent private messages yet", strlen("### You haven't sent private messages yet"));
    }
    else {
        char s[BUF_SIZE];
        strcpy(s, "### You've send private messages to: ");
        while (q != NULL) {
            strcat(s, q->name);
            strcat(s, " ");
            q = q->nxt;
        }
        write(lst->fd, s, strlen(s));
    }
}


int main(int argc, char * argv[]){
    int main_socket, port, clients, max_clients, events, temp_socket, i;
    ssize_t count;
    char *buf, s[BUF_SIZE];
    struct sockaddr_in adr;
    struct pollfd *fds, *temp_fds; 
    list temp_lst, lst, tmp;

    if(argc < 2){
        fprintf(stderr,"ERROR! Необходимо указать номер порта в параметрах\n");
        return 1;
    }


//Create an AF_INET stream socket to receive incoming connections

    main_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (main_socket == -1){
        perror("ERROR! Socket wasn't created\n"); 
        exit(1);
    }


//Bind the socket

    memset(&adr, 0, sizeof(adr));
    port = atoi(argv[1]);
    adr.sin_family = AF_INET;
    adr.sin_port = htons(port);
    adr.sin_addr.s_addr = INADDR_ANY;
    if(bind(main_socket, (struct sockaddr *) &adr, sizeof(adr)) == -1){
        perror("ERROR! Can't bind the socket\n"); 
        close(main_socket);
        exit(1);
    }


//Set the listen

    if(listen(main_socket, MAX_QUEUE) == -1){
        perror("ERROR! Can't listen to socket\n"); 
        close(main_socket);
        exit(1);
    }


//Initialize the pollfd structure

    max_clients = MEM_INC_SIZE;
    fds = malloc(sizeof(struct pollfd)*(max_clients + 1));
    if(fds == NULL){
        perror("ERROR! The structure pollfd wasn't created\n");
        exit(1);
    }   


//Set up the initial listening socket

    fds[0].fd = main_socket;
    fds[0].events = POLLIN | POLLERR | POLLPRI | POLLOUT;
    fds[0].revents = 0;


//Loop waiting for incoming connects or for incoming data
//on any of the connected sockets.

    lst = NULL;
    temp_lst = lst;
    clients = 0;
    buf = NULL;

    for(;;){
        events = poll(fds, clients + 1, 100);
        if(events == -1){
            perror("ERROR! The problems with poll()"); 
            exit(1);
        }
        if(events == 0)
           continue;

        if(fds[0].revents){
            temp_socket = accept(main_socket, NULL, NULL);
            if(temp_socket == -1){
                if (errno != EWOULDBLOCK){
                    perror("ERROR! Can't accept a client\n");
                    exit(1);    
                }
                break;
            }

        //Read the nickname
        
            buf = realloc(buf, BUF_SIZE*sizeof(buf));
            count = read(temp_socket, buf, BUF_SIZE);
            buf = realloc(buf, count*sizeof(buf));
            buf[count] = '\0';  

        if (in_list(lst, buf) != NULL){
            printf("ERROR! Cant reconnect client\n");
        }else{
            
            //Add client to list
            
                clients++;
                if(lst == NULL){
                    lst = (list)malloc(sizeof(node));
                    lst->nick = buf;
                    lst->fd = temp_socket;
                    lst->online = 1;
                    lst->next = NULL;
                    lst->privates = NULL;
                    temp_lst = lst;
                }
                else{
                    add_client_to_list(temp_lst, buf, temp_socket);
                    temp_lst = temp_lst->next;
                }
            
            //Send the message
            //about successful connection
            
                write(temp_socket, "### You was connected", strlen("### You was connected"));
                strcpy(s, "### Client ");
                strcat(s, buf);
                strcat(s, " has connected to server");
                for(i = 1; i <= clients - 1; i++)
                    write(fds[i].fd, s, strlen(s));
                buf = NULL;
        }

            
            //Realloc memory for clients
            
            if(clients >= max_clients){
                max_clients += MEM_INC_SIZE;
                temp_fds = fds;
                fds = realloc(fds, sizeof(struct pollfd)*(max_clients + 1));
                if(fds == NULL){
                    perror("ERROR! Problem with realloc()"); 
                    free(temp_fds);
                    exit(1);
                }
            }

            fds[clients].fd = temp_socket;
            fds[clients].events = POLLIN | POLLERR | POLLPRI | POLLHUP;
            fds[clients].revents = 0;
            fds[0].revents = 0;
        }


        for(i = 1; i <= clients; i++){
            if(fds[i].revents){
                tmp = search_list(lst, i);
                buf = realloc(buf, BUF_SIZE*sizeof(buf));
                count = read(fds[i].fd, buf, BUF_SIZE);
                switch(count){
                    case 0: 
                        //disconnected(tmp, fds[i].fd, buf);
                        close(fds[i].fd);
                        fds[i].fd = -1;
                        tmp->online = 0;
                        strcpy(buf, "### ");
                        strcat(buf,  tmp->nick);
                        strcat(buf, " disconnected from server");
                        for(int j = 1; j <= clients; j++)
                            if (j != i) write(fds[j].fd, buf, strlen(buf));
                        break;

                    default:
                        buf = realloc(buf, count*sizeof(buf));
                        buf[count]='\0';

                        if (strncmp(buf, "\\quit", 5) == 0){
                            close(fds[i].fd);
                            fds[i].fd = -1;
                            tmp->online = 0;
                            strcpy(s, "### ");
                            strcat(s,  tmp->nick);
                            strcat(s, " disconnected from server with the message: ");
                            strcat(s, &buf[6]);
                            for(int j = 1; j <= clients; j++)
                                if (j != i) write(fds[j].fd, s, strlen(s));
                        } 
                        else if (strncmp(buf, "\\users", 6) == 0){
                            print_users(lst, fds[i].fd);
                        } 
                        else if (strncmp(buf, "\\privates", 9) == 0) {
                            print_privates(tmp);
                        }
                        else if (strncmp(buf, "\\private", 8) == 0){  
                            memset(s, 0, BUF_SIZE);
                            int j = 9, k = 0;
                            while(buf[j] == ' ') j++;
                            for(; buf[j] != ' '; s[k] = buf[j], j++, k++);
                            list nick_to_send = in_list(lst, s);
                            if (nick_to_send == NULL){
                                write(fds[i].fd, "### This user is not on the server", strlen("### This user is not on the server"));
                            }else{
                                memset(s, 0, BUF_SIZE);
                                while(buf[j] == ' ') j++;
                                for(k = 0; (buf[j] != '\0' && buf[j] != '\n'); s[k] = buf[j], j++, k++);
                                private(tmp, nick_to_send, s);
                            }
                        } 
                        else if (strncmp(buf, "\\help", 5) == 0){
                            strcpy(s, "### Available commands:\n###  \\quit <message>\n###  \\users\n");
                            strcat(s, "###  \\private <nickname> <message>\n###  \\privates\n###  \\help");
                            write(fds[i].fd, s, strlen(s));
                        }
                        else{
                            strcpy(s, tmp->nick);
                            strcat(s, ": ");
                            strcat(s, buf);
                            for(int j = 1; j <= clients; j++)
                                if(j != i) write(fds[j].fd, s, strlen(s));
                        }
                }
                fds[i].revents = 0;
            }
            buf = NULL;
        }
    }
    shutdown(main_socket, 2);
    close(main_socket);
    return 100;
}
