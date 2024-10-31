/*
 * Synchronization primitives.
 * See synch.h for specifications of the functions.
 */

#include <types.h>
#include <lib.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <machine/spl.h>

////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *namearg, int initial_count)
{
	struct semaphore *sem;

	sem = kmalloc(sizeof(struct semaphore));
	if (sem == NULL) {
		return NULL;
	}

	sem->name = kstrdup(namearg);
	if (sem->name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->count = initial_count;
	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	spl = splhigh();
	assert(thread_hassleepers(sem)==0);
	splx(spl);

	/*
	 * Note: while someone could theoretically start sleeping on
	 * the semaphore after the above test but before we free it,
	 * if they're going to do that, they can just as easily wait
	 * a bit and start sleeping on the semaphore after it's been
	 * freed. Consequently, there's not a whole lot of point in 
	 * including the kfrees in the splhigh block, so we don't.
	 */

	kfree(sem->name);
	kfree(sem);
}

void 
P(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	assert(in_interrupt==0);

	spl = splhigh();
	while (sem->count==0) {
		thread_sleep(sem);
	}
	assert(sem->count>0);
	sem->count--;
	splx(spl);
}

void
V(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);
	spl = splhigh();
	sem->count++;
	assert(sem->count>0);
	thread_wakeup(sem);
	splx(spl);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;

	lock = kmalloc(sizeof(struct lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->name = kstrdup(name);
	if (lock->name == NULL) {
		kfree(lock);
		return NULL;
	}
	
	lock->holder = NULL;
	// add stuff here as needed
	
	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);
	assert(lock->holder == NULL);
	// add stuff here as needed
	
	kfree(lock->name);
	kfree(lock);
}

void
lock_acquire(struct lock *lock)
{
	// Write this
	int spl = splhigh();


	if (lock_do_i_hold(lock)){ /*check deadlock*/
		panic("lock %s at %p: Deadlock./n", lock->name, lock);
	}

	/*wait for lock to become free*/
	while (lock->holder != NULL){
		thread_sleep(lock);
	}

	/*this thread is holding the lock*/
	lock->holder = curthread; /*lock's holder is set to this process*/
	splx(spl); /*set spl back to what it was*/
}	

void
lock_release(struct lock *lock)
{
	// Write this
	int spl = splhigh();

	/*only holder can release*/
	assert(lock_do_i_hold(lock));


	lock->holder = NULL; /*lock's holder is no longer this thread*/
	thread_wakeup(lock); /*signal that lock is available*/
	splx(spl); /*set spl back to what it was*/

	
}

int
lock_do_i_hold(struct lock *lock)
{
	// Write this
	int spl = splhigh();
	int same;
	assert(lock != NULL);

	if (lock->holder == curthread){
		same = 1; /* true */
	}
	else{
		same = 0; /* false */
	}

	splx(spl); /*turn on interrupts to previous level*/

	/* 1 means lock is held by this thread*/
	return same;
}
////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(struct cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->name = kstrdup(name);
	if (cv->name==NULL) {
		kfree(cv);
		return NULL;
	}
	
	// add stuff here as needed
	
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	// add stuff here as needed
	
	kfree(cv->name);
	kfree(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	// Write this
	assert(cv != NULL);
	assert(lock != NULL);

	int spl = splhigh(); /*turn off interrupts*/
	
	lock_release(lock); /*release lock*/
	
	/*put thread to sleep for reason given by cv
	until another thread wakes it up with cv being
	passed in*/
	thread_sleep(cv);
	
	lock_acquire(lock); /*acquire lock now that thread is ready*/

	splx(spl); /*set priority level back to what it was*/
	
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	// Write this
	assert(cv != NULL);
	assert(lock != NULL);
	
	int spl = splhigh(); /*turn off interrupts*/

	if (!(lock_do_i_hold(lock))){
		panic("cv_signal error: cv %s at %p, lock %s at %p.\n", cv->name, cv, lock->name, lock); 
	}

	thread_wakeone(cv); /*wake up first thread with cv indicator*/	
	splx(spl); /*set priority level back to what it was*/

} 

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	// Write this
	assert(cv != NULL);
	assert(lock != NULL);

	int spl = splhigh(); /*turn off interrupts*/

	thread_wakeup(cv); /*wake up ALL threads with cv indicator*/

	splx(spl); /*set priority level back to what it was*/
}
