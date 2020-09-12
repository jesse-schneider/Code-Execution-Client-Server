#include "include.h"
#define PORT 8080
#define BACKLOG 10


int main(int argc, char** argv) {

    //initalise sockets, server and client address and client buffer
    int s_socket, c_socket;
    int valread;
    struct sockaddr_in server, client;
    int serverlen = sizeof(server), clientlen = sizeof(client);
    char client_buffer[SIZE] = {0};

    pid_t child;

    //create socket 
    s_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(s_socket == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    } else {
        printf("socket created successfully.\n");
    }

    //set server variables
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    //binding server to port 80
    if(bind(s_socket, (struct sockaddr *) &server, serverlen) < 0) {
        perror("bind failed\n");
        exit(EXIT_FAILURE);
    } else {
        printf("socket bind successful\n");
    }

    //server listen
    if (listen(s_socket, BACKLOG) != 0) {
        perror("listen failed\n");
        exit(EXIT_FAILURE);
    } else {
        printf("socket listening...\n");
    }

    while(1) {

        //accept incoming client connection
        c_socket = accept(s_socket, (struct sockaddr *)&client, &clientlen);
        if (c_socket < 0) {
            perror("accept failed\n");
            return 1;
        } else {
            puts("\n--Client connected--\n");
        }

        //fork process for multiple clients
        child = fork();
        signal(SIGCHLD, killZombie);
               
        if(child < 0) {
            perror("fork failed\n");
            exit(1);

        } else if (child == 0) {     

            //infinte loop to continue processing client until it disconnects
            while (1) {
                //clear client buffer
                memset(client_buffer, '\0', SIZE);

                //receive client command
                valread = recv(c_socket, client_buffer, SIZE, 0);
                if(valread < 0) {
                    perror("error reading from client\n");
                    exit(1);
                }
                if(strcmp(client_buffer, "exit") == 0) {
                    close(c_socket);
                    close(s_socket);
                    signal(SIGCHLD, killZombie);
                    printf("Client Exited Successfully.");
                    exit(0);
                } else if(strcmp(client_buffer, "sys") == 0) {
                    systemInfo(c_socket);
                } else if(strcmp(client_buffer, "put") == 0) {
                    //put file into <progname> directory
                    char progname[SIZE] = {0};
                    struct stat st;

                    recv(c_socket, progname, SIZE, 0);
                    printf("progname: %s\n",progname);

                    char *path = getPath(progname);
                    strcat(path, "/");

                    // printf("pathname: %s\n",path);
                    int statval = stat(path, &st);
                    if(statval == -1) {
                        mkdir(path, 0700);
                        send(c_socket, "doesn't exist", SIZE, 0);
                        putFile(c_socket, path);
                        send(c_socket, "success", SIZE, 0);
                    } else if(statval == 0) {
                        char overwrite_buffer[SIZE] = {0};
                        send(c_socket, "exists", SIZE, 0);
                        recv(c_socket, overwrite_buffer, SIZE, 0);
                        if(strcmp(overwrite_buffer, "overwrite") == 0) {
                            //overwrite dir
                            putFile(c_socket, path);
                            send(c_socket, "success", SIZE, 0);
                        } else {
                            char err[200] = "  already exists, try a new name or use the -f flag to overwrite.\n";
                            sendError(progname, err, c_socket);
                        } 
                    }
                } else if(strcmp(client_buffer, "run") == 0) {
                    //complie and run <progname>
                    char progname[SIZE] = {0};
                    char mode[SIZE] = {0};
                    struct stat st;

                    recv(c_socket, progname, SIZE, 0);
                    printf("progname: %s\n",progname);

                    recv(c_socket, mode, SIZE, 0);
                    int argcount = atoi(mode);

                    char *path = getPath(progname);

                    //create args list
                    char args[SIZE] = {0};
                    char incoming [SIZE];

                    for(int i = 0; i < argcount ; i++) {
                        recv(c_socket, incoming, SIZE, 0);
                        strcat(args, incoming);
                        strcat(args, " ");
                    }

                    int statval = stat(path, &st);
                    if(statval == 0) {
                        //dir exists
                        struct stat binst;
                        char binaryPath [SIZE] = {0};
                        strcpy(binaryPath, path);
                        strcat(binaryPath, progname);
                        if(stat(binaryPath, &binst) == 0) {
                            //binary exists, check binary created date vs source code modifed date
                            //st_mtime == modified time
                            time_t binMod = binst.st_mtime;
                            int ifComp = checkIfCompile(binMod, progname);
                            if(ifComp == 1) {
                                //compile and run
                                compile(path, progname);
                                char* response = executeBin(path, progname, args);
                                send(c_socket, response, SIZE, 0);
                                free(response);

                            } else {
                                //don't compile just run
                                char* response = executeBin(path, progname, args);
                                send(c_socket, response, SIZE, 0);
                                free(response);
                            }

                        } else {
                            //binary does not exist, compile and run it
                            compile(path, progname);
                            char* response = executeBin(path, progname, args);
                            send(c_socket, response, SIZE, 0);
                            free(response);
                        }
                    } else if(statval == -1) {
                        //dir doesn't exist
                        char err[200] = "  doesn't exist, please use an existing program.\n";
                        sendError(progname, err, c_socket);
                    }
                } else if(strcmp(client_buffer, "list") == 0) {
                     //list [-l]
                    char progname[SIZE] = {0};
                    char mode[SIZE] = {0};
                    struct stat st;

                    recv(c_socket, progname, SIZE, 0);
                    printf("progname: %s\n",progname);

                    if(strcmp(progname, ".") == 0) {
                        //list prognames
                        char *buffer = performLs(progname);
                        send(c_socket, buffer, SIZE, 0);
                        continue;
                    }

                    recv(c_socket, mode, SIZE, 0);
                    printf("mode: %s\n",mode);

                    char *path = getPath(progname);
                    int statval = stat(path, &st);

                    if(statval == 0) {
                        //dir exists
                        if(strcmp(mode, "short") == 0) {
                            char *buffer = performLs(progname);
                            printf("buffer = %s\n", buffer);
                            send(c_socket, buffer, SIZE, 0);
                        } else {
                            //go into dir
                            FILE * fp;
                            chdir(path);
                            //execute programe
                            char str[SIZE] = {0};
                            char buffer[SIZE] = {0};
                            char *response = (char*) calloc(SIZE, sizeof(char));
                            strcpy(str, "ls -l");
                            fp = popen(str, "r");
                            while(fgets(buffer, SIZE, fp) != NULL) {
                                strcat(response, buffer);
                            }
                            pclose(fp);
                            chdir("..");
                            send(c_socket, response, SIZE, 0);
                            free(response);
                        }
                    } else if(statval == -1) {
                        //dir doesn't exist
                        char err[50] = "  directory doesn't exist.\n";
                        sendError(progname, err, c_socket);
                    }
                } else if(strcmp(client_buffer, "get") == 0) {
                    //get progname sourcefile
                    char progname[SIZE] = {0};
                    char source[SIZE] = {0};
                    struct stat st;

                    recv(c_socket, progname, SIZE, 0);
                    printf("progname: %s\n",progname);

                    recv(c_socket, source, SIZE, 0);
                    printf("source: %s\n",source);
                    char *path = getPath(progname);
                    char *sourcePath = getSourcePath(source, path);

                    int statval = stat(path, &st);
                    if(statval == 0) {
                        //dir exists
                        int sourceStat = stat(sourcePath, &st);
                        if(sourceStat == 0) {
                            //sourcefile exists
                            sendFile(c_socket, sourcePath);
                            // send(c_socket, "source file exists", SIZE, 0);
                        } else {
                            //sourcefile does not exist
                            char err[200] = "  doesn't exist.\n";
                            sendError(source, err, c_socket);
                        } 
                      
                    } else if(statval == -1) {
                        //dir doesn't exist
                        char err[200] = "  directory doesn't exist.\n";
                        sendError(progname, err, c_socket);
                    }
                    free(path);
                    free(sourcePath);
                }
            }
        }
    }
    //close sockets
    close(c_socket);
    close(s_socket);
    return 0;
}