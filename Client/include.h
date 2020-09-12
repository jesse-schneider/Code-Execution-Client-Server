#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define SIZE 1024

/*  put functions */
void readFile(FILE*, int);
char* parseFFlag(char * , char*);
void sendFiles(int, char*, char*, FILE *);

/*  run functions   */
void writeToFile(char*, char * );
int countCommands(char*, char* );
void allocateArgs(char ***, int );
