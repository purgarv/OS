#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ftw.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_TOKENS 256

char nameBuf[64] = "mysh";
int err = 0;
char lastFunc[256];
int console = 0;
int numpipes = 0;

int numOfChar(char* str){
    int i, count;
    for (i=0, count=0; str[i]; i++)
        count += (str[i] == '"');

    return count;
}

int split(char *buffer, char *argv[], int argv_size){
    char *p, *start_of_word;
    int c;
    enum states { DULL, IN_WORD, IN_STRING } state = DULL;
    int argc = 0;

    for (p = buffer; argc < argv_size && *p != '\0'; p++) {
        c = (unsigned char) *p;
        switch (state) {
        case DULL:
            if (isspace(c)) {
                continue;
            }

            if (c == '"') {
                state = IN_STRING;
                start_of_word = p + 1; 
                continue;
            }
            state = IN_WORD;
            start_of_word = p;
            continue;

        case IN_STRING:
            if (c == '"') {
                *p = 0;
                argv[argc++] = start_of_word;
                state = DULL;
                numpipes++;
            }
            continue;

        case IN_WORD:
            if (isspace(c)) {
                *p = 0;
                argv[argc++] = start_of_word;
                state = DULL;
            }
            continue;
        }
    }

    if (state != DULL && argc < argv_size)
        argv[argc++] = start_of_word;

    return argc;
}

int is_empty_or_comment(char *str, int* comment) {

    while (*str != '\0') {
        if (!isspace((unsigned char)*str)){
            if((unsigned char)*str == '#')
                *comment = 1;
            else
                *comment = 0;
            return 0;
        }
        str++;
    }
    return 1;
}

void help(){

    printf(" name [ime] - nastavi ime lupine, če imena ne podamo, izpiše ime lupine (privzeto ime je mysh),\
    \n help - izpiše spisek podprtih ukazov,\
    \n status - izpiše izhodni status zadnjega (v ospredju) izvedenega ukaza,\
    \n exit [status] - konča lupino s podanim izhodnim statusom,\
    \n print [args...] - izpiše podane argumente na standardni izhod (brez končnega skoka v novo vrstico),\
    \n echo [args...] - kot print, le da izpiše še skok v novo vrstico,\
    \n pid - izpiše pid procesa (kot $BASHPID), \
    \n ppid - izpiše pid starša.\
    \n dirchange [imenik] - zamenjava trenutnega delovnega imenika, če imenika ne podamo, skoči na /,\
    \n dirwhere - izpis trenutnega delovnega imenika,\
    \n dirmake [imenik] - ustvarjanje podanega imenika,\
    \n dirremove [imenik] - brisanje podanega imenika,\
    \n dirlist [imenik] - preprost izpis vsebine imenika (le imena datotek, ločena z dvema presledkoma), če imena ne podamo, se privzame trenutni delovni imenik.\
    \n linkhard [cilj] [ime] - ustvarjanje trde povezave na cilj,\
    \n linksoft [cilj] [ime] - ustvarjanje simbolične povezave na cilj,\
    \n linkread [ime] - izpis cilja podane simbolične povezave,\
    \n linklist [ime] - izpiše vse trde povezave na datoteko z imenom ime,\
    \n unlink [ime] - brisanje datoteke,\
    \n rename [izvor] [ponor] - preimenovanje datoteke,\
    \n cpcat [izvor] [ponor] - ukaz cp in cat združena\n");
}


int name(int argc, char* name){
    if(argc > 1){
        return 1;
    }
    else if(argc == 1){
        strcpy(nameBuf, name);
    }
    else{
        printf("%s\n", nameBuf);
    }

    return 0;
}

void status(){
    if(err != 0){
        perror(lastFunc);
    }
    printf("%d\n", err);
}


int exit_(int stat){

    exit(stat);

}

int print(int argc, char* args[]){
    for(int i = 1; i < argc-1; i++){
        printf("%s ", args[i]);
    }

    printf("%s", args[argc-1]);
}

int echo(int argc, char* args[]){
    for(int i = 1; i < argc-1; i++){
        printf("%s ", args[i]);
    }
    printf("%s", args[argc-1]);
    printf("\n");
}

void pid(){
    printf("%d\n", getpid());
}

void ppid(){
    printf("%d\n", getppid());
}

void dirchange(int argc, char* dir){
    if(argc == 1){
        chdir(dir);
    }
    else{
        chdir("/");
    }
}

char* dirwhere(){
    char s[256];
    printf("%s\n", getcwd(s, 256));
    return s;
}

void dirmake(char* dir){
    struct stat st = {0};

    mkdir(dir, 0700);
    err = errno;

    strcpy(lastFunc, "dirmake");
}

void dirremove(char* path){

    DIR *dir;
    struct dirent *file;

    char basepath[256];
    strcpy(basepath, path);

    dir = opendir(path);

    while ((file = readdir(dir))) {
        if(strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0){

            strcat(basepath, "/");
            strcat(basepath, file->d_name);

            remove(basepath);

            strcpy(basepath, path);
        }
    }
    remove(path);
}

void dirlist(int argc, char* dir){

    char directory[256];

    if(argc == 0){
        getcwd(directory, 256);
    }
    else{
        strcpy(directory, dir);
    }

    struct dirent *de;  

    DIR *dr = opendir(directory);

    printf(".  ..  ");
  
    while ((de = readdir(dr)) != NULL){
        if(strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
            printf("%s  ", de->d_name);
    }
    printf("\n");

    closedir(dr);
}

int linkhard(char* izvor,char* cilj){

    int l = link(izvor, cilj);

    return 0;
}

int linksoft(char* izvor,char* cilj){

    int l = symlink(izvor, cilj);

    return 0;
}

void linkread(char* name){

    char buf[256];

    readlink(name, buf, 256);

    printf("%s\n", buf);

    memset(buf, 0, strlen(buf));
}

void linklist(char* name){

    int inodeNum;
    struct stat s;
    stat(name, &s);
    inodeNum = s.st_ino;

    struct dirent *de;  

    char dir[256];
    getcwd(dir, 256);

    DIR *dr = opendir(dir);
  
    while ((de = readdir(dr)) != NULL){
        if(de->d_ino == inodeNum)
            printf("%s  ", de->d_name);
    }
    printf("\n");
    closedir(dr);
}


void changename(char* old,char* new){
    rename(old, new);
}


int cpcat(int argc, char* argv[]){
    int in;
	int out;
	int err = 0;

	if(argc <= 1 || *argv[1] == '-')
		in = STDIN_FILENO;
	else
		if( (in = open(argv[1], O_RDONLY)) < 0){
			err = errno;
			perror(argv[0]);
			exit(err);
		}
	
	if(argc <= 2)
		out = STDOUT_FILENO;
	else
		if( (out = open(argv[2], O_RDWR | O_CREAT, S_IRWXU)) < 0){
			err = errno;
			perror(argv[0]);
			exit(err);
		}

	
	char ch;
	int len;
	while(read(in, &ch, 1)){
		len = write(out, &ch, 1);
	}
	
    if(in != 0){

        if( close(in) < 0){
            err = errno;
            perror(argv[0]);
            exit(err);
        }
    }
    if(out != 1){
        if( close(out) < 0){
            err = errno;
            perror(argv[0]);
            exit(err);
        }
    }

	return 0;
}

void ozadje(int numWords, char* strings[]){

    int pid = fork();

    if(pid == 0){
        // child

        pid = fork();

        if (pid > 0)
            exit(EXIT_SUCCESS);

        if (setsid() < 0)
            exit(EXIT_FAILURE);

        pid = fork();

        if (pid > 0)
            exit(EXIT_SUCCESS);

        umask(0);

        evaluate(numWords, strings);
        exit(0);
    }
    else{
        // parent
        wait(NULL);

    }

}

void redirect_in(int numWords, char* strings[]){

    strings[numWords]++;

    int fd = open(strings[numWords], O_RDONLY, 0644);

    int savefd = dup(0);
    dup2(fd, 0);

    evaluate(numWords, strings);

    dup2(savefd, 0);

    close(fd);
    close(savefd);

}
void redirect_out(int numWords, char* strings[]){

    strings[numWords]++;

    int fd = open(strings[numWords], O_CREAT | O_RDWR, 0644);

    int savefd = dup(1);
    dup2(fd, 1);

    evaluate(numWords, strings);

    dup2(savefd, 1);
    close(fd);
    close(savefd);
}


void pipes_begin(int numWords, char* strings[], int* fd){

    pipe(fd);

    int pid = fork();

    if(pid == 0){
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        evaluate(numWords, strings);
        exit(0);
    }

}

void pipes_cont(int numWords, char* strings[], int* fd1, int* fd2){

    pipe(fd2);

    int pid = fork();

    if(pid == 0){
        dup2(fd1[0], 0);
        dup2(fd2[1], 1);
        close(fd1[0]);
        close(fd1[1]);
        close(fd2[0]);
        close(fd2[1]);
        evaluate(numWords, strings);
        
        exit(0);
    }
    close(fd1[0]);
    close(fd1[1]);
    
}

void pipes_end(int numWords, char* strings[], int* fd2){

    int pid = fork();
    
    if(pid == 0){
        dup2(fd2[0], 0);
        close(fd2[0]);
        close(fd2[1]);
        evaluate(numWords, strings);

        exit(0);
    }
    close(fd2[0]);
    close(fd2[1]);

}

int count_words(char* string){
    int count = 0;

    for(int i = 0; i < strlen(string); i++){
        if(string[i] == ' '){
            count++;
        }
    }

    return count + 1;
}

void startpipes(int numWords, char* strings[]){
    int fd1[2];
    int fd2[2];

    char* args[MAX_TOKENS];

    char buf[1024];
    strcpy(buf, strings[1]);

    numWords = split(buf, args, MAX_TOKENS);

    pipes_begin(numWords, args, &fd2);

    //evaluate(numWords, args);

    for(int j = 0; j < numWords; j++)
        memset(args[j], '\0', strlen(args[j]));

    for(int i = 2; i < numpipes; i++){
        fd1[0] = fd2[0];
        fd1[1] = fd2[1];

        strcpy(buf, strings[i]);

        numWords = split(buf, args, MAX_TOKENS);
        
        pipes_cont(numWords, args, &fd1, &fd2);

        for(int j = 0; j < numWords; j++)
            memset(args[j], 0, strlen(args[j]));
    
    }

    strcpy(buf, strings[numpipes]);

    numWords = split(buf, args, MAX_TOKENS);

    pipes_end(numWords, args, &fd2);


    numpipes = 0;

    int wpid = 0;
    while ((wpid = wait(NULL)) > 0);

}


void evaluate(int numWords, char* strings[]){
    
    if(strcmp(strings[numWords-1], "&") == 0){
        numWords--;
        ozadje(numWords, strings);
        return;
    }

    if(strings[numWords-1][0] == '>'){
        numWords--;
        redirect_out(numWords, strings);
        return;
    }
    if(strings[numWords-1][0] == '<'){
        numWords--;
        redirect_in(numWords, strings);
        return;
    }

    if(strcmp(strings[0], "pipes") == 0){
        startpipes(numWords, strings);
    }
    else if(strcmp(strings[0], "name") == 0) {
        name(numWords - 1, strings[1]);
    }
    else if(strcmp(strings[0], "help") == 0){
        help();
    }
    else if(strcmp(strings[0], "status") == 0){
        status();
    }
    else if(strcmp(strings[0], "exit") == 0){
        exit_(atoi(strings[1]));
    }
    else if(strcmp(strings[0], "print") == 0){
        print(numWords, strings);
    }
    else if(strcmp(strings[0], "echo") == 0){
        echo(numWords, strings);
    }
    else if(strcmp(strings[0], "pid") == 0){
        pid();
    }
    else if(strcmp(strings[0], "ppid") == 0){
        ppid();
    }
    else if(strcmp(strings[0], "dirchange") == 0){
        dirchange(numWords - 1, strings[1]);
    }
    else if(strcmp(strings[0], "dirwhere") == 0){
        dirwhere();
    }
    else if(strcmp(strings[0], "dirmake") == 0){
        dirmake(strings[1]);
    }
    else if(strcmp(strings[0], "dirremove") == 0){
        dirremove(strings[1]);
    }
    else if(strcmp(strings[0], "dirlist") == 0){
        dirlist(numWords - 1, strings[1]);
    }
    else if(strcmp(strings[0], "linkhard") == 0){
        linkhard(strings[1], strings[2]);
    }
    else if(strcmp(strings[0], "linksoft") == 0){
        linksoft(strings[1], strings[2]);
    }
    else if(strcmp(strings[0], "linkread") == 0){
        linkread(strings[1]);
    }
    else if(strcmp(strings[0], "linklist") == 0){
        linklist(strings[1]);
    }
    else if(strcmp(strings[0], "unlink") == 0){
        unlink(strings[1]);
        err = errno;
        strcpy(lastFunc, "unlink");
    }
    else if(strcmp(strings[0], "rename") == 0){
        changename(strings[1], strings[2]);
    }
    else if(strcmp(strings[0], "cpcat") == 0){
        cpcat(numWords, strings);
    }
    else{
            
        int pid = fork();

        if(pid == 0){
            strings[numWords] = NULL;
            execvp(strings[0], strings);
            exit(0);
        }

        wait(NULL);
    }
}


int main(int argc, char *argv[]){

    console = isatty(0);

    setbuf(stdout, NULL);

    char *line = NULL;
    size_t len = 0;

    char* strings[MAX_TOKENS];

    int* comment = 0;
    char buf[1024];

    if(console){
        printf("%s>", nameBuf);
    }

    while(getline(&line, &len, stdin) != -1){
        
        if(is_empty_or_comment(line, &comment) || comment){
            comment = 0;
            continue;
        }

        strcpy(buf, line);

        int numWords = split(buf, strings, MAX_TOKENS);
        
        evaluate(numWords, strings);
        
        /*
        for (int i = 0; i < numWords; i++)
            printf("%s\n", strings[i]);
        */

        if(console){
            printf("%s>", nameBuf);
        }

    }

	return 0;
}