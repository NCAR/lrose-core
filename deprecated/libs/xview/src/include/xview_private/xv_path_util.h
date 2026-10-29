/*      @(#)xv_path_util.h 1.7 93/06/29 SMI      */

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL_NOTICE
 *	file for terms of the license.
 */

#ifndef xv_path_util_DEFINED
#define xv_path_util_DEFINED

/*
 * The Path Utilities is a Pkg_private set of functions and
 * macros that are considered private to the File Chooser and
 * related packages.  They are not supported as a public
 * interface.  Period.  Nope.  Nada.
 */

#include <xview/frame.h>
#include <sys/stat.h>
#include <sys/param.h>



/* try to out-perform strcmp() for common checks */
#define is_dot(dir)		(((dir)[0] == '.') && ((dir)[1] == '\0'))
#define is_dotdot(dir)		(((dir)[0] == '.') && ((dir)[1] == '.') && ((dir)[2] == '\0'))
#define is_dot_file(dir)	(((dir)[0] == '.') && !((dir)[1] == '\0'))
#define is_root(dir)		(((dir)[0] == '/') && ((dir)[1] == '\0'))
#define is_relative(dir)	((dir)[0] != '/')
#define is_absolute(dir)	((dir)[0] == '/')
#define no_string(str)		(!(str) || !(*str))

#define MAX_MSG_SIZE		128


/* 4.x doesn't define this in sys/param.h */
#ifndef MAXNAMELEN
#define MAXNAMELEN	256
#endif


#define xv_free_ref(str)	{if((str)) {xv_free((str)); (str) = NULL;}}


EXTERN_FUNCTION(void  xv_error_sprintf, (Frame frame, int use_footer, char *format, DOTDOTDOT) );
EXTERN_FUNCTION(char *xv_strcpy, (char *dest, char *src) );
EXTERN_FUNCTION(void  xv_dirname, (char *path) );
EXTERN_FUNCTION(char *xv_dircat, (char *path, char *dir) );
EXTERN_FUNCTION(int   xv_isdir, (char *path) );
EXTERN_FUNCTION(char *xv_dirpart, (char *path) );
EXTERN_FUNCTION(char *xv_basepart, (char *path) );
EXTERN_FUNCTION(int   xv_file_exists, (char *path) );
EXTERN_FUNCTION(int   xv_stat, (char *path, struct stat *stats) );
EXTERN_FUNCTION(int   xv_access, (char *path, int mode) );
EXTERN_FUNCTION(char *xv_expand_path, (char *path) );
EXTERN_FUNCTION(char *xv_realpath, (char *path, char *resolved_name) );

#endif	/* ~xv_path_util_DEFINED */
