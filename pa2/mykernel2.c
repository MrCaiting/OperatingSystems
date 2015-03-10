/* mykernel2.c: your portion of the kernel
 *
 *	Below are procedures that are called by other parts of the kernel. 
 *	Your ability to modify the kernel is via these procedures.  You may
 *	modify the bodies of these procedures any way you wish (however,
 *	you cannot change the interfaces).  
 * 
 */

#include "aux.h"
#include "sys.h"
#include "mykernel2.h"

#define TIMERINTERVAL 10	/* in ticks (tick = 10 msec) */

/*	A sample process table.  You may change this any way you wish. 
 */

static struct {
	int valid;		/* is this entry valid: 1 = yes, 0 = no */
	int pid;		/* process id (as provided by kernel) */
	int nextProc;
	int prevProc;
	int requestFlag;
	int utilization;
	int stride;
	int passValue;
} proctab[MAXPROCS];

static int currProc;
static int lastRunProc;
static int firstProc;
static int headOfQueue;
static int numOfProcs;
static int numProcsWithUtil;
static int availableCPU;
static int lowestPassValue;
static int lowestPValIndex;
static int L;
static int firstPass;


/*	InitSched () is called when kernel starts up.  First, set the
 *	scheduling policy (see sys.h). Make sure you follow the rules
 *	below on where and how to set it.  Next, initialize all your data
 *	structures (such as the process table).  Finally, set the timer
 *	to interrupt after a specified number of ticks.  
 */

void InitSched ()
{
	int i;

	/* First, set the scheduling policy. You should only set it
	 * from within this conditional statement.  While you are working
	 * on this assignment, GetSchedPolicy will return NOSCHEDPOLICY,
	 * and so the condition will be true and you may set the scheduling
	 * policy to whatever you choose (i.e., you may replace ARBITRARY). 
	 * After the assignment is over, during the testing phase, we will
	 * have GetSchedPolicy return the policy we wish to test, and so
	 * the condition will be false and SetSchedPolicy will not be
	 * called, thus leaving the policy to whatever we chose to test.  
	 */
	if (GetSchedPolicy () == NOSCHEDPOLICY) {	/* leave as is */
		SetSchedPolicy (PROPORTIONAL);		/* set policy here */
	}

	currProc = 0;
	lastRunProc = 0;
	firstProc = 0;
	headOfQueue = 0;
	numOfProcs = 0;
	numProcsWithUtil = 0;
	availableCPU = 1;
	lowestPassValue = 0;
	L = 100000;
	firstPass = 0;
		
	/* Initialize all your data structures here */
	for (i = 0; i < MAXPROCS; i++) {
		proctab[i].valid = 0;
		proctab[i].prevProc = -1;
		proctab[i].nextProc = -1;
		proctab[i].utilization = -1;
		proctab[i].stride = 0;
		proctab[i].passValue = 0;
		proctab[i].requestFlag = 0;
	}

	/* Set the timer last */
	SetTimer (TIMERINTERVAL);
}


/*	StartingProc (pid) is called by the kernel when the process
 *	identified by pid is starting.  This allows you to record the
 *	arrival of a new process in the process table, and allocate
 *	any resources (if necessary).  Returns 1 if successful, 0 otherwise. 
 */

int StartingProc (pid)
	int pid;
{
	int i;

	for (i = 0; i < MAXPROCS; i++) {
		if (! proctab[i].valid) {
			proctab[i].valid = 1;
			proctab[i].pid = pid;

			if(firstProc != 0) {
				proctab[i].prevProc = lastRunProc;
				proctab[lastRunProc].nextProc = i;
			}
			else {
				firstProc = 1;
				headOfQueue = 0;
			}

			lastRunProc = i;

			if(GetSchedPolicy () == LIFO) {
				currProc = i;
				DoSched();
			}

			//Printf("starting %d proc\n", i);
			numOfProcs++;
			return (1);
		}
	}

	Printf ("Error in StartingProc: no free table entries\n");
	return (0);
}
			

/*	EndingProc (pid) is called by the kernel when the process
 *	identified by pid is ending.  This allows you to update the
 *	process table accordingly, and deallocate any resources (if
 *	necessary). Returns 1 if successful, 0 otherwise.  
 */


int EndingProc (pid)
	int pid;
{
	int i;

	for (i = 0; i < MAXPROCS; i++) {
		if (proctab[i].valid && proctab[i].pid == pid) {
			proctab[i].valid = 0;

			//Printf("\n Proc %d ending. \n", i);

			numOfProcs--;
			if(proctab[i].requestFlag == 1)
				numProcsWithUtil--;

			if(GetSchedPolicy () == FIFO)
				currProc = proctab[i].nextProc;
			else if(GetSchedPolicy () == LIFO) {
				if(proctab[i].prevProc != i)
					currProc = proctab[i].prevProc;
				else
					currProc = -1;
				lastRunProc = currProc;
			}
			else if(GetSchedPolicy () == ROUNDROBIN || GetSchedPolicy () == PROPORTIONAL) {
				if(i == headOfQueue)
					headOfQueue = proctab[i].nextProc;
				else {
					proctab[ proctab[i].prevProc ].nextProc =
						proctab[i].nextProc;
					currProc = proctab[i].prevProc;
				}
				if(proctab[i].nextProc != -1) {
					proctab[ proctab[i].nextProc ].prevProc =
						proctab[i].prevProc;
				}
			}

			if(i == lastRunProc)
				lastRunProc = proctab[i].prevProc;
			proctab[i].prevProc = -1;
			proctab[i].nextProc = -1;

			availableCPU += proctab[i].utilization;

			proctab[i].utilization = 0;
			proctab[i].requestFlag = 0;




			return (1);
		}
	}

	Printf ("Error in EndingProc: can't find process %d\n", pid);
	return (0);
}


/*	SchedProc () is called by kernel when it needs a decision for
 *	which process to run next.  It calls the kernel function
 *	GetSchedPolicy () which will return the current scheduling policy
 *	which was previously set via SetSchedPolicy (policy).  SchedProc ()
 *	should return a process id, or 0 if there are no processes to run. 
 */

int SchedProc ()
{
	int i;

	switch (GetSchedPolicy ()) {

	case ARBITRARY:

		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
				return (proctab[i].pid);
			}
		}
		break;

	case FIFO:

		/* Return pid only if there exist a next process */
		if(currProc >= 0)
			return proctab[currProc].pid;

		break;

	case LIFO:

		if(currProc >= 0)
			return proctab[currProc].pid;

		break;

	case ROUNDROBIN:

		if(currProc >= 0 && proctab[currProc].nextProc != -1)
			currProc = proctab[currProc].nextProc;
		else
			currProc = headOfQueue;

		//Printf("\n Proc %d running. prevProc is %d. nextProc is %d \n", 
			//currProc, proctab[currProc].prevProc, proctab[currProc].nextProc);

		if(currProc >= 0)
			return proctab[currProc].pid;

		break;

	case PROPORTIONAL:


		if(currProc >= 0 && proctab[currProc].nextProc != -1)
			currProc = proctab[currProc].nextProc;
		else
			currProc = headOfQueue;

		//Printf("\n Proc %d running. prevProc is %d. nextProc is %d \n", 
			//currProc, proctab[currProc].prevProc, proctab[currProc].nextProc);

		if(currProc >= 0) {
			// If this process does not have specific requested CPU rate,
			// give it fair share of available ticks
			if( proctab[currProc].requestFlag == 0 && availableCPU > 0) {
				int ratioTime = (numOfProcs - numProcsWithUtil) / availableCPU;
				SetTimer (TIMERINTERVAL * ratioTime /100);
			}
			else if ( proctab[currProc].requestFlag == 1 )
				SetTimer (TIMERINTERVAL * proctab[currProc].utilization / 100);
			return proctab[currProc].pid;
		}

		break;

	}
	return (0);
}


/*	HandleTimerIntr () is called by the kernel whenever a timer
 *	interrupt occurs.  
 */

void HandleTimerIntr ()
{
	SetTimer (TIMERINTERVAL);


	//if(proctab[ proctab[currProc].nextProc ].



	switch (GetSchedPolicy ()) {	/* is policy preemptive? */

	case ROUNDROBIN:		/* ROUNDROBIN is preemptive */
		DoSched ();
		break;

	case PROPORTIONAL:		/* PROPORTIONAL is preemptive */

		DoSched ();		/* make scheduling decision */
		break;

	default:			/* if non-preemptive, do nothing */
		break;
	}
}

/*	MyRequestCPUrate (pid, m, n) is called by the kernel whenever a process
 *	identified by pid calls RequestCPUrate (m, n). This is a request for
 *	a fraction m/n of CPU time, effectively running on a CPU that is m/n
 *	of the rate of the actual CPU speed.  m of every n quantums should
 *	be allocated to the calling process.  Both m and n must be greater
 *	than zero, and m must be less than or equal to n.  MyRequestCPUrate
 *	should return 0 if successful, i.e., if such a request can be
 *	satisfied, otherwise it should return -1, i.e., error (including if
 *	m < 1, or n < 1, or m > n). If MyRequestCPUrate fails, it should
 *	have no effect on scheduling of this or any other process, i.e., as
 *	if it were never called.  
 */

int MyRequestCPUrate (pid, m, n)
	int pid;
	int m;
	int n;
{
	//  No more resources available, can't satisfy request
	if (availableCPU <= 0)
		return (-1);

	if ( availableCPU >= (100*m/n) ) {
		for ( int i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid && proctab[i].pid == pid) {
				proctab[i].utilization = 100*m / n;
				proctab[i].stride = L / proctab[i].utilization;
				proctab[i].requestFlag = 1;
				numProcsWithUtil++;
				availableCPU -= 100*m/n;
				if(firstPass==0) {
					firstPass = 1;
					lowestPassValue = proctab[i].stride;
				}
				return (0);
			}
		}
	}

	return (-1);
}
