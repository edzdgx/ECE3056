/*
 * student.c
 * Multithreaded OS Simulation for ECE 3056
 *
 * This file contains the CPU scheduler for the simulation.
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "os-sim.h"

/** Function prototypes **/
extern void idle(unsigned int cpu_id);
extern void preempt(unsigned int cpu_id);
extern void yield(unsigned int cpu_id);
extern void terminate(unsigned int cpu_id);
extern void wake_up(pcb_t *process);

static void add_to_queue(pcb_t *pcb);
static pcb_t *remove_from_queue();

/*
 * current[] is an array of pointers to the currently running processes.
 * There is one array element corresponding to each CPU in the simulation.
 *
 * current[] should be updated by schedule() each time a process is scheduled
 * on a CPU.  Since the current[] array is accessed by multiple threads, you
 * will need to use a mutex to protect it.  current_mutex has been provided
 * for your use.
 */
static pcb_t **current;
static pthread_mutex_t current_mutex;

// linked list args
static pcb_t *head;
static pthread_mutex_t mutex;
static pthread_cond_t isIdle;
static char alg;
static int countCPU;
static int time_slice;





/*
 * schedule() is your CPU scheduler.  It should perform the following tasks:
 *
 *   1. Select and remove a runnable process from your ready queue which
 *	you will have to implement with a linked list or something of the sort.
 *
 *   2. Set the process state to RUNNING
 *
 *   3. Call context_switch(), to tell the simulator which process to execute
 *      next on the CPU.  If no process is runnable, call context_switch()
 *      with a pointer to NULL to select the idle process.
 *	The current array (see above) is how you access the currently running process indexed by the cpu id.
 *	See above for full description.
 *	context_switch() is prototyped in os-sim.h. Look there for more information
 *	about it and its parameters.
 */
static void add_to_queue(pcb_t *pcb) {
    pcb_t *current;
    // lock
    pthread_mutex_lock(&mutex);
    current = head;
    if (current == NULL) {
        head = pcb;
        pcb->next = NULL;
    } else {
        while (current->next != NULL) {
            current = current -> next;
        }
        // update last
        current->next = pcb;
        pcb->next = NULL;
    }
    // update pcb
    pthread_cond_broadcast(&isIdle);
    // unlock
    pthread_mutex_unlock(&mutex);
}

static pcb_t* remove_from_queue() {
    pcb_t *current;
    pcb_t *previous;
    pcb_t *removed = NULL;
    int t = 0;

    if (alg == 'l') {
        // lock
        pthread_mutex_lock(&mutex);
        if (head == NULL) {
            removed = NULL;
        } else {
            current = head;
            while (current != NULL) {
                if (current->time_remaining > t) {
                    t = current->time_remaining;
                }
                current = current->next;
            }
            current = head;
            previous = head;
            while (current != NULL) {
                if (current->time_remaining == t) {
                    removed = current;
                    if (current != head) {
                        previous->next = current->next;
                    } else {
                        head = current->next;
                    }
                    break;
                }
                // pushing
                previous = current;
                current = current->next;
            }
        }
        // unlock
        pthread_mutex_unlock(&mutex);
        return removed;
    } else {
        // lock
        pthread_mutex_lock(&mutex);
        removed = head;
        if (removed != NULL) {
            head = removed -> next;
        }
        // imlock
        pthread_mutex_unlock(&mutex);
        return removed;
    }
}

static void schedule(unsigned int cpu_id)
{
    /* FIX ME */
    pcb_t *pcb = remove_from_queue();

    if (pcb != NULL) {
        pcb->state = PROCESS_RUNNING;
    }
    // LOCK
    pthread_mutex_lock(&current_mutex);
    current[cpu_id] = pcb;
    // UNLOCK
    pthread_mutex_unlock(&current_mutex);
    context_switch(cpu_id, pcb, time_slice);

}


/*
 * idle() is your idle process.  It is called by the simulator when the idle
 * process is scheduled.
 *
 * This function should block until a process is added to your ready queue.
 * It should then call schedule() to select the process to run on the CPU.
 */
extern void idle(unsigned int cpu_id)
{
    /* FIX ME */
    // LOCK
    pthread_mutex_lock(&mutex);


    /*
     * REMOVE THE LINE BELOW AFTER IMPLEMENTING IDLE()
     *
     * idle() must block when the ready queue is empty, or else the CPU threads
     * will spin in a loop.  Until a ready queue is implemented, we'll put the
     * thread to sleep to keep it from consuming 100% of the CPU time.  Once
     * you implement a proper idle() function using a condition variable,
     * remove the call to mt_safe_usleep() below.
     */
    while (head == NULL) {
        // WAIT
        pthread_cond_wait(&isIdle, &mutex);
    }
    // UNLOCK
    pthread_mutex_unlock(&mutex);
    schedule(cpu_id);
}


/*
 * preempt() is the handler called by the simulator when a process is
 * preempted due to its timeslice expiring.
 *
 * This function should place the currently running process back in the
 * ready queue, and call schedule() to select a new runnable process.
 */
extern void preempt(unsigned int cpu_id)
{
    /* FIX ME */
    pcb_t *pcb;

    // LOCK
    pthread_mutex_lock(&current_mutex);
    pcb = current[cpu_id];
    pcb->state = PROCESS_READY;

    // UNLOCK
    pthread_mutex_unlock(&current_mutex);
    add_to_queue(pcb);
    schedule(cpu_id);
}


/*
 * yield() is the handler called by the simulator when a process yields the
 * CPU to perform an I/O request.
 *
 * It should mark the process as WAITING, then call schedule() to select
 * a new process for the CPU.
 */
extern void yield(unsigned int cpu_id)
{
    /* FIX ME */
    pcb_t *pcb;

    // LOCK
    pthread_mutex_lock(&current_mutex);
    pcb = current[cpu_id];
    pcb->state = PROCESS_WAITING;

    // UNLOCK
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}


/*
 * terminate() is the handler called by the simulator when a process completes.
 * It should mark the process as terminated, then call schedule() to select
 * a new process for the CPU.
 */
extern void terminate(unsigned int cpu_id)
{
    /* FIX ME */
    pcb_t *pcb;

    // LOCK
    pthread_mutex_lock(&current_mutex);
    pcb = current[cpu_id];
    pcb->state = PROCESS_TERMINATED;

    // UNLOCK
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}


/*
 * wake_up() is the handler called by the simulator when a process's I/O
 * request completes.  It should perform the following tasks:
 *
 *   1. Mark the process as READY, and insert it into the ready queue.
 *
 *   2. If the scheduling algorithm is LRTF, wake_up() may need
 *      to preempt the CPU with lower remaining time left to allow it to
 *      execute the process which just woke up with higher reimaing time.
 * 	However, if any CPU is currently running idle,
* 	or all of the CPUs are running processes
 *      with a higher remaining time left than the one which just woke up, wake_up()
 *      should not preempt any CPUs.
 *	To preempt a process, use force_preempt(). Look in os-sim.h for
 * 	its prototype and the parameters it takes in.
 */
extern void wake_up(pcb_t *process)
{
    /* FIX ME */
    process->state = PROCESS_READY;
    add_to_queue(process);
    if (alg == 'l') {
        // LOCK
        pthread_mutex_lock(&current_mutex);
        int low = 10;
        int id = -1;
        for (int i = 0; i < countCPU; i++) {
            if (current[i] == NULL) {
                id = -1;
                break;
            }
            if (current[i]->time_remaining < low) {
                low = current[i]->time_remaining;
                id = i;
            }
        }
        // UNLOCK
        pthread_mutex_unlock(&current_mutex);
        if (process->time_remaining > low && id != -1) {
            force_preempt(id);
        }
    }
}


/*
 * main() simply parses command line arguments, then calls start_simulator().
 * You will need to modify it to support the -l and -r command-line parameters.
 */
int main(int argc, char *argv[])
{

    /* Parse command-line arguments */
    if (argc < 1 || argc > 4)
    {
        fprintf(stderr, "ECE 3056 OS Sim -- Multithreaded OS Simulator\n"
            "Usage: ./os-sim <# CPUs> [ -l | -r <time slice> ]\n"
            "    Default : FIFO Scheduler\n"
            "         -l : Longest Remaining Time First Scheduler\n"
            "         -r : Round-Robin Scheduler\n\n");
        return -1;
    }
    unsigned int countCPU;

    countCPU = strtoul(argv[1], NULL, 0);
    time_slice = -1;

    /* FIX ME - Add support for -l and -r parameters*/
    for (int i = 0; i < argc; i++) {
        if ((strcmp(argv[i], "-l")) == 0) {
            alg = 'l';
        } else if ((strcmp(argv[i], "-r")) == 0) {
            alg = 'r';
            time_slice = atoi(argv[i + 1]);
        } else {
            alg = 'f';
        }
    }

    head = NULL;

    /* Allocate the current[] array and its mutex */
    current = malloc(sizeof(pcb_t*) * countCPU);
    assert(current != NULL);
    pthread_mutex_init(&current_mutex, NULL);
    pthread_cond_init(&isIdle, NULL);

    pthread_mutex_init(&mutex, NULL);

    /* Start the simulator in the library */
    start_simulator(countCPU);

    return 0;
}


