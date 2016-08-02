#ifdef __cplusplus
 extern "C" {
#endif
/* Copyright 1990 Sun Microsystems. Inc */
/* "@(#)stdarg.h 1.5 2/11/91 SMI"; */

#ifdef SUNOS4

#ifndef __sys_varargs_h
#define __sys_varargs_h

#if defined(__STDC__)

typedef void *va_list;
#define va_start(list, name) (void) (list = (va_list) &__builtin_va_alist)
#define va_arg(list, mode) ((mode *)__builtin_va_arg_incr((mode *)list))[0]
extern void va_end(va_list);
#define va_end(list) (void)0

 
#else   /* not __STDC__ */
 
#include <varargs.h>
 
#endif  /* __STDC__ */


#endif /* __sys_varargs_h*/

#endif /* SUNOS4 */
#ifdef __cplusplus
}
#endif
