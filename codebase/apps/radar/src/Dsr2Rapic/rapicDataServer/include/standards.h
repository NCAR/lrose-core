#ifndef __STANDARDS_H__
#define __STANDARDS_H__
/*
 *
 * Copyright 1995-1996 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Silicon Graphics, Inc.
 *
 * RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to restrictions
 * as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
 * and Computer Software clause at DFARS 252.227-7013, and/or in similar or
 * successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
 * rights reserved under the Copyright Laws of the United States.
 */
#ident "$Id: standards.h,v 1.1 2007/10/25 03:37:27 dixon Exp $"

/*
 * We defines short forms for the predicates required by various
 * headers to support the various standards.
 *
 * These predicates are used to build up permitted declarations.
 * For ANSI-C specified headers, only ANSI-C definitions need no predicate.
 * Most are restrictive predicates - declaring when a symbol is NOT
 * permitted rather than when they are. Often, if no feature test macro is
 * declared then ALL symbols in a header are visible.(the notable exception
 * is ANSI headers - they MUST contain only ANSI symbols unless some other
 * feature test macro is used.
 * These predicates are built up from the following feature test macros:
 *	_SGI_SOURCE - used to extend ANSI headers to contain SGI specific
 *		      symbols. Both XOPEN and POSIX feature test macros
 *		      must override ALL symbols added by this macro
 *	_XOPEN_SOURCE - enable Xopen symbols and suppress non-Xopen symbols
 *			According to XPG4 section 2.2 defining _XOPEN_SOURCE
 *			should suffice to enable all _POSIX_SOURCE and
 *			_POSIX_C_SOURCE symbols (well, POSIX90 and POSIX1003.2
 *			symbols)
 *	_POSIX_SOURCE
 *	_POSIX_C_SOURCE - enable POSIX symbols and suppress non-POSIX symbols.
 * In ANSI mode, _SGI_SOURCE is not defined.
 *
 * One main assumption here is that for the most part, the symbol space
 * is ever increasing: ANSI -> POSIX90 -> (POSIX93 -> XOPEN4) -> SGI
 * The main hassle is POSIX93 and XPG4
 * For the few things that violate this, special predicates must be used.
 *	
 */

/*
 * ANSI symbols - In ANSI headers only ANSI symbols are permitted. But
 * in other headers we give all POSIX and XPG4 symbols. We currently do
 * not provide SGI specific symbols in ANSI mode.
 */
#define _ANSIMODE	(!defined(__EXTENSIONS__))

#define _NO_ANSIMODE	(defined(__EXTENSIONS__))

/*
 * POSIX symbols - additions various POSIX standards add over ANSI -
 * mode:ANSI	-> symbols are visible (except in ANSI headers)
 * 	POSIX	-> symbols are visible
 *	SGI	-> symbols are visible
 *	XOPEN	-> symbols are visible
 *
 * POSIX90 - 1003.1a (first POSIX)
 * POSIX2  - 1003.2 (shells & utilities)
 * POSIX93 - 1003.1b (real-time ++)
 * POSIX1C - 1003.1c (pthreads..)
 *
 * Note that _POSIX_SOURCE is obsolete and corresponds to _POSIX_C_SOURCE = 1.
 * _POSIX_C_SOURCE=2 was defined by 1003.2
 * 1003.1b sets _POSIX_C_SOURCE to 199309L.
 */
#define _POSIX90	(defined(_POSIX_SOURCE) || \
			 defined(_POSIX_C_SOURCE) || \
			 defined(_XOPEN_SOURCE) || \
			 defined(_SGI_SOURCE) || \
			 _ANSIMODE)
#define _POSIX2		((_POSIX_C_SOURCE > 1) || \
			 defined(_XOPEN_SOURCE) || \
			 defined(_SGI_SOURCE) || \
			 _ANSIMODE)

/*
 * Note that POSIX93 and XPG4 each have extensions over the other.
 * Thus one can make NO assumptions (as one could previously) that
 * if its POSIX then it must be XOPEN. According to XPG4 (section:2.2) if
 * _POSIX_C_SOURCE is >2 then the the behavior is undefined.
 * For maximal developer latitude, we don't turn off POSIX93 extensions
 * if they specify _XOPEN_SOURCE - they can get both if they wish.
 *
 * New symbols that are both in XPG4 and 1003.1b need to be enabled via:
 * 	#if _POSIX93 || _XOPEN4
 * These symbols shouldn't be visible in POSIX90/2 mode.
 */
#define _POSIX93	((_POSIX_C_SOURCE >= 199309L) || \
			 (defined(_SGI_SOURCE) && _NO_XOPEN4 && _NO_POSIX) || \
			 _ANSIMODE)
#define _NO_POSIX	(!defined(_POSIX_SOURCE) && !defined(_POSIX_C_SOURCE))

/*
 * POSIX 1003.1c - new symbols made visible by pthreads, etc.
 * None of these are in XPG4.
 * We add _SGI_REENTRANT_FUNCTIONS for backward compatibility
 * Of course these symbols shouldn't be visible when in POSIX90/2/93 mode.
 * These really can't be visible in MIPS ABI mode either.
 */
#define _POSIX1C	((_POSIX_C_SOURCE >= 199506L) || \
			 (defined(_SGI_SOURCE) && _NO_XOPEN4 && _NO_POSIX && _NO_ABIAPI) || \
			 defined(_SGI_REENTRANT_FUNCTIONS) || \
			 _ANSIMODE)

/*
 * predicate for XOPEN XPG4 additions and extensions -
 * mode:ANSI	-> symbols visible (except in ANSI headers)
 * 	POSIX	-> symbols not visible
 * 	SGI	-> symbols visible
 * 	XOPEN	-> symbols visible
 */
#define _XOPEN4		(defined(_XOPEN_SOURCE) || \
				((defined(_SGI_SOURCE) && _NO_POSIX)) || \
				_ANSIMODE)
#define _NO_XOPEN4	(!defined(_XOPEN_SOURCE))
/*
 * Following is a define for XOPEN XPG4 Unix eXtensions
 */
#define _XOPEN4UX ((defined(_XOPEN_SOURCE) && _XOPEN_SOURCE_EXTENDED == 1) || \
			_SGIAPI || \
			_ANSIMODE)

/*
 * predicates for SGI extensions
 *	These include 'standard' unix functions/defines that aren't part of
 *	any published standard but are common in the 'standard' unix world.
 * mode:ANSI	-> symbols not visible
 * 	POSIX	-> symbols not visible
 * 	SGI	-> symbols visible
 *	XOPEN	-> symbols not visible
 */
#define _SGIAPI		(defined(_SGI_SOURCE) && \
			 _NO_POSIX && \
			 _NO_XOPEN4)

/*
 * predicates for MIPS ABI extensions
 * mode:ANSI	-> symbols not visible
 * 	POSIX	-> symbols not visible
 * 	SGI	-> symbols visible
 *	XOPEN	-> symbols not visible
 */
#define _ABIAPI		(defined(_ABI_SOURCE))
#define _NO_ABIAPI	(!defined(_ABI_SOURCE))

/*
 * predicates to get reentrant functions
 */
#define _REENTRANT_FUNCTIONS	(defined(_SGI_REENTRANT_FUNCTIONS))

#endif /* __STANDARDS_H */
