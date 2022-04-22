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
#define LOOPS2 10000
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
    char data2[LENGTH*2];
    for (i = 0; i < LENGTH*2; i++) {
        data2[i] = 'z';
    }

    // printf("Size: %ld\n", sizeof(data));
    
    int fd = open("/tmp/tpatro01.txt", O_RDWR | O_CREAT | O_DIRECT | O_SYNC, S_IWUSR | S_IRUSR);

    long unsigned int start_buff = ((((unsigned long)data2 + PAGELEN - 1) / PAGELEN) * PAGELEN);

    gettimeofday(&start2, NULL);
    for(i = 0; i < LOOPS2; i++){
        write(fd, (void *)start_buff, LENGTH);
    }
    gettimeofday(&end2, NULL);


    close(fd);
    remove("/tmp/tpatro01.txt");



    printf("**************** WRITE BYPASS DISK PAGE CACHE ****************\n");
    printf("Wall time:\t%e\n",
            (getRealTime(&start2, &end2)-wallTimeLoop)/LOOPS2);





     /* ******************** READ BYPASS DISK PAGE CACHE ******************* */
    
    fd = open("/tmp/tpatro01.txt", O_APPEND | O_RDWR | O_CREAT | O_DIRECT | O_SYNC, S_IWUSR | S_IRUSR);

    gettimeofday(&start2, NULL);
    for(i = 0; i < LOOPS2; i++){}
    gettimeofday(&end2, NULL);

    wallTimeLoop = getRealTime(&start2, &end2);

    // LOOPS2 = 5;

    // long unsigned int start_buff = ((((unsigned long)data2 + PAGELEN - 1) / PAGELEN) * PAGELEN);
    char readdata2[2*LENGTH];

    for (i = 0; i < LENGTH*2; i++) {
        data2[i] = 'z';
    }

    for(i = 0; i < LOOPS2; i++){
        write(fd, (void *)start_buff, LENGTH);
    }

    lseek(fd, 0, SEEK_SET);

    long unsigned int read_buff = ((((unsigned long)readdata2 + PAGELEN - 1) / PAGELEN) * PAGELEN);

    gettimeofday(&start2, NULL);
    for(i = 0; i < LOOPS2; i++){
        read(fd, (void *)read_buff, LENGTH);
        // printf("read: %d\n", read(fd, read_buff, LENGTH));
    }
    gettimeofday(&end2, NULL);

    close(fd);
    remove("/tmp/tpatro01.txt");

    /* make sure you're actually reading and writing */
    /* align your buffers, check bytes read is as expected */
    /* make sure the file has enough data */
     /* around e-05 */
     /* around e-5 (slower than write but not Significantly) */



    printf("***************** READ BYPASS DISK PAGE CACHE ****************\n");
    printf("Wall time:\t%e\n",
            (getRealTime(&start2, &end2)-wallTimeLoop)/LOOPS2);


     /* ******************** WRITE DISK PAGE CACHE ******************* */

    fd = open("/tmp/tpatro01.txt", O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

    for (i = 0; i < LENGTH; i++) {
        data[i] = 'z';
    }
    
    // start_buff = ((((unsigned long)data + PAGELEN - 1) / PAGELEN) * PAGELEN);


    gettimeofday(&start2, NULL);
    for(i = 0; i < LOOPS2; i++){
        write(fd, (void *)start_buff, LENGTH);
    }
    gettimeofday(&end2, NULL);

    close(fd);
    remove("/tmp/tpatro01.txt");

    printf("****************** WRITE DISK PAGE CACHE *********************\n");
    printf("Wall time:\t%e\n",
            (getRealTime(&start2, &end2)-wallTimeLoop)/LOOPS2);

    
    
    /* ******************** READ DISK PAGE CACHE ******************* */
    
    
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