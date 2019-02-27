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
/**********************************************************************
 * uparams_etc.c
 *
 * utility routines for program parameters
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * September 1991
 *
 **********************************************************************/

#include <toolsa/umisc.h>

typedef struct {
  char *name;
  char *entry;
} param_list_t;

static int perform_read(const char *params_file_path, const char *prog_name);
static void check_memory(void);
static void finalize_memory(void);
static char *param_get(const char *search_name);

static int Nparams = 0;
static int Nallocated = 0;
static param_list_t *Plist;

#define SUCCESS 0
#define FAILURE 1
#define N_ALLOCATE 100

/**********************************************************************
 * uparams_read()
 *
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
 *
 **********************************************************************/

char *uparams_read(char **argv, int argc, char *prog_name)
{

  char *default_params_var;
  char *params_file_path;
  int i;
  int use_default = TRUE;

  /*
   * look at command line args to see if default has been overridden
   */

  for (i = 1; i < argc; i++) {

    if (!strcmp(argv[i], "-params")) {

      if (i < argc - 1) {
	params_file_path = argv[i+1];
	use_default = FALSE;
	break;
      } else {
	fprintf(stderr, "WARNING - uparams_read\n");
	fprintf(stderr,
		"File name missing after -params arg. on command line\n");
      }

    } else if (!strcmp(argv[i], "-noparams")) {

      return ((char *) NULL);

    }
  }
      
  /*
   * if '-params' not specified, use default params variable
   */

  if (use_default == TRUE) {

    /*
     * compute the default params var name from the program name
     */

    default_params_var = (char *)
      umalloc(strlen(prog_name) + strlen("_params") + 1);
    sprintf(default_params_var, "%s%s", prog_name, "_params");
    params_file_path = getenv(default_params_var);
    if (! params_file_path) {
      ufree(default_params_var);
      return ((char *) NULL);
    } 
    ufree(default_params_var);

  } /* if (use_default == TRUE) */

  /*
   * read parameters from file
   */

  if(perform_read(params_file_path, prog_name) == FAILURE) {

    fprintf(stderr, "WARNING - uparams_read\n");
    fprintf(stderr, "Cannot find parameters file '%s'.\n",
	    params_file_path);
    fprintf(stderr, "No parameters file used.\n");
    fprintf(stderr,
    "Use '-noparams' arg on command line to suppress this message.\n");

  }
  
  return params_file_path;

}

/**************************************************************************
 * perform_read()
 *
 * performs the read from the file
 *
 *************************************************************************/

static int perform_read(const char *params_file_path, const char *prog_name)
{

  FILE *params_file;

  int nline = 0;
  int previous_entry_found;
  int iparam;

  char line[BUFSIZ];
  char *name, *end_of_name;
  char *entry, *end_of_entry;
  char *colon;
  char *sptr;

  /*
   * open file
   */
  
  if ((params_file = fopen(params_file_path, "r")) == NULL)
    return(FAILURE);

  /*
   * loop through file
   */

  while (!feof(params_file)) {

    /*
     * read a line
     */

  get_next_line:

    fgets(line, BUFSIZ, params_file);
    nline++;

    if (feof(params_file))
      break;

    /*
     * substitute in any environment variables
     */

    usubstitute_env(line, BUFSIZ);

    /*
     * delete past any hash-bang
     */

    if ((sptr = strstr(line, "#!")) != NULL) {
      *sptr = '\0';
    }

    /*
     * process only if the line has the program name followed by a period.
     * otherwise go to the end of the loop
     */

    name = line;

    if (strncmp(prog_name, name, strlen(prog_name)) ||
	name[strlen(prog_name)] != '.') {

      if (!strncmp(prog_name, name, strlen(prog_name))) {
	fprintf(stderr, "WARNING - uparams_read.\n");
	fprintf(stderr, "Check file '%s'\n", params_file_path);
	fprintf(stderr, "Format error - params file line %d\n", nline);
	fprintf(stderr, "Program name must be followed by a period.\n");
      }

      goto get_next_line;

    }

    /*
     * check that there is a colon
     */

    colon = strchr(name, ':');

    if (colon == NULL) {

      fprintf(stderr, "WARNING - uparams_read.\n");
      fprintf(stderr, "Check file '%s'\n", params_file_path);
      fprintf(stderr, "Format error - params file line %d\n", nline);
      fprintf(stderr, "Resource name must be followed by a colon ':',\n");
      goto get_next_line;

    }

    /*
     * back up past any white space
     */

    end_of_name = colon - 1;

    while (*end_of_name == ' ' || *end_of_name == '\t')
      end_of_name--;

    /*
     * place null at end of name
     */

    *(end_of_name + 1) = '\0';

    /*
     * get entry string
     */

    entry = colon + 1;

    /*
     * advance past white space
     */

    while (*entry == ' ' || *entry == '\t')
      entry++;

    /*
     * back up past white space
     */

    end_of_entry = entry + strlen(entry);

    while (*end_of_entry == ' ' || *end_of_entry == '\t' ||
	   *end_of_entry == '\r' || *end_of_entry == '\n' ||
	   *end_of_entry == '\0')
      end_of_entry--;

    /*
     * place null at end of entry
     */

    *(end_of_entry + 1) = '\0';

    /*
     * check that there is enough memory allocated
     */

    check_memory();

    /*
     * check that resource has not been previously declared
     */

    previous_entry_found = FALSE;

    for (iparam = 0; iparam < Nparams; iparam++) {

      if (strcmp(Plist[iparam].name, name) == 0) {

	Plist[iparam].entry = (char *) urealloc
	  (Plist[iparam].entry, strlen(entry) + 1);

	strcpy(Plist[iparam].entry, entry);

	previous_entry_found = TRUE;

	break;

      } /* if (strcmp(Plist[iparam].name, name) == 0) */

    } /* iparam */

    /*
     * if previous entry was not found,
     * store name and entry pointers in params list
     */

    if (previous_entry_found == FALSE) {

      Plist[Nparams].name = (char *) umalloc(strlen(name) + 1);

      strcpy(Plist[Nparams].name, name);

      Plist[Nparams].entry = (char *) umalloc(strlen(entry) + 1);

      strcpy(Plist[Nparams].entry, entry);

      /*
       * increment Nparams
       */
      
      Nparams++;

    } /* if (previous_entry_found == FALSE) */

  } /* while (!feof(params_file)) */

  /*
   * tidy up memory allocation
   */

  finalize_memory();

  /*
   * close file
   */

  fclose(params_file);

  return(SUCCESS);

}

/************************************************************************
 * check_memory()
 *
 * checks that there is enough memory for another entry, and
 * adds to the allocation if necessary
 *
 ************************************************************************/

static void check_memory(void)
{

  param_list_t *p_list;
  int n_allocated;

  if (Nparams + 1 > Nallocated) {

    /*
     * allocate another block of space
     */

    n_allocated = Nallocated + N_ALLOCATE;

    p_list = (param_list_t *) umalloc(n_allocated * sizeof(param_list_t));

    if (Nallocated > 0) {
      memcpy(p_list, Plist, Nallocated * sizeof(param_list_t));
      ufree(Plist);
    } /* if (Nallocated > 0) */

    Plist = p_list;

    Nallocated = n_allocated;

  } /* if (Nparams + 1 > Nallocated) */

}

/************************************************************************
 * finalize_memory()
 *
 * finalizes the memory allocation for the param list
 *
 ************************************************************************/

static void finalize_memory(void)
{

  param_list_t *p_list;

  if (Nparams < Nallocated) {

    /*
     * allocate just the right amount of space
     */

    p_list = (param_list_t *) umalloc(Nparams * sizeof(param_list_t));
    memcpy(p_list, Plist, Nparams * sizeof(param_list_t));
    ufree(Plist);
    Plist = p_list;
    Nallocated = Nparams;

  } /* if (Nparams < Nallocated) */

}

/**************************************************************************
 * param_get()
 *
 * gets an entry from the param list
 *
 * returns NULL if this fails
 *
 ************************************************************************/

static char *param_get(const char *search_name)
{

  int iparam;

  /*
   * search for param entry which matches search name
   */

  for (iparam =  0; iparam < Nparams; iparam++) {

    if (!strcmp(search_name, Plist[iparam].name))
      return (Plist[iparam].entry);
	      
  } /* iparam */

  return ((char *) NULL);

}

/***************************************************************************
 * uGetParamDouble()
 *
 * returns the value of a double parameter, or a default
 * if this is unsuccessful
 *
 **************************************************************************/

double uGetParamDouble(const char *name, const char *param_string, double hard_def)
{

  double paramval;
  char *paramstr, *end_pt;
  char *search_str;

  search_str = (char *)
    umalloc(strlen(param_string) + strlen(name) + 2);
  sprintf(search_str, "%s.%s", name, param_string);

  if ((paramstr = param_get(search_str)) == NULL) {
    paramval = hard_def;
  } else {
    errno = 0;
    paramval = (double) strtod(paramstr, &end_pt);
    if(errno != 0)
      paramval = hard_def;
  }

  ufree((char *) search_str);
  return paramval;

}

/***************************************************************************
 * uGetParamLong()
 *
 * returns the value of a long parameter, or a default
 * if this is unsuccessful
 *
 **************************************************************************/

long uGetParamLong(const char *name, const char *param_string, long hard_def)
{

  long paramval;
  char *paramstr, *end_pt;
  char *search_str;

  search_str = (char *)
    umalloc(strlen(param_string) + strlen(name) + 2);
  sprintf(search_str, "%s.%s", name, param_string);

  if ((paramstr = param_get(search_str)) == NULL) {
    paramval = hard_def;
  } else {
    errno = 0;
    paramval = strtol(paramstr, &end_pt, 10);
    if(errno != 0)
      paramval = hard_def;
  }

  ufree((char *) search_str);
  return paramval;

}

/***************************************************************************
 * uGetParamString()
 *
 * returns the value of a string parameter, or a default
 * if this is unsuccessful
 *
 **************************************************************************/

char *uGetParamString(const char *name, const char *param_string, const char *hard_def)
{

  char *paramstr;
  char *valstr;
  char *search_str;

  search_str = (char *)
    umalloc(strlen(param_string) + strlen(name) + 2);
  sprintf(search_str, "%s.%s", name, param_string);

  if ((paramstr = param_get(search_str)) != NULL) {
    valstr = (char *) umalloc(strlen(paramstr) + 1);
    strcpy(valstr, paramstr);
  } else {
    valstr = (char *) umalloc(strlen(hard_def) + 1);
    strcpy(valstr, hard_def);
  }

  ufree((char *) search_str);

  return valstr;

}

/*****************************************************************************
 * uset_true_false_param()
 *
 * sets a parameter option with true/false choice, checking for validity
 *
 * returns 0 on success, -1 on failure
 */

int uset_true_false_param(const char *prog_name, const char *routine_name,
			  const char *params_path_name, const char *option_str,
			  int *option, const char *error_str)
{
  
  if (!strcmp(option_str, "true")) {
    *option = TRUE;
    return (0);
  } else if (!strcmp(option_str, "false")) {
    *option = FALSE;
    return (0);
  } else {
    fprintf(stderr, "ERROR - %s:%s:uset_true_false_param\n",
	    prog_name, routine_name);
    fprintf(stderr, "%s option '%s' not recognized.\n",
	    error_str, option_str);
    fprintf(stderr, "Valid options are 'true' and 'false'\n");
    fprintf(stderr, "Check params file '%s'\n", params_path_name);
    return (-1);
  }
  
}


/*****************************************************************************
 * uset_double_param()
 *
 * sets a parameter option with 2 choices, checking for validity
 *
 * returns 0 on success, -1 on failure
 */

int uset_double_param(const char *prog_name, const char *routine_name,
		      const char *params_path_name, const char *option_str,
		      int *option,
		      const char *option_str_1, int option_val_1,
		      const char *option_str_2, int option_val_2,
		      const char *error_str)
{
  
  if (!strcmp(option_str, option_str_1)) {
    *option = option_val_1;
    return (0);
  } else if (!strcmp(option_str, option_str_2)) {
    *option = option_val_2;
    return (0);
  } else {
    fprintf(stderr, "ERROR - %s:%s:uset_double_param\n",
	    prog_name, routine_name);
    fprintf(stderr, "%s option '%s' not recognized.\n",
	    error_str, option_str);
    fprintf(stderr, "Valid options are '%s' and '%s'\n",
	    option_str_1, option_str_2);
    fprintf(stderr, "Check params file '%s'\n", params_path_name);
    return (-1);
  }
  
}

/*****************************************************************************
 * uset_triple_param()
 *
 * sets a parameter option with 3 choices, checking for validity
 *
 * returns 0 on success, -1 on failure
 */

int uset_triple_param(const char *prog_name, const char *routine_name,
		      const char *params_path_name, const char *option_str,
		      int *option,
		      const char *option_str_1, int option_val_1,
		      const char *option_str_2, int option_val_2,
		      const char *option_str_3, int option_val_3,
		      const char *error_str)
{
  
  if (!strcmp(option_str, option_str_1)) {
    *option = option_val_1;
    return (0);
  } else if (!strcmp(option_str, option_str_2)) {
    *option = option_val_2;
    return (0);
  } else if (!strcmp(option_str, option_str_3)) {
    *option = option_val_3;
    return (0);
  } else {
    fprintf(stderr, "ERROR - %s:%s:uset_triple_param\n",
	    prog_name, routine_name);
    fprintf(stderr, "%s option '%s' not recognized.\n",
	    error_str, option_str);
    fprintf(stderr, "Valid options are '%s', '%s' and '%s'\n",
	    option_str_1, option_str_2, option_str_3);
    fprintf(stderr, "Check params file '%s'\n", params_path_name);
    return (-1);
  }

}

/*****************************************************************************
 * uset_quad_param()
 *
 * sets a parameter option with 4 choices, checking for validity
 *
 * returns 0 on success, -1 on failure
 */

int uset_quad_param(const char *prog_name, const char *routine_name,
		    const char *params_path_name, const char *option_str,
		    int *option,
		    const char *option_str_1, int option_val_1,
		    const char *option_str_2, int option_val_2,
		    const char *option_str_3, int option_val_3,
		    const char *option_str_4, int option_val_4,
		    const char *error_str)
{
  
  if (!strcmp(option_str, option_str_1)) {
    *option = option_val_1;
    return (0);
  } else if (!strcmp(option_str, option_str_2)) {
    *option = option_val_2;
    return (0);
  } else if (!strcmp(option_str, option_str_3)) {
    *option = option_val_3;
    return (0);
  } else if (!strcmp(option_str, option_str_4)) {
    *option = option_val_4;
    return (0);
  } else {
    fprintf(stderr, "ERROR - %s:%s:uset_quad_param\n",
	    prog_name, routine_name);
    fprintf(stderr, "%s option '%s' not recognized.\n",
	    error_str, option_str);
    fprintf(stderr, "Valid options are '%s', '%s', '%s' and '%s'\n",
	    option_str_1, option_str_2, option_str_3, option_str_4);
    fprintf(stderr, "Check params file '%s'\n", params_path_name);
    return (-1);
  }

}

/*****************************************************************************
 * uset_quin_param()
 *
 * sets a parameter option with 5 choices, checking for validity
 *
 * returns 0 on success, -1 on failure
 */

int uset_quin_param(const char *prog_name, const char *routine_name,
		    const char *params_path_name, const char *option_str,
		    int *option,
		    const char *option_str_1, int option_val_1,
		    const char *option_str_2, int option_val_2,
		    const char *option_str_3, int option_val_3,
		    const char *option_str_4, int option_val_4,
		    const char *option_str_5, int option_val_5,
		    const char *error_str)
{
  
  if (!strcmp(option_str, option_str_1)) {
    *option = option_val_1;
    return (0);
  } else if (!strcmp(option_str, option_str_2)) {
    *option = option_val_2;
    return (0);
  } else if (!strcmp(option_str, option_str_3)) {
    *option = option_val_3;
    return (0);
  } else if (!strcmp(option_str, option_str_4)) {
    *option = option_val_4;
    return (0);
  } else if (!strcmp(option_str, option_str_5)) {
    *option = option_val_5;
    return (0);
  } else {
    fprintf(stderr, "ERROR - %s:%s:uset_quin_param\n",
	    prog_name, routine_name);
    fprintf(stderr, "%s option '%s' not recognized.\n",
	    error_str, option_str);
    fprintf(stderr, "Valid options are '%s', '%s', '%s', '%s' and '%s'\n",
	    option_str_1, option_str_2, option_str_3,
	    option_str_4, option_str_5);
    fprintf(stderr, "Check params file '%s'\n", params_path_name);
    return (-1);
  }

}

/******************************************************************
 * substitute_env()
 *
 * Substitute environment variables into the line. The env variables
 * must be in the $(ENV_VAR) format
 *
 */

int usubstitute_env(char *line, int max_len)
{

  char *tmp_line;
  char *env_cpy;
  char *dollar_bracket;
  char *closing_bracket;
  char *pre_str;
  char *env_str;
  char *env_val;
  char *post_str;

  int pre_len, env_len, post_len, tot_len;

  tmp_line = (char *) umalloc(max_len);
  env_cpy = (char *) umalloc(max_len);

  memset(env_cpy, 0, max_len);

  /*
   * look for opening '$(' sequence
   */

  while ((dollar_bracket = ustrstr(line, "$(")) != NULL) {

    pre_str = line;
    env_str = dollar_bracket + 2;

    if ((closing_bracket = strchr(env_str, ')')) == NULL) {

      /*
       * no closing bracket
       */

      fprintf(stderr, "WARNING - uparams_read:substitute_env\n");
      fprintf(stderr, "No closing bracket for env variable\n");
      fprintf(stderr, "Reading '%s'", line);
      ufree(tmp_line);
      ufree(env_cpy);
      return (-1);

    } /* if ((closing_bracket = ... */

    post_str = closing_bracket + 1;
    
    /*
     * load up env string and null terminate
     */

    env_len = (int) (closing_bracket - env_str);
    strncpy(env_cpy, env_str, env_len );
    env_cpy[env_len] = '\0';

    /*
     * get env val
     */

    if ((env_val = getenv(env_cpy)) == NULL) {

      /*
       * no env variable set 
       */

      fprintf(stderr, "WARNING - uparams_read:substitute_env\n");
      fprintf(stderr, "Env variable '%s' not set\n", env_cpy);
      ufree(tmp_line);
      ufree(env_cpy);
      return (-1);

    }

    /*
     * compute total length after substitution
     */

    pre_len = (int) (dollar_bracket - pre_str);
    env_len = strlen(env_val);
    post_len = strlen(post_str);

    tot_len = pre_len + env_len + post_len + 1;

    if (tot_len > max_len) {

      /*
       * substituted string too long
       */

      fprintf(stderr, "WARNING - uparams_read:substitute_env\n");
      fprintf(stderr, "Env str too long.\n");
      fprintf(stderr, "Reading '%s'", line);
      
      ufree(tmp_line);
      ufree(env_cpy);
      return (-1);

    } /* if (tot_len > max_len) */

    /*
     * set tmp line and copy over
     */

    *dollar_bracket = '\0';
    sprintf(tmp_line, "%s%s%s", pre_str, env_val, post_str);
    ustrncpy(line, tmp_line, max_len);

  } /* while */

  ufree(tmp_line);
  ufree(env_cpy);
  return (0);

}
