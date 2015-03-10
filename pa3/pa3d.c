/* Programming Assignment 3: Exercise D
 *
 * Now that you have a working implementation of semaphores, you can
 * implement a more sophisticated synchronization scheme for the car
 * simulation.
 *
 * Study the program below.  Car 1 begins driving over the road, entering
 * from the East at 40 mph.  After 900 seconds, both Car 3 and Car 4 try to
 * enter the road.  Car 1 may or may not have exited by this time (it should
 * exit after 900 seconds, but recall that the times in the simulation are
 * approximate).  If Car 1 has not exited and Car 4 enters, they will crash
 * (why?).  If Car 1 has exited, Car 3 and Car 4 will be able to enter the
 * road, but since they enter from opposite directions, they will eventually
 * crash.  Finally, after 1200 seconds, Car 2 enters the road from the West,
 * and traveling twice as fast as Car 4.  If Car 3 was not coming from the
 * opposite direction, Car 2 would eventually reach Car 4 from the back and
 * crash.  (You may wish to experiment with reducing the initial delay of
 * Car 2, or increase the initial delay of Car 3, to cause Car 2 to crash
 * with Car 4 before Car 3 crashes with Car 4.)
 *
 *
 * Exercises
 *
 * 1. Modify the procedure driveRoad such that the following rules are obeyed:
 *
 *	A. Avoid all collisions.
 *
 *	B. Multiple cars should be allowed on the road, as long as they are
 *	traveling in the same direction.
 *
 *	C. If a car arrives and there are already other cars traveling in the
 *	SAME DIRECTION, the arriving car should be allowed enter as soon as it
 *	can. Two situations might prevent this car from entering immediately:
 *	(1) there is a car immediately in front of it (going in the same
 *	direction), and if it enters it will crash (which would break rule A);
 *	(2) one or more cars have arrived at the other end to travel in the
 *	opposite direction and are waiting for the current cars on the road
 *	to exit, which is covered by the next rule.
 *
 *	D. If a car arrives and there are already other cars traveling in the
 *	OPPOSITE DIRECTION, the arriving car must wait until all these other
 *	cars complete their course over the road and exit.  It should only wait
 *	for the cars already on the road to exit; no new cars traveling in the
 *	same direction as the existing ones should be allowed to enter.
 *
 *	E. This last rule implies that if there are multiple cars at each end
 *	waiting to enter the road, each side will take turns in allowing one
 *	car to enter and exit.  (However, if there are no cars waiting at one
 *	end, then as cars arrive at the other end, they should be allowed to
 *	enter the road immediately.)
 *	
 *	F. If the road is free (no cars), then any car attempting to enter
 *	should not be prevented from doing so.
 *
 *	G. All starvation must be avoided.  For example, any car that is
 *	waiting must eventually be allowed to proceed.
 *
 * This must be achieved ONLY by adding synchronization and making use of
 * shared memory (as described in Exercise C).  You should NOT modify the
 * delays or speeds to solve the problem.  In other words, the delays and
 * speeds are givens, and your goal is to enforce the above rules by making
 * use of only semaphores and shared memory.
 *
 * 2. Devise different tests (using different numbers of cars, speeds,
 * directions) to see whether your improved implementation of driveRoad
 * obeys the rules above.
 *
 * IMPLEMENTATION GUIDELINES
 * 
 * 1. Avoid busy waiting. In class one of the reasons given for using
 * semaphores was to avoid busy waiting in user code and limit it to
 * minimal use in certain parts of the kernel. This is because busy
 * waiting uses up CPU time, but a blocked process does not. You have
 * semaphores available to implement the driveRoad function, so you
 * should not use busy waiting anywhere.
 *
 * 2. Prevent race conditions. One reason for using semaphores is to
 * enforce mutual exclusion on critical sections to avoid race conditions.
 * You will be using shared memory in your driveRoad implementation.
 * Identify the places in your code where there may be race conditions
 * (the result of a computation on shared memory depends on the order
 * that processes execute).  Prevent these race conditions from occurring
 * by using semaphores.
 *
 * 3. Implement semaphores fully and robustly.  It is possible for your
 * driveRoad function to work with an incorrect implementation of
 * semaphores, because controlling cars does not exercise every use of
 * semaphores.  You will be penalized if your semaphores are not correctly
 * implemented, even if your driveRoad works.
 *
 * 4. Avoid starvation.  This is especially relevant when implementing the
 * Signal function.  If there are multiple processes that blocked on the
 * same semaphore, then a good policy is to unblock them in FIFO order.
 *
 * 5. Control cars with semaphores: Semaphores should be the basic
 * mechanism for enforcing the rules on driving cars. You should not
 * force cars to delay in other ways inside driveRoad such as by calling
 * the Delay function or changing the speed of a car. (You can leave in
 * the delay that is already there that represents the car's speed, just
 * don't add any additional delaying).  Also, you should not be making
 * decisions on what cars do using calculations based on car speed (since
 * there are a number of unpredictable factors that can affect the
 * actual cars' progress).
 *
 * GRADING INFORMATION
 *
 * 1. Semaphores: We will run a number of programs that test your
 * semaphores directly (without using cars at all). For example:
 * enforcing mututal exclusion, testing robustness of your list of
 * waiting processes, calling signal and wait many times to make sure
 * the semaphore state is consistent, etc.
 *
 * 2. Cars: We will run some tests of cars arriving in different ways,
 * to make sure that you correctly enforce all the rules for cars given
 * in the assignment.  We will use a correct semaphore implementation for
 * these tests so that even if your semaphores are not correct you could
 * still get full credit on the driving part of the grade.  Think about
 * how your driveRoad might handle different situations and write your
 * own tests with cars arriving in different ways to make sure you handle
 * all cases correctly.
 *
 *
 * WHAT TO TURN IN
 *
 * You must turn in two files: mykernel3.c and p3d.c.  mykernel3.c should
 * contain you implementation of semaphores, and p3d.c should contain
 * your modified version of InitRoad and driveRoad (Main will be ignored).
 * Note that you may set up your static shared memory struct and other
 * functions as you wish. They should be accessed via InitRoad and driveRoad,
 * as those are the functions that we will call to test your code.
 *
 * Your programs will be tested with various Main programs that will exercise
 * your semaphore implementation, AND different numbers of cars, directions,
 * and speeds, to exercise your driveRoad function.  Our Main programs will
 * first call InitRoad before calling driveRoad.  Make sure you do as much
 * rigorous testing yourself to be sure your implementations are robust.
 */

#include <stdio.h>
#include "aux.h"
#include "umix.h"
#include "sys.h"
#include "cartest0.c"
#include "cartest1.c"
#include "cartest2.c"
#include "cartest3.c"
#include "cartest4.c"
#include "cartest5.c"


void InitRoad ();
void driveRoad (int from, int mph);

struct {
	int first; // 0 if first process to enter driverRoad()
	int numOfCars;
	int currDir;  // 0 for west, 1 for east
	int currSlowest; //
	int carsOnRoad; // number of cars currently on the road
	int oppWaiting; // 0 if no cars waiting on opposite end, else 1
	int fromWest;
	int fromEast;
	int waitingWest;
	int waitingEast;
	int firstCellWest;
	int firstCellEast;
} shm;

int sem;
int sem1, sem2, sem3, sem4, sem5, sem6, sem7, sem8, sem9, sem10;
int semCell, semCellPrev;
int semDoorWest, semDoorEast;
int semCric, semPrint;

void Main (argc, argv)
     int argc;
     char** argv;
{

  switch (argv[1][0]) {
  case '0': cartest0 (); break;
  case '1': cartest1 (); break;
  case '2': cartest2 (); break;
  case '3': cartest3 (); break;
  case '4': cartest4 (); break;
  case '5': cartest5 (); break;
  }


  Exit ();

//	InitRoad ();

	/* The following code is specific to this particular simulation,
	 * e.g., number of cars, directions, and speeds.  You should
	 * experiment with different numbers of cars, directions, and
	 * speeds to test your modification of driveRoad.  When your
	 * solution is tested, we will use different Main procedures,
	 * which will first call InitRoad before any calls to driveRoad.
	 * So, you should do any initializations in InitRoad.
	 */


}

/* Our tests will call your versions of InitRoad and driveRoad, so your
 * solution to the car simulation should be limited to modifying the code
 * below.  This is in addition to your implementation of semaphores
 * contained in mykernel3.c.
 */

void InitRoad ()
{
	/* do any initializations here */
	Regshm ((char *) &shm, sizeof (shm));
	shm.first = 0;
	shm.numOfCars = 0;
	shm.carsOnRoad = 0;
	shm.currDir = -1;
	shm.oppWaiting = 0;
	shm.fromWest = 0;
	shm.fromEast = 0;
	shm.waitingWest = 0;
	shm.waitingEast = 0;
	shm.firstCellWest = 0;
	shm.firstCellEast = 0;
	
	// Initialize semaphores for driveRoad and for each block of the road
	sem = Seminit (1);
	sem1 = Seminit(1);
	sem2 = Seminit(1);
	sem3 = Seminit(1);
	sem4 = Seminit(1);
	sem5 = Seminit(1);
	sem6 = Seminit(1);
	sem7 = Seminit(1);
	sem8 = Seminit(1);
	sem9 = Seminit(1);
	sem10 = Seminit(1);
	semDoorWest = Seminit(0);
	semDoorEast = Seminit(0);
	semCric = Seminit(1);
	// Semaphore so the printing order and format is preserved
	semPrint = Seminit(1);
}

#define IPOS(FROM)	(((FROM) == WEST) ? 1 : NUMPOS)

void driveRoad (from, mph)
	int from, mph;
{
	shm.numOfCars++;

	int c;					/* car id c = process id */
	int p, np, i;				/* positions */

	c = Getpid ();				/* learn this car's id */

/*	if(from==WEST)
		Printf("\nCar %d arrived from WEST\n\n", c);
	else
		Printf("\nCar %d arrived from EAST\n\n", c);
*/


	/* Check the condition that there are cars on road coming from opposite
	   direction or if there are cars waiting on other end */
	/*** Added firstCellWest and firstCellEast so processes don't make a decision
	     to come in to cell yet if the first cell is blocked, even if no cars waiting 
	     on opposite end. Make decision when first cell is cleared and check if cars
	     are waiting on opposite end. *****/
	//Printf ("Calling Wait(%d)\n", semCric);
	Wait (semCric);
	if( from == WEST && (shm.fromEast > 0 || 
		(shm.fromWest > 0 && shm.waitingEast > 0) || shm.firstCellWest > 0 )) {
			shm.waitingWest++;
			//Printf ("Calling Signal(%d)\n", semCric);
			Signal (semCric);
			//Printf ("Calling Wait(%d)\n", semDoorWest);
			Wait (semDoorWest);
	}
	else if( from == EAST && (shm.fromWest > 0 || 
		(shm.fromEast > 0 && shm.waitingWest > 0) || shm.firstCellEast > 0 )) {
			shm.waitingEast++;
			//Printf ("Calling Signal(%d)\n", semCric);
			Signal (semCric);
			//Printf ("Calling Wait(%d)\n", semDoorWest);
			Wait (semDoorEast);
	}
	else
		Signal (semCric);


	/* Semaphore for block 1 or 10 of the road, depending on
	   which side the car is coming from */
	Wait(semCric);
	if( from == WEST ) {
		//Printf ("Calling Wait(%d)\n", sem1);
		Wait (sem1);
		shm.firstCellWest = 1;
		semCellPrev = sem1;
		shm.fromWest++;
	}
	else {
		//Printf ("Calling Wait(%d)\n", sem10);
		Wait (sem10);
		shm.firstCellEast = 1;
		semCellPrev = sem10;
		shm.fromEast++;
	}
	Signal(semCric);


	EnterRoad (from);

	Wait(semPrint);
	PrintRoad ();
	Signal(semPrint);

	Printf ("Car %d enters at %d at %d mph\n", c, IPOS(from), mph);

	for (i = 1; i < NUMPOS; i++) {

		if (from == WEST) {
			p = i;
			np = i + 1;
		} else {
			p = NUMPOS + 1 - i;
			np = p - 1;
		}

		if( i > 1 )
			semCellPrev = semCell;

		switch ( np ) {
			case 1:
				semCell = sem1;
				break;
			case 2:
				semCell = sem2;
				break;
			case 3:
				semCell = sem3;
				break;
			case 4:
				semCell = sem4;
				break;
			case 5:
				semCell = sem5;
				break;
			case 6:
				semCell = sem6;
				break;
			case 7:
				semCell = sem7;
				break;
			case 8:
				semCell = sem8;
				break;
			case 9:
				semCell = sem9;
				break;
			case 10:
				semCell = sem10;
				break;
			default:
				break;
		}

		//Printf ("Calling Wait(%d)\n", semCell);
		Wait( semCell );

		Delay (3600/mph);
		ProceedRoad ();

		Wait(semPrint);
		PrintRoad ();
		Printf ("Car %d moves from %d to %d\n", c, p, np);
		Signal(semPrint);

		//Printf ("Calling Signal(%d)\n", semCellPrev);
		Signal( semCellPrev );

		//Printf ("Calling Wait(%d)\n", semCric);
		Wait(semCric);
		if(i==1 && from==WEST){
			if(shm.waitingEast==0 && shm.waitingWest>0) {
				shm.waitingWest--;
				//Printf ("Calling Signal(%d)\n", semDoorWest);
				Signal(semDoorWest);
			}
			shm.firstCellWest = 0;
		}
		else if (i==1 && from==EAST) {
			if(shm.waitingWest==0 && shm.waitingEast>0) {
				shm.waitingEast--;
				//Printf ("Calling Signal(%d)\n", semDoorEast);
				Signal(semDoorEast);
			}
			shm.firstCellEast = 0;
		}
		else { }
		//Printf ("Calling Signal(%d)\n", semCric);
		Signal(semCric);

	}

	Delay (3600/mph);
	ProceedRoad ();

	Wait(semPrint);
	PrintRoad ();
	Signal(semPrint);

	Printf ("Car %d exits road\n", c);

	if( from == WEST ) {
		//Printf ("Calling Signal(%d)\n", sem10);
		Signal (sem10);
	}
	else {
		//Printf ("Calling Signal(%d)\n", sem1);
		Signal (sem1);
	}

	//Printf ("Calling Wait(%d)\n", semCric);
	Wait(semCric);
	if(from==WEST) {
		shm.fromWest--;
		if(shm.fromWest==0 && shm.waitingEast>0) {
			shm.waitingEast--;
			//Printf ("Calling Signal(%d)\n", semDoorEast);
			Signal(semDoorEast);
		}
	}
	else {
		shm.fromEast--;
		if(shm.fromEast==0 && shm.waitingWest>0) {
			shm.waitingWest--;
			//Printf ("Calling Signal(%d)\n", semDoorWest);
			Signal(semDoorWest);
		}
	}
	//Printf ("Calling Signal(%d)\n", semCric);
	Signal(semCric);

/*
	shm.numOfCars--;
	shm.carsOnRoad--;
	//if(shm.carsOnRoad==0) {
		Printf("\nCalling Signal(sem)\n");
		Signal (sem);
	//}
*/
}
