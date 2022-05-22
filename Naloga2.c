#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include <ctype.h>
#include <limits.h>
#include <wait.h>


struct Queue {
    int front, rear, size;
    unsigned capacity;
    int* array;
};

struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    queue->rear = capacity - 1;
    queue->array = (int*)malloc(
        queue->capacity * sizeof(int));
    return queue;
}

int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}

int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}

void enqueue(struct Queue* queue, int item){
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

int dequeue(struct Queue* queue){
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}


typedef struct {
    int *array;
    size_t used;
    size_t size;
} Array;

void initArray(Array *a, size_t initialSize) {
    a->array = malloc(initialSize * sizeof(int));
    a->used = 0;
    a->size = initialSize;
}

void insertArray(Array *a, int element) {
  
    if (a->used == a->size) {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(int));
    }
    a->array[a->used++] = element;
}

void freeArray(Array *a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

int sys(int argc, char *argv[]){

    char* path = (char*)malloc(256);

    if(argc == 0){
        path = "/proc";
    }
    else{
        strcpy(path, argv[1]);
    }

    strcat(path, "/version");

    FILE *fptr;
    fptr = fopen(path, "r");

    if(fptr == NULL){
        printf("Error!");   
        exit(1);        
    }

    char* lin = (char*)malloc(256);
    char* gcc = (char*)malloc(256);

    fscanf(fptr, "Linux version %s %*s %*s %*s %s", lin, gcc);

    fclose(fptr);

    printf("Linux: %s\n", lin);
    printf("gcc: %s\n", gcc);

    return 0;
}


int sysex(int argc, char *argv[]){

    sys(argc, argv);

    char* module_path = (char*)malloc(256);
    char* swap_path = (char*)malloc(256);
    char* swap = (char*)malloc(256);

    if(argc == 0){
        module_path = "/proc";
        swap_path = "/proc";
    }
    else{
        strcpy(module_path, argv[1]);
        strcpy(swap_path, argv[1]);
    }

    strcat(module_path, "/modules");
    strcat(swap_path, "/swaps");

    FILE *fptr;
    fptr = fopen(module_path, "r");

    if(fptr == NULL){
        printf("Error!");   
        exit(1);        
    }

    int count = 0;
    char c;
    for (c = getc(fptr); c != EOF; c = getc(fptr)){
        if (c == '\n')
            count = count + 1;
    }

    fclose(fptr);

    fptr = fopen(swap_path, "r");

    if(fptr == NULL){
        printf("Error!");
        exit(1);        
    }

    fscanf(fptr, "%*s %*s %*s %*s %*s %s", swap);

    fclose(fptr);

    printf("Swap: %s\n", swap);
    printf("Modules: %d\n", count);

    exit(count);

}

int me(){
    int uid = getuid();
    int euid = geteuid();
    int gid = getgid();
    int egid = getegid();
    char cwd[255]; getcwd(cwd, 255);
    int pid = getpid();
    int priority = getpriority(PRIO_PROCESS, pid);

    char acpath[255]; snprintf(acpath, 255, "/proc/%d/", pid);
    int acc = access(acpath, F_OK);


    struct utsname u;
    uname(&u);


    struct rlimit lim;
    getrlimit(RLIMIT_CPU, &lim);


    struct timeval tim;
    struct timezone zone;

    gettimeofday(&tim, &zone);
    
    printf("Uid: %d\n", uid);
    printf("EUid: %d\n", euid);
    printf("Gid: %d\n", gid);
    printf("EGid: %d\n", egid);
    printf("Cwd: %s\n", cwd);
    printf("Priority: %d\n", priority);
    printf("Process proc path: /proc/%d/\n", pid);
    if(acc == 0){
        printf("Process proc access: yes\n");
    }
    else{
        printf("Process proc access: no\n");
    }
    printf("OS name: %s\n", u.sysname);
    printf("OS release: %s\n", u.release);
    printf("OS version: %s\n", u.version);
    printf("Machine: %s\n", u.machine);
    printf("Node name: %s\n", u.nodename);
    printf("Timezone: %d\n", zone.tz_dsttime);
    printf("CPU limit: %ld\n", lim.rlim_max);
    return 0;
}


int is_int(struct dirent *file) {
    char *p;

    for (p = file->d_name; *p; p++) {
        if (!isdigit(*p))
            return 0;

    }

    return 1;
}


void swap(int* xp, int* yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

int pids(int argc, char *argv[], Array* arr){
    char* path = (char*)malloc(261);

    if(argc == 0){
        path = "/proc";
    }
    else{
        path = argv[1];
    }

    DIR *dir;
    struct dirent *file;

    dir = opendir(path);

    Array a;
    initArray(&a, 20);
    int val;
    while ((file = readdir(dir))) {
        if (!is_int(file))
            continue;

        val = atoi(file->d_name);
        insertArray(&a, val);
    }

    int i, j, min_idx;
    for (i = 0; i < a.used - 1; i++) {
 
        min_idx = i;
        for (j = i + 1; j < a.used; j++)
            if (a.array[j] < a.array[min_idx])
                min_idx = j;
            
        swap(&a.array[min_idx], &a.array[i]);
    }


    if(arr == NULL){
        for(int k = 0; k < a.used; k++){
            printf("%d\n", a.array[k]);
        }
    }
    else{
        *arr = a;
    }

    closedir(dir);

    return 0;
}


char* getProcessName(char *dirPath, int pid) {

  FILE *f = fopen(dirPath, "r");
  char *name = malloc(256);

  if (f) {
    fscanf(f, "%*s %s", name);
    fclose(f);
  }

  return name;
}

char getProcessState(char *dirPath, int pid) {

  FILE *f = fopen(dirPath, "r");

  char line[256];
  for (int i = 0; i < 1; i++)
    if (fgets(line, 256, f) == NULL)
      break;

  char state;

  if (f) {
    fscanf(f, "%*s %s", &state);

    fclose(f);
  }

  return state;
}

int getProcessPpid(char *dirPath, int pid) {

  FILE *f = fopen(dirPath, "r");

  int ppid = 0;

  if (f) {
    char line[256];
    for (int i = 0; i < 5; i++)
      if (fgets(line, 256, f) == NULL)
        break;

    char* ppidBuffer = malloc(256);
    fscanf(f, "%*s %s", ppidBuffer);

    ppid = atoi(ppidBuffer);

    fclose(f);
  }

  return ppid;
}

int getStNiti(char *dirPath, int pid) {
    
    int filecount = 0;
    DIR * dirp;
    struct dirent * entry;

    dirp = opendir(dirPath);

    while ((entry = readdir(dirp)) != NULL) {
        if (is_int(entry))
            filecount++;
    }

    closedir(dirp);

    return filecount;
}


int getStDatotek(char *dirPath, int pid) {
    
    int filecount = 0;
    DIR * dirp;
    struct dirent * entry;

    dirp = opendir(dirPath);

    while ((entry = readdir(dirp)) != NULL) {
        if (is_int(entry))
            filecount++;
    }

    closedir(dirp);

    return filecount;
}


int names(int argc, char *argv[], Array* arr, char **name){

    char* path = (char*)malloc(261);

    if(argc == 0){
        path = "/proc";
    }
    else{
        path = argv[1];
    }

    DIR *dir;
    struct dirent *file;

    dir = opendir(path);

    Array a;
    initArray(&a, 20);
    int val;
    FILE *fp;

    while ((file = readdir(dir))) {
        if (!is_int(file))
            continue;

        val = atoi(file->d_name);
        insertArray(&a, val);
    }

    char **names = malloc(sizeof(char*) * 1000);

    for(int i = 0; i < a.used; i++){
        names[i] = malloc(sizeof(char) * 256);
    }

    char procpath[256];
    for(int i = 0; i < a.used; i++) {
        sprintf(procpath, "%s/%d/status", path, a.array[i]);
        names[i] = getProcessName(procpath, a.array[i]);
    }

    char* temp = (char*)malloc(256);

    for(int i = 0; i < a.used; i++){
        for(int j = i + 1; j < a.used; j++){
            if(strcasecmp(names[i],names[j]) > 0){
                strcpy(temp,names[i]);
                strcpy(names[i],names[j]);
                strcpy(names[j],temp);

                swap(&a.array[i], &a.array[j]);
            }
            else if(strcmp(names[i],names[j]) == 0){
                if(a.array[i] > a.array[j]){
                    swap(&a.array[i], &a.array[j]);
                }
            }
        }
    }

    if(arr == NULL){
        for(int k = 0; k < a.used; k++){
            printf("%d %s\n", a.array[k], names[k]);
        }
    }
    else{
        for(int k = 0; k < a.used; k++){
            name[k] = names[k];
        }
    }    

    closedir(dir);

    return 0;
}

int contains(int arr[], int el, int size){

    for(int i = 0; i < size; i++){
        if(arr[i] == el)
            return 1;
    }
    return 0;
}

int ps(int argc, char *argv[]){

    char* path = (char*)malloc(261);
    
    if(argc == 0){
        path = "/proc";
    }
    else{
        path = argv[1];
    }

    Array arr;
    initArray(&arr, 50);
    

    char **name = malloc(sizeof(char*) * 1000);

    for(int i = 0; i < arr.used; i++){
        name[i] = malloc(sizeof(char) * 256);
    }


    pids(argc, argv, &arr);

    int ppid[arr.used];
    char status[arr.used];


    char procpath[256];
    for(int i = 0; i < arr.used; i++) {
        sprintf(procpath, "%s/%d/status", path, arr.array[i]);

        ppid[i] = getProcessPpid(procpath, arr.array[i]);
        status[i] = getProcessState(procpath, arr.array[i]);
        name[i] = getProcessName(procpath, arr.array[i]);
    }


    printf("%5s %5s %6s %s\n", "PID", "PPID", "STANJE", "IME");

    if(argc <= 1){
        for(int i = 0; i < arr.used; i++){
            printf("%5d %5d %6c %s\n", arr.array[i], ppid[i], status[i] ,name[i]);
        }
    }
    else{
        int izpis[arr.used];
        int index = 0;

        struct Queue* q = createQueue(arr.used);
        enqueue(q, atoi(argv[2]));

        int done = 0;

        while(1){

            if(isEmpty(q)){
                break;
            }

            int current = dequeue(q);

            for(int i = 0; i < arr.used; i++){
                if(ppid[i] == current){
                    izpis[index] = i;
                    index+=1;
                    enqueue(q, arr.array[i]);
                }
                if(arr.array[i] == atoi(argv[2]) && !done){
                    izpis[index] = i;
                    index+=1;
                    done = 1;
                }
            }
        }

        int i, j, min_idx;
        for (i = 0; i < index - 1; i++) {
            min_idx = i;
            for (j = i + 1; j < index; j++){
                if (izpis[j] < izpis[min_idx])
                    min_idx = j;
            }
            swap(&izpis[min_idx], &izpis[i]);
        }
        
        for(int i = 0; i < index; i++){
            printf("%5d %5d %6c %s\n", arr.array[izpis[i]], ppid[izpis[i]], status[izpis[i]] ,name[izpis[i]]);
        }
        
    }

    return 0;
}

int psext(int argc, char *argv[]){

    char* path = (char*)malloc(261);
    
    if(argc == 0){
        path = "/proc";
    }
    else{
        path = argv[1];
    }

    Array arr;
    initArray(&arr, 50);
    
    char **name = malloc(sizeof(char*) * 1000);

    for(int i = 0; i < arr.used; i++){
        name[i] = malloc(sizeof(char) * 256);
    }

    pids(argc, argv, &arr);

    int ppid[arr.used];
    int niti[arr.used];
    char status[arr.used];
    int files[arr.used];


    char procpath[256];
    for(int i = 0; i < arr.used; i++) {
        sprintf(procpath, "%s/%d/status", path, arr.array[i]);

        ppid[i] = getProcessPpid(procpath, arr.array[i]);
        status[i] = getProcessState(procpath, arr.array[i]);
        name[i] = getProcessName(procpath, arr.array[i]);

        sprintf(procpath, "%s/%d/task", path, arr.array[i]);

        niti[i] = getStNiti(procpath, arr.array[i]);

        sprintf(procpath, "%s/%d/fd", path, arr.array[i]);

        files[i] = getStDatotek(procpath, arr.array[i]);
    }

    printf("%5s %5s %6s %6s %6s %s\n", "PID", "PPID", "STANJE", "#NITI", "#DAT.", "IME");

    if(argc <= 1){
        for(int i = 0; i < arr.used; i++){
            printf("%5d %5d %6c %6d %6d %s\n", arr.array[i], ppid[i], status[i], niti[i], files[i], name[i]);
        }
    }
    else{
        int izpis[arr.used];
        int index = 0;

        struct Queue* q = createQueue(arr.used);
        enqueue(q, atoi(argv[2]));

        int done = 0;

        while(1){

            if(isEmpty(q)){
                break;
            }

            int current = dequeue(q);

            for(int i = 0; i < arr.used; i++){
                if(ppid[i] == current){
                    izpis[index] = i;
                    index+=1;
                    enqueue(q, arr.array[i]);
                }
                if(arr.array[i] == atoi(argv[2]) && !done){
                    izpis[index] = i;
                    index+=1;
                    done = 1;
                }
            }
        }

        int i, j, min_idx;
        for (i = 0; i < index - 1; i++) {
            min_idx = i;
            for (j = i + 1; j < index; j++){
                if (izpis[j] < izpis[min_idx])
                    min_idx = j;
            }
            swap(&izpis[min_idx], &izpis[i]);
        }
        
        for(int i = 0; i < index; i++){
            printf("%5d %5d %6c %6d %6d %s\n", arr.array[izpis[i]], ppid[izpis[i]], status[izpis[i]], niti[izpis[i]], files[izpis[i]], name[izpis[i]]);
        }
        
    }



    return 0;
}

void createChildren(int nums[], int kumulativa[], int index){

    for(int i = 0; i < nums[index]; i++){
        int pid = fork();
        if(pid == 0){
            createChildren(nums, kumulativa, kumulativa[index] + i + 1);
            sleep(2);
            exit(0);
        }
    }

}

int forktree(int argc, char *argv[]){

    int nums[500];
    memset(nums, 0, sizeof(nums));
    int index = 0;
    while(1){
        if(!(scanf("%d", &nums[index]) == 1))
            break;

        index+=1;
    }

    int kumulativa[index+1];
    kumulativa[0] = 0;

    for(int i = 1; i < index+1; i++)
        kumulativa[i] = kumulativa[i-1] + nums[i-1];


    createChildren(nums, kumulativa, 0);

    /*
    for(int i = 0; i < index; i++)
        printf("%d\n", nums[i]);

    printf("\n");

    for(int i = 0; i < index + 1; i++)
        printf("%d\n", kumulativa[i]);
    */

    int num = getpid();
    char root_pid[50];
    sprintf(root_pid, "%d", num);

    sleep(1);
    int pid = fork();
    //sleep(1);

    if (pid == 0){

        char *arr[] = {"pstree", "-c", root_pid,  NULL};
        execvp(arr[0], arr);
        //child process
    }
    else{
        wait(NULL);
        //parent process
    }
    

    return 0;
}

int main(int argc, char *argv[]){

    if(strcmp(argv[1], "sys") == 0){
        sys(argc - 2, &argv[1]);
    }
    else if(strcmp(argv[1], "sysext") == 0){
        sysex(argc - 2, &argv[1]);
    }
    else if(strcmp(argv[1], "me") == 0){
        me();
    }
    else if(strcmp(argv[1], "pids") == 0){
        pids(argc - 2, &argv[1], NULL);
    }
    else if(strcmp(argv[1], "names") == 0){
        names(argc - 2, &argv[1], NULL, NULL);
    }
    else if(strcmp(argv[1], "ps") == 0){
        ps(argc - 2, &argv[1]);
    }
    else if(strcmp(argv[1], "psext") == 0){
        psext(argc - 2, &argv[1]);
    }
    else if(strcmp(argv[1], "forktree") == 0){
        forktree(argc, &argv[0]);
    }

	return 0;
}
