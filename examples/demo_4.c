/**@file demo_4.c
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
 * Copyright (c) 2006   Joseph D Poirier
 *
 *
 *
 * @cmd<<%PRJ% %PFE%>>
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sched.h>
#include <limits.h>


#include "cr.h"

static void Thread_A(void);
static void Thread_B(void);
static void Thread_C(void);
static void Thread_D(void);
static void cleanup(void);
static void signal_handler(int signal);
static void* isr_thread(void* args);

//--------------- Some simulated ISR thread objects
struct sched_param          isr_thread_sched;
pthread_attr_t              isr_thread_attr;
int32_t                     isr_thread_run  = false;

#ifdef _MSC_VER
    pthread_t                   isr_thread_id   = { 0, 0 };
#else
    pthread_t                   isr_thread_id   = 0;
#endif

// Our simulated ISR thread
void* isr_thread( void* args )
{
    static cr_uint32_t     cnt = 0;

    while ( isr_thread_run )
    {
        cnt = 0;

        // we want to simulate an external event
        // wait long enough so that all 4 coroutines have time to complete
        while ( cnt < 100000 )
        {
            ++cnt;
        }

        printf( "----------- ISR Posting ----------\n" );

        // Set the thread activate flag, in this case it's Thread_A.
        // cr_g_activate_id is an exported object. cr_idle spins
        // on this object, provided cr_idle is explicitly called by
        // the user, waiting for it to change from CR_IDLE_THREAD_ID.
        // The changed-value is the ID of the coroutine to be called.
        assert( Thread_A && "isr_thread: Thread_A pointer is invalid!" );
        cr_g_activate_id = cr_get_id( Thread_A );

        assert( ( cr_g_activate_id != CR_INVALID_ID ) && "isr_thread: cr_get_id returned CR_INVALID_ID!" );
    }

    printf( "isr_thread exiting...\n" );

    return 0;
}

void Thread_A( void )
{
    // A. locals use the 'volatile' qualifier
    cr_int32_t      i;
    cr_uint32_t     cnt = 0;

    // B. the required init call
    CR_THREAD_INIT( );

    while ( true )
    {
        i = 1000;

        while( i-- )
        {
            cnt += 1;
        }

        printf( "- Thread_A yielding to Thread_B\n" );
        assert( Thread_B && "Thread_A: Thread_B pointer is invalid!") ;

        // C. coroutine yield - to Thread_B
        CR_YIELD( Thread_B );
    }
}

void Thread_B( void )
{
    // A. locals use the 'volatile' qualifier
    cr_int32_t      i;
    cr_uint32_t     cnt = 0;

    // B. the required init call
    CR_THREAD_INIT( );

    while ( true )
    {
        i = 1000;

        while ( i-- )
        {
            cnt += 1;
        }

        printf( "- Thread_B yielding to Thread_C\n" );
        assert( Thread_C && "Thread_B: Thread_C pointer is invalid!" );

        // C. explicitly yield - to Thread_C
        CR_YIELD( Thread_C );
    }
}

void Thread_C( void )
{
    // A. locals use the 'volatile' qualifier
    cr_int32_t      i;
    cr_uint32_t     cnt = 0;

    // B. the required init call
    CR_THREAD_INIT( );

    while ( true )
    {
        i = 1000;

        while ( i-- )
        {
            cnt += 1;
        }

        printf( "- Thread_C yielding to Thread_D\n" );
        assert( Thread_D && "Thread_C: Thread_D pointer is invalid!" );

        // C. explicitly yield - to Thread_D
        CR_YIELD( Thread_D );
    }
}

void Thread_D( void )
{
    // A. locals use the 'volatile' qualifier
    cr_int32_t      i;
    cr_uint32_t     cnt = 0;

    // B. the required init call
    CR_THREAD_INIT( );

    while ( true )
    {
        i = 1000;

        while ( i-- )
        {
            cnt += 1;
        }

        printf( "- Thread_D yielding to Thread_A\n" );
        assert( cr_idle && "Thread_D: cr_idle pointer is invalid!" );

        // C. explicitly yield - to cr_idle
        CR_YIELD( cr_idle );
    }
}

void signal_handler( int signal )
{
    switch( signal )
    {
        case SIGFPE:
            perror( "A floating point exception occured.\n" );
            break;
        case SIGILL:
            perror( "An illegal instruction occured.\n" );
            break;
        case SIGINT:
            // user hit CTRL-C
            break;
        case SIGSEGV:
            perror( "A segmentation violation occured.\n" );
            break;
        default:
            perror( "An unknown signal was caught.\n" );
            break;
    }

    // Pass EXIT_SUCCESS the registered atexit function handler gets called
    exit( EXIT_SUCCESS );
}

void cleanup( void )
{
    uint32_t volatile timing_barrier  = 0;

    // kindly kill our simulated ISR thread
    isr_thread_run = false;

    timing_barrier = 800000;

    // Code block to stop the compiler from messing with the code
    {
        while( timing_barrier-- )
            /* no code */ ;
    }
    
    cr_reset( );

//    pthread_exit( 0 );
    printf( "    Exiting...\n"  );
}

// 4 user threads plus 1 the system's idle thread
#define CONTEXT_ARRAY_CNT ( 4 + 1 )

int main( int argc, char* argv[] )
{
    uint32_t            tmp             = 0;
    uint32_t volatile   timing_barrier  = 0;

    // we explicitly create the array of CR_CONTEXT structures
    CR_CONTEXT context_array[CONTEXT_ARRAY_CNT];

    // register a cleanup function
    atexit( cleanup );

    // some signal handlers
    if ( signal( SIGFPE, signal_handler ) == SIG_ERR )
    {
        perror( "An error occured while setting the SIGFPE signal handler.\n" );
    }
    else if ( signal( SIGILL, signal_handler ) == SIG_ERR )
    {
        perror( "An error occured while setting the SIGILL signal handler.\n" );
    }
    else if ( signal( SIGINT, signal_handler ) == SIG_ERR )
    {
        perror( "An error occured while setting the SIGINT signal handler.\n" );
    }
    else if ( signal( SIGSEGV, signal_handler ) == SIG_ERR )
    {
        perror( "An error occured while setting the SIGSEGV signal handler.\n" );
    }

    // 1. init the cr library
    assert( ( ( sizeof( context_array ) / sizeof( CR_CONTEXT ) ) == CONTEXT_ARRAY_CNT )
                                        && "context_array size mismatch!\n" );

    cr_init( context_array, CONTEXT_ARRAY_CNT );

    // 2. register the threads
    cr_register_thread( Thread_A );
    cr_register_thread( Thread_B );
    cr_register_thread( Thread_C );
    cr_register_thread( Thread_D );

    //--------------------------------------------------------------------------
    //-------------------- Simulated ISR Thread Setup --------------------------
    pthread_attr_init( &isr_thread_attr );

    pthread_attr_setschedpolicy( &isr_thread_attr,
                                 SCHED_RR );

    pthread_attr_setstacksize( &isr_thread_attr,
                               PTHREAD_STACK_MIN );

// XXX: the scheduling priority could be set here

    pthread_attr_setschedparam( &isr_thread_attr,
                                &isr_thread_sched );

    isr_thread_run = true;

    tmp = pthread_create( &isr_thread_id,
                          &isr_thread_attr,
                          isr_thread,
                          0 );

    assert( ( tmp == 0 ) && "pthread_create failed!" );

    timing_barrier = 1000;

    // Code block to stop the compiler from messing with the code
    {
        while( timing_barrier-- )
            /* no code */ ;
    }
    //--------------------------------------------------------------------------

    // 3. bootstrap the system
    CR_START( cr_idle );

    return EXIT_SUCCESS;
}
