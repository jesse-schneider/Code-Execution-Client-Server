#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <sys/utsname.h>
#include <signal.h>
#include <wait.h>
#include <sys/stat.h>
#include <dirent.h>

#define SIZE 1024

/*  server functions    */
void killZombie(int);
void sendError(char *, char * , int);
char * getPath(char *);
char * getSourcePath(char *, char *);

/*  put functions    */
void writeFile(char*, char*, char*);
void putFile(int, char*);

/*  get functions    */
void sendFile(int , char * );
void readFile(FILE *, int );

/*  run functions    */
int checkIfCompile(time_t, char*);
void compile(char* , char* );
char* executeBin(char*, char*, char*);

/*  sys functions    */
void systemInfo(int);

/*  list functions    */
char* performLs(char *);