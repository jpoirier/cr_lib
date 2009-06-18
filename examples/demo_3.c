/**@file demo_3.c
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

#include "cr.h"

static void Thread_A(void);
static void Thread_B(void);
static void worker(void);
static void signal_handler(int signal);

static void cleanup(void);

void Thread_A(void)
{
    // A. the required init call
    CR_THREAD_INIT();

    for(;;)
    {
        worker();
        assert(Thread_B && "Thread_A: Thread_B pointer is invalid!");

        // B. explicitly yield - to Thread_B
        CR_YIELD(Thread_B);
        printf("- Thread_A resuming at      location A...\n");

        worker();
        assert(Thread_B && "Thread_A: Thread_B pointer is invalid!");

        // B. explicitly yield - to Thread_B
        CR_YIELD(Thread_B);
        printf("- Thread_A resuming at      location B...\n");

        worker();
        assert(Thread_B && "Thread_A: Thread_B pointer is invalid!");

        // B. explicitly yield - to Thread_B
        CR_YIELD(Thread_B);
        printf("- Thread_A resuming at      location C...\n");

        worker();
        assert(Thread_B && "Thread_A: Thread_B pointer is invalid!");

        // B. explicitly yield - to Thread_B
        CR_YIELD(Thread_B);
        printf("- Thread_A resuming at      location D...\n");

        worker();
        assert(Thread_B && "Thread_A: Thread_B pointer is invalid!");

        // B. explicitly yield - to Thread_B
        CR_YIELD(Thread_B);
        printf("- Thread_A resuming at      location E...\n");

        worker();
        assert(Thread_B && "Thread_A: Thread_B pointer is invalid!");

        // B. explicitly yield - to Thread_B
        CR_YIELD(Thread_B);
        printf("- Thread_A resuming at      location F...\n");

        worker();
        assert(Thread_B && "Thread_A: Thread_B pointer is invalid!");

        // B. explicitly yield - to Thread_B
        CR_YIELD(Thread_B);
        printf("- Thread_A resuming at      location G...\n");
    }
}

void Thread_B(void)
{
    // A. the required init call
    CR_THREAD_INIT();

    for(;;)
    {
        worker();

        printf("- Thread_B yielding to Thread_A\n");

        // B. explicitly yield - to Thread_A
        CR_YIELD(Thread_A);
    }
}

void worker(void)
{
    int32_t     i   = 100000;
    uint32_t    cnt = 0;

    while(i--)
        cnt += 1;
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

// 9 user threads plus 1 the system's idle thread
#define CONTEXT_ARRAY_CNT (9 + 1)

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
    assert(((sizeof(context_array) / sizeof(CR_CONTEXT)) == CONTEXT_ARRAY_CNT)
                                        && "context_array size mismatch!\n");

    cr_init(context_array, CONTEXT_ARRAY_CNT);

    // 2. register the threads
    cr_register_thread(Thread_A);
    cr_register_thread(Thread_B);

    // 3. bootstrap the system
    CR_START(Thread_A);

    return EXIT_SUCCESS;
}
