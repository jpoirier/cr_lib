/**@file demo_1.c
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
 *  LICENSE:
 *          see the license.txt file
 *
 *
 * start ------
 *            |
 *           \/
 *          thread_A --> Thread_B --> Thread_C --> Thread_D ---
 *             /\                                             |
 *             |                                              |
 *             -----------------------------------------------
 *
 *
 * @cmd<<%PRJ% %PFE%>>
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "cr.h"

static void Thread_A( void );
static void Thread_B( void );
static void Thread_C( void );
static void Thread_D( void );
static void cleanup( void );
static void signal_handler( int signal );

void Thread_A( void )
{
    // A. locals use the 'volatile' qualifier
    cr_int32_t      i;
    cr_uint32_t     cnt = 0;

    // B. the required init call
    CR_THREAD_INIT( );

    while ( true )
    {
        i = 100000;

        while ( i-- )
        {
            cnt += 1;
        }

        printf( "- Thread_A yielding to Thread_B\n" );

        assert( Thread_B && "Thread_A: Thread_B pointer is invalid!" );

        // C. explicitly yield - to Thread_B
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
        i = 100000;

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
        i = 100000;

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
        i = 100000;

        while ( i-- )
        {
            cnt += 1;
        }

        printf( "- Thread_D yielding to Thread_A\n" );

        assert( Thread_A && "Thread_D: Thread_A pointer is invalid! ");

        // C. explicitly yield - to Thread_A
        CR_YIELD( Thread_A );
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

    // be sure and pass EXIT_SUCCESS for the registered atexit function handler
    exit( EXIT_SUCCESS );
}

void cleanup( void )
{
    cr_reset( );

    printf( "    Exiting...\n" );
}

// 4 user threads plus 1 the system's idle thread
#define CONTEXT_ARRAY_CNT ( 4 + 1 )

int main( int argc, char* argv[] )
{
    // we explicitly create the array of CR_CONTEXT structures
    CR_CONTEXT context_array[ CONTEXT_ARRAY_CNT ];

    // register a cleanup function
    atexit( cleanup );

    // some signal handlers
    if ( signal( SIGFPE, signal_handler ) == SIG_ERR )
    {
        perror( "An error occured while setting the SIGFPE signal handler.\n" );
    }
    else if (signal( SIGILL, signal_handler ) == SIG_ERR )
    {
        perror("An error occured while setting the SIGILL signal handler.\n" );
    }
    else if ( signal( SIGINT, signal_handler ) == SIG_ERR )
    {
        perror("An error occured while setting the SIGINT signal handler.\n" );
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

    // 3. bootstrap the system
    CR_START( Thread_A );

    return EXIT_SUCCESS;
}
