#include "include.h"

void systemInfo(int socket) {
    //system name struct
    struct utsname sys;
    char *str = (char*) calloc(1024, sizeof(char));

    //populate the struct
    if(uname(&sys)) { exit(-1); }
    char * mach = sys.sysname;
    char * cpu = sys.machine;
    strcat(str, "OS: ");
    strcat(str, mach);
    strcat(str, " CPU Type: ");
    strcat(str, cpu);
    send(socket, str, SIZE, 0);
    free(str);
}


void putFile(int socket, char* path) {
    char file_size[SIZE] = {0};
    char file_name[SIZE] = {0};
    
    while(1) {
        memset(file_size, '\0', SIZE);
        memset(file_name, '\0', SIZE);
        recv(socket, file_name, SIZE, 0);
        
        if(strcmp(file_name, "end") == 0)
            break;

        printf("file name = %s\n", file_name);

        recv(socket, file_size, SIZE, 0);
        printf("file size = %s\n", file_size);

        char *file_buffer = (char *) calloc(atoi(file_size), sizeof(char));              
        recv(socket, file_buffer, atoi(file_size), 0);
        //printf("file recieved.\n");
        // printf("file = %s\n", file_buffer);
        char outfile[SIZE];
        strcpy(outfile, path);
        writeFile(file_name, file_buffer, outfile);
        free(file_buffer);
    }
}

void writeFile(char * filename, char * buffer, char * path) {
    FILE * out = NULL;
    strcat(path, filename);
    out = fopen(path, "w");
    fputs(buffer, out);
    fclose(out);
}

int checkIfCompile(time_t binMod, char* progname) {
    //loop through all source files, get m_time, compare to binray m_time
    DIR *d;
    struct stat st;
    int ret = -1;
    struct dirent *dir;
    char path[SIZE];
    strcpy(path, "./");
    strcat(path, progname);
    d = opendir(path);
    if(d) {
        while((dir = readdir(d)) != NULL) {
            char name[SIZE];
            strcpy(name, dir->d_name);
            stat(name, &st);
            time_t fileMod = st.st_mtime;
            double diff = difftime(fileMod, binMod);
            if(diff < 0) {
                ret = 1;
                break;
            }
        }
        closedir(d);
    }
    return ret;
}

void compile(char* path, char* progname) {
    //compile each source file cc -c <source.c>  
    char compileStr[SIZE];
    strcpy(compileStr, "cc ");
    DIR *d;
    FILE * binFp;
    struct stat st;
    int ret = -1;
    struct dirent *dir;
    printf("path = %s\n", path);
    d = opendir(path);
    if(d) {
        while((dir = readdir(d)) != NULL) {
            char name[SIZE];
            strcpy(name, dir->d_name);
            stat(name, &st);
            mode_t mode = st.st_mode;

            if(S_ISREG(mode) == 0) {
                int len = strlen(name);
                if(name[len-1] == 'c' && name[len-2] == '.') {
                    //compile object
                    FILE * fp;
                    char objStr[SIZE];
                    chdir(path);
                    strcpy(objStr, "cc -c ");
                    strcat(objStr, name);
                    printf("cc = %s\n", objStr);
                    fp = popen(objStr, "w");
                    pclose(fp);
                    chdir("..");
                    name[len-1] = 'o';
                    strcat(compileStr, name);
                    strcat(compileStr, " ");
                }
            }
        }
        //compile cc <objects> -o <progname>
        strcat(compileStr, " -o ");
        strcat(compileStr, progname);
        chdir(path);
        binFp = popen(compileStr, "w");
        pclose(binFp);
        chdir("..");
        closedir(d);
    } else {
        printf("cannot open dir\n");
    }
}

char* executeBin(char* path, char *progname, char *args) {
    //go into dir
    FILE * fp;
    chdir(path);
    //execute programe
    char str[SIZE] = {0};
    char *buffer = (char*) calloc(SIZE, sizeof(char));

    strcpy(str, "./");
    strcat(str, progname);
    strcat(str, " ");
    strcat(str, args);
    fp = popen(str, "r");

    fgets(buffer, SIZE, fp);
    pclose(fp);
    chdir("..");
    return buffer;
}

char* performLs(char *dirname) {
    char *files = (char*) calloc(SIZE , sizeof(char));
	DIR *d;
	struct dirent *dir;
    struct stat st;

	if ((d = opendir(dirname)) == NULL) {
		fprintf(stderr,"ls:can't open %s\n",dirname);
	} else {
		while ((dir = readdir(d)) != NULL) {
			printf("%s\n",dir->d_name);
            if(strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                if(strcmp(dirname, ".") == 0) {
                    stat(dir->d_name, &st);
                    if(S_ISDIR(st.st_mode) != 0) {
                        strcat(files, strcat(dir->d_name, " "));
                    }
                } else {
                    strcat(files, strcat(dir->d_name, " "));
                }
            }
		}
		closedir(d);
	}
    return files;
}

void killZombie(int sig) {
    if(waitpid(-1, NULL, WNOHANG) < 0)
        printf("decapitation\n");
}

char * getPath(char *progname) {
    char *path = getcwd(NULL, 0);
    strcat(path, "/");
    strcat(path, progname);
    return path;
}

char * getSourcePath(char *source, char *path) {
    char *sourcePath = (char*) calloc(SIZE, sizeof(char));
    strcpy(sourcePath, path);
    strcat(sourcePath, "/");
    strcat(sourcePath, source);
    return sourcePath;
}

void sendError(char *progname, char * error, int socket) {
    char str[SIZE] = {0};
    strcat(str, progname);
    strcat(str, error);
    send(socket, str, SIZE, 0);
}

void sendFile(int socket, char * file) {
    FILE * fp = NULL;
    //open file
    fp = fopen(file, "r");
    if(fp == NULL) {
        printf("Error opening file.\n");
        close(socket);
        exit(1);
    }

    //read and send file 
    readFile(fp, socket);
    fclose(fp);
}

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