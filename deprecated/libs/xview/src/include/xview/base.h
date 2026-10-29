/*      @(#)base.h 20.33 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */

#ifndef xview_base_DEFINED
#define xview_base_DEFINED
#include <string.h>
#include <stdlib.h>

#include <xview/xv_c_types.h>

#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#include <stdlib.h>
#endif /* __cplusplus || __STDC__ */

/*
 ***********************************************************************
 *			Definitions and Macros
 ***********************************************************************
 */

#define XV_OK		0
#define	XV_ERROR	1

#ifndef TRUE
#define	TRUE		1
#endif

#ifndef FALSE
#define FALSE		0
#endif

#ifndef NULL
#define NULL		0
#endif

#ifndef MIN
#define MIN(x, y) 	( ((x) < (y)) ? (x) : (y) )
#endif

#ifndef MAX
#define MAX(x, y) 	( ((x) > (y)) ? (x) : (y) )
#endif

/* These are portability #defines needed by public header files. Please see
 * misc/portable.h for the bulk of the portability #defines.
 * 
 * Currently, the default environment is SVR4. This is checked by looking at
 * the macro SUNOS41. This has to be fixed later to accomodate non-SUNOS41 and
 * non-SVR4 systems.
 */
#if !defined(SUNOS41) && !defined(__linux) && !defined(__APPLE__)
#define XV_OS_SVR4
#define XV_USE_TTCOMPAT

#ifndef SUNOS5
#define SYSV_WAIT 
#endif

#define SYSV_UCONTEXT 
#define XV_USE_XVFCNTL 
#elif defined(__linux) || defined(__APPLE__)
#define XV_OS_SVR4
#undef XV_USE_TTCOMPAT
#define SYSV_UCONTEXT 
#define XV_USE_XVFCNTL 
#endif
 
/*
 * 	These alloc macros should be functions someday with an error call out
 * 	to cleanup, if the underlying malloc fails.
 */

extern void *xv_alloc_save_ret;
extern void xv_alloc_error();
extern void *xv_calloc();

#ifdef XV_NO_STRDUP
extern char *strdup();
#endif /* XV_NO_STRDUP */

#ifdef XV_USE_ALLOC_FUNCS
#define xv_alloc(t) (void *)xv_alloc_func(sizeof(t))
#define xv_alloc_n(t, n) (void *)xv_alloc_n_func(sizeof(t), n)
#define xv_malloc(t) (void *)xv_malloc_func(t)
#define xv_realloc(ptr, size) (void *)xv_realloc_func(ptr, size)
#define xv_valloc(t) (void)xv_valloc_func(t)
#else

#define xv_alloc(t)  \
  ((( xv_alloc_save_ret = (void *)calloc( 1, sizeof( t ))) ? (void)0 : \
    xv_alloc_error()) \
   , xv_alloc_save_ret )

#define xv_alloc_n(t, n)  \
  ((( xv_alloc_save_ret = (void *)calloc( n, sizeof( t ))) ? (void)0 : \
    xv_alloc_error()) \
   , xv_alloc_save_ret )

#define xv_malloc( size )  \
   ((( xv_alloc_save_ret = (void *)malloc( size )) ? (void)0 : \
     xv_alloc_error())  \
   , xv_alloc_save_ret )

#define xv_realloc( ptr, size )  \
 ((( xv_alloc_save_ret = (void *)realloc( ptr, size )) ? (void)0 : \
   xv_alloc_error()) \
   , xv_alloc_save_ret )

#define xv_valloc( size )  \
   ((( xv_alloc_save_ret = (void *)valloc( size )) ? (void)0 : \
     xv_alloc_error()) \
   , xv_alloc_save_ret )

#endif /* XV_USE_ALLOC_FUNCS */

#define xv_free(s)		((void) free((char *)s))
#define xv_strsave(s)		strcpy( (char *)xv_malloc(strlen(s)+1), (s) )

#define XV_NULL			((Xv_opaque)NULL)
#define XV_ZERO			(0L)

/*
 ***********************************************************************
 *		Typedefs, Enumerations, and Structs
 ***********************************************************************
 */

typedef unsigned long	Xv_opaque;
typedef unsigned long   Xv_object;


/*
 ***********************************************************************
 *		Global Functions
 ***********************************************************************
 */

extern int defeat_event_security;

#endif /* xview_base_DEFINED */
