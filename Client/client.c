#include "include.h"

#define PORT 8080

void executePut(int, char*);
void executeGet(int, char*);
char* executeRun(int, char*);
void executeList(int, char*);

int main(int argc, char **argv) {

    //initialise socket, server address and buffers
    int c_socket;    
    struct sockaddr_in server;
    char client_buffer[SIZE];
    char server_buffer[SIZE];
    clock_t start, stop;
    double duration;

    if(argc < 2) {
        printf("Error: Please add the correct IP Address as a command-line argument.\n");
        exit(-1);
    }

    //set server variables
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(PORT);

     //creating client socket
    if((c_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(1);
    } else {
        printf("socket created\n");
    }

    //connect socket to server
    if(connect(c_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("\nConnection failed \n");
        exit(1);
    } else {
        printf("connection successful\n");
    }


    //accept user commands and send to the server, printing out the status code
    while(1) {
        //clear the buffers
        memset(client_buffer, '\0', sizeof(client_buffer));
        memset(server_buffer, '\0', sizeof(server_buffer));

        //user prompt and input
        printf("command> ");
        gets(client_buffer);

        //send command, receive server response
        if (strcmp(client_buffer, "quit") == 0) {
            send(c_socket, client_buffer, SIZE, 0);
            close(c_socket);
            printf("Exited Successfully.\n");
            exit(0);
            
        } else if (*(client_buffer) == 'p' && *(client_buffer+1) == 'u' && *(client_buffer+2) == 't') {
            //put file into server
            start = clock();

            send(c_socket, "put", SIZE, 0);
            executePut(c_socket, &*(client_buffer+4));

            recv(c_socket, server_buffer, SIZE, 0);
            stop = clock();
            duration = (double) (stop-start)/CLOCKS_PER_SEC;

        } else if (*(client_buffer) == 'g' && *(client_buffer+1) == 'e' && *(client_buffer+2) == 't')  {
            //read file from server
            start = clock();
            send(c_socket, "get", SIZE, 0);
            executeGet(c_socket, &*(client_buffer+4));

            stop = clock();
            duration = (double) (stop-start)/CLOCKS_PER_SEC;
            printf("time taken>> %lf seconds\n\n", duration);
            continue;

        } else if (*(client_buffer) == 'r' && *(client_buffer+1) == 'u' && *(client_buffer+2) == 'n')  {
            //compile and run program on server
            start = clock();
            send(c_socket, "run", SIZE, 0);
            char * local = executeRun(c_socket, &*(client_buffer+4));

            recv(c_socket, server_buffer, (SIZE*SIZE), 0);

            if(strcmp(local, "no") != 0) {
                writeToFile(server_buffer, local);
                printf(">>>> Output saved to %s\n", local);
                continue;
            }
            stop = clock();
            duration = (double) (stop-start)/CLOCKS_PER_SEC;

        } else if (*(client_buffer) == 'l' && *(client_buffer+1) == 'i' && *(client_buffer+2) == 's' && *(client_buffer+3) == 't')  {
            //list files in progname dir
            start = clock();
            send(c_socket, "list", SIZE, 0);
            executeList(c_socket, &*(client_buffer+5));

            recv(c_socket, server_buffer, SIZE, 0);
            stop = clock();
            duration = (double) (stop-start)/CLOCKS_PER_SEC;

        } else if (strcmp(client_buffer,"sys" ) == 0)  {
            //find out server system info
            start = clock();
            send(c_socket, client_buffer, SIZE, 0);
            recv(c_socket, server_buffer, SIZE, 0);
            stop = clock();
            duration = (double) (stop-start)/CLOCKS_PER_SEC;

        } else {
            printf("Error: Command not recognised.\n");
            continue;
        }   
        //print status
        printf("server : %s\n", server_buffer);
        printf("time taken>> %lf seconds\n\n", duration);
    }
    close(c_socket);
    return 0;
}

void executePut(int socket, char* commands) {
    FILE *fp = NULL;
    char *delim = " ";
    char *buffer;
    char commandsCopy[SIZE] = {0};
    char overWriteCmds[SIZE] = {0};
    char exists[SIZE] = {0};

    strcpy(commandsCopy, commands);
    strcpy(overWriteCmds, commands); 

    char *ptr = strtok(commands, delim);
    send(socket, ptr, SIZE, 0);
    recv(socket, exists, SIZE, 0);

    if(strcmp(exists, "exists") != 0) {
        ptr = strtok(NULL, delim);
        sendFiles(socket, ptr, delim, fp);
    } else {
        //check for -f flag
        char* overwrite = parseFFlag(commandsCopy, delim);
        if(strcmp(overwrite, "found") == 0) {
            send(socket, "overwrite", SIZE, 0);
            char* owPtr = strtok(overWriteCmds, delim);
            owPtr = strtok(NULL, delim);
            sendFiles(socket, owPtr, delim, fp);
        } else {
            send(socket, "no overwrite", SIZE, 0);
        }
    }
}


void executeGet(int socket, char* commands) {
    //get progname sourcefile    
    char **args;
    char cmdsCount[SIZE] = {0};
    strcpy(cmdsCount, commands);
    int argcount = countCommands(cmdsCount, " ");
    if(argcount < 2) {
        printf("no sourcefile specifed.\n");
        return;
    }

    allocateArgs(&args, argcount);
    int j = 0;
    char *ptr = strtok(commands, " ");

    while(ptr != NULL) {
        args[j] = ptr;
        ptr = strtok(NULL, " ");
        j++;
    }
    char progname[SIZE] ={0};
    char sourcefile[SIZE] = {0};
    strcpy(progname, args[0]);
    strcpy(sourcefile, args[1]);
    send(socket, progname, SIZE, 0);
    send(socket, sourcefile, SIZE, 0);
    char bytes[SIZE] = {0}; 
    recv(socket, bytes, SIZE, 0);

    char *file_buffer = (char *) calloc(atoi(bytes), sizeof(char));              
    recv(socket, file_buffer, atoi(bytes), 0);

    char *lineptr = strtok(file_buffer, "\n");
    int count = 0;
    while(lineptr != NULL) {
        if(count == 40) {
            count = 0;
            printf("Press Any key to show more lines...\n");
            fflush(stdin);
            getchar();
        }
        printf("%s\n", lineptr);
        count++;
        lineptr = strtok(NULL, "\n");
    }
    free(file_buffer);
 }


char* executeRun(int socket, char* commands) {
    //run progname [args] [-f localfile]    
    char **args;
    char cmdsCount[SIZE] = {0};
    strcpy(cmdsCount, commands);
    int argcount = countCommands(cmdsCount, " ");

    allocateArgs(&args, argcount);
    
    int j = 0;
    char *ptr = strtok(commands, " ");

    while(ptr != NULL) {
        args[j] = ptr;
        ptr = strtok(NULL, " ");
        j++;
    }

    char progname[SIZE] ={0};
    char *localfile = (char*) malloc(SIZE * sizeof(char));
    int flagInd = -1;
    strcpy(progname, args[0]);
    send(socket, progname, SIZE, 0);
    char *argcountChar = (char*) malloc(SIZE * sizeof(char));
    
    //check for localfile
    int flag = -1;
    for(int i = 0; i < argcount; i++) {
        if(strcmp(args[i], "-f") == 0) {
            flag = 1;
            flagInd = i;
            strcpy(localfile, args[i+1]);
            argcount -= 2;
            break;
        }
    }
    sprintf(argcountChar, "%d", (argcount-1));
    send(socket, argcountChar, SIZE, 0);
    int k = 1;

    while(k != (argcount)) {
        if(k == flagInd || k == flagInd+1) {
            k++;
            continue;
        }
        send(socket, args[k], SIZE, 0);
        k++;
    }
    if(flag == -1) {
        strcpy(localfile, "no");
    }
    return localfile;
}


void executeList(int socket, char* commands) {
    //list [progname] [-l]
    char *ptr = strtok(commands, " ");

    if(ptr == NULL) {
        //list prognames
        send(socket, ".", SIZE, 0);
    } else {
        char progname[SIZE] ={0};
        strcpy(progname, ptr);
        ptr = strtok(NULL, " ");
        send(socket, progname, SIZE, 0);

        if(ptr == NULL) {
            send(socket, "short", SIZE, 0);

        } else if(strcmp(ptr, "-l") == 0) {
            //long list
            send(socket, "long", SIZE, 0);
        }
    }
}
