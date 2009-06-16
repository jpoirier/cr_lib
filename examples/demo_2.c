/**@file demo_2.c
 *
 * @author Original author  : Joseph D Poirier
 * @date   Creation date    : February 2006
 *
 *
 * @version X.X.X
 *
 * VERSION    DATE/AUTHOR              COMMENT
 *
 *
 *
 *
 * @cmd<<%PRJ% %PFE%>>
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>

#include "cr.h"

static void Thread_A(void);
static void Thread_B(void);
static void Thread_C(void);
static void Thread_D(void);
static void cleanup(void);
static void signal_handler(int signal);

void Thread_A(void)
{
    int32_t     i;
    int32_t     cnt = 0;

    // A. the required init call
    CR_THREAD_INIT();

    for(;;)
    {
        i   = 100000;
        cnt = 0;

        while(i--)
            cnt += 1;

        printf("\n- Thread_A setting cr_g_activate_id to Thread_B\n");
        printf("Thread_A yielding to CR_IDLE_THREAD_ID\n");

        assert(Thread_B && "Thread_A: Thread_B pointer is invalid!");

        // we're yielding to the idle thread but we don't want it - the idle thread -
        // to spin forever so we simulate an ISR setting the activate thread
        // variable which causes the idle thread to yield to the thread that was
        // set via cr_g_activate_id
        // fetch the thread's id and set the flag
        cr_g_activate_id = cr_get_id(Thread_B);

        assert((cr_g_activate_id != CR_INVALID_ID) && "Thread_A: cr_get_id returned CR_INVALID_ID!");

        // B. explicitly yield - to the idle coroutine in this case
        CR_YIELD(cr_idle);
    }
}

void Thread_B(void)
{
    int32_t     i;
    uint32_t    cnt = 0;

    // A. the required init call
    CR_THREAD_INIT();

    for(;;)
    {
        i = 100000;

        while(i--)
            cnt += 1;

        printf("- Thread_B setting cr_g_activate_id to Thread_C\n");
        printf("Thread_B yielding to CR_IDLE_THREAD_ID\n");

        assert(Thread_C && "Thread_B: Thread_C pointer is invalid!");

        // fetch the thread's id and set the flag
        cr_g_activate_id = cr_get_id(Thread_C);

        assert((cr_g_activate_id != CR_INVALID_ID) && "Thread_B: cr_get_id returned CR_INVALID_ID!");

        // B. explicitly yield - to the idle coroutine in this case
        CR_YIELD(cr_idle);
    }
}

void Thread_C(void)
{
    int32_t     i;
    uint32_t    cnt = 0;

    // A. the required init call
    CR_THREAD_INIT();

    for(;;)
    {
        i = 100000;

        while(i--)
            cnt += 1;

        printf("- Thread_C setting cr_g_activate_id to Thread_D\n");
        printf("Thread_C yielding to CR_IDLE_THREAD_ID\n");

        assert(Thread_D && "Thread_C: Thread_D pointer is invalid!");

        // fetch the thread's id and set the flag
        cr_g_activate_id = cr_get_id(Thread_D);

        assert((cr_g_activate_id != CR_INVALID_ID) && "Thread_C: cr_get_id returned CR_INVALID_ID!");

        // B. explicitly yield - to the idle coroutine in this case
        CR_YIELD(cr_idle);
    }
}

void Thread_D(void)
{
    int32_t     i;
    uint32_t    cnt = 0;

    // A. the required init call
    CR_THREAD_INIT();

    for(;;)
    {
        i = 100000;

        while(i--)
            cnt += 1;

        printf("- Thread_D setting cr_g_activate_id to Thread_A\n");
        printf("Thread_D yielding to CR_IDLE_THREAD_ID\n");

        assert(Thread_A && "Thread_D: Thread_A pointer is invalid!");

        // fetch the thread's id and set the flag
        cr_g_activate_id = cr_get_id(Thread_A);

        assert((cr_g_activate_id != CR_INVALID_ID) && "Thread_D: cr_get_id returned CR_INVALID_ID!");

        // B. explicitly yield - to the idle coroutine in this case
        CR_YIELD(cr_idle);
    }
}

void signal_handler(int signal)
{
    switch(signal)
    {
        case SIGFPE:
            perror("A floating point exception occured.\n");
            break;
        case SIGILL:
            perror("An illegal instruction occured.\n");
            break;
        case SIGINT:
            // user hit CTRL-C
            break;
        case SIGSEGV:
            perror("A segmentation violation occured.\n");
            break;
        default:
            perror("An unknown signal was caught.\n");
            break;
    }

    // be sure and pass EXIT_SUCCESS for the registered atexit function handler
    exit(EXIT_SUCCESS);
}

void cleanup(void)
{
    cr_reset();

    printf("    Exiting...\n" );
}

#define CONTEXT_ARRAY_CNT 10

int main(int argc, char* argv[])
{
    // we explicitly create the array of CR_CONTEXT structures
    CR_CONTEXT context_array[CONTEXT_ARRAY_CNT];

    // register a cleanup function
    atexit(cleanup);

    // some signal handlers
    if(signal(SIGFPE, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGFPE signal handler.\n");

    if(signal(SIGILL, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGILL signal handler.\n");

    if(signal(SIGINT, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGINT signal handler.\n");

    if(signal(SIGSEGV, signal_handler) == SIG_ERR)
        perror("An error occured while setting the SIGSEGV signal handler.\n");

    // 1. init the cr library
    cr_init(context_array, CONTEXT_ARRAY_CNT);

    // 2. register the threads
    cr_register_thread(Thread_A);
    cr_register_thread(Thread_B);
    cr_register_thread(Thread_C);
    cr_register_thread(Thread_D);

    // 3. bootstrap the system
    CR_START(Thread_A);

    return EXIT_SUCCESS;
}
