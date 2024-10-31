/*
 * catsem.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use SEMAPHORES to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>


/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2

/*Time that cat/mouse will take to eat*/
#define EATTIME 3

/*Rounds that each mouse and cat will eat*/
#define EATROUNDS 5

/*Mouse names*/
const char* const miceNames[NMICE] = {"Theseus", "Minotaur"};

/*Cat names*/
const char* const catNames[NCATS] = {"Reggie", "Pink Panther", "Tom", "Mr. Mittens", "Cheshire", "Garfield"};


/*Instantiating global variables*/

static volatile int dish1Busy = 0; //no one at dish1 to begin
static volatile int dish2Busy = 0; //no one at dish2 to begin

static volatile struct semaphore* semDish; //used when modifying dish variables
static volatile struct semaphore* mutex; //used when modifying or checking wait counts for cat/mice
static volatile struct semaphore* done; //done in kitchen, leaving
static volatile struct semaphore* catsQueue; //cat waiting
static volatile struct semaphore* miceQueue; //mice waiting
static volatile struct semaphore* allDoneLock; //lock when updating allDone

static volatile int allDone; //count how many animals have finished all rounds

static volatile int catsWaitCount = 0;
static volatile int miceWaitCount = 0;
static volatile int allDishAvailable = 1;

static volatile int noCatEat = 1; //intially, no cats eating
static volatile int noMouseEat = 1; //intially, no mice eating


//create semaphores and intialize other integer values
//if they are NULL after trying to create, we are out of MEM and panic

static void setup()
{       
        dish1Busy = 0;
        dish2Busy = 0;
        catsWaitCount = 0;
        miceWaitCount = 0;
        allDishAvailable = 1;

        noCatEat = 1; 
        noMouseEat = 1;


        if (catsQueue == NULL){
                //0 initially waiting
                catsQueue = sem_create("catsQueue", 0);
                if (catsQueue == NULL){
                        panic("catsQueue: Out of MEM.\n");
                }
        }

        if (miceQueue == NULL){
                //0 intially waiting
                miceQueue = sem_create("miceQueue", 0);
                if (miceQueue == NULL){
                        panic("miceQueue: Out of MEM.\n");
                }
        }

        if (semDish == NULL){
                semDish = sem_create("semDish", 1);
                if (semDish == NULL){
                        panic("semDish: Out of MEM.\n");
                }
        }

        if (mutex == NULL){
                mutex = sem_create("mutex", 1);
                if (mutex == NULL){
                        panic("mutex: Out of MEM.\n");
                }
        }
        
        if (allDoneLock == NULL){
                allDoneLock = sem_create("allDoneLock", 1);
                if (allDoneLock == NULL){
                        panic("allDoneLock: Out of MEM.\n");
                }
        }

        if (done == NULL){
                done = sem_create("done", 0);
                if (done == NULL){
                        panic("done: Out of MEM.\n");
                }
        }

}


//destroy all semaphores to save memory
static void destroy()
{
        sem_destroy(catsQueue);
        sem_destroy(miceQueue);
        sem_destroy(semDish);
        sem_destroy(mutex);
        sem_destroy(done);
        sem_destroy(allDoneLock);

        catsQueue = NULL;
        miceQueue = NULL;
        semDish = NULL;
        mutex = NULL;
        done = NULL;
        allDoneLock = NULL;
}


/*
 * 
 * Function Definitions
 * 
 */




/*
 * catsem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
catsem(void * unusedpointer, 
       unsigned long catnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;
        (void) catnumber;

        int j;
        
        kprintf("Cat %s starting.\n", catNames[catnumber]); //cat thread starting

        //must enter and exit kitchen for EATROUNDS times
        for (j = 0; j < EATROUNDS; j++){

                clocksleep(random() % EATTIME); //play around before entering kitchen
        
                //local variables
                int firstCatEat = 0;
                int anotherCatEat = 0;
                int mydish = 0;

                //First cat and no mouse

                //critical section
                P(mutex); //wait
                if (allDishAvailable == 1){
                        allDishAvailable = 0; //false
                        V(catsQueue); // let first cat in
                }

                catsWaitCount++;
                V(mutex);
                //end critical section

                P(catsQueue); //first cat in, other wait

                if(noCatEat == 1){
                        noCatEat = 0; //false
                        firstCatEat = 1; //true
                }
                else{
                        firstCatEat = 0;
                }


                
                //first cat controls the kitchen
                if (firstCatEat == 1){
                        P(mutex);
                        if(catsWaitCount > 1){
                                anotherCatEat = 1;
                                V(catsQueue); //let another cat in
                        }
                        V(mutex);
                }
                kprintf("Cat %s in the kitchen.\n", catNames[catnumber]); //print cat in



                //all cats (first cat and non-first cat) in the kitchen
                P(semDish); //protect shared variables
                if (dish1Busy == 0){
                        dish1Busy = 1; //global so others can't take dish
                        mydish = 1; //local to cat/mouse
                }
                else{
                        assert(dish2Busy == 0);
                        dish2Busy = 1;
                        mydish = 2;
                }

                V(semDish);

                kprintf("Cat %s is eating.\n", catNames[catnumber]); //print cat that is eating
                clocksleep(random() % EATTIME); //enjoys food
                kprintf("Cat %s finished eating.\n", catNames[catnumber]); //done eating

                
                //done eating, so release dish
                P(semDish);
                if (mydish == 1){ //release dish 1
                        dish1Busy = 0;
                }
                else{ //release dish 2
                        dish2Busy = 0;
                }
                V(semDish);

                P(mutex);
                catsWaitCount--; //update number of waiting cats
                V(mutex);


                //switch turn
                if (firstCatEat == 1){
                        if(anotherCatEat == 1){
                                P(done);
                        }
                        kprintf("First cat, %s, is leaving.\n", catNames[catnumber]);
                        noCatEat = 1;

                        P(mutex);
                        if (miceWaitCount > 0){ //mice waiting
                                V(miceQueue); //let mice in first since cats just ate
                        }
                        else if (catsWaitCount > 0){ 
                                V(catsQueue); //let cat eat
                        }
                        else {
                                allDishAvailable = 1; //no one ready to eat
                        }
                        V(mutex);

                }
                else{
                        kprintf("Non-first cat, %s, is leaving.\n", catNames[catnumber]);
                        V(done); //inform the first cat
                }

        }
        

        //after all rounds of eating, will be tired and won't eat again
        P(allDoneLock);
        kprintf("Cat %s exiting.\n", catNames[catnumber]);
        allDone++;
        V(allDoneLock);
}
        

/*
 * mousesem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */


//Pretty much the exact same as catsem, but for mice
static
void
mousesem(void * unusedpointer, 
         unsigned long mousenumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;
        (void) mousenumber;

        int j; //count times eating

        kprintf("Mouse %s starting.\n", miceNames[mousenumber]); //mouse thread starting


        //must enter and exit kitchen for EATROUNDS times

        for(j = 0; j < EATROUNDS; j++){

                clocksleep(random() % EATTIME); //play around before entering kitchen

                //local variables
                int firstMouseEat = 0;
                int anotherMouseEat = 0;
                int mydish = 0;

                //First mouse and no cat

                //critical section
                P(mutex); //wait
                if (allDishAvailable == 1){
                        allDishAvailable = 0; //false
                        V(miceQueue); // let first mouse in
                }

                miceWaitCount++;
                V(mutex);
                //end critical section

                P(miceQueue); //first mouse in, other wait

                if(noMouseEat == 1){
                        noMouseEat = 0; //false
                        firstMouseEat = 1; //true
                }
                else{
                        firstMouseEat = 0;
                }


                
                //first mouse controls the kitchen
                if (firstMouseEat == 1){
                        P(mutex);
                        if(miceWaitCount > 1){
                                anotherMouseEat = 1;
                                V(miceQueue); //let another mouse in
                        }
                        V(mutex);
                }
                kprintf("Mouse %s in the kitchen.\n", miceNames[mousenumber]); //print mouse in



                //all mice (first mouse and non-first mouse) in the kitchen
                P(semDish); //protect shared variables
                if (dish1Busy == 0){
                        dish1Busy = 1;
                        mydish = 1; //local to cat/mouse
                }
                else{
                        assert(dish2Busy == 0);
                        dish2Busy = 1;
                        mydish = 2;
                }

                V(semDish);

                kprintf("Mouse %s is eating.\n", miceNames[mousenumber]); //prints name of mouse eating
                clocksleep(random() % EATTIME); //enjoys food
                kprintf("Mouse %s finished eating.\n", miceNames[mousenumber]); //done eating

                
                //done eating, so release dish
                P(semDish);
                if (mydish == 1){ //release dish 1
                        dish1Busy = 0;
                }
                else{ //release dish 2
                        dish2Busy = 0;
                }
                V(semDish);

                P(mutex);
                miceWaitCount--; //update number of waiting mice
                V(mutex);


                //switch turn
                if (firstMouseEat == 1){
                        if(anotherMouseEat == 1){
                                P(done);
                        }
                        kprintf("First mouse, %s, is leaving.\n", miceNames[mousenumber]);
                        noMouseEat = 1;

                        P(mutex);
                        if (catsWaitCount > 0){ //cat waiting
                                V(catsQueue); //let cats in first since mice just ate
                        }
                        else if (miceWaitCount > 0){ 
                                V(miceQueue); //let mice eat
                        }
                        else {
                                allDishAvailable = 1; //no one waiting to eat
                        }
                        V(mutex);

                }
                else{
                        kprintf("Non-first mouse, %s, is leaving.\n", miceNames[mousenumber]);
                        V(done); //inform the first mouse
                }


        }

        P(allDoneLock);
        kprintf("Mouse %s exiting.\n", miceNames[mousenumber]); //mouse finished all rounds
        allDone++;
        V(allDoneLock);
}


/*
 * catmousesem()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catsem() and mousesem() threads.  Change this 
 *      code as necessary for your solution.
 */

int
catmousesem(int nargs,
            char ** args)
{
        int index, error;

        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;

        setup(); //create semaphores and other variables
   
   
        /*
         * Start NCATS catsem() threads.
         */

        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catsem Thread", 
                                    NULL, 
                                    index, 
                                    catsem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catsem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }
        
        /*
         * Start NMICE mousesem() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mousesem Thread", 
                                    NULL, 
                                    index, 
                                    mousesem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mousesem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        //while not all animals have finished all rounds of eating, wait
        while(allDone < NCATS + NMICE){
                thread_yield();
        }

        destroy(); //destroy semaphores and other variables used to save memory

        return 0;
}


/*
 * End of catsem.c
 */
