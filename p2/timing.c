#define  _GNU_SOURCE
#include <stdio.h> // printf()
#include <stdlib.h> // exit()

#include <sys/mman.h> // mmap()
#include <sys/resource.h> // getrusage

#include <pthread.h> // lock
#include <unistd.h> // write
// #include <sys/wait.h> // wait()
// #include <unistd.h> // getpid(), fork()

#include <fcntl.h> // open/close
#include <sys/time.h> // timeval



#define PAGELEN 4096
#define LOOPS 1000000
#define LOOPS2 20000
#define LENGTH 4096

static pthread_mutex_t mutex[LOOPS];


float getUserTime(struct rusage* start, struct rusage* end){
    return (end->ru_utime.tv_sec - start->ru_utime.tv_sec) +
           1e-6*(end->ru_utime.tv_usec - start->ru_utime.tv_usec);
}

float getSystemTime(struct rusage* start, struct rusage* end){
    return (end->ru_stime.tv_sec - start->ru_stime.tv_sec) +
           1e-6*(end->ru_stime.tv_usec - start->ru_stime.tv_usec);
}

float getRealTime(struct timeval* start, struct timeval* end){
    return (end->tv_sec - start->tv_sec) +
           1e-6*(end->tv_usec - start->tv_usec);
}


int main(){
    struct rusage start, end;
    struct timeval start2, end2;
    float user_sub, sys_sub;
    int i;


    /* ************************** NMAP ************************** */
    getrusage(RUSAGE_SELF, &start);
    for(i = 0; i < LOOPS; i++){}
    getrusage(RUSAGE_SELF, &end);

    user_sub = getUserTime(&start, &end);
    sys_sub = getSystemTime(&start, &end);

    getrusage(RUSAGE_SELF, &start);
    for(i = 0; i < LOOPS; i++){
        mmap(NULL, PAGELEN, PROT_READ, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    }
    getrusage(RUSAGE_SELF, &end);

    printf("**************************** MMAP ****************************\n");
    printf("User time:\t%e, System time:\t%e\n",
            (getUserTime(&start, &end) - user_sub)/LOOPS,
            (getSystemTime(&start, &end) - sys_sub)/LOOPS);




    /* ************************** mutex LOCK ************************** */
    for (i = 0; i < LOOPS; i++) {
        pthread_mutex_init(&mutex[i], NULL);
    }

    getrusage(RUSAGE_SELF, &start);
    for(i = 0; i < LOOPS; i++){
        pthread_mutex_lock(&mutex[i]);
    }
    getrusage(RUSAGE_SELF, &end);

    printf("************************* mutex LOCK *************************\n");
    printf("User time:\t%e, System time:\t%e\n",
            (getUserTime(&start, &end) - user_sub)/LOOPS,
            (getSystemTime(&start, &end) - sys_sub)/LOOPS);






    /* ************************* WRITE AND READING ************************* */
    char data[LENGTH];
    char readdata[LENGTH];

    float wallTimeLoop = 0;

    gettimeofday(&start2, NULL);
    for(i = 0; i < LOOPS2; i++){}
    gettimeofday(&end2, NULL);

    wallTimeLoop = getRealTime(&start2, &end2);






    /* ******************** WRITE BYPASS DISK PAGE CACHE ******************* */
    int fd = open("/tmp/tpatro01.txt", O_RDWR | O_CREAT | O_DIRECT | O_SYNC, S_IWUSR | S_IRUSR);

    
    for (i = 0; i < LENGTH; i++) {
        data[i] = 'z';
    }


    gettimeofday(&start2, NULL);
    for(i = 0; i < LOOPS2; i++){
        write(fd, data, LENGTH);
    }
    gettimeofday(&end2, NULL);


    close(fd);
    remove("/tmp/tpatro01.txt");



    printf("**************** WRITE BYPASS DISK PAGE CACHE ****************\n");
    printf("Wall time:\t%e\n",
            (getRealTime(&start2, &end2)-wallTimeLoop)/LOOPS2);


    fd = open("/tmp/tpatro01.txt", O_RDWR | O_CREAT | O_DIRECT | O_SYNC, S_IWUSR | S_IRUSR);

    gettimeofday(&start2, NULL);
    for(i = 0; i < LOOPS2; i++){}
    gettimeofday(&end2, NULL);

    wallTimeLoop = getRealTime(&start2, &end2);


    for (i = 0; i < LENGTH; i++) {
        data[i] = 'z';
    }

    for(i = 0; i < LOOPS2; i++){
        write(fd, data, LENGTH);
    }


    gettimeofday(&start2, NULL);
    for(i = 0; i < LOOPS2; i++){
        read(fd, readdata, LENGTH);
    }
    gettimeofday(&end2, NULL);

    close(fd);
    remove("/tmp/tpatro01.txt");

    printf("***************** READ BYPASS DISK PAGE CACHE ****************\n");
    printf("Wall time:\t%e\n",
            (getRealTime(&start2, &end2)-wallTimeLoop)/LOOPS2);




    fd = open("/tmp/tpatro01.txt", O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

    for (i = 0; i < LENGTH; i++) {
        data[i] = 'z';
    }


    gettimeofday(&start2, NULL);
    for(i = 0; i < LOOPS2; i++){
        write(fd, data, LENGTH);
    }
    gettimeofday(&end2, NULL);

    close(fd);
    remove("/tmp/tpatro01.txt");

    printf("****************** WRITE DISK PAGE CACHE *********************\n");
    printf("Wall time:\t%e\n",
            (getRealTime(&start2, &end2)-wallTimeLoop)/LOOPS2);


    
    
    fd = open("/tmp/tpatro01.txt", O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

    for (i = 0; i < LENGTH; i++) {
        data[i] = 'z';
    }

    for(i = 0; i < LOOPS2; i++){
        write(fd, data, LENGTH);
    }


    gettimeofday(&start2, NULL);
    for(i = 0; i < LOOPS2; i++){
        read(fd, readdata, LENGTH);
    }
    gettimeofday(&end2, NULL);

    close(fd);
    remove("/tmp/tpatro01.txt");

    printf("******************* READ DISK PAGE CACHE *********************\n");
    printf("Wall time:\t%e\n",
            (getRealTime(&start2, &end2)-wallTimeLoop)/LOOPS2);


    return 0;
}