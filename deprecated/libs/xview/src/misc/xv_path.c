#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_path.c 50.7 93/06/28";
#endif
#endif


/*
 *      (c) Copyright 1989 Sun Microsystems, Inc. Sun design patents
 *      pending in the U.S. and foreign countries. See LEGAL NOTICE
 *      file for terms of the license.
 */

#ifdef OW_I18N
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#ifdef SVR4
#include <netdb.h>
#else
#include <sys/dir.h>
#endif /* SVR4 */
#include <xview_private/portable.h>
#include <xview/pkg.h>

extern int stat();
extern char *getenv();

Pkg_private char *
in_path(filename, nameof_pathvar)
    char *filename, *nameof_pathvar;
{
    struct stat	component_stat;
    static char	component[MAXPATHLEN+1];
    char	*path, *tmp;
    char 	*end = NULL;
    int		len, lastchar; 
    int		allocd = 0;

    if (!(path = getenv(nameof_pathvar))) {
	return (NULL);
    }
	
    lastchar = (len = strlen(path)) - 1;
    if (len == 0) {
	return (NULL);
    }

    if (path[lastchar] != ':') {
	tmp = calloc(1, len+2);
	allocd = 1;
	strcpy(tmp, path);
	path = tmp;
	path[len] = ':';
    }

    while (end = XV_INDEX(path, ':')) {
	if (!(len = (int) (end - path))) {
	    path++;
	    continue;
	}

	XV_BCOPY(path, component, len);
	component[len] = '\0';

	if (component[len-1] != '/') {
	    strcat(component, "/");
	}
	strcat(component, filename);
	if (!stat(component, &component_stat)) {
	    if (allocd) {
		free (tmp);
	    }
	    return (component);
	} else {
	    path = end + 1;
	}

	XV_BZERO(component, sizeof(component));
    }
    if (allocd) {
	free (tmp);
    }
    return ((char *) NULL);
}
#endif /* OW_I18N */
