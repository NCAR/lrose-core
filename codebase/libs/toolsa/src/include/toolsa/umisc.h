/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */

/*********
 * umisc.h
 *********/

#ifndef umisc_h
#define umisc_h

#ifdef __cplusplus
extern "C" {
#endif

/*
 * includes
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <toolsa/port.h>
#include <toolsa/mem.h>
#include <toolsa/udatetime.h>
#include <toolsa/ushmem.h>
#include <toolsa/uusleep.h>
#include <toolsa/ucopyright.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define N_BYTE_DATA_VALS 256
#define NBYTES_WORD 4

#ifndef ASSERT
#define ASSERT (test) if (test) {fprintf(stderr, \
                                 "ASSERT failed, line %d, file %s\n", \
                                 __LINE__, __FILE__); \
                                 kill(getpid(), SIGSEGV);}
#endif

/*
 * path parts structure type - used for path elements
 */

typedef struct {
  char *dir;
  char *name;
  char *base;
  char *ext;
} path_parts_t;

/*
 * function prototypes
 */

/***************************************************************
 * udaemonize.c
 */

/* udaemonize()
 * Transform calling process into a daemon by disassociating it
 * from the controlling terminal.
 */

extern void udaemonize(void);

/***************************************************************
 * ufread.c
 */

/* ufread()
 * wrapper for fread - takes care of interrupted read
 */

extern int ufread(void *ptr, int size, int nitems, FILE *stream);

/***************************************************************
 * ufree_parsed_path.c
 */

/* ufree_parsed_path()
 * frees file path elements
 */

extern void ufree_parsed_path(path_parts_t *parts);

/***************************************************************
 * ufwrite.c
 */

/* ufwrite()
 * wrapper for fwrite - takes care of interrupted write
 */

extern int ufwrite(void *ptr, int size, int nitems, FILE *stream);

/***************************************************************
 * uparams_etc.c
 */

/* uparams_read()
 * reads param file into parameter data base
 *
 * The parameters file has the same format as a simple Xdefaults
 * file. A typical entry would be:
 *
 * rview.plot_forecast: true
 *
 * where rview is the program name
 *       plot_forecast is the resource name
 *       true is the resource value
 *
 * returns param_file_path, NULL is no relevant file
 */

extern char *uparams_read(char **argv, int argc, char *prog_name);

/* uGetParamDouble()
 * returns the value of a double parameter, or a default
 * if this is unsuccessful
 */

extern double uGetParamDouble(const char *name, const char *param_string, double hard_def);

/* uGetParamLong()
 * returns the value of a long parameter, or a default
 * if this is unsuccessful
 */

extern long uGetParamLong(const char *name, const char *param_string, long hard_def);

/* uGetParamString()
 * returns the value of a string parameter, or a default
 * if this is unsuccessful
 */

extern char *uGetParamString(const char *name, const char *param_string, const char *hard_def);

/* uset_true_false_param()
 * sets a parameter option with true/false choice, checking for validity
 *
 * returns 0 on success, -1 on failure
 */

extern int uset_true_false_param(const char *prog_name, const char *routine_name,
				 const char *params_path_name, const char *option_str,
				 int *option, const char *error_str);

/* uset_double_param()
 * sets a parameter option with 2 choices, checking for validity
 *
 * returns 0 on success, -1 on failure
 */

extern int uset_double_param(const char *prog_name, const char *routine_name,
			     const char *params_path_name, const char *option_str,
			     int *option,
			     const char *option_str_1, int option_val_1,
			     const char *option_str_2, int option_val_2,
			     const char *error_str);

/* uset_triple_param()
 * sets a parameter option with 3 choices, checking for validity
 *
 * returns 0 on success, -1 on failure
 */

extern int uset_triple_param(const char *prog_name, const char *routine_name,
			     const char *params_path_name, const char *option_str,
			     int *option,
			     const char *option_str_1, int option_val_1,
			     const char *option_str_2, int option_val_2,
			     const char *option_str_3, int option_val_3,
			     const char *error_str);

/* uset_quad_param()
 * sets a parameter option with 4 choices, checking for validity
 *
 * returns 0 on success, -1 on failure
 */

extern int uset_quad_param(const char *prog_name, const char *routine_name,
			   const char *params_path_name, const char *option_str,
			   int *option,
			   const char *option_str_1, int option_val_1,
			   const char *option_str_2, int option_val_2,
			   const char *option_str_3, int option_val_3,
			   const char *option_str_4, int option_val_4,
			   const char *error_str);

/* uset_quin_param()
 *
 * sets a parameter option with 5 choices, checking for validity
 *
 * returns 0 on success, -1 on failure
 */

extern int uset_quin_param(const char *prog_name, const char *routine_name,
			   const char *params_path_name, const char *option_str,
			   int *option,
			   const char *option_str_1, int option_val_1,
			   const char *option_str_2, int option_val_2,
			   const char *option_str_3, int option_val_3,
			   const char *option_str_4, int option_val_4,
			   const char *option_str_5, int option_val_5,
			   const char *error_str);

/* usubstitute_env()
 * Substitute environment variables into the line. The env variables
 * must be in the $(ENV_VAR) format
 */

extern int usubstitute_env(char *line, int max_len);

/***************************************************************
 * uparse_path.c
 */

/*
 * parses file path, returns the basic elements
 */

extern void uparse_path(const char *path, path_parts_t *parts);

/***************************************************************
 * ustring.c
 */

/* uinsert_crs()
 * inserts carriage returns into a string at the
 * blank space immediately preceding the end of line. Formatting for
 * output.
 */

extern char *uinsert_crs(char *string, int line_length);

/* urem_wspace()
 * removes extra tabs and spaces from a string.
 * leaving only one space as a delimiter.
 *
 * returns pointer to the string if successful, NULL if only spaces and
 * tabs in the string.
 */

extern char *urem_wspace(char *string);

/* ustrdelim()
 *  returns a pointer to a string delimited by the delimiting
 *  constant
 *
 * (char *) strdelim( char *nptr, char **endptr, int delim);
 *
 * strdelim searches the string pointed to by nptr for the first
 * occurrence of the delimiter. It returns a pointer to the character
 * after the delimiter.
 * The second occurrence of the delimiter is overwritten by a NULL.
 * A pointer to the first character after the string is assigned to
 * endptr
 *
 * Returns NULL if delimited string is not found
 */

extern char *ustrdelim(char *nptr, char **endptr, int delim);

/* ustrstr()
 * this is an implementation of the ANSI standard strstr
 * function which is missing from the Unix package
 *
 * char *strstr(char *s1, char *s2)
 *
 * strstr locates the first occurrence of s2 (not including the terminating
 * null character) in the string s1. It returns a pointer to the string
 * located in s1, or a null pointer if no match is found.
 */

extern char *ustrstr(char *s1, char *s2);

/* ustr_token()
 * Returns pointer to the original string where the token was found,
 * and NULL if no token was found.
 * str_ptr is updated past the token, or set to NULL
 * when theres nothing more to scan.
 *
 * char  **str_ptr;     current point in the string to parse
 * char  *token;        put the found token here
 * int   max_toksiz;    maximum size of token (include zero byte)
 * char  *delim;        delimiter for tokens
 */

extern char *ustr_token(char **str_ptr,
			char *token,
			int max_toksiz,
			char *delim);

/* ustr_concat()
 * Concats two strings to a given max length
 */

extern char *ustr_concat(char *s1,
			 const char *s2,
			 int maxs1);

/* ustr_clear_to_end()
 * Clears chars after the first null char, to given limit.
 * Forces a null termination
 */

extern void ustr_clear_to_end(char *str, int maxlen);

/* ustrtola()
 * takes
 *  lngstr, which is assumed to be a string consisting of digits
 *  separated by whitespace (newlines, spaces, etc.), and
 *  converts it into an array of long ints (whose size is
 *  given by "size") with a starting address at lngarr.
 *
 *  strtola returns size if it executes without errors, -1 otherwise.
 *
 *  Preconditions: "lngstr" has already been initialized.
 *    "size" is equal to the number of integers to be
 *    contained in the array, or 0 if that number is
 *    unknown.
 *
 *  Postconditions: "lngarr" points to the first element of the si32
 *    array.
 */

extern int ustrtola (char *lngstr, long **lngarr_P, int size);

/* ustrncpy()
 * copies string to max length, and ensures NULL termination
 */

extern char *ustrncpy(char *s1, const char *s2, int maxs1);

/* uGetLong()
 * Decodes the next long off a string, advancing the pointer
 * to the next token
 *
 * Returns 0 on success, -1 on failure
 */

extern long uGetLong(char **ptr, long *lval);

/* uGetDouble()
 * Decodes the next double off a string, advancing the pointer
 * to the next token
 *
 * Returns 0 on success, -1 on failure
 */

extern int uGetDouble(char **ptr, double *dval);

/***************************************************************
 * usystem_call.c
 *
 * The usystem_call group of functions perform the equivalent of the
 * 'system' function, without the overhead of duplicating the entire
 * program memory.
 */

/* usystem_call_init()
 * sets up a child process for the execution of the system function
 *
 * returns 0 on success, -1 on failure
 */

extern int usystem_call_init(void);

/* usystem_call()
 * sends the system call to the child process, waits for the
 * done message to come back
 */

extern void usystem_call(char *call_str);

/* usystem_call_clean()
 * kills the child
 */

extern void usystem_call_clean(void);

/**********************************************************************
 * SAFE_SYS. Safely execute a command, susch that it will be killed if it
 * doesn't exit with the timeout_seconds
 
 * Returns 0 on Success, -1 : on command error,
 *                       -2 : Child Timed out
 *                       -3 : Couldn't kill the child
 */
 
int safe_system(const char * command, int timeout_seconds);


#ifdef __cplusplus
}
#endif

#endif
