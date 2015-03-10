/* mykernel.c: your portion of the kernel
 *
 *	Below are procedures that are called by other parts of the kernel. 
 *	Your ability to modify the kernel is via these procedures.  You may
 *	modify the bodies of these procedures any way you wish (however,
 *	you cannot change the interfaces).  
 */

#include "aux.h"
#include "sys.h"
#include "mykernel3.h"

#define FALSE 0
#define TRUE 1

/*	A sample semaphore table.  You may change this any way you wish. 
 */

static struct {
	int valid;	/* Is this a valid entry (was sem allocated)? */
	int value;	/* value of semaphore */
	int head;
	int tail;
	struct {
		int pid;
		int next;
	} blockedProcs[MAXPROCS];
} semtab[MAXSEMS];


/*	InitSem () is called when kernel starts up.  Initialize data
 *	structures (such as the semaphore table) and call any initialization
 *	procedures here. 
 */

void InitSem ()
{
	int s;

	/* modify or add code any way you wish */

	for (s = 0; s < MAXSEMS; s++) {		/* mark all sems free */
		semtab[s].valid = FALSE;
	}
}

/*	MySeminit (p, v) is called by the kernel whenever the system
 *	call Seminit (v) is called.  The kernel passes the initial
 * 	value v, along with the process ID p of the process that called
 *	Seminit.  MySeminit should allocate a semaphore (find a free entry
 *	in semtab and allocate), initialize that semaphore's value to v,
 *	and then return the ID (i.e., index of the allocated entry).  
 */

int MySeminit (p, v)
	int p, v;
{
	int s;

	/* modify or add code any way you wish */

	for (s = 0; s < MAXSEMS; s++) {
		if (semtab[s].valid == FALSE) {
			break;
		}
	}
	if (s == MAXSEMS) {
		Printf ("No free semaphores\n");
		return (-1);
	}

	semtab[s].valid = TRUE;
	semtab[s].value = v;
	semtab[s].head = -1;

	for(int i=0; i<MAXPROCS; i++) {
		semtab[s].blockedProcs[i].pid = -1;
		semtab[s].blockedProcs[i].next = -1;
	}


	return (s);
}

/*	MyWait (p, s) is called by the kernel whenever the system call
 *	Wait (s) is called. 
 */

void MyWait (p, s)
	int p, s;
{
	/* modify or add code any way you wish */

	semtab[s].value--;
	//Printf("semtab[%d].value is now %d\n", s, semtab[s].value);

	if( semtab[s].value < 0 ) {
		for( int i=0; i<MAXPROCS; i++) {
			if(semtab[s].blockedProcs[i].pid == -1) {
				semtab[s].blockedProcs[i].pid = p;
				if(semtab[s].head == -1)
					semtab[s].head = i;
				else
					semtab[s].blockedProcs[semtab[s].tail].next = i;
				semtab[s].tail = i;
				break;
			}
		}
		Block(p);
	}
}

/*	MySignal (p, s) is called by the kernel whenever the system call
 *	Signal (s) is called.  
 */

void MySignal (p, s)
	int p, s;
{
	/* modify or add code any way you wish */

	semtab[s].value++;
	//Printf("semtab[%d].value is now %d\n", s, semtab[s].value);

	if(semtab[s].head != -1) {
		int toUnblock = semtab[s].head;
		int toUnblockPID = semtab[s].blockedProcs[toUnblock].pid;

		if(semtab[s].blockedProcs[toUnblock].next == -1 ) {
			semtab[s].blockedProcs[toUnblock].pid = -1;
			semtab[s].head = -1;
			semtab[s].tail = -1;
		}
		else {
			semtab[s].head = semtab[s].blockedProcs[toUnblock].next;
			semtab[s].blockedProcs[toUnblock].next = -1;
			semtab[s].blockedProcs[toUnblock].pid = -1;
		}

		Unblock(toUnblockPID);
	}

}

