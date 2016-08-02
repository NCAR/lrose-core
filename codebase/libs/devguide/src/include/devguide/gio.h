/*
 * @(#)gio.h	2.14 91/10/15 Copyright 1989 Sun Microsystems.
 *
 * This file is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify this file without charge, but are not authorized to
 * license or distribute it to anyone else except as part of a product
 * or program developed by the user.
 * 
 * THIS FILE IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * This file is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS FILE
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even
 * if Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

/*
 * GUIDE Intermediate Language (GIL) file input / output interface.
 */

#ifndef guide_gio_DEFINED
#define guide_gio_DEFINED

#include	<devguide/c_varieties.h>

/*
 * Public functions.
 */
EXTERN_FUNCTION( char	*gio_boolean_string,	(int) );
EXTERN_FUNCTION( void	gio_close_input,	(_VOID_) );
EXTERN_FUNCTION( void	gio_close_output,	(_VOID_) );
EXTERN_FUNCTION( char	*gio_comment_string,	(_VOID_) );
EXTERN_FUNCTION( char	*gio_false_string,	(_VOID_) );
EXTERN_FUNCTION( int	gio_get_boolean,	(int *) );
EXTERN_FUNCTION( int	gio_get_eof,		(_VOID_) );
EXTERN_FUNCTION( int	gio_get_file_begin,	(_VOID_) );
EXTERN_FUNCTION( int	gio_get_file_end,	(_VOID_) );
EXTERN_FUNCTION( int	gio_get_handler,	(char **) );
EXTERN_FUNCTION( int	gio_get_integer,	(int *) );
EXTERN_FUNCTION( int	gio_get_keyword,	(char **) );
EXTERN_FUNCTION( int	gio_get_list,		(char **) );
EXTERN_FUNCTION( int	gio_get_list_begin,	(_VOID_) );
EXTERN_FUNCTION( int	gio_get_list_end,	(_VOID_) );
EXTERN_FUNCTION( int	gio_get_name,		(char **) );
EXTERN_FUNCTION( int	gio_get_full_name,	(char **, char **, char **) );
EXTERN_FUNCTION( int	gio_get_proj_full_name,	(char **, char **, char **, char **) );
EXTERN_FUNCTION( int	gio_get_object_begin,	(_VOID_) );
EXTERN_FUNCTION( int	gio_get_object_end,	(_VOID_) );
EXTERN_FUNCTION( int	gio_get_string,		(char **) );
EXTERN_FUNCTION( int	gio_get_string_begin,	(_VOID_) );
EXTERN_FUNCTION( int	gio_get_string_end,	(_VOID_) );
EXTERN_FUNCTION( char	*gio_integer_string,	(int) );
EXTERN_FUNCTION( char	*gio_keyword_string,	(char *) );
EXTERN_FUNCTION( char	*gio_list_begin_string,	(_VOID_) );
EXTERN_FUNCTION( char	*gio_list_end_string,	(_VOID_) );
EXTERN_FUNCTION( char	*gio_name_string,	(char *) );
EXTERN_FUNCTION( char	*gio_object_begin_string,	(_VOID_) );
EXTERN_FUNCTION( char	*gio_object_end_string,	(_VOID_) );
EXTERN_FUNCTION( char	*gio_open_output,	(char *) );
EXTERN_FUNCTION( char   *gio_open_gil_input,	(char *) );
EXTERN_FUNCTION( char   *gio_open_gil_output,	(char *) );
EXTERN_FUNCTION( char   *gio_open_proj_input,	(char *) );
EXTERN_FUNCTION( char   *gio_open_proj_output,	(char *) );
EXTERN_FUNCTION( char   *gio_open_resfile_input,(char *) );
EXTERN_FUNCTION( void	gio_printf,		(char *, DOTDOTDOT) );
EXTERN_FUNCTION( void	gio_putc,		(char) );
EXTERN_FUNCTION( void	gio_puts,		(char *) );
EXTERN_FUNCTION( void	gio_put_boolean,	(int) );
EXTERN_FUNCTION( void	gio_put_float,		(double) );
EXTERN_FUNCTION( void	gio_put_integer,	(int) );
EXTERN_FUNCTION( void	gio_put_keyword,	(char *) );
EXTERN_FUNCTION( void	gio_put_handler,	(char *) );
EXTERN_FUNCTION( void	gio_put_name,		(char *) );
EXTERN_FUNCTION( void	gio_put_full_name,	(char *, char *, char *) );
EXTERN_FUNCTION( void	gio_put_proj_full_name,	(char *, char *, char *, char *) );
EXTERN_FUNCTION( void	gio_put_string,		(char *) );
EXTERN_FUNCTION( void	gio_put_string_to_file,	(char *) );
EXTERN_FUNCTION( void	gio_set_indent,		(int) );
EXTERN_FUNCTION( int	gio_get_indent,		(_VOID_) );
EXTERN_FUNCTION( char	*gio_string_begin_string,	(_VOID_) );
EXTERN_FUNCTION( char	*gio_string_end_string,	(_VOID_) );
EXTERN_FUNCTION( char	*gio_string_string,	(char *) );
EXTERN_FUNCTION( char	*gio_true_string,	(_VOID_) );

EXTERN_FUNCTION( int	gio_expand_path,	(char *) );
EXTERN_FUNCTION( int	gio_expand_gil_path,	(char *) );
EXTERN_FUNCTION( int	gio_expand_proj_path,	(char *) );
EXTERN_FUNCTION( int	gio_is_gil_path,	(char *) );
EXTERN_FUNCTION( int	gio_is_proj_path,	(char *) );


#endif /* ~guide_gio_DEFINED */
