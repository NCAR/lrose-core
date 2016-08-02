.\" *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
.\" ** Copyright UCAR (c) 1992 - 2010 */
.\" ** University Corporation for Atmospheric Research(UCAR) */
.\" ** National Center for Atmospheric Research(NCAR) */
.\" ** Research Applications Laboratory(RAL) */
.\" ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
.\" ** 2010/10/7 23:12:36 */
.\" *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
.\" @(#)tdrp.3 1.0 97/06/25 DAG; from Dixon/Caron
.TH TDRP 3L "25 June 1997"
.SH NAME
tdrp \- table-driven runtime parameter reading library
.SH DESCRIPTION
tdrp is a standard way to read parameter files at runtime. It is based 
on dixon/yunkers's rtp design, and uses the same syntax with minor 
exceptions.

An application uses tdrp_gen to generate <mod>_tdrp.c and <mod>_tdrp.h.
It then links them and libtdrp.a into its own executable.  An
application passes the name of the parameter file to TDRP_read() at
runtime.  

All external identifiers are prepended with a "Module id", so that
an application can link in several modules that each use tdrp
independently.

.SH SYNOPSIS
.nf
#include <tdrp/tdrp.h>
#include "<mod>_tdrp.h"
<mod>_tdrp_struct struct_out;

Module specific:
    TDRPtable *<mod>_tdrp_init( <mod>_tdrp_struct *struct_out);

General routines:
    boolean TDRP_read(char *pfile, TDRPtable *table, 
                      void *struct_out, char **list);
    void TDRP_print_struct( TDRPtable *t, void *struct_out);
    void TDRP_set_defaults(TDRPtable *table, void *struct_out);
    boolean TDRP_check_is_set(TDRPtable *t, void *struct_out);
    int TDRP_check_update_time(char *in_file, 
                               void *struct_out);

Utility routines:
    void TDRP_print_params(TDRPtable *t, void *struct_in, 
                           char *module, int print_comments);
    void TDRP_init_override(tdrp_override_t *override);
    void TDRP_add_override(tdrp_override_t *override, 
                           char *override_str);
    void TDRP_free_override(tdrp_override_t *override);

.p
The override routines provide the means to set up a null-terminated 
array of strings to be used to override the entries in the 
parameter file. The basic idea is that the command line args 
may be used as overrides. Using the argv list, you may construct
override strings in the same format as the entries in the parameter
file which you wish to override.

To initialize, use TDRP_init_override and pass in a pointer to a
tdrp_override_t struct. Then, add override strings with
TDRP_add_override. When calling TDRP_read, pass in override.list as 
the (char **) list argument. To free up associated memory use 
TDRP_free_override.

.SH MAKEFILE MODIFICATIONS
.nf
- add $(TDRP_LIBS) to libraries (LOC_LIBS)
- add $(TDRP_C) to the list of source files
- add statements to include 
    /rap/make_include/rap_make_tdrp_macros and 
    /rap/make_include/rap_tdrp_make_targets
- add $(TDRP_C) to the dependencies

.p

.SH FUNCTION DESCRIPTIONS
.nf
TDRPtable *<mod>_tdrp_init( <mod>_tdrp_struct *struct_out);
    This is the module specific table initializer. It also 
    initializes the structure.

boolean TDRP_read(char *in_file, TDRPtable *table, 
                  void *struct_out, char **list);
    char *in_file: the parameter file to read in.
    TDRPtable *table: table obtained from <mod>_tdrp_init().
    void *struct_out: this is actually of type <mod>_tdrp_struct, 
                      as declared in <mod>_tdrp.h. The parameter 
                      values are placed in this structure.
    char **list: A null-terminated list of overrides to the 
                 parameter file. An override string has exactly 
                 the format of the parameter file itself. 
                 tdrp_override_t.list is suitable for this argument.
	 	
void TDRP_print_struct( TDRPtable *t, void *struct_out);
    Print out the values of the parameters in the 
    <mod>_params structure.

void TDRP_set_defaults(TDRPtable *table, void *struct_out);
    Set the parameters back to their default values. 
 
boolean TDRP_check_is_set(TDRPtable *t, void *struct_out);
    Print out warning messages if any parameters are neither 
    set nor have defaults. Return TRUE if all ok.

int TDRP_check_update_time(char *in_file, void *struct_out);
    Return 1 if the parameter file has changed since the last 
    time the parameter structure was changed.
    Return 0 if not changed.
    Return -1 if error on opening file.

int TDRP_print_params(TDRPtable *t, void *struct_in, 
                      char *module, int print_comments);
    Prints a default parameter file to standard out.  This
    can be used to create a parameter file by redirecting output.
 
void TDRP_init_override(char ***list_p);
    Create a list of override strings.

void TDRP_add_override(char ***list_p, char *override_str);
    Add an element to a list of override strings.

void TDRP_free_override(char **override_list);
    Free the override list.

    The above override functions keep a static count of strings, 
    so they should only be used on one list at a time.

.SH PARAMETER FILE SYNTAX
.nf
<parameter name> = value;
<struct or array> = { value1, value2, value3, ... };
<array of structs> = {{ value1, value2, value3, ... },
		      { value1, value2, value3, ... },
		      { value1, value2, value3, ... }}

The following styles of comments are allowable:
	
1. Lines starting with #
2. Normal C style comments /* */
3. C++ style comments ignoring everything after //

.SH PARAMETER FILE RULES
.p
Specified values must match the parameter type.

The number of values for a structure must equal the number of
fields in the structure.

Each value for an array creates a new array element. If there are no
array elements, the array pointer is null.

.SH COMMENTS

Since the table that tdrp uses is generated by a C routine that is
compiled, there is no portability problem with structure alignment.

.SH "SEE ALSO"
.BR tdrp_gen(1),
.SH "AUTHOR(S)"
.nf
Mike Dixon (John Caron, John Yunker), NCAR/RAP

.SH BUGS
.p
Simple errors in the parameter file generate "syntax error"
messages from yacc/lex.

An application cannot use lex/yacc routines for anything other than
tdrp, unless you hand modify the generated routines to eliminate
external variable conflicts.

There is memory leakage when parameter files are read in multiple times.


