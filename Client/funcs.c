#include "include.h"


void readFile(FILE *fp, int socket) {
    long numberBytes;
    char size[50];
    fseek(fp, 0L, SEEK_END);
    numberBytes = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    sprintf(size, "%ld", numberBytes);
    printf("bytes = %s \n", size);
    send(socket, size, SIZE, 0);

    char *buffer = (char*) malloc(numberBytes * sizeof(char));
    memset(buffer, '\0', numberBytes);
    fread(buffer, sizeof(char), numberBytes, fp);

    send(socket, buffer, numberBytes, 0);
    memset(buffer, '\0', numberBytes);
    free(buffer);
}


void sendFiles(int socket, char * ptr, char* delim, FILE * fp) {
        while(ptr != NULL) {
            if(strcmp(ptr, "-f") == 0) {
                ptr = strtok(NULL, delim);
                continue;
            }

            //open file
            fp = fopen(ptr, "r");
            if(fp == NULL) {
                printf("Error opening file.\n");
                close(socket);
                exit(1);
            }
            //send filename
            send(socket, ptr, SIZE, 0);

            //read and send file 
            readFile(fp, socket);
            fclose(fp);
            ptr = strtok(NULL, delim);
        }
        //send end message
        send(socket, "end", SIZE, 0);
}


void allocateArgs(char ***args, int count) {
    *args = malloc(count * sizeof(char*));
    
    for(int i = 0; i < count; i++) {
        (*args)[i] = (char*) malloc(SIZE * sizeof(char));
    }
}

char* parseFFlag(char * str, char* delim) {
    char *ptr = strtok(str, delim);
    while(ptr != NULL) {
        if(strcmp(ptr, "-f") == 0) {
            return "found";
        }
        ptr = strtok(NULL, delim);
    }
    return "not found";
}


int countCommands(char * str, char* delim) {
    char *ptr = strtok(str, delim);
    int count = 0;

    while(ptr != NULL) {
        count++;
        ptr = strtok(NULL, delim);
    }
    return count;

}


void writeToFile(char* buffer, char * filename) {
    FILE * fp;
    fp = fopen(filename, "w");
    fputs(buffer, fp);
    fclose(fp);
}
