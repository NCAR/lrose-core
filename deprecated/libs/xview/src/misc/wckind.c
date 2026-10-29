#ifndef lint
#ifdef sccs
static char sccsid[] = "@(#)wckind.c 1.793/06/28 Copyr 1992 SMI";
#endif
#endif

/*
 *	(c) Copyright 1992 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE 
 *	file for terms of the license.
 */


/*
 * wckind.c:  This is the set of functions to helping the word
 * selection support in the OpenWindows X toolkit.  However, SunOS 5.x
 * will have a this kind of feature in future release, therefor this
 * function is considered as TEMPORARY SOLUTION.  Should never be
 * publish any of the information about this functions but SMI
 * internal (ako, 17-March-1992).
 */

/*
 * #ifdef flag "OPTIMIZED" has been used to in this source code to
 * distinguish between static library (OPTIMIZED is on), and shared
 * library build (this is very common method in OpenWindows build
 * environment).  In the case of the static library, we can not use
 * dynamic loadable module, therefor static version always behaive as
 * "C" locale regardless of the current locale.
 */

#include	<stdlib.h>
#include	<string.h>
#include	<locale.h>
#include	<wctype.h>
#include	<widec.h>
#include	<dlfcn.h>
#include	<sys/param.h>	/* to get MAXPATHLEN */


#ifndef OPTIMIZED
static int	_wckind_c_locale();

static int	(*__wckind)() = _wckind_c_locale;
static void	*dlhandle = NULL;
#endif


void
_wckind_init()
{
#ifdef OPTIMIZED
	/*
	 * We do not need to do anything for static library case.
	 */
#else
	char	*locale;
	char	*openwinhome;
	char	path[MAXPATHLEN + 1];


	if (dlhandle != NULL) {
		(void) dlclose(dlhandle);
		dlhandle = NULL;
	}

	locale = setlocale(LC_CTYPE, NULL);
	if (strcmp(locale, "C") == 0)
		goto c_locale;
	if ((openwinhome = getenv("OPENWINHOME")) == NULL)
#ifdef OPENWINHOME_DEFAULT
		/* martin-2.buck@student.uni-ulm.de */
		openwinhome = OPENWINHOME_DEFAULT;
#else
		goto c_locale;
#endif

	(void) sprintf(path, "%s/lib/locale/%s/libs/wckind.so",
		openwinhome, locale);

	if ((dlhandle = dlopen(path, RTLD_LAZY)) != NULL) {
		__wckind = (int (*)(int))dlsym(dlhandle, "_l10n_wckind");
		if (__wckind != NULL)
			return;
		(void) dlclose(dlhandle);
		dlhandle = NULL;
	}

c_locale:
	__wckind = _wckind_c_locale;
#endif /* OPTIMIZED */
}


#ifndef OPTIMIZED
int
_wckind(wc)
wchar_t	wc;
{
	return (*__wckind) (wc);
}
#endif


#ifdef OPTIMIZED
	/*
	 * In case of static library, "C" locale buildin module is the
	 * _wckind itself.
	 */
int
_wckind(wc)
#else
static int
_wckind_c_locale(wc)
#endif
wchar_t	wc;
{
	int	ret;

	/*
	 * DEPEND_ON_ANSIC: L notion for the character is new in
	 * ANSI-C, k&r compiler won't work.
	 */
	if (iswascii(wc))
		ret = (iswalnum(wc) || wc == L'_') ? 0 : 1;
	else
		ret = wcsetno(wc) + 1;

	return ret;
}


#ifdef DEBUG_BY_ITSELF
#include	<stdio.h>

main()
{
	wchar_t		*p;
	wchar_t		line[257];

	setlocale(LC_ALL, "");
	_wckind_init();
	while (getws(line) != NULL) {
		for (p = line; *p; p++)
			printf("%2wc,", *p);
		putchar('\n');
		for (p = line; *p; p++)
			printf("%2d,", _wckind(*p));
		putchar('\n');
	}
}
#endif /* DEBUG */
