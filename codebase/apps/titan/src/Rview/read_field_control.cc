// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
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

#include "Rview.hh"
#include <toolsa/str.h>
using namespace std;

static int tokenize_line(char *line,
			 char **description,
			 char **url,
			 int *field_num,
			 char *field_name,
			 int *time_window,
			 char **x_colorscale_name,
			 char **ps_colorscale_name,
			 double *contour_min,
			 double *contour_max,
			 double *contour_int);

void read_field_control()

{
 
  char *description, *url;
  char *x_colorscale_name, *ps_colorscale_name;
  char line[BUFSIZ];
  
  char field_name[MDV_LONG_FIELD_LEN];
  int field_num, time_window;
  
  double contour_min, contour_max, contour_int;

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
  
  Glob->fcontrol.erase(Glob->fcontrol.begin(), Glob->fcontrol.end()); 

  /*
   * read in lines, and count the number of lines which have a valid
   * format - set the number of fields accordingly
   */

  while (!feof(pfile)) {

    if (fgets(line, BUFSIZ, pfile) != NULL) {

      /*
       * substitute in environment variables
       */
      
      usubstitute_env(line, BUFSIZ);

      if (tokenize_line(line, &description, &url,
			&field_num, field_name, &time_window,
			&x_colorscale_name, &ps_colorscale_name,
			&contour_min, &contour_max, &contour_int)) {
	
	field_control_t fcont;
	
	fcont.description = description;
	fcont.url = url;
	fcont.x_colorscale_name = x_colorscale_name;
	fcont.ps_colorscale_name = ps_colorscale_name;
	
	fcont.field_num = field_num;
	STRncopy(fcont.field_name, field_name, MDV_LONG_FIELD_LEN);
	fcont.time_window = time_window;

	fcont.contour_min = contour_min;
	fcont.contour_max = contour_max;
	fcont.contour_int = contour_int;

	Glob->fcontrol.push_back(fcont);
	
      }

    } /* if (fgets... */

  } /* while */

  Glob->nfields = Glob->fcontrol.size();

  /*
   * check that the current field is valid - if not, change it
   */

  if (Glob->field >= Glob->nfields) {
    Glob->field = Glob->nfields - 1;
  }

  fclose(pfile);

}

/*************************************************************************
 * tokenize_line()
 *
 * The line should contain the following tokens:
 *
 * '#fc' - starting code
 * descritption - char *
 * URL - char *
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
			 char **url,
			 int *field_num,
			 char *field_name,
			 int *time_window,
			 char **x_colorscale_name,
			 char **ps_colorscale_name,
			 double *contour_min,
			 double *contour_max,
			 double *contour_int)

{

  char *token;

  if (strncmp(line, "#fc", 3))
    return(FALSE);
  
  if ((*description = strtok(line + 3, DELIMS)) == NULL)
    return (FALSE);
  
  if ((*url = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if ((token = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if (sscanf(token, "%d", field_num) == 1) {
    field_name[0] = '\0';
  } else {
    *field_num = -1;
    if (strlen(token) > MDV_LONG_FIELD_LEN) {
      cerr << "ERROR - Rview::read_field_control" << endl;
      cerr << "  Field name too long: " << token << endl;
      field_name[0] = '\0';
      return (FALSE);
    }
    STRncopy(field_name, token, MDV_LONG_FIELD_LEN);
  }

  if ((token = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if (sscanf(token, "%d", time_window) != 1) {
    return (FALSE);
  }

  if ((*x_colorscale_name = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if ((*ps_colorscale_name = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if ((token = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if (sscanf(token, "%lg", contour_min) != 1) {
    return (FALSE);
  }

  if ((token = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if (sscanf(token, "%lg", contour_max) != 1) {
    return (FALSE);
  }
  
  if ((token = strtok((char *) NULL, DELIMS)) == NULL)
    return (FALSE);

  if (sscanf(token, "%lg", contour_int) != 1) {
    return (FALSE);
  }

  return (TRUE);

}
