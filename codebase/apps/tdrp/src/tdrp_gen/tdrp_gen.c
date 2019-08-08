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
/******************************************************************
 * tdrp_gen.c
 *
 *   table driven runtime parameter generating
 *
 *    Create:
 *   	1/10/94: JCaron from John Yunker's rtp stuff
 *    Modified:
 *	1/25/94:
 *	1) <mod>_tdrp.h now includes <tdrp/tdrp.h>
 *	2) values not stored if outside min,max
 *	3) param.temp file removed
 *	4) -f option works
 *      5) explicitly declare a structure of type <mod>_tdrp_struct,
 *	and pass it into <mod>_tdrp_init() to initialize:
 *	    TDRPtable *<mod>_tdrp_init(<mod>_tdrp_struct *struct_out);	
 *    Major mod:
 *      April 1998
 *      1) removed dependency on yacc and lex.
 *      2) added many features
 *      3) added man pages.
 */

#include "tdrp_gen.h"

/*
 * file scope prototypes
 */

static void Usage(FILE *out);

/*
 * Main
 */

int main(int argc, char **argv)

{
  
  char *paramdef_path = NULL;
  char *module = "";
  char *class_name = "Params";
  char *prog_name = NULL;
  char *lib_name = NULL;
  int i;
  int ntok;
  int max_defs;
  int n_defs;
  int debug = FALSE;
  int cplusplus = FALSE;
  int singleton = FALSE;
  int fname_set = FALSE;
  tdrpToken_t *tokens;
  TDRPtable *t_entries;
  token_handle_t tok_handle;
  
  /*
   * check the command line args
   *
   * Must at least have '-f paramdef_path', so argc >= 3
   */
  
  if (argc < 3) {
    Usage(stderr);
    exit (-1);
  }

  if (argv[1][0] != '-') {
    module = argv[1];
  }

  for (i =  1; i < argc; i++) {
    
    if (!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "--")) {
      Usage(stdout);
      exit(0);
    } else if (!strcmp(argv[i], "-debug")) {
      debug = TRUE;
    } else if (!strcmp(argv[i], "-c++")) {
      cplusplus = TRUE;
    } else if (!strcmp(argv[i], "-singleton")) {
      singleton = TRUE;
    } else if (!strcmp(argv[i], "-f")) {
      if (i < argc - 1) {
	paramdef_path = argv[++i];
	fname_set = TRUE;
      } else {
	Usage(stderr);
	exit(-1);
      }
    } else if (!strcmp(argv[i], "-class")) {
      if (i < argc - 1) {
	class_name = argv[++i];
      } else {
	Usage(stderr);
	exit(-1);
      }
    } else if (!strcmp(argv[i], "-lib")) {
      if (i < argc - 1) {
	lib_name = argv[++i];
      } else {
	Usage(stderr);
	exit(-1);
      }
    } else if (!strcmp(argv[i], "-prog")) {
      if (i < argc - 1) {
	prog_name = argv[++i];
      } else {
	Usage(stderr);
	exit(-1);
      }
    }
    
  } /* i */

  if (!fname_set) {
    Usage(stderr);
    exit (-1);
  }

  /*
   * message output
   */

  if (!cplusplus) {
    fprintf(stdout, "tdrp_gen:\n");
    fprintf(stdout, "  C-mode, reading paramdef file '%s'.\n",
	    paramdef_path);
    fprintf(stdout, "  Creating files '%s_tdrp.h' and '%s_tdrp.c'.\n",
	    module, module);
  } else {
    fprintf(stdout, "tdrp_gen:\n");
    fprintf(stdout, "  C++ mode, reading paramdef file '%s'.\n",
	    paramdef_path);
    fprintf(stdout, "  Creating files '%s.hh' and '%s.cc'.\n",
	    class_name, class_name);
  }
  
  /*
   * tokenization of paramdef file
   */

  tdrpInitTokenize(&tok_handle);
  if (tdrpInitFileForTokens(&tok_handle, paramdef_path, NULL)) {
    return (-1);
  }
  tdrpTokenize(&tok_handle);
  tokens = tdrpTokens(&tok_handle);
  ntok = tdrpNtok(&tok_handle);

  /*
   * debug print
   */

  if (debug) {
    for (i = 0; i < ntok; i++) {
      if (tokens[i].is_string) {
	fprintf(stderr, "Token # %4d, line %4d: \"%s\"\n",
		i, tokens[i].line_num, tokens[i].tok);
      } else {
	fprintf(stderr, "Token # %4d, line %4d: %s\n",
		i, tokens[i].line_num, tokens[i].tok);
      }
    } /* i */
  }

  /*
   * determine number of paramdefs
   */
  
  max_defs = 0;
  for (i = 0; i < ntok; i++) {
    if (!strcmp(tokens[i].tok, "paramdef") ||
	!strcmp(tokens[i].tok, "commentdef")) {
      max_defs++;
    }
  }

  /*
   * allocate table entries
   */

  t_entries = (TDRPtable *) tdrpCalloc(max_defs, sizeof(TDRPtable));

  /*
   * initialize the lists
   */

  ctype_list_init(module);
  enum_list_init();
  struct_list_init();
  
  /*
   * parse the paramdefs
   */

  n_defs = 0;
  if (parse_defs(&tok_handle, ntok, tokens, t_entries, max_defs, &n_defs, debug)) {
    fprintf(stderr, "\n");
    fprintf(stderr, ">>>>> ERRORS FOUND - tdrp_gen <<<<<\n");
    fprintf(stderr, "Exiting.\n\n");
    return (-1);
  }

  /*
   * free up tokens
   */

  tdrpFreeTokenize(&tok_handle);

  if (cplusplus) {

    /*
     * write out C++ mode header file
     */
    
    if (write_hh_file(class_name, t_entries, n_defs, 
                      prog_name, lib_name, singleton)) {
      return (-1);
    }
    
    /*
     * write out C++ code file
     */
    
    if (write_cc_file(class_name, t_entries, n_defs,
                      prog_name, lib_name, singleton)) {
      return (-1);
    }

  } else {

    /*
     * write out C mode header file
     */
    
    if (write_h_file(module, t_entries, n_defs, prog_name, lib_name)) {
      return (-1);
    }
    
    /*
     * write out C code file
     */
    
    if (write_c_file(module, t_entries, n_defs, prog_name, lib_name)) {
      return (-1);
    }

  }

  /*
   * free the lists
   */

  ctype_list_free();
  enum_list_free();
  struct_list_free();

  /*
   * free the table
   */

  for (i = 0; i < n_defs; i++) {
    tdrpFreeEntry(t_entries + i);
  }
  tdrpFree(t_entries);
  
  fprintf(stdout, "  Done\n");

  return (0);

}

/********
 * usage()
 */

static void Usage(FILE *out)

{

  fprintf(out,
	  "Usage:\n"
	  "  tdrp_gen [moduleName] -f paramdef_path [-h] [-c++] [-debug]\n"
	  "           [-class className] [-prog progName] [-lib libName]\n"
	  "\n"
	  "where:\n"
	  "  [moduleName] in C mode all externals are prepended\n"
	  "    with this name.\n"
	  "    moduleName must be first arg if it is specified.\n"
	  "    If first arg begins with -, moduleName is set\n"
	  "    to empty string.\n"
	  "  [-f paramdef_path] parameter definition file path.\n"
	  "    This arg is REQUIRED.\n"
	  "  [-h] gives usage.\n"
	  "  [-c++] C++ mode - generates .hh and .cc class files.\n"
	  "  [-class className] In C++ mode, set the name of the params class.\n"
	  "    Default is 'Params'.\n"
	  "  [-lib libName] Library name if the params reside in a library.\n"
	  "    This ensures the includes are set correctly.\n"
	  "  [-prog progName] Program name for documenting code files.\n"
	  "  [-singleton] Create a singleton object. Only in C++ mode.\n"
	  "  [-debug] print debug messages.\n"
	  );

  fprintf(out,
	  "\n"
	  "NOTES: TDRP - Table Driven Runtime Parameters.\n"
	  "  tdrp_gen performs code generation.\n"
	  "  tdrp_gen will generate two files, one header and one for code.\n"
	  "  In C mode, the default, it will generate the files:\n"
	  "    moduleName_tdrp.h and moduleName_tdrp.c.\n"
	  "  If moduleName is left out of the command line, "
	  "the files will be:\n"
	  "    _tdrp.h and _tdrp.c.\n"
	  "  In C++ mode, it will generate the files:\n"
	  "    className.hh and classname.cc.\n"
	  "  If the -class arg is not specified, the files will be:\n"
	  "    Params.hh and Params.cc.\n"
	  );

  return;

}



