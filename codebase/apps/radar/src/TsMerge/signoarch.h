/* *******************************************************************
 * *                                                                 *
 * *  Minimal Subset of Architecture-Independent SIGMET Definitions  *
 * *                                                                 *
 * *******************************************************************
 * File: include/signoarch.h
 *
 *                       COPYRIGHT (c) 2003  BY 
 *        SIGMET INCORPORATED, WESTFORD MASSACHUSETTS, U.S.A.  
 * 
 * THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
 * ONLY  IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
 * INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE  OR  ANY OTHER
 * COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
 * OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
 * TRANSFERED. 
 *
 * NOTE: This file contains only those definitions that must cross the
 * line between radically different compiler and runtime environments,
 * e.g., kernel module builds in which it would be inappropriate to
 * include the larger sigtypes.h directly.
 */ 
#ifndef SIGMET_SIGNOARCH_H
#define SIGMET_SIGNOARCH_H 1

/* Terminal I/O definitions
 */
#define TTYPARITY_NONE   0	/* Available parity choices */
#define TTYPARITY_ODD    1
#define TTYPARITY_EVEN   2

#define TTYPROTO_ASYNC   0	/* Available serial protocols */

/* Some handy macros
 */
#ifndef MAX
#define MAX( a, b ) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN( a, b ) (((a) < (b)) ? (a) : (b))
#endif

#define ELEMENTOFFSET( PTR, ELEMENT ) \
  ( ((UINT4)&((PTR)->ELEMENT)) - ((UINT4)(PTR)) )

#endif  /* #ifndef SIGMET_SIGNOARCH_H */
