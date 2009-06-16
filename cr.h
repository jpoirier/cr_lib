/** @file cr.h
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
 *          use either zlib_license.txt or lgpl_licesne.txt, your choice.
 *
 */
#ifndef CR_H
#define CR_H

#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#ifdef __INTEL_COMPILER
    // ICC - disable assert warning: "warning #279: controlling expression is constant"
    #pragma warning (disable: 279)
#endif


/** \brief Annonymous enumerations that define various states and settings.
 */
enum {
    CR_IDLE_THREAD_ID           = 0,                    /*!< ID of the system's cr_idle coroutine */
    CR_REGISTER_THREAD_ID       = CR_IDLE_THREAD_ID,    /*!< ID used by the cr_register_thread function */
    CR_SYSTEM_STARTED           = -1,                   /*!< Sentinal flag used internall */
    CR_INVALID_ID               = -1                    /*!< Returned by cr_get_id when an ID is not found */
};

/** \brief Coroutine ID typedef
 *
 *  Coroutine ID type used by the cr_lib system.
 *
 */
typedef int32_t cr_id_t;

/** \brief Coroutine context information
 *
 *  A pointer to an array of this structure type is passed to the cr_init function.
 *
 *  \note The context array _must_ include 1 extra element for the system's cr_idle coroutine.
 */
typedef struct CR_CONTEXT {
    void        (*pFunc)(void);
    jmp_buf     env;
    cr_id_t     id;
} CR_CONTEXT;

/** \brief Initializes a function as a coroutine
 *
 *  This macro creates the static variable this_id__ which holds the ID of
 *  the new coroutine. cr_g_current_cr_id is used by cr_register_thread to pass
 *  the newly assigned coroutine ID to the function being registered. The ID is
 *  then used to save the new coroutine's context, which serves as the initial
 *  entry point in to the coroutine. Rather then returning normally a longjmp
 *  is performed to get back to the cr_register_thread function.
 *
 *  \hideinitializer
 *  \attention This macro should be placed at the start of the function being
 *  registered as a coroutine. Also, a coroutine should never return normally.
 */
#define CR_THREAD_INIT()                                                \
            static cr_id_t this_id__;                                   \
            this_id__ = cr_g_current_cr_id;                             \
            cr_g_previous_cr_id = this_id__;                            \
            cr_g_current_cr_id = CR_REGISTER_THREAD_ID;                 \
            if(!setjmp(cr_g_context[this_id__].env))                    \
                longjmp(cr_g_context[CR_REGISTER_THREAD_ID].env, 1)

/** \brief Starts the cr_lib system
 *
 *  This macro basically boostraps the system.
 *  Bootstrap the system by fetching the ID of the coroutine to call and call it.
 *
 *  \param func_name the name of a user coroutine or cr_idle
 *  \hideinitializer
 *  \attention cr_init must have been called and coroutine registration completed
 */
#define CR_START(func_name)                                                                         \
            cr_idle();                                                                              \
            cr_g_previous_cr_id = CR_INVALID_ID;                                                    \
            cr_g_current_cr_id = cr_get_id(func_name);                                              \
            assert((cr_g_current_cr_id != CR_INVALID_ID) && "CR_START:  CR_INVALID_ID!\n");         \
            assert((cr_g_current_cr_id <= cr_g_context_cnt) && "CR_START: ID out of bounds!\n");    \
            cr_g_sys_started = CR_SYSTEM_STARTED;                                                   \
            longjmp(cr_g_context[cr_g_current_cr_id].env, 1)

/** \brief Explicitly yields to a coroutine
 *
 *  This macro fetches the ID of the coroutine to be called, sets the current
 *  environment, then makes a longjmp to the desired coroutine.
 *
 *  \param func_name the name of a user coroutine or cr_idle
 *  \hideinitializer
 */
#define CR_YIELD(func_name)                                                                         \
            cr_g_current_cr_id = cr_get_id(func_name);                                              \
            assert((cr_g_current_cr_id != CR_INVALID_ID) && "CR_YIELD: CR_INVALID_ID!\n");          \
            assert((cr_g_current_cr_id <= cr_g_context_cnt) && "CR_YIELD: ID out of bounds!\n");    \
            assert((cr_g_current_cr_id != this_id__) && "CR_YIELD: recursive coroutine call!\n");   \
            cr_g_previous_cr_id = this_id__;                                                        \
            if(!setjmp(cr_g_context[this_id__].env))                                                \
                longjmp(cr_g_context[cr_g_current_cr_id].env, 1)

#ifdef __cplusplus
extern "C" {
#endif

    /* Should _not_ be modified by the user. extern because they're used in the macros. */
    extern CR_CONTEXT*  cr_g_context;
    extern uint32_t     cr_g_context_cnt;
    extern int32_t      cr_g_sys_started;
    extern cr_id_t      cr_g_previous_cr_id;
    extern cr_id_t      cr_g_current_cr_id;

    /* Can be modified by the user. */
    extern cr_id_t volatile cr_g_activate_id;

    /* User callable functions. */
    extern void         cr_reset(void);
    extern void         cr_init(CR_CONTEXT* cr_context, size_t cr_context_count);
    extern cr_id_t      cr_get_id(void(*pFunc)(void));
    extern void         cr_idle(void);
    extern cr_id_t      cr_register_thread(void (*pFunc)(void));

#ifdef __cplusplus
}
#endif

#endif // CR_H

// end of file
