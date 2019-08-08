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

/***********************
 * TDRP private includes
 *
 * Programmers should use information in this file in their
 * programs. The file <tdrp/tdrp.h> describes the official API.
 *
 */

#ifndef TDRP_P_WAS_INCLUDED
#define TDRP_P_WAS_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

#include <sys/types.h>
#include <tdrp/tdrpbuf.h>

/*
 * define TRUE and FALSE if they are not already defined
 */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*
 * enums
 */
   
typedef enum {BOOL_TYPE,
	      STRING_TYPE,
	      INT_TYPE,
	      LONG_TYPE,
	      FLOAT_TYPE,
	      DOUBLE_TYPE,
	      ENUM_TYPE,
	      STRUCT_TYPE,
	      COMMENT_TYPE
} tableEntry_t;

/*
 * tokens for parsing
 */

typedef struct {
  int used;
  int line_num;
  int is_string;
  char *tok;
} tdrpToken_t;

/*
 * union for different field value types
 */

typedef union {

  tdrp_bool_t b;
  int i;
  long l;
  float f;
  double d;
  int e;
  char *s;
  
  /* if env vars are not expanded and we
   * cannot decode a numeric or enum value,
   * use tok for printing instead of value */

  struct {
    double spacer;
    char *tok;
  } print;
  
} tdrpVal_t;

/*
 * typedefs for dealing with ctypes, enums and structs
 */

typedef struct {
  char *iname;  /* input name */
  char *cname;  /* C name */
} ctype_def_t;

typedef struct {
  char *name;
  int val;
} enum_field_t;

typedef struct {
  char *name;
  int nfields;
  enum_field_t *fields;
} enum_def_t;

typedef struct {
  char *ftype;
  char *fname;
  int ptype;           /* param type */
  int ctype;           /* C language type  - see ctype in TDRPtable struct */
  enum_def_t enum_def; /* for enums fields only */
  int rel_offset;      /* byte offset relative to start of struct */
} struct_field_t;

typedef struct {
  char *name;
  int nfields;
  struct_field_t *fields;
} struct_def_t;

/*
 * token handle struct
 */

#define TDRP_LINE_MAX 4096

typedef struct {

  int comment_in_progress;
  int string_in_progress;

  int line_num;
  int char_pos;
  char *line_ptr;
  int reading_from_buf;

  char **override_list;
  int n_override;

  tdrpToken_t *tokens;
  int ntok;
  int ntok_alloc;

  char *buffer;
  int nbuf;
  int nbuf_alloc;

  char *read_buf;
  char *read_pos;

  char line[TDRP_LINE_MAX];
  char line_num_info[128];

  tdrpBuf *list_buf;

} token_handle_t;

/***************
 * table struct
 */

typedef struct TDRPtable {

  tableEntry_t ptype;  /* entry type */

  int ctype;           /* c language type for this entry - the ASCII
			* representation of the type is obtained
			* from the ctype_list. The types include
			* the builtins (int, long, float, double and
			* char*, and the various enums and structs whic
			* have been defined.
			*/

  const char *param_name;
 
  const char *comment_hdr;   /* comment header */
  const char *comment_text;  /* comment text */

  const char *descr;         /* description string */
  const char *help;          /* help string */

  /*
   * flag for private params - i.e. those for which the default
   * cannot be overridden by the param file
   */
  
  int is_private;

  /*
   * flag to indicate default is overridden by the param file
   */
  
  int is_set;

  /*
   * enums
   */

  enum_def_t enum_def;

  /*
   * structs
   */

  struct_def_t struct_def;

  /*
   * arrays
   */

  int is_array;         /* is an array */
  int is_array2D;       /* is a 2D array */
  int array_len_fixed;  /* fixed len array */
  int array_elem_size;  /* the size of the array elements */
  int array_n;          /* size of 1D arrays - array[n] */
  int array_n1;         /* outer size of 2D arrays - array[n1][n2] */
  int array_n2;         /* inner size of 2D arrays - array[n1][n2]  */

  /*
   * array vals - used for all arrays except structs
   */
  
  tdrpVal_t *array_vals;

  /*
   * single val - non-struct
   */
  
  tdrpVal_t single_val;

  /*
   * struct vals - used for both single and arrays of structs
   */
  
  int n_struct_vals;
  tdrpVal_t *struct_vals;

  /*
   * min, max - only for numerics (int, long, float or double)
   */
  
  int has_min;
  int has_max;
  tdrpVal_t min_val;
  tdrpVal_t max_val;

  /*
   * parameter offset in parameter struct
   */
  
  int val_offset;  /* byte offset of parameter value
		    * relative to start of params struct */

  int len_offset;  /* byte offset of array len value
		    * relative to start of struct for
		    * old-style array representation using val/len */

  int array_n_offset;   /* offset of array_n value */
  int array_n1_offset;  /* offset of array_n1 value */
  int array_n2_offset;  /* offset of array_n2 value */

  int array_offset;     /* offset to array vals */
  int array2D_offset;   /* offset to 2D array vals */

  /*
   * location in parsing token list
   */

  int start_tok;
  int end_tok;

} TDRPtable;

/*******************************************************
 * Private tdrp function prototypes - not in public API
 */

/*
 * usage
 */

extern void tdrpUsage(FILE *out);
extern int tdrpIsArgValid(const char *arg);
extern int tdrpIsArgValidN(const char *arg);

/*
 * overrides
 */

extern void tdrpInitOverride(tdrp_override_t *override);
extern void tdrpAddOverride(tdrp_override_t *override,
			    const char *override_str);
extern void tdrpFreeOverride(tdrp_override_t *override);

/*
 * loading
 */

extern int tdrpLoadFromArgs(int argc, char **argv,
			    TDRPtable *table, void *params,
			    char **override_list,
			    char **params_path_p);

extern int _tdrpLoadFromArgs(int argc, char **argv,
			     TDRPtable *table,
			     void *params,
			     char **override_list,
			     char **params_path_p,
			     const char *module,
			     int defer_exit,
			     int *exit_deferred_p);

extern int tdrpLoadApplyArgs(const char *params_path,
			     int argc, char **argv,
			     TDRPtable *table,
			     void *params,
			     char **override_list,
			     const char *module,
			     int defer_exit,
			     int *exit_deferred_p);

extern int tdrpLoad(const char *param_file_path,
		    TDRPtable *table, void *params,
		    char **override_list,
		    int expand_env, int debug);

extern int tdrpLoadFromBuf(const char *param_source_str, TDRPtable *table,
			   void *params, char **override_list,
			   const char *inbuf, int inlen,
			   int start_line_num,
			   int expand_env, int debug);
     
extern int tdrpLoadDefaults(TDRPtable *table, void *params,
			    int expand_env);

/*
 * printing
 */

extern void tdrpPrint(FILE *out, const TDRPtable *table,
		      const char *module, tdrp_print_mode_t mode);

extern char *sprintf_val(int ptype, const enum_def_t *enum_def, const tdrpVal_t *val);

/*
 * checking
 */
     
extern int tdrpCheckAllSet(FILE *out, const TDRPtable *table, const void *params);
extern int tdrpCheckIsSet(const char *paramName, const TDRPtable *table, const void *params);
extern void tdrpSetWarnOnExtraParams(int value);
extern void tdrpWarnOnExtraParams(token_handle_t *handle, const char *param_file_path,
                                  TDRPtable *table, tdrpToken_t *tokens, int ntok);

/*
 * memory allocation
 */

extern void *tdrpMalloc(size_t size);
extern void *tdrpCalloc (size_t nelem, size_t elsize);
extern void *tdrpRealloc (void *ptr, size_t size);
extern void tdrpFree(const void *ptr);
extern char *tdrpStrDup(const char *s1);
extern void tdrpStrReplace(char **s1, const char *s2);
extern char *tdrpStrNcopy(char *s1, const char *s2, int maxs1);
extern void tdrpFreeTable(TDRPtable *table);
extern void tdrpFreeEntry(TDRPtable *tt);
extern void tdrpFreeUser(TDRPtable *table, void *params);
extern void tdrpFreeAll(TDRPtable *table, void *params);
extern int tdrpArrayRealloc(TDRPtable *table, void *params,
			    const char *param_name, int new_array_n);
extern int tdrpArray2DRealloc(TDRPtable *table, void *params,
			      const char *param_name,
			      int new_array_n1, int new_array_n2);


/*
 * tokenization
 */

extern void tdrpInitTokenize(token_handle_t *handle);
extern int tdrpInitFileForTokens(token_handle_t *handle,
				 const char *file_path,
				 char **override_list);
extern void tdrpInitBufForTokens(token_handle_t *handle,
				 const char *inbuf, int buflen,
				 int start_line_num,
				 char **override_list);
extern int tdrpTokenize(token_handle_t *handle);
extern int tdrpNtok(token_handle_t *handle);
extern tdrpToken_t *tdrpTokens(token_handle_t *handle);
extern void tdrpFreeTokenize(token_handle_t *handle);
extern int tdrpReservedChar(int c);
extern int tdrpReservedStr(char *str);
extern char *tdrpLineInfo(token_handle_t *handle, const tdrpToken_t *token);

/*
 * token list
 */

extern void tdrpTokenListInit(token_handle_t *handle);
extern void tdrpTokenListReset(token_handle_t *handle);
extern void tdrpTokenListAdd(token_handle_t *handle, const tdrpToken_t *token);
extern int tdrpNTokenList(token_handle_t *handle);
extern tdrpToken_t *tdrpTokenListPtr(token_handle_t *handle);
extern void tdrpTokenListFree(token_handle_t *handle);

/*
 * conversions
 */

extern char *tdrpTableEntry2Str(int type);
extern int tdrpStr2TableEntry(const char *type_str);
extern char *tdrpFieldType2Str(int param_type);
extern void tdrpEntryType2Str(const char *module, const TDRPtable *tt, char *out_str);
extern char *tdrpType2Str(const TDRPtable *tt);
extern int tdrpLoadVal(const char *val_str, int type, tdrpVal_t *val);
extern int tdrpBoolStrTrue(const char *bool_str, int *is_true_p);
extern char *tdrpBool2Str(int b);
extern char *tdrpEnum2Str(int ioption, const TDRPtable *tt);

/*
 * reading params file
 */

extern int tdrpRead(char *in_fname, TDRPtable *table,
		    void *struct_out, char **override_list,
		    int expand_env, int debug);

extern void tdrpTable2User(TDRPtable *table, void *params);
extern void tdrpUser2Table(TDRPtable *table, void *params);

/*
 * reading items
 */

extern int tdrpReadBool(token_handle_t *handle,
			TDRPtable *tt, const char *label,
			tdrpToken_t *tokens, int ntok, int expand_env);

extern int tdrpReadInt(token_handle_t *handle,
		       TDRPtable *tt, const char *label,
		       tdrpToken_t *tokens, int ntok, int expand_env);

extern int tdrpReadLong(token_handle_t *handle,
			TDRPtable *tt, const char *label,
			tdrpToken_t *tokens, int ntok, int expand_env);
     
extern int tdrpReadFloat(token_handle_t *handle,
			 TDRPtable *tt, const char *label,
			 tdrpToken_t *tokens, int ntok, int expand_env);
     
extern int tdrpReadDouble(token_handle_t *handle,
			  TDRPtable *tt, const char *label,
			  tdrpToken_t *tokens, int ntok, int expand_env);
     
extern int tdrpReadString(token_handle_t *handle,
			  TDRPtable *tt, const char *label,
			  tdrpToken_t *tokens, int ntok, int expand_env);

extern int tdrpReadEnum(token_handle_t *handle,
			TDRPtable *tt, const char *label,
			tdrpToken_t *tokens, int ntok, int expand_env);

extern int tdrpReadStruct(token_handle_t *handle,
			  TDRPtable *tt, const char *label,
			  tdrpToken_t *tokens, int ntok, int expand_env);
   
extern int tdrpReadMin(token_handle_t *handle,
		       TDRPtable *tt, tdrpToken_t *tokens,
		       int ntok, int min_tok, int expand_env);

extern int tdrpReadMax(token_handle_t *handle,
		       TDRPtable *tt, tdrpToken_t *tokens,
		       int ntok, int max_tok, int expand_env);
     
extern int tdrpCheckValRange(TDRPtable *tt);

/*
 * finding items
 */

extern int tdrpFindLabel(int start_pos, int end_pos,
			 const tdrpToken_t *tokens, int ntok,
			 const char *item_label, int *label_pos_p);

extern int tdrpFindParamLast(int start_tok, int end_tok,
			     const tdrpToken_t *tokens, int ntok,
			     const char *param_label, int *param_tok_p);

extern int tdrpFindArrayBrackets(token_handle_t *handle,
				 int start_tok,
				 const tdrpToken_t *tokens, int ntok,
				 int *last_tok_used_p,
				 int *is_fixed_p,
				 int *fixed_len_p);

extern int tdrpFindBracesPair(token_handle_t *handle,
			      const char *label,
			      int start_tok, int end_tok,
			      const tdrpToken_t *tokens, int ntok,
			      int *n_gap_p, int *n_prs_enclosed_p,
			      int *braces_start_tok_p,
			      int *braces_end_tok_p);
     
extern int tdrpFindSingleItem(token_handle_t *handle,
			      int start_pos, int end_pos,
			      const tdrpToken_t *tokens, int ntok,
			      const char *item_label, int *item_pos_p);

extern int tdrpFindMultipleItem(token_handle_t *handle,
				int start_pos, int end_pos,
				const tdrpToken_t *tokens, int ntok,
				const char *item_label, int *label_tok_p,
				int *last_tok_p);

extern int tdrpLoadMultipleItem(token_handle_t *handle,
				int start_tok, int end_tok,
				const tdrpToken_t *tokens, int ntok,
				const char *item_label,
				char *following_token,
				int *last_tok_p);

extern int tdrpLoadStruct(token_handle_t *handle,
			  TDRPtable *tt, 
			  int start_tok, int end_tok,
			  const tdrpToken_t *tokens, int ntok,
			  const char *item_label,
			  const char *following_token,
			  int *last_tok_p);

/*
 * copy the table
 */

extern void tdrpCopyTable(const TDRPtable *source, TDRPtable *target);
  
#include <tdrp/tdrp_obsolete.h>

#ifdef __cplusplus
}
#endif

#endif /* TDRP_P_WAS_INCLUDED */
