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
/*********************************************************************
 * read_field_control.c
 *
 * Reads the field control parameters from the parameters file
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "rview.h"

static int substitute_env(char *line);

static int tokenize_line(char *line,
			 char **description,
			 char **type,
			 char **subtype,
			 char **instance,
			 char **host,
			 int *port,
			 si32 *field,
			 si32 *time_window,
			 char **x_colorscale_name,
			 char **ps_colorscale_name,
			 double *contour_min,
			 double *contour_max,
			 double *contour_int);

void read_field_control(void)

{
 
  char *description, *type, *subtype, *instance;
  char *default_host;
  char *x_colorscale_name, *ps_colorscale_name;
  char line[BUFSIZ];
  
  int default_port;
  int messages_flag, debug_flag;
  si32 nfields, field, time_window;
  
  double contour_min, contour_max, contour_int;

  field_control_t *fcontrol;

  FILE *pfile;

  if (Glob->debug) {
    fprintf(stderr, "** read_field_controls **\n");
  }

  /*
   * open parameters file
   */

  if((pfile = fopen(Glob->params_path_name, "r")) == NULL) {

    fprintf(stderr, "ERROR - %s:read_field_control\n", Glob->prog_name);
    fprintf(stderr, "Could not open parameters file for reading.\n");
    perror(Glob->params_path_name);
    tidy_and_exit(1);

  }
  
  /*
   * read in lines, and count the number of lines which have a valid
   * format - set the number of fields accordingly
   */

  nfields = 0;

  while (!feof(pfile)) {

    if (fgets(line, BUFSIZ, pfile) != NULL) {

      /*
       * get the field control tokens from the line
       */

      if (tokenize_line(line, &description,
			&type, &subtype, &instance,
			&default_host, &default_port,
			&field, &time_window,
			&x_colorscale_name, &ps_colorscale_name,
			&contour_min, &contour_max, &contour_int)) {
	
	nfields++;
      }

    } /* if (fgets... */

  } /* while */

  Glob->nfields = nfields;

  /*
   * check that the current field is valid - if not, change it
   */

  if (Glob->field >= nfields)
    Glob->field = nfields - 1;

  /*
   * alloc memory for the field control structs
   */

  fcontrol = (field_control_t *) ucalloc
    ((ui32) nfields, (ui32) sizeof(field_control_t));

  Glob->fcontrol = fcontrol;

  /*
   * read in file again, assigning the field control data
   */

  fseek(pfile, (si32) 0, 0);

  while (!feof(pfile)) {

    if (fgets(line, BUFSIZ, pfile) != NULL) {

      /*
       * substitute in environment variables
       */
      
      substitute_env(line);
      
      if (tokenize_line(line, &description,
			&type, &subtype, &instance,
			&default_host, &default_port,
			&field, &time_window,
			&x_colorscale_name, &ps_colorscale_name,
			&contour_min, &contour_max, &contour_int)) {
	
	fcontrol->description = (char *) umalloc
	  ((ui32) (strlen(description) + 1));
	strcpy(fcontrol->description, description);
	
	fcontrol->field = field;
	fcontrol->time_window = time_window;

	fcontrol->x_colorscale_name = (char *) umalloc
	  ((ui32) (strlen(x_colorscale_name) + 1));
	strcpy(fcontrol->x_colorscale_name, x_colorscale_name);

	fcontrol->ps_colorscale_name = (char *) umalloc
	  ((ui32) (strlen(ps_colorscale_name) + 1));
	strcpy(fcontrol->ps_colorscale_name, ps_colorscale_name);

	fcontrol->contour_min = contour_min;
	fcontrol->contour_max = contour_max;
	fcontrol->contour_int = contour_int;

	if (Glob->debug) {
	  debug_flag = TRUE;
	} else {
	  debug_flag = FALSE;
	}
	
	messages_flag = FALSE;

	cdata_init(Glob->prog_name,
		   Glob->servmap_host1,
		   Glob->servmap_host2,
		   type,
		   subtype,
		   instance,
		   default_host,
		   default_port,
		   messages_flag,
		   debug_flag,
		   &fcontrol->cdata_index);

	fcontrol++;

      }
      
    } /* if (fgets... */
    
  } /* while */

  fclose(pfile);

}

/*************************************************************************
 * tokenize_line()
 *
 * The line should contain the following tokens:
 *
 * '#fc' - starting code
 * descritption - char *
 * instance - char *
 * default port - char *
 * default port - int
 * field - si32
 * time_window - si32
 * x_colorscale_name - char *
 * ps_colorscale_name - char *
 * contour_min - double
 * contour_max - double
 * contour_int - double
 */

#define DELIMS "\t "

static int tokenize_line(char *line,
			 char **description,
			 char **type,
			 char **subtype,
			 char **instance,
			 char **host,
			 int *port,
			 si32 *field,
			 si32 *time_window,
			 char **x_colorscale_name,
			 char **ps_colorscale_name,
			 double *contour_min,
			 double *contour_max,
			 double *contour_int)

{

  char *token;
  char *end_pt;

  if (strncmp(line, "#fc", 3))
    return(FALSE);

  if ((*description = strtok(line + 3, DELIMS)) == NULL)
    return (FALSE);

  if ((*type = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if ((*subtype = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if ((*instance = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if ((*host = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if (!strcmp(*host, "local")) {
    *host = PORThostname();
  }

  if ((token = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  errno = 0;
  *port = (int) strtol(token, &end_pt, 10);
  if (errno != 0)
    return(FALSE);

  if ((token = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  errno = 0;
  *field = strtol(token, &end_pt, 10);
  if (errno != 0)
    return(FALSE);

  if ((token = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  errno = 0;
  *time_window = strtol(token, &end_pt, 10);
  if (errno != 0)
    return(FALSE);

  if ((*x_colorscale_name = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if ((*ps_colorscale_name = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if ((token = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  errno = 0;
  *contour_min = strtod(token, &end_pt);
  if (errno != 0)
    return(FALSE);

  if ((token = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  errno = 0;
  *contour_max = strtod(token, &end_pt);
  if (errno != 0)
    return(FALSE);

  if ((token = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  errno = 0;
  *contour_int = strtod(token, &end_pt);
  if (errno != 0)
    return(FALSE);

  return (TRUE);

}

/******************************************************************
 * substitute_env()
 *
 * Substitute environment variables into the line. The env variables
 * must be in the $(ENV_VAR) format
 *
 */

static int substitute_env(char *line)

{

  char tmp_line[BUFSIZ];
  char env_cpy[BUFSIZ];

  char *dollar_bracket;
  char *closing_bracket;
  char *pre_str;
  char *env_str;
  char *env_val;
  char *post_str;

  int pre_len, env_len, post_len, tot_len;

  memset ((void *)  env_cpy,
          (int) 0, (size_t) BUFSIZ);

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

      fprintf(stderr, "WARNING - %s:read_field_control:substitute_env\n",
	      Glob->prog_name);
      fprintf(stderr, "No closing bracket for env variable\n");
      fprintf(stderr, "Reading '%s'", line);
      return (-1);

    } /* if ((closing_bracket = ... */

    post_str = closing_bracket + 1;
    
    /*
     * load up env string
     */

    strncpy(env_cpy, env_str, (int) (closing_bracket - env_str));

    /*
     * get env val
     */

    if ((env_val = getenv(env_cpy)) == NULL) {

      /*
       * no env variable set 
       */

      fprintf(stderr, "WARNING - %s:read_field_control:substitute_env\n",
	      Glob->prog_name);
      fprintf(stderr, "Env variable '%s' not set\n", env_cpy);
      return (-1);

    }

    /*
     * compute total length after substitution
     */

    pre_len = (int) (dollar_bracket - pre_str);
    env_len = strlen(env_val);
    post_len = strlen(post_str);

    tot_len = pre_len + env_len + post_len + 1;

    if (tot_len > BUFSIZ) {

      /*
       * substituted string too long
       */

      fprintf(stderr, "WARNING - %s:read_field_control:substitute_env\n",
	      Glob->prog_name);
      fprintf(stderr, "Env str too long.\n");
      fprintf(stderr, "Reading '%s'", line);
      
      return (-1);

    } /* if (tot_len > BUFSIZ) */

    /*
     * set tmp line and copy over
     */

    *dollar_bracket = '\0';
    sprintf(tmp_line, "%s%s%s", pre_str, env_val, post_str);
    ustrncpy(line, tmp_line, BUFSIZ);

  } /* while */

  return (0);

}
