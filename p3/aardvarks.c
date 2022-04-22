#include "/comp/111/assignments/aardvarks/anthills.h" 
#include <semaphore.h>
#include <time.h> // nanosleep

#define SLOTS AARDVARKS_PER_HILL*ANTHILLS
double eating[SLOTS];

int survivor_ants[ANTHILLS] = {
    ANTS_PER_HILL, ANTS_PER_HILL, ANTS_PER_HILL
};

// * Mutexes must be global variables. 
pthread_mutex_t lock;


int initialized=FALSE; // semaphores and mutexes are not initialized 

// main function
void *aardvark(void *input) { 
    char aardvark = *(char *)input; // name of aardvark

    int i, hill_num, opt_spot, opt_ants;
    double curr_time;
    /* INITIALIZATION BY FIRST THREAD */
    pthread_mutex_lock(&init_lock); 
    if (!initialized++) { // this succeeds only for one thread
        pthread_mutex_init(&lock, NULL);
        for (i = 0; i < SLOTS; i++) {
            eating[i] = -3.0;
        }
    } 
    pthread_mutex_unlock(&init_lock); 
    
    
    
    // now be an aardvark
    // while there are ants to eat
    while (chow_time()) {
        // try to slurp
        curr_time = elapsed();
        opt_ants = 0;
        opt_spot = -1;
        pthread_mutex_lock(&lock);

        // for each avaliable anthill slurping slot
        for (i = 0; i < SLOTS; i++) {
            hill_num = i/ANTHILLS;

            // if there are anthills and can eat
            if (survivor_ants[hill_num] > 0 && eating[i] + 1 < curr_time) {
                if (survivor_ants[hill_num] > opt_ants) {
                    opt_ants = survivor_ants[hill_num];
                    opt_spot = i;
                }
            }
        }
        if (opt_spot != -1) {
            eating[opt_spot] = curr_time + .02; 
            survivor_ants[opt_spot / 3]--;
            pthread_mutex_unlock(&lock);
            slurp(aardvark, opt_spot / 3);
        }
        else {
        
        pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}