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
#ifndef TDRP_GEN_WAS_INCLUDED
#define TDRP_GEN_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>

#include <tdrp/tdrp.h>

/*
 * parse the comment, param and type defs
 */

extern int parse_defs(token_handle_t *handle,
		      int ntok, tdrpToken_t *tokens,
		      TDRPtable *t_entries, int max_defs,
		      int *n_defs_p, int debug);

/*
 * ctype list
 */

extern void ctype_list_init(char *module);
extern int ctype_def_add(char *name);
extern int n_ctype_defs(void);
extern ctype_def_t *ctype_def_by_index(int index);
extern char *ctype_iname_by_index(int index);
extern char *ctype_cname_by_index(int index);
extern char *ctype_cname_by_iname(char *iname);
extern int ctype_index_by_iname(char *iname);
extern int ctype_index_by_cname(char *cname);
extern void ctype_list_free(void);
extern char *ptype2ctypeStr(TDRPtable *tt);

/*
 * enum list
 */

extern void enum_list_init(void);
extern int enum_def_add(char *name, int nfields, enum_field_t *fields);
extern int n_enum_defs(void);
extern enum_def_t *enum_def_by_index(int index);
extern enum_def_t *enum_def_by_name(char *name);
extern void enum_list_free(void);

/*
 * struct list
 */

extern void struct_list_init(void);
extern int struct_def_add(char *name, int nfields, struct_field_t *fields);
extern int n_struct_defs(void);
extern struct_def_t *struct_def_by_index(int index);
extern struct_def_t *struct_def_by_name(char *name);
extern void struct_list_free(void);

/*
 * setting items
 */

extern int set_comment_text(token_handle_t *tok_handle,
			    TDRPtable *tt,
			    tdrpToken_t *tokens, int ntok);

extern int set_bool(token_handle_t *tok_handle,
		    TDRPtable *tt,
		    tdrpToken_t *tokens, int ntok);

extern int set_int(token_handle_t *tok_handle,
		   TDRPtable *tt,
		   tdrpToken_t *tokens, int ntok);

extern int set_long(token_handle_t *tok_handle,
		    TDRPtable *tt,
		    tdrpToken_t *tokens, int ntok);

extern int set_float(token_handle_t *tok_handle,
		     TDRPtable *tt,
		     tdrpToken_t *tokens, int ntok);

extern int set_double(token_handle_t *tok_handle,
		      TDRPtable *tt,
		      tdrpToken_t *tokens, int ntok);

extern int set_string(token_handle_t *tok_handle,
		      TDRPtable *tt,
		      tdrpToken_t *tokens, int ntok);

extern int set_enum(token_handle_t *tok_handle,
		    TDRPtable *tt,
		    tdrpToken_t *tokens, int ntok);

extern int set_enum_fields(token_handle_t *tok_handle,
			   int start_tok, int end_tok,
			   tdrpToken_t *tokens, int ntok,
			   char *enum_name, char *field_label,
			   tdrpBuf *entry_buf);
  
extern int set_struct(token_handle_t *tok_handle,
		      TDRPtable *tt,
		      tdrpToken_t *tokens, int ntok);
     
extern int set_struct_fields(token_handle_t *tok_handle,
			     int start_tok, int end_tok,
			     tdrpToken_t *tokens, int ntok,
			     char *struct_name, char *field_label,
			     tdrpBuf *field_buf);

extern int set_descr(token_handle_t *tok_handle,
		     TDRPtable *tt,
		     tdrpToken_t *tokens, int ntok);

extern int set_help(token_handle_t *tok_handle,
		    TDRPtable *tt,
		    tdrpToken_t *tokens, int ntok);

extern int set_private(token_handle_t *tok_handle,
		       TDRPtable *tt,
		       tdrpToken_t *tokens, int ntok);

/*
 *  write code-generation files
 */

extern int write_h_file(const char *module,
                        const TDRPtable *t_entries,
			int n_defs, 
                        const char *prog_name,
                        const char *lib_name);

extern int write_c_file(const char *module,
                        const TDRPtable *t_entries,
			int n_defs, 
                        const char *prog_name,
                        const char *lib_name);
   
extern int write_hh_file(const char *module,
                         const TDRPtable *t_entries,
			 int n_defs,
                         const char *prog_name,
                         const char *lib_name,
                         int singleton);

extern int write_cc_file(const char *module,
                         const TDRPtable *t_entries,
			 int n_defs,
                         const char *prog_name,
                         const char *lib_name,
                         int singleton);

#ifdef __cplusplus
}
#endif

#endif
