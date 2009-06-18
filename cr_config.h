/** @file cr_config.h
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
#ifndef CR_CONFIG_H
#define CR_CONFIG_H

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/** \def NPRINTF
 *  \brief User setable: set or unset the standard printf macro
 *
 */
//#undef          NPRINTF
#define         NPRINTF

/** \def NDPRINTF
 *  \brief User setable: set or un-set the debug printf macro
 *
 */
//#undef          NDPRINTF
#define         NDPRINTF

/** \def NEPRINTF
 *  \brief User setable: set or un-set the error printf macro
 *
 */
//#undef          NEPRINTF
#define         NEPRINTF

// __func__ (C99) : The function where the printf is called
// __LINE__ : The line where the printf is called
// __FILE__ : The file where the printf is called

/** \def PRINTF
 *  \brief Standard printf macro
 */
/** \def PRINTF_VA
 *  \brief Standard printf macro with a variable number of arguments
 */
#ifdef NPRINTF
    #define PRINTF(fmt)
    #define PRINTF_VA(fmt, ...)
#else
    #define PRINTF(fmt)                printf(fmt)
    #define PRINTF_VA(fmt, ...)        printf(fmt, __VA_ARGS__)
#endif

/** \def DPRINTF
 *  \brief Debug printf macro
 */
/** \def DPRINTF_VA
 *  \brief Debug printf macro with a variable number of arguments
 */
#ifdef NDPRINTF
    #define DPRINTF(fmt)
    #define DPRINTF_VA(fmt, ...)
#else
    #define DPRINTF(fmt)                printf("[%s:%s:%d] " fmt, __FILE__, __func__, __LINE__)
    #define DPRINTF_VA(fmt, ...)        printf("[%s:%s:%d] " fmt, __FILE__, __func__, __LINE__, __VA_ARGS__)
#endif

/** \def EPRINTF
 *  \brief Error printf macros
 */
/** \def EPRINTF_VA
 *  \brief Error printf macros with a variable number of arguments
 */
#ifdef NEPRINTF
    #define EPRINTF(fmt)
    #define EPRINTF_VA(fmt, ...)
#else
    #define EPRINTF(fmt)                printf("[%s:%s:%d] " fmt, __FILE__, __func__, __LINE__)
    #define EPRINTF_VA(fmt, ...)        printf("[%s:%s:%d] " fmt, __FILE__, __func__, __LINE__, __VA_ARGS__)
#endif


/*----------------------- Language Standards -----------------------------------
- C89           __STDC__                                    ANSI X3.159-1989
- C90           __STDC_VERSION__                            ISO/IEC 9899:1990
- C94           __STDC_VERSION__        = 199409L           ISO/IEC 9899-1:1994
- C99           __STDC_VERSION__        = 199901L           ISO/IEC 9899:1999
- C++98         __cplusplus             = 199707L           ISO/IEC 14882:1998
- C++/CLI       __cplusplus_cli         = 200406L           ECMA-372
- EC++          __embedded_cplusplus                        Embedded C++
------------------------------------------------------------------------------*/
// Note that not all compliant compilers provide the correct pre-defined macros.
#ifdef __STDC__
    #define PREDEF_STANDARD_C_1989

    #ifdef __STDC_VERSION__
        #define PREDEF_STANDARD_C_1990

        #if (__STDC_VERSION__ >= 199409L)
            #define PREDEF_STANDARD_C_1994
        #endif

        #if (__STDC_VERSION__ >= 199901L)
            #define PREDEF_STANDARD_C_1999
        #endif
    #endif
#endif

#ifndef __cplusplus
    #if !defined(__DMC__) && !defined(_MSC_VER)
        #ifndef PREDEF_STANDARD_C_1999
            #error "ISO C99 compiler is required for this code to compile and work properly"
        #endif
    #endif
#endif

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#ifdef __INTEL_COMPILER
    #pragma warning (disable: 279)

    // OS X
    #ifdef __APPLE__
        // none
    #endif

    // GNUC on *nix
    #if !defined(__APPLE__) && defined(__GNUC__)
        #define restrict    __restrict
        #define inline      __inline
    #endif

    // *MS VC++ with Intel compiler
    #if defined(_MSC_VER) && !defined(__APPLE__) && !defined(__GNUC__)
        #define alloca  _alloca

        #ifndef __cplusplus
            #define inline  __inline
        #endif
    #endif
#endif

// *MS VC++ only
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    #ifndef __cplusplus
        #define inline  __inline
    #endif

    #define restrict    __restrict
#endif


#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif // CR_CONFIG_H

// end of file
