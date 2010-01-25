/** @file cr.c
 *
 *  @author Original author  : Joseph D Poirier
 *  @date   Creation date    : February 2006
 *
 *
 *  @version X.X.X
 *
 *  VERSION    DATE/AUTHOR              COMMENT
 *
 *
 *
 *  LICENSE:
 *          see the license.txt file
 *
 * @cmd<<%PRJ% %PFE%>>
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>

#include "cr_config.h"
#include "cr.h"

/** \mainpage
 *
 *  cr_lib is a simple, portable, cooperative multitasking environment using coroutines. It features a
 *  mechanism for exiting and re-entering a function in a non-standard way using the standard C library's
 *  setjmp and longjmp functions. Coroutine threads run one at a time where each thread must be explicitly
 *  scheduled. cr_lib does include a system coroutine, cr_idle, that can be used in conjunction with the
 *  variable cr_g_activate_id to activate a coroutine. cr_g_activate_id could be used, for example, in
 *  an ISR routine of an event driven system. \n
 *
 *  cr_lib is probably most useful for micro-controllers, DSPs, small GPPs, or educational purposes.
 *
 *  <b> What is a coroutine: </b> \n
 *
 *  "A coroutine is represented by a closure (a code address and a referencing environment), into which
 *  we can jump by means of a nonlocal goto - in this case a special operation known as a transfer.
 *  In effect, coroutines are execution contexts that exist concurrently but execute one at a time,
 *  and transfer control to each other explicitly by name." From  <I> Programming Language Pragmatics by Morgan Kaufmann </I> \n
 *
 *  <b> Using cr_lib </b>
 *  - Initialize cr_lib by calling "cr_init" with the appropriate paramters
 *  - Create a coroutine by calling "cr_register_thread"
 *  - Call "CR_START" with the name of the coroutine to bootstrap the system
 *  - Call "CR_THREAD_INIT" at the start of each coroutine thread
 *  - Call "CR_YIELD" at the desired points within coroutine thread
 *
 *  \code
    example of initialization, registration, and start of cr_lib
    ------------------------------------------------------------

    #define CONTEXT_ARRAY_CNT   ( 2 + 1 )

    CR_CONTEXT context_array[ CONTEXT_ARRAY_CNT ];

    int main( int argc, char* argv[ ] )
    {
        // 1. initialize cr_lib
        cr_init( context_array, CONTEXT_ARRAY_CNT );

        // 2. register functions
        cr_register_thread( Thread_A );
        cr_register_thread( Thread_B );

        // 3. bootstrap system
        CR_START( Thread_A );

        return 0;
    }

         coroutine 1                                     coroutine 2
         -----------                                     -----------
    void Thread_A( void )                           void Thread_B( void )
    {                                               {
        CR_THREAD_INIT( );                              CR_THREAD_INIT( );

        for ( ; ; )                                     while ( 1 )
        {                                               {
            // main body of code                             // main body of code
            CR_YIELD( Thread_B );                            CR_YIELD( cr_idle );

            // user code                                     // user code
        }                                               }
    }                                               }

    *Note: locals within a corouitne thread should use the 'volatile' qualifier

    E.g.:
         coroutine 1
         -----------
    void Thread_A( void )
    {
        // locals use the 'volatile' qualifier
        int32_t volatile count = 0;

        CR_THREAD_INIT( );

        for ( ;; )
        {
            count += 1;

            // main body of code
            CR_YIELD( Thread_B );

            // user code
        }
    }

 *  \endcode
 *
 *  <b> Misc. Notes </b>
 *
 *  - cr_init's cr_context parameter requires 1 extra element for the system's "cr_idle" coroutine. Therefore,
 *    set the cr_context_count equal to the number of coroutine threads plus one.
 *
 *  - "cr_init" must be called before registering any coroutine threads
 *
 *  - Coroutine thread's must take a "void" parameter and return "void."
 *
 *  - A coroutine thread must call "CR_THREAD_INIT" at the top of the function before calling "CR_YIELD".
 *
 *  - Register a coroutine thread using "cr_register_thread" where the coroutine thread's name is passed as a parameter.
 *
 *  - Once all the coroutine threads are registered call "CR_START" with either a user coroutine thread name or the cr_idle coroutine name.
 *
 *  - "cr_init" takes a user declared array of "CR_CONTEXT" and its element count plus 1. There needs to be an
 *    extra element reserved for the "cr_idle" coroutine.
 *
 *  - Functions registered as coroutine threads should not return. I.e. the body of the main part of the code should be
 *    wrapped in an endless "for" or "while" loop.
 *
 *  - The global variable "cr_g_activate_id" can be used to activate a coroutine by setting it to a coroutine's ID.
 *    For it to work the idle coroutine (cr_idle) must be yielded to explicitly at some point. Have a look at the demo_2
 *    and demo_4 files. Each simulates an ISR type of external event. Be careful when scheduling events using "cr_g_activate_id";
 *    there's no buffering of the coroutine ID's being assigned. Use "cr_get_id()" - passing a coroutine function name - to determine
 *    the ID of a coroutine.
 *
 *  - The longjmp's second parameter could probably be used to identify the previously running coroutine and to set the
 *    cr_g_previous_cr_id variable once the jump is completed. To do so would require sanity checking that there aren't
 *    more coroutines registered than the size of the longjmp's second parameter type, but it would really only save two or
 *    three lines of code in the macros.
 */

/*! \example demo_1.c */
/*! \example demo_2.c */
/*! \example demo_3.c */
/*! \example demo_4.c */

/** \brief Internally used annonymous enumerations that define various states and settings
 *  \note For internal use only.
 */
enum {
    CR_ERROR_CR_EXITING = 0 /*!< Used in asserts for clarity */
};

jmp_buf             cr_g_reg_func_env;

/** \brief Pointer to the user defined array of coroutine contexts
 *  \note For internal use only.
 */
CR_CONTEXT*         cr_g_context        = 0;

/** \brief The number of elements in the coroutine context array
 *  \note For internal use only.
 */
uint32_t            cr_g_context_cnt    = 0;

/** \brief Holds the ID of the coroutine to be activated - by cr_idle
 */
cr_id_t             cr_g_activate_id    = CR_IDLE_THREAD_ID;

/** \brief Flag that's set when CR_START is called
 *  \note For internal use only.
 */
int32_t             cr_g_sys_started    = false;

/** \brief The ID of the coroutine that's active
 *  \note For internal use only.
 */
cr_id_t             cr_g_current_cr_id  = CR_IDLE_THREAD_ID;

/** \brief The ID of the previously active coroutine
 *  \note For internal use only.
 */
cr_id_t             cr_g_previous_cr_id  = CR_IDLE_THREAD_ID;

/** \brief The total number of registered coroutines
 *  \note For internal use only.
 */
static int32_t      cr_g_thread_cnt      = CR_THREAD_CNT_INIT;

/** \brief Find the ID of a coroutine.
 *
 *  Returns the ID of a registered coroutine.
 *
 *  \param pFunc the registered coroutine whose ID is requested
 *  \retval CR_INVALID_ID when no ID is found
 *  \retval id a valid coroutine's ID
 */
cr_id_t cr_get_id( void ( *pFunc )( void ) )
{
    int32_t i;
    cr_id_t id = CR_INVALID_ID;

    for ( i = 0; i <= cr_g_thread_cnt; i++ )
    {
        if ( cr_g_context[ i ].pFunc == pFunc )
        {
            id = i;

            break;
        }
    }

//    printf( "cr_get_id pFunc: 0x%X\n", pFunc );
//    printf( "cr_get_id id: %d\n", id );

    return id;
}

/** \brief Resets the system to an initial state.
 *
 *  This function sets all the system's global variables to their original state.
 */
void cr_reset( void )
{
    cr_g_context        = 0;
    cr_g_context_cnt    = 0;
    cr_g_thread_cnt     = CR_THREAD_CNT_INIT;
    cr_g_previous_cr_id = CR_INVALID_ID;
    cr_g_current_cr_id  = CR_INVALID_ID;
    cr_g_activate_id    = CR_IDLE_THREAD_ID;
    cr_g_sys_started    = false;
}

/** \brief cr_lib's initialization function.
 *
 *  This function sets a global pointer to the array holding all of the registered
 *  coroutines context information, as well as, the element count of the array.
 *
 *  \param cr_context the context array
 *  \param cr_context_count the element count of the array
 *  \attention This initialization function must be the first function called in the library.
 */
void cr_init( CR_CONTEXT*    cr_context,
              size_t         cr_context_count )
{
    // Init the array of CR_CONTEXT structs as well as the globals.
    assert( cr_context && "user param cr_context must be non-zero!\n" );
    assert( ( cr_context_count > 0 ) && "user param cr_context_count must be non-zero!\n" );

    memset( cr_context, 0, cr_context_count * sizeof( CR_CONTEXT ) );

    cr_g_context        = cr_context;
    cr_g_context_cnt    = ( uint32_t ) cr_context_count;

    // Register the idle thread, which should _always_ be the first coroutine registered.
    cr_register_thread( cr_idle );
}

/** \brief The internal system's coroutine thread.
 *
 *  This coroutine thread is used in conjunction with the cr_g_activate_id variable.
 *  cr_g_activate_id can be set to a coroutine's ID, e.g. in an ISR handler, and
 *  this function will activate the coroutine. This will work only if this function
 *  is explicitly called via CR_YIELD.
 *
 *  \attention This coroutine thread must be explicitly called for it to run.
 */
void cr_idle( void )
{
    // This needs to be static because the function initially returns normally.
    static cr_id_t temp_id;

    CR_THREAD_INIT( );

    assert( ( this_id__ == CR_IDLE_THREAD_ID ) && "cr_idle: this_id__ != CR_IDLE_THREAD_ID!\n" );

    // This will be the entry point when longjump is called with cr_idle's context.
    // No need to perform another setjmp from within the loop; not much happening.
    for ( ; ; )
    {
        // Spin until a non-idle thread is activated.
        if ( cr_g_activate_id == CR_IDLE_THREAD_ID )
        {
            continue;
        }

        assert( ( cr_g_activate_id != CR_INVALID_ID ) && "cr_idle: cr_g_activate_id == CR_INVALID_ID!\n" );
        assert( ( ( uint32_t ) cr_g_activate_id <= cr_g_context_cnt) && "cr_idle: cr_g_activate_id out of bounds!\n" );
        assert( ( cr_g_activate_id != this_id__ ) && "cr_idle: recursive coroutine call!\n" );

        temp_id             = cr_g_activate_id;
        cr_g_activate_id    = CR_IDLE_THREAD_ID;
        cr_g_previous_cr_id = CR_IDLE_THREAD_ID;

        if ( !setjmp( cr_g_context[ CR_IDLE_THREAD_ID ].env ) )
        {
            longjmp( cr_g_context[ temp_id ].env, SETJMP_DFLT_RET_VAL );
        }
        else
        {
            /* explicit block for the longjmp */ ;
        }
    }
}

/** \brief Register a function as a coroutine thread
 *
 *  This function creates an ID for the new coroutine thread and associates
 *  the ID with a context element. Information about the coroutine is set in
 *  the context element. cr_g_current_cr_id is set to the new coroutine's ID.
 *  The cr_register_thread function's context is saved and the user function is
 *  called. The CR_THREAD_INIT macro performs a longjmp back to this function
 *  using the previously saved context. The cr_g_sys_started flag is checked
 *  to determine if a user coroutine has exited via the function's epilog code.
 *
 *  \param pFunc the function to be registered
 *  \return The ID of the registered coroutine.
 *  \attention All coroutine thread registration must be completed before calling CR_START.
 */
cr_id_t cr_register_thread( void ( *pFunc )( void ) )
{
    // Increase the coroutine count.
    cr_g_thread_cnt += 1;

    if ( cr_g_thread_cnt == CR_IDLE_THREAD_ID )
    {
        if ( pFunc != cr_idle )
        {
            perror( "cr_idle error: cr_g_thread_cnt !=  CR_IDLE_THREAD_ID. Note, coroutines shouldn't be registered prior to calling cr_init." );
        }
    }

    assert( pFunc && "cr_register_thread: pFunc is invalid!\n" );

    // Account fot the cr_idle coroutine (one reserved index in the array) in the assert check.
    assert( ( cr_g_thread_cnt < cr_g_context_cnt) &&
                "cr_register_thread: [ cr_g_thread_cnt < cr_g_context_cnt ] failed\n" );

    // Assign the coroutine's ID and function pointer to its context structure
    cr_g_context[ cr_g_thread_cnt ].id    = cr_g_thread_cnt;
    cr_g_context[ cr_g_thread_cnt ].pFunc = pFunc;

    // cr_g_current_cr_id is used by the CR_THREAD_INIT macro to identify
    // the coroutine.
    cr_g_current_cr_id = cr_g_thread_cnt;

    if ( !setjmp( cr_g_reg_func_env ) )
    {
        pFunc( ); // this function won't return normally
    }
    else
    {
        /* explicit block for the longjmp */ ;
    }

    // The function is now a coroutine.
    // This is where we return from CR_THREAD_INIT via the longjump.

    // A sentinal. At init the coroutine returned via a longjump so the return
    // point saved in the function's prolog is valid and will return here
    // should the coroutine actually return normally via the epilog code.
    if ( cr_g_sys_started == CR_SYSTEM_STARTED )
    {
        // What to do?
        // - nothing and return normally
        // - yield to whatever is in cr_g_previous_cr_id
        // - yield to cr_idle
        // - assert
        // - exit

        // Note, cr_g_current_cr_id holds the ID of the exiting coroutine.
        // Note, cr_g_previous_cr_id holds the ID of the previous coroutine.

        // Note, it's not possible to remove the exiting coroutine from the
        // context array because the remaining coroutine ID's cannot be changed,
        // a coroutine's ID is the index into the context array.

        // Look at the cr_g_current_cr_id variable in a debugger to see the ID
        // of the exiting coroutine.
        assert( CR_ERROR_CR_EXITING && "cr_register_thread: coruotine exiting!\n" );

        // Pass EXIT_SUCCESS; we want any user registered clean-up functions to be called.
        exit( EXIT_SUCCESS );
    }

    return cr_g_thread_cnt;
}

// end of file
