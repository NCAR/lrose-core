/*      @(#)primal.h 20.17 93/06/28 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifndef primal_DEFINED
#define primal_DEFINED

#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>

#						ifndef sunwindow_sun_DEFINED
#include <xview/sun.h>
#						endif
#						ifndef makedev
#include <sys/types.h>
#						endif

#ifdef lint
#define	LINT_IGNORE(arg)
#else
#define	LINT_IGNORE(arg)	arg
#endif

#define EOS		'\0'
	/* End Of String */
#define EQ		==
	/* Avoids typo making test into assignment */

#define NEW(type)	(type *) calloc(1, sizeof(type))

#ifndef SVR4
#define IDENTITY(formal)\
	formal
#endif /* SVR4 */
#ifndef SVR4
#define CONCATENATE(formal1,formal2)\
	IDENTITY(formal1)formal2
#else /* SVR4 */
#define CONCATENATE(formal1,formal2)\
        formal1##formal2
#endif /* SVR4 */

#define	pkg_private	extern


#ifdef USING_SETS
		/* definitions for sets */
#define	BITS_PER_CHAR		8	/* number of bits in a char */
#define	MAX_SET_ELEMENTS	256	/* number of elements in a set */
#define MAX_SET_BYTES		(1 + (MAX_SET_ELEMENTS / BITS_PER_CHAR))

struct set
{
   char bytes[MAX_SET_BYTES];
};

typedef struct set SET;

#ifdef OW_I18N
#define	BYTE(n)		((unsigned char)(n) >> 3) /* byte for element n */
#else
#define	BYTE(n)		((unsigned)(n) >> 3)	/* byte for element n */
#endif
#define	BIT(n)		((n) & 07)		/* bit in byte for element n */

#define	ADD_ELEMENT(set, n)	(set)->bytes[BYTE(n)] |= (1 << BIT(n))
#define	REMOVE_ELEMENT(set, n)	(set)->bytes[BYTE(n)] &= ~(1 << BIT(n))

#define IN(set, n)   (((unsigned)((set)->bytes[BYTE(n)]) >> (int)BIT(n)) & 01)

#define	CLEAR_SET(set)	XV_BZERO((set)->bytes, MAX_SET_BYTES)

#define FILL_SET(set)	{ register int i;\
			  for (i = 0; i < MAX_SET_BYTES; i++)\
				(set)->bytes[i] = 0xFF;\
			}
#endif

#ifdef XV_DEBUG
#define AN_ERROR(expr)	((expr) && take_breakpoint())
#define ASSERT(expr)	if (expr) {} else abort()
#define ASSUME(expr)	if (expr) {} else take_breakpoint()
#else
#define AN_ERROR(expr)	(expr)
#define ASSERT(expr)
#define ASSUME(expr)
#endif

#endif
