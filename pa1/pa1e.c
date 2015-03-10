/* Programming Assignment 1: Exercise E
 *
 * Study the program below.  This will be used for your next and final
 * exercise, so make sure you thoroughly understand why the execution
 * sequence of the processes is the way it is.
 *
 * Questions
 *
 * 1. Can you explain the order of what gets printed based on the code?
 *
 */

#include <stdio.h>
#include "aux.h"
#include "umix.h"

#define NUMPROCS 3

void handoff (int p);

void Main ()
{
	int i, p, c, r;

	for (i = 0, p = Getpid (); i < NUMPROCS; i++, p = c) {
		Printf ("%d about to fork\n", Getpid ());
    Printf ("value of p is %d\n", p);
    Printf ("value of c is %d\n", c);

		if ((c = Fork ()) == 0) {
			Printf ("%d starting\n", Getpid ());
      Printf ("value of p here is %d going into handoff\n", p);
			handoff (p);
			Printf ("%d exiting\n", Getpid ());
			Exit ();
		}

        Printf ("value of c is %d\n", c);

		Printf ("%d just forked %d\n\n", Getpid (), c);
	}

	Printf ("%d yielding to %d\n", Getpid (), c);
	r = Yield (c);
  Printf("value of r in parent is %d\n", r);
	Printf ("%d resumed by %d, yielding to %d\n", Getpid (), r, c);
	Yield (c);
	Printf ("%d exiting\n", Getpid ());
}

void handoff (p)
	int p;
{
	int r;

	Printf ("%d yielding to %d\n", Getpid (), p);
	r = Yield (p);
  Printf("value of r in handoff %d is %d\n", Getpid(), r);
	Printf ("%d is now resumed by %d, yielding to %d\n", Getpid (), r, p);
	Yield (p);
}
