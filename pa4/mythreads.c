/*	User-level thread system
 *
 */

#include <setjmp.h>
#include <string.h>

#include "aux.h"
#include "umix.h"
#include "mythreads.h"

static int MyInitThreadsCalled = 0;	/* 1 if MyInitThreads called, else 0 */

static struct thread {			/* thread table */
	int valid;			/* 1 if entry is valid, else 0 */
	jmp_buf env;			/* current context */
	jmp_buf envCopy;
	void (*f)();
	int p;
} thread[MAXTHREADS];

static struct threadQueue {
	int valid;
	int prev;
	int next;
} threadQueue[MAXTHREADS];

static int head = -1;
static int tail = -1;
static int numOfThreads = 0;
static int runningThread = 0;
static int lastSpawned = 0;
static int yieldingThread;
static int threadExit = 0;

#define STACKSIZE	65536		/* maximum size of thread stack */


/* Helper methods to manipulate elements inside threadQueue */

void addHead( int t ) {
	threadQueue[t].valid = 1;
	threadQueue[t].prev = -1;
	threadQueue[t].next = head;
	if(numOfThreads==0)
		tail = t;
	else
		threadQueue[head].prev = t;
	head = t;
	numOfThreads++;
}

void addTail( int t ) {
	threadQueue[t].valid = 1;
	threadQueue[t].prev = tail;
	threadQueue[t].next = -1;
	if(numOfThreads==0)
		head = t;
	else
		threadQueue[tail].next = t;
	tail = t;
	numOfThreads++;
}

void removeFromQueue ( int t ) {
	threadQueue[t].valid = 0;
	
	if(numOfThreads > 1) {
		if(t == head) {
			head = threadQueue[t].next;
			threadQueue[head].prev = -1;
		}
		else if(t == tail) {
			tail = threadQueue[t].prev;
			threadQueue[tail].next = -1;
		}
		else {
			threadQueue[threadQueue[t].next].prev = threadQueue[t].prev;
			threadQueue[threadQueue[t].prev].next = threadQueue[t].next;
		}
	}
	else {
		head = -1;
		tail = -1;
	}

	threadQueue[t].prev = -1;
	threadQueue[t].next = -1;
	numOfThreads--;
}




/* Helper method to partition the stack into MAXTHREADS parts */
int stackPartition( int numPartitions ) {

	if( numPartitions <= 1 )
		return -1;
	else {

		char s[STACKSIZE];

		if (((int) &s[STACKSIZE-1]) - ((int) &s[0]) + 1 != STACKSIZE) {
			Printf ("Stack space reservation failed\n");
			Exit ();
		}

		int id = MAXTHREADS - numPartitions + 1;

		if( setjmp(thread[id].env) != 0 ) {
			(*thread[runningThread].f) (thread[runningThread].p);
			//Printf("Thread %d calling MyExitThread.\n", runningThread);
			MyExitThread();
		}
		else {
			/* Make a copy of the evironment - (SP, PC, FP, etc..)
			 * When a new thread is spawned at an index that has previously
			 * been used, the SP needs to be restored to its assigned
			 * starting address in memory, and PC restored to its original
			 * value, which in this case will be stored in thread[id].envCopy 
			 * and will never be altered.
			 */
			memcpy(thread[id].envCopy, thread[id].env, sizeof(thread[id].env));
			stackPartition(numPartitions-1);
		}
	}
}



/*	MyInitThreads () initializes the thread package. Must be the first
 *	function called by any user program that uses the thread package.  
 */

void MyInitThreads ()
{
	int i;

	if (MyInitThreadsCalled) {                /* run only once */
		Printf ("InitThreads: should be called only once\n");
		Exit ();
	}

	for (i = 0; i < MAXTHREADS; i++) {	/* initialize thread table */
		thread[i].valid = 0;
		threadQueue[i].valid = 0;
		threadQueue[i].prev = -1;
		threadQueue[i].next = -1;
	}

	thread[0].valid = 1;			/* initialize thread 0 */

	//Printf("Head is: %d  Tail is: %d \n", head, tail);
	//addHead(0);		/* add thread 0 to head of queue */
	//Printf("Inisde MyInitThreads. Performed addHead(0). ");
	//Printf("Head is now: %d  Tail is now: %d \n", head, tail);

	if (setjmp (thread[0].env) == 0) {	/* save context of thread 0 */

		memcpy( thread[0].envCopy, thread[0].env, sizeof(thread[0].env) );

		stackPartition( MAXTHREADS );
	}
	else {
		(*thread[0].f) (thread[0].p);
		MyExitThread();
	}

	MyInitThreadsCalled = 1;
}




/*	MySpawnThread (func, param) spawns a new thread to execute
 *	func (param), where func is a function with no return value and
 *	param is an integer parameter.  The new thread does not begin
 *	executing until another thread yields to it.  
 */

int MySpawnThread (func, param)
	void (*func)();		/* function to be executed */
	int param;		/* integer parameter */
{
	if (! MyInitThreadsCalled) {
		Printf ("SpawnThread: Must call InitThreads first\n");
		Exit ();
	}


	int indexToCheck = ( lastSpawned + 1 ) % MAXTHREADS;
	int indexAvailable = 0;
	int i = 0;

	while( i < MAXTHREADS ) {
		if( thread[indexToCheck].valid == 1 ) {
			indexToCheck = ( indexToCheck + 1 ) % MAXTHREADS;
			i++;
		}
		else {
			indexAvailable = 1;
			break;
		}
	}

	if( indexAvailable == 0 )
		return (-1);

	lastSpawned = indexToCheck;

	memcpy(thread[lastSpawned].env, thread[lastSpawned].envCopy, sizeof(thread[lastSpawned].envCopy));
	thread[lastSpawned].valid = 1;
	thread[lastSpawned].f = func;
	thread[lastSpawned].p = param;


	addTail(lastSpawned);

	//Printf("Spawned new thread at index %d\n", lastSpawned);
	//Printf("Head is now: %d  Tail is now: %d \n", head, tail);
	//Printf("numOfThreads: %d\n", numOfThreads);
	//Printf("Running thread is: %d\n", runningThread);

	return lastSpawned;

}

/*	MyYieldThread (t) causes the running thread, call it T, to yield to
 *	thread t.  Returns the ID of the thread that yielded to the calling
 *	thread T, or -1 if t is an invalid ID.  Example: given two threads
 *	with IDs 1 and 2, if thread 1 calls MyYieldThread (2), then thread 2
 *	will resume, and if thread 2 then calls MyYieldThread (1), thread 1
 *	will resume by returning from its call to MyYieldThread (2), which
 *	will return the value 2.
 */

int MyYieldThread (t)
	int t;				/* thread being yielded to */
{
	if (! MyInitThreadsCalled) {
		Printf ("YieldThread: Must call InitThreads first\n");
		Exit ();
	}

	if (t < 0 || t >= MAXTHREADS) {
		Printf ("YieldThread: %d is not a valid thread ID\n", t);
		return (-1);
	}
	if (! thread[t].valid) {
		Printf ("YieldThread: Thread %d does not exist\n", t);
		return (-1);
	}


	if( t == runningThread )
		return t;

	yieldingThread = runningThread;

	removeFromQueue(t);
	//Printf("Removing %d from queue, head: %d, tail: %d.\n", t, head, tail);
	if(threadExit == 0)
		addTail(runningThread);
	else
		threadExit = 0;
	//Printf("Adding runningThread %d to tail, head: %d, tail: %d.\n", runningThread, head, tail);

	if( setjmp(thread[runningThread].env) == 0 ) {
		runningThread = t;
		//Printf("Running thread is now %d.\n", runningThread);
		longjmp(thread[t].env, 1);
	}

	return yieldingThread;

}

/*	MyGetThread () returns ID of currently running thread. 
 */

int MyGetThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("GetThread: Must call InitThreads first\n");
		Exit ();
	}

	return runningThread;
}

/*	MySchedThread () causes the running thread to simply give up the
 *	CPU and allow another thread to be scheduled.  Selecting which
 *	thread to run is determined here. Note that the same thread may
 * 	be chosen (as will be the case if there are no other threads).  
 */

void MySchedThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("SchedThread: Must call InitThreads first\n");
		Exit ();
	}

	if( head == -1 ) {
		if( threadExit )	/* If last thread exiting */
			Exit();
		else				/* If last thread, but not exiting */
			MyYieldThread(runningThread);
	}
	else		/* More than one thread, yield to head of threadQueue */
		MyYieldThread(head);
}

/*	MyExitThread () causes the currently running thread to exit.  
 */

void MyExitThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("ExitThread: Must call InitThreads first\n");
		Exit ();
	}

	thread[runningThread].valid = 0;
	threadExit = 1;
	MySchedThread();
}
