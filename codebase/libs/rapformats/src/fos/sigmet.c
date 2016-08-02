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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 18:45:40 $
 *   $Id: sigmet.c,v 1.7 2016/03/03 18:45:40 dixon Exp $
 *   $Revision: 1.7 $
 *   $State: Exp $
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * sigmet.c: Routines to manipulate SIGMET data.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>

#include <toolsa/os_config.h>

#include <dataport/bigend.h>
#include <dataport/port_types.h>

#include <toolsa/globals.h>
#include <toolsa/mem.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>

#include <rapformats/fos.h>

#include <toolsa/toolsa_macros.h>


/*
 * Local constants
 */

static char *module_name = "rapformats";

#define MAX_TOKENS      128
#define MAX_TOKEN_LEN    80

#define MAX_SLOPE   1000000.0

static char *sigmet_regions[] =
{
    "BOS",
    "CHI",
    "MIA",
    "DFW",
    "SLC",
    "SFO",
    "JNU",
    "MKC"
};

static int num_sigmet_regions = 8;

/*
 * Global variables
 */

static char *sigmet_tokens[MAX_TOKENS];
static int sigmet_tokens_allocated = FALSE;

/*
 * Forward declarations for static functions.
 */

static time_t get_sigmet_start(WXADV_text_t *sigmet, time_t sigmet_time,
			       char *region,
                               SIGMET_report_type_t *report_type,
                               char *forecast_letter);

static time_t get_sigmet_end(WXADV_text_t *sigmet, int *curr_line,
                             time_t start_time);

static SIGMET_vertex_t *get_sigmet_vertices(WXADV_text_t *sigmet,
					    WXADV_fix_t *fixes, int num_fixes,
					    int *curr_line,
					    int *num_vertices,
					    SIGMET_box_t *bounding_box);

static void offset_vertex(double *lat, double *lon,
			  char *range_bearing);


/************************************************************************
 * SIGMET_box_from_BE() - Convert the SIGMET box information from big-
 *                         endian format to native format.
 */

void SIGMET_box_from_BE(SIGMET_box_t *box)
{
  BE_from_array_32(&box->min_lat, 4);
  BE_from_array_32(&box->min_lon, 4);
  BE_from_array_32(&box->max_lat, 4);
  BE_from_array_32(&box->max_lon, 4);
  
  return;
}


/************************************************************************
 * SIGMET_box_to_BE() - Convert the SIGMET box information from native
 *                      format to big-endian format.
 */

void SIGMET_box_to_BE(SIGMET_box_t *box)
{
  BE_from_array_32(&box->min_lat, 4);
  BE_from_array_32(&box->min_lon, 4);
  BE_from_array_32(&box->max_lat, 4);
  BE_from_array_32(&box->max_lon, 4);
  
  return;
}


/************************************************************************
 * SIGMET_decoded_to_spdb() - Convert a decoded SIGMET to the SPDB format.
 */

SIGMET_spdb_t *SIGMET_decoded_to_spdb(WXADV_text_t *sigmet_text,
				      SIGMET_decoded_t *decoded_sigmet)
{
  SIGMET_spdb_t *spdb_sigmet = NULL;
  int text_len;
  int text_offset;
  int sigmet_len;
  int line;
  int vertex;
  char *text_ptr;
  
  /*
   * Calculate the total length of the SIGMET text.  Make sure the
   * text length is a multiple of 4 to help avoid bus errors.
   */

  text_len = 0;
  for (line = 0; line < sigmet_text->num_lines; line++)
    text_len += strlen(sigmet_text->lines[line])
      + 1;                                      /* add CR at end of lines */

  text_len++;      /* NULL terminated */
  
  if (text_len % 4 != 0)
    text_len += 4 - (text_len % 4);
  
  /*
   * Allocate space for the returned structure.
   */

  text_offset = sizeof(SIGMET_spdb_t) +
    (decoded_sigmet->num_vertices - 1) * sizeof(SIGMET_vertex_t);
  
  sigmet_len = text_offset + text_len;
  
  spdb_sigmet = (SIGMET_spdb_t *)umalloc(sigmet_len);
  
  /*
   * Update the header information.
   */

  STRcopy((char *) spdb_sigmet->sigmet_id, decoded_sigmet->sigmet_id,
	  SIGMET_ID_LEN);
  spdb_sigmet->sigmet_type     = decoded_sigmet->sigmet_type;
  spdb_sigmet->report_type     = decoded_sigmet->report_type;
  spdb_sigmet->cancel_flag     = decoded_sigmet->cancel_flag;
  spdb_sigmet->start_time      = decoded_sigmet->start_time;
  spdb_sigmet->end_time        = decoded_sigmet->end_time;
  STRcopy((char *) spdb_sigmet->forecast_region, decoded_sigmet->forecast_region,
	  FORECAST_REGION_LEN);
  spdb_sigmet->forecast_letter = decoded_sigmet->forecast_letter;
  spdb_sigmet->bounding_box    = decoded_sigmet->bounding_box;
  spdb_sigmet->num_vertices    = decoded_sigmet->num_vertices;
  spdb_sigmet->text_len        = text_len;
  spdb_sigmet->text_offset     = sizeof(SIGMET_spdb_t) +
    (decoded_sigmet->num_vertices - 1) * sizeof(SIGMET_vertex_t);
  
  /*
   * Copy the vertices.
   */

  for (vertex = 0; vertex < decoded_sigmet->num_vertices; vertex++)
    spdb_sigmet->vertices[vertex] = decoded_sigmet->vertices[vertex];
  
  /*
   * Copy the text.
   */

  text_ptr = (char *)spdb_sigmet + text_offset;
  
  for (line = 0; line < sigmet_text->num_lines; line++)
  {
    int string_len = strlen(sigmet_text->lines[line]);
    
    STRcopy(text_ptr, sigmet_text->lines[line],
	    string_len + 1);    /* include the NULL */
    text_ptr[string_len] = '\n';
    text_ptr += string_len + 1;
  } /* endfor - line */
  
  return(spdb_sigmet);
}


/************************************************************************
 * SIGMET_print_box() - Print the SIGMET box information to the indicated
 *                      stream in ASCII format.
 */

void SIGMET_print_box(FILE *stream, SIGMET_box_t *box)
{
  fprintf(stream, "\nBox information:\n");
  fprintf(stream, "   min lat = %f\n", box->min_lat);
  fprintf(stream, "   min lon = %f\n", box->min_lon);
  fprintf(stream, "   max lat = %f\n", box->max_lat);
  fprintf(stream, "   max lon = %f\n", box->max_lon);
  fprintf(stream, "\n");
  
  return;
}


/************************************************************************
 * SIGMET_print_decoded() - Print the decoded SIGMET information to the
 *                          indicated stream in ASCII format.
 */

void SIGMET_print_decoded(FILE *stream, SIGMET_decoded_t *sigmet)
{
  int i;
  
  fprintf(stream, "\nSIGMET info:\n");
  fprintf(stream, "   sigmet id = %s\n", sigmet->sigmet_id);
  fprintf(stream, "   sigmet_type = %s\n",
	  SIGMET_type_to_string(sigmet->sigmet_type));
  fprintf(stream, "   report type = %s\n",
	  SIGMET_report_type_to_string(sigmet->report_type));
  fprintf(stream, "   cancel flag = %d\n", sigmet->cancel_flag);
  fprintf(stream, "   start time = %s", ctime(&sigmet->start_time));
  fprintf(stream, "   end time = %s", ctime(&sigmet->end_time));
  fprintf(stream, "   forecast region = %s\n", sigmet->forecast_region);
  fprintf(stream, "   forecast letter = %c\n", sigmet->forecast_letter);

  SIGMET_print_box(stream, &sigmet->bounding_box);

  fprintf(stream, "   num vertices = %d\n", sigmet->num_vertices);

  for (i = 0; i < sigmet->num_vertices; i++)
  {
    fprintf(stream, "\nvertex #%d:\n", i);
    SIGMET_print_vertex(stream, &sigmet->vertices[i]);
  }
  
  fprintf(stream, "\n");
  
  return;
}


/************************************************************************
 * SIGMET_print_spdb() - Print the SPDB SIGMET information to the
 *                       indicated stream in ASCII format.
 */

void SIGMET_print_spdb(FILE *stream, SIGMET_spdb_t *sigmet)
{
  time_t print_time;
  int i;
  char *text_ptr;
  
  fprintf(stream, "\nSIGMET info:\n");
  fprintf(stream, "   sigmet id = %s\n", sigmet->sigmet_id);
  fprintf(stream, "   sigmet_type = %s\n",
	  SIGMET_type_to_string(sigmet->sigmet_type));
  fprintf(stream, "   report type = %s\n",
	  SIGMET_report_type_to_string(sigmet->report_type));
  fprintf(stream, "   cancel flag = %d\n", sigmet->cancel_flag);

  print_time = sigmet->start_time;
  fprintf(stream, "   start time = %s", ctime(&print_time));

  print_time = sigmet->end_time;
  fprintf(stream, "   end time = %s", ctime(&print_time));

  fprintf(stream, "   forecast region = %s\n", sigmet->forecast_region);
  fprintf(stream, "   forecast letter = %c\n", sigmet->forecast_letter);

  SIGMET_print_box(stream, &sigmet->bounding_box);

  fprintf(stream, "   num vertices = %d\n", sigmet->num_vertices);
  fprintf(stream, "   text len = %d\n", sigmet->text_len);
  fprintf(stream, "   text offset = %d\n", sigmet->text_offset);
  
  for (i = 0; i < sigmet->num_vertices; i++)
  {
    fprintf(stream, "\nvertex #%d:\n", i);
    SIGMET_print_vertex(stream, &sigmet->vertices[i]);
  }
  
  text_ptr = (char *)sigmet + sigmet->text_offset;
  
  fprintf(stream, "Text:\n");
  fprintf(stream, "%s", text_ptr);
  
  fprintf(stream, "\n");
  
  return;
}


/************************************************************************
 * SIGMET_print_vertex() - Print the SIGMET vertex information to the
 *                         indicated stream in ASCII format.
 */

void SIGMET_print_vertex(FILE *stream, SIGMET_vertex_t *vertex)
{
  fprintf(stream, "   lat = %f\n", vertex->lat);
  fprintf(stream, "   lon = %f\n", vertex->lon);
  fprintf(stream, "   slope = %f\n", vertex->slope);
  fprintf(stream, "   intercept = %f\n", vertex->intercept);
  
  return;
}


/************************************************************************
 * SIGMET_report_type_to_string() - Convert the SIGMET report type to a
 *                                  string, generally for printing.
 */

char *SIGMET_report_type_to_string(SIGMET_report_type_t report_type)
{
  switch (report_type)
  {
  case SIGMET_REPORT_NEW :
    return("SIGMET_REPORT_NEW");
    
  case SIGMET_REPORT_CORRECTION :
    return("SIGMET_REPORT_CORRECTION");
    
  case SIGMET_REPORT_AMENDMENT :
    return("SIGMET_REPORT_AMENDMENT");
  }
  
  return("** INVALID REPORT TYPE **");
}


/************************************************************************
 * SIGMET_spdb_from_BE() - Convert the SPDB SIGMET information from big-
 *                         endian format to native format.
 */

void SIGMET_spdb_from_BE(SIGMET_spdb_t *sigmet)
{
  int i;
  
  /* sigmet_id is okay */
  sigmet->sigmet_type  = BE_to_si32(sigmet->sigmet_type);
  sigmet->report_type  = BE_to_si32(sigmet->report_type);
  sigmet->cancel_flag  = BE_to_si32(sigmet->cancel_flag);
  sigmet->start_time   = BE_to_si32(sigmet->start_time);
  sigmet->end_time     = BE_to_si32(sigmet->end_time);
  /* forecast_region is okay */
  /* forecast_letter is okay */
  /* forecast_alignment is okay */
  SIGMET_box_from_BE(&sigmet->bounding_box);
  sigmet->num_vertices = BE_to_si32(sigmet->num_vertices);
  sigmet->text_len     = BE_to_si32(sigmet->text_len);
  sigmet->text_offset  = BE_to_si32(sigmet->text_offset);
  
  for (i = 0; i < sigmet->num_vertices; i++)
    SIGMET_vertex_from_BE(&sigmet->vertices[i]);
  
  /* text is okay */

  return;
}


/************************************************************************
 * SIGMET_spdb_to_BE() - Convert the SPDB SIGMET information from native
 *                       format to big-endian format.
 */

void SIGMET_spdb_to_BE(SIGMET_spdb_t *sigmet)
{
  int i;
  int num_vertices = sigmet->num_vertices;
  
  /* sigmet_id is okay */
  sigmet->sigmet_type  = BE_from_si32(sigmet->sigmet_type);
  sigmet->report_type  = BE_from_si32(sigmet->report_type);
  sigmet->cancel_flag  = BE_from_si32(sigmet->cancel_flag);
  sigmet->start_time   = BE_from_si32(sigmet->start_time);
  sigmet->end_time     = BE_from_si32(sigmet->end_time);
  /* forecast_region is okay */
  /* forecast_letter is okay */
  /* forecast_alignmet is okay */
  SIGMET_box_to_BE(&sigmet->bounding_box);
  sigmet->num_vertices = BE_from_si32(sigmet->num_vertices);
  sigmet->text_len     = BE_from_si32(sigmet->text_len);
  sigmet->text_offset  = BE_from_si32(sigmet->text_offset);
  
  for (i = 0; i < num_vertices; i++)
    SIGMET_vertex_to_BE(&sigmet->vertices[i]);
  
  /* text is okay */

  return;
}


/************************************************************************
 * SIGMET_type_to_string() - Convert the SIGMET type to a string,
 *                           generally for printing.
 */

char *SIGMET_type_to_string(SIGMET_type_t type)
{
  switch (type)
  {
  case SIGMET_TYPE_UNKNOWN :
    return("SIGMET_TYPE_UNKNOWN");
    
  case SIGMET_UNPARSABLE :
    return("SIGMET_UNPARSABLE");
    
  case SIGMET_CONVECTIVE :
    return("SIGMET_CONVECTIVE");

  case SIGMET_TURB :
    return("SIGMET_TURB");
    
  case SIGMET_ICING :
    return("SIGMET_ICING");
    
  case SIGMET_MISC :
    return("SIGMET_MISC");
  }
  
  return("** INVALID SIGMET TYPE **");
}


/************************************************************************
 * SIGMET_vertex_from_BE() - Convert the SIGMET vertex information from big-
 *                         endian format to native format.
 */

void SIGMET_vertex_from_BE(SIGMET_vertex_t *vertex)
{
  BE_from_array_32(&vertex->lat, 4);
  BE_from_array_32(&vertex->lon, 4);
  BE_from_array_32(&vertex->slope, 4);
  BE_from_array_32(&vertex->intercept, 4);
  
  return;
}


/************************************************************************
 * SIGMET_vertex_to_BE() - Convert the SIGMET vertex information from
 *                         native format to big-endian format.
 */

void SIGMET_vertex_to_BE(SIGMET_vertex_t *vertex)
{
  BE_from_array_32(&vertex->lat, 4);
  BE_from_array_32(&vertex->lon, 4);
  BE_from_array_32(&vertex->slope, 4);
  BE_from_array_32(&vertex->intercept, 4);
  
  return;
}


/************************************************************************
 * SIGMET_wxadv_to_decoded() - Converts a textual SIGMET stored in the
 *                             WXADV_text_t format into an array of
 *                             decoded SIGMETS.
 */

SIGMET_decoded_t *SIGMET_wxadv_to_decoded(WXADV_text_t *sigmet,
					  time_t sigmet_date,
					  WXADV_fix_t *fixes,
					  int num_fixes,
					  int *num_sigmets)
{
  SIGMET_decoded_t *return_buffer = NULL;
    
  time_t   start_time;
  char     region_name[FORECAST_REGION_LEN];
  SIGMET_report_type_t report_type;
  char          forecast_letter;
    
  int i, j;
    
  /*
   * Initialize the return values.
   */

  *num_sigmets = 0;
    
  /*
   * Initialize the line pointer in the textual SIGMET structure.
   */

  sigmet->curr_line = 0;
    
  /*
   * Get the start time for this group of SIGMETs.
   */

  if ((start_time = get_sigmet_start(sigmet, sigmet_date, region_name,
				     &report_type, &forecast_letter))
      <= 0)
  {
    if (return_buffer != NULL)
    {
      for (j = 0; j < *num_sigmets; j++)
	ufree(return_buffer[j].vertices);
      ufree(return_buffer);
      return_buffer = NULL;
    }
    return(return_buffer);
  }
  

  /*
   * Process all of the SIGMETs in the text buffer.
   */

  for (i = sigmet->curr_line; i < sigmet->num_lines; /* NULL EXPRESSION */)
  {
    /*
     * Check for the beginning of an SIGMET.
     */

    if (strncmp(sigmet->lines[i],
		"CONVECTIVE SIGMET",
		strlen("CONVECTIVE SIGMET")) == 0)
    {
      /*
       * Check for "non-existant" SIGMET.
       */

      if (strstr(sigmet->lines[i], "NONE") != NULL)
      {
	if (return_buffer != NULL)
	{
	  for (j = 0; j < *num_sigmets; j++)
	    ufree(return_buffer[j].vertices);
	  ufree(return_buffer);
	  return_buffer = NULL;
	}
	return(return_buffer);
      }
            
      /*
       * Allocate space for the return buffer.
       */

      if (return_buffer == (SIGMET_decoded_t *)NULL)
      {
	*num_sigmets = 1;
	return_buffer =
	  (SIGMET_decoded_t *)umalloc(sizeof(SIGMET_decoded_t));
      }
      else
      {
	(*num_sigmets)++;
	return_buffer =
	  (SIGMET_decoded_t *)urealloc(return_buffer,
				       *num_sigmets *
				       sizeof(SIGMET_decoded_t));
      }
            
      /*
       * Fill decoded SIGMET structure with default values.
       */

      return_buffer[*num_sigmets-1].sigmet_id[0] = '\0';
      return_buffer[*num_sigmets-1].report_type = report_type;
      return_buffer[*num_sigmets-1].cancel_flag = FALSE;
      return_buffer[*num_sigmets-1].sigmet_type = SIGMET_CONVECTIVE;
      return_buffer[*num_sigmets-1].start_time = start_time;
      return_buffer[*num_sigmets-1].end_time = 0;
      STRcopy(return_buffer[*num_sigmets-1].forecast_region,
	      region_name, FORECAST_REGION_LEN);
      return_buffer[*num_sigmets-1].forecast_letter = forecast_letter;
      return_buffer[*num_sigmets-1].num_vertices = 0;
      return_buffer[*num_sigmets-1].vertices = (SIGMET_vertex_t *)NULL;
      return_buffer[*num_sigmets-1].bounding_box.min_lat = -1;
      return_buffer[*num_sigmets-1].bounding_box.min_lon = -1;
      return_buffer[*num_sigmets-1].bounding_box.max_lat = -1;
      return_buffer[*num_sigmets-1].bounding_box.max_lon = -1;

      i++;
      continue;
    }
    else if (strncmp(sigmet->lines[i],
		     "SIGMET",
		     strlen("SIGMET")) == 0)
    {
      /*
       * Allocate space for the return buffer.
       */

      if (return_buffer == (SIGMET_decoded_t *)NULL)
      {
	*num_sigmets = 1;
	return_buffer = 
	  (SIGMET_decoded_t *)umalloc(sizeof(SIGMET_decoded_t));
      }
      else
      {
	(*num_sigmets)++;
	return_buffer =
	  (SIGMET_decoded_t *)urealloc(return_buffer,
				       *num_sigmets *
				       sizeof(SIGMET_decoded_t));
      }
            
      /*
       * Fill decoded SIGMET structure with default values.
       */

      return_buffer[*num_sigmets-1].sigmet_id[0] = '\0';
      return_buffer[*num_sigmets-1].report_type = report_type;
      return_buffer[*num_sigmets-1].cancel_flag = FALSE;
      return_buffer[*num_sigmets-1].sigmet_type = SIGMET_MISC;
      return_buffer[*num_sigmets-1].start_time = start_time;
      return_buffer[*num_sigmets-1].end_time = 0;
      STRcopy(return_buffer[*num_sigmets-1].forecast_region,
	      region_name, FORECAST_REGION_LEN);
      return_buffer[*num_sigmets-1].forecast_letter = forecast_letter;
      return_buffer[*num_sigmets-1].num_vertices = 0;
      return_buffer[*num_sigmets-1].vertices = (SIGMET_vertex_t *)NULL;
      return_buffer[*num_sigmets-1].bounding_box.min_lat = -1;
      return_buffer[*num_sigmets-1].bounding_box.min_lon = -1;
      return_buffer[*num_sigmets-1].bounding_box.max_lat = -1;
      return_buffer[*num_sigmets-1].bounding_box.max_lon = -1;
      
      i++;
      continue;
    }
    else if (strncmp(sigmet->lines[i],
		     "PAZA SIGMET",
		     strlen("PAZA SIGMET")) == 0)
    {
      if (return_buffer != NULL)
      {
	for (j = 0; j < *num_sigmets; j++)
	  if (return_buffer[j].vertices != (SIGMET_vertex_t *)NULL)
	    ufree(return_buffer[j].vertices);
                
	ufree(return_buffer);
	return_buffer = NULL;
      }
            
      return(return_buffer);
    }
        
    if (*num_sigmets <= 0)
    {
      i++;
      continue;
    }
        
    if (strncmp(sigmet->lines[i], "VALID UNTIL",
		strlen("VALID UNTIL")) == 0)
    {
      return_buffer[*num_sigmets-1].end_time =
	get_sigmet_end(sigmet, &i,
		       return_buffer[*num_sigmets-1].start_time);
      continue;
    }

    /*
     * Get the vertices for the SIGMET polygon.
     */

    if (return_buffer[*num_sigmets-1].num_vertices <= 0 &&
	strncmp(sigmet->lines[i], "FROM", strlen("FROM")) == 0)
    {
      if ((return_buffer[*num_sigmets-1].vertices =
	   get_sigmet_vertices(sigmet, fixes, num_fixes,
			       &i, &return_buffer[*num_sigmets-1].num_vertices,
			       &return_buffer[*num_sigmets-1].bounding_box))
	  == NULL)
      {
	ufree(return_buffer);
		
	return((SIGMET_decoded_t *)NULL);
      }
	    
      continue;
    }

    /*
     * Check for a single point for the polygon.  If this is the case
     * the text should contain a string like "Dnn" giving the diameter
     * of this event.  We need to create a circle of this diameter for
     * the advisory.
     */

    if (return_buffer[*num_sigmets-1].num_vertices == 1)
    {
    }
        
    /*
     * Check for a line for the polygon.  When this is the case, the
     * text for the advisory should contain the text "LINE adv nn"
     * giving the width of the line for the event.  We need to create
     * a box with the given width containing the line.
     */

    if (return_buffer[*num_sigmets-1].num_vertices == 2)
    {
      SIGMET_vertex_t   *vertices;
      int        line_width;
      double     lat_offset;
            
      /*
       * Create the polygon to be used instead of the line.
       */

      line_width = 20;
      lat_offset = (double)line_width / 2.0 *
	KM_PER_NM * DEG_PER_KM_AT_EQ;
                
      vertices = (SIGMET_vertex_t *)umalloc(5 * sizeof(SIGMET_vertex_t));
            
      vertices[0].lat =
	return_buffer[*num_sigmets-1].vertices[0].lat - lat_offset;
      vertices[0].lon =
	return_buffer[*num_sigmets-1].vertices[0].lon;
      vertices[0].slope = MAX_SLOPE;
      vertices[0].intercept = vertices[0].lon;
            
      vertices[1].lat =
	return_buffer[*num_sigmets-1].vertices[0].lat + lat_offset;
      vertices[1].lon =
	return_buffer[*num_sigmets-1].vertices[0].lon;
      vertices[1].slope =
	return_buffer[*num_sigmets-1].vertices[0].slope;
      vertices[1].intercept =
	return_buffer[*num_sigmets-1].vertices[0].intercept + lat_offset;
            
      vertices[2].lat =
	return_buffer[*num_sigmets-1].vertices[1].lat + lat_offset;
      vertices[2].lon =
	return_buffer[*num_sigmets-1].vertices[1].lon;
      vertices[2].slope = MAX_SLOPE;
      vertices[2].intercept = vertices[1].lon;
      
      vertices[3].lat =
	return_buffer[*num_sigmets-1].vertices[1].lat - lat_offset;
      vertices[3].lon =
	return_buffer[*num_sigmets-1].vertices[1].lon;
      vertices[3].slope =
	return_buffer[*num_sigmets-1].vertices[0].slope;
      vertices[3].intercept =
	return_buffer[*num_sigmets-1].vertices[0].intercept - lat_offset;
            
      vertices[4].lat = vertices[0].lat;
      vertices[4].lon = vertices[0].lon;
      vertices[4].slope = 0.0;
      vertices[4].intercept = 0.0;
            
      /*
       * Replace the polygon information in the return buffer.
       */

      ufree(return_buffer[*num_sigmets-1].vertices);
            
      return_buffer[*num_sigmets-1].num_vertices = 5;
      return_buffer[*num_sigmets-1].vertices = vertices;

      /*
       * Update the bounding box information.
       */

      if (vertices[1].lat > vertices[2].lat)
      {
	return_buffer[*num_sigmets-1].bounding_box.max_lat =
	  vertices[1].lat;
	return_buffer[*num_sigmets-1].bounding_box.min_lat =
	  vertices[3].lat;
      }
      else
      {
	return_buffer[*num_sigmets-1].bounding_box.max_lat =
	  vertices[2].lat;
	return_buffer[*num_sigmets-1].bounding_box.min_lat =
	  vertices[0].lat;
      }
    }
        
        
    /*
     * Check for a cancellation.
     */

    if (strncmp(sigmet->lines[i], "CANCEL", strlen("CANCEL")) == 0 ||
	strncmp(sigmet->lines[i], "CNCL", strlen("CNCL")) == 0)
      return_buffer[*num_sigmets-1].cancel_flag = TRUE;
    
    i++;
        
  } /* endfor - i */
    
  return(return_buffer);
}


/************************************************************************
 * STATIC FUNCTIONS
 ************************************************************************/

/************************************************************************
 * get_sigmet_start() - Gets the SIGMET start time from the SIGMET text.
 *                      Also gets the region, report type and the forecast
 *                      letter since these are on the same line.
 */

static time_t get_sigmet_start(WXADV_text_t *sigmet, time_t sigmet_date,
			       char *region,
                               SIGMET_report_type_t *report_type,
                               char *forecast_letter)
{
  time_t    start_time = 0;
  date_time_t start_time_tm;
    
  int    num_tokens;
  int    i, j;
    
  /*
   * Initialize returned values.
   */

  region[0] = '\0';
  *report_type = SIGMET_REPORT_NEW;
  *forecast_letter = '\0';
  
  /*
   * Allocate space for the parsing tokens.
   */

  if (!sigmet_tokens_allocated)
  {
    for (i = 0; i < MAX_TOKENS; i++)
      sigmet_tokens[i] = (char *)umalloc(MAX_TOKEN_LEN);
    
    sigmet_tokens_allocated = TRUE;
  }
  
  /*
   * The line containing the start time also contains the SIGMET region.
   */

  for (i = sigmet->curr_line; i < sigmet->num_lines; i++)
  {
    for (j = 0; j < num_sigmet_regions; j++)
      if (strncmp(sigmet->lines[i], sigmet_regions[j],
		  strlen(sigmet_regions[j])) == 0)
	break;

    if (j < num_sigmet_regions)
      break;
  }
    
  if (i < sigmet->num_lines)
  {
    /*
     * Save the region name.
     */

    STRcopy(region, sigmet_regions[j], FORECAST_REGION_LEN);
        
    /*
     * Save the forecast letter.
     */

    *forecast_letter = sigmet->lines[i][3];
        
    /*
     * Get the start time from the line.
     */

    num_tokens = STRparse(sigmet->lines[i], sigmet_tokens,
			  strlen(sigmet->lines[i]), MAX_TOKENS, MAX_TOKEN_LEN);
            
    if (num_tokens == 3)
    {
      sigmet->curr_line = i + 1;
                
      start_time = sigmet_date;

      start_time_tm = *(udate_time(start_time));

      start_time_tm.day = 
	(sigmet_tokens[2][0] - '0') * 10 +
	  (sigmet_tokens[2][1] - '0');
      start_time_tm.hour =
	(sigmet_tokens[2][2] - '0') * 10 +
	  (sigmet_tokens[2][3] - '0');
      start_time_tm.min =
	(sigmet_tokens[2][4] - '0') * 10 +
	  (sigmet_tokens[2][5] - '0');
      start_time_tm.sec = 0;

      start_time = uunix_time(&start_time_tm);
    }

    /*
     * Check for correction or amendment.
     */

    if (strstr(sigmet->lines[i], "COR") != NULL)
      *report_type = SIGMET_REPORT_CORRECTION;
    else if (strstr(sigmet->lines[i], "AMD") != NULL)
      *report_type = SIGMET_REPORT_AMENDMENT;
    else
      *report_type = SIGMET_REPORT_NEW;
  }
    
  return(start_time);
}


/************************************************************************
 * get_sigmet_end() - Gets the SIGMET end time from the SIGMET text.
 */

static time_t get_sigmet_end(WXADV_text_t *sigmet, int *curr_line,
                             time_t start_time)
{
  static char *routine_name = "get_sigmet_end";
  
  time_t      end_time = 0;
  date_time_t end_time_tm;
    
  int   num_tokens;
    
  num_tokens = STRparse(sigmet->lines[*curr_line], sigmet_tokens,
			strlen(sigmet->lines[*curr_line]),
			MAX_TOKENS, MAX_TOKEN_LEN);
    
  if (num_tokens < 3)
  {
    fprintf(stderr,
	    "WARNING: %s:%s\n", module_name, routine_name);
    fprintf(stderr,
	      "Could not parse end time from line <%s>\n",
	    sigmet->lines[*curr_line]);
    return(end_time);
  }
    
  end_time = start_time;
    
  end_time_tm = *udate_time(end_time);
  if (sigmet_tokens[2][4] == 'Z')
  {
    end_time_tm.hour =
      (sigmet_tokens[2][0] - '0') * 10 +
	(sigmet_tokens[2][1] - '0');
    end_time_tm.min =
      (sigmet_tokens[2][2] - '0') * 10 +
	(sigmet_tokens[2][3] - '0');
  }
  else
  {
    end_time_tm.day =
      (sigmet_tokens[2][0] - '0') * 10 +
	(sigmet_tokens[2][1] - '0');
    end_time_tm.hour =
      (sigmet_tokens[2][2] - '0') * 10 +
	(sigmet_tokens[2][3] - '0');
    end_time_tm.min =
      (sigmet_tokens[2][4] - '0') * 10 +
	(sigmet_tokens[2][5] - '0');
  }
  end_time_tm.sec = 0;
        
  end_time = uunix_time(&end_time_tm);
  
  if (end_time < start_time)
    end_time += SECS_IN_DAY;
  
  (*curr_line)++;
  
  return(end_time);
}


/************************************************************************
 * get_sigmet_vertices() - Gets the SIGMET vertices from the SIGMET text
 *                         using the given array of NWS fix locations.
 *
 * Returns TRUE if the decoding was successful, FALSE if there was an
 * error.
 */

static SIGMET_vertex_t *get_sigmet_vertices(WXADV_text_t *sigmet,
					    WXADV_fix_t *fixes,
					    int num_fixes,
					    int *curr_line,
					    int *num_vertices,
					    SIGMET_box_t *bounding_box)
{
  static char *routine_name = "get_sigmet_vertices";
  
  SIGMET_vertex_t *vertices = NULL;
  
  char new_line[BUFSIZ];
  int num_tokens;
  int i;
  int j, k;
  int station_flag;
  int done;
  int range_bearing_flag;
  char *range_bearing = NULL;
  double vertex_lat, vertex_lon;
  int    curr_vertex;
    
  int fix;
  
  /*
   * Initialize returned values
   */

  *num_vertices = 0;
  
  bounding_box->min_lat = -1;
  bounding_box->min_lon = -1;
  bounding_box->max_lat = -1;
  bounding_box->max_lon = -1;
  
  station_flag = FALSE;
  range_bearing_flag = FALSE;
  done = FALSE;

  i = 0;
  num_tokens = -1;
    
  while (!done)
  {
    if (i >= num_tokens)
    {
      if (*curr_line >= sigmet->num_lines)
      {
	if (station_flag)
	{
	  fprintf(stderr,
		  "WARNING: %s:%s\n",
		  module_name, routine_name);
	  fprintf(stderr,
		  "Incomplete polygon in SIGMET\n");

	  *num_vertices = 0;
		
	  if (vertices != NULL)
	    ufree(vertices);
	  
	  return((SIGMET_vertex_t *)NULL);
	}
                
	break;
      }
            
      /*
       * Put spaces around the "-" characters so we can use the
       * parsing routines.
       */

      for (j = 0, k = 0; (size_t)j < strlen(sigmet->lines[*curr_line]); j++)
      {
	if (sigmet->lines[*curr_line][j] == '-')
	{
	  new_line[k] = ' ';
	  new_line[k+1] = '-';
	  new_line[k+2] = ' ';
	  k += 3;
	}
	else
	{
	  new_line[k] = sigmet->lines[*curr_line][j];
	  k++;
	}
      }
      new_line[k] = '\0';
            
      num_tokens = STRparse(new_line, sigmet_tokens,
			    strlen(new_line), MAX_TOKENS, MAX_TOKEN_LEN);

      if (num_tokens <= 0)
      {
	(*curr_line)++;
	continue;
      }
      
      if (!station_flag &&
	  !(STRequal_exact(sigmet_tokens[0], "TO") ||
	    STRequal_exact(sigmet_tokens[0], "FROM") ||
	    STRequal_exact(sigmet_tokens[0], "-")))
	break;

      i = 0;
      (*curr_line)++;
    }
        
    if (station_flag)
    {
      /*
       * Check for offset from fix.
       */

      if (isdigit(sigmet_tokens[i][0]))
      {
	if (range_bearing_flag)
	{
	  fprintf(stderr,
		  "WARNING: %s:%s\n",
		  module_name, routine_name);
	  fprintf(stderr,
		  "Two range/bearing indicators before fix in line <%s>\n",
		  sigmet->lines[*curr_line]);

	  *num_vertices = 0;
		    
	  if (vertices != NULL)
	    ufree(vertices);
	  
	  return((SIGMET_vertex_t *)NULL);
	}
                
	range_bearing_flag = TRUE;
	range_bearing = STRdup(sigmet_tokens[i]);
      }
            
      else
      {
	/*
	 * Find fix in fix list.
	 */

	for (fix = 0; fix < num_fixes; fix++)
	  if (STRequal_exact(sigmet_tokens[i], fixes[fix].id))
	    break;
            
	if (fix >= num_fixes)
	{
	  fprintf(stderr,
		  "WARNING: %s:%s\n",
		  module_name, routine_name);
	  fprintf(stderr,
		  "Fix %s not found in fix list\n",
		  sigmet_tokens[i]);

	  *num_vertices = 0;
		
	  if (vertices != NULL)
	    ufree(vertices);
	  
	  return((SIGMET_vertex_t *)NULL);
	}
            
	vertex_lat = fixes[fix].lat;
	vertex_lon = fixes[fix].lon;
            
	if (range_bearing_flag)
	{
	  offset_vertex(&vertex_lat, &vertex_lon, range_bearing);

	  ufree(range_bearing);
	  range_bearing = NULL;
	  range_bearing_flag = FALSE;
	}
            
	/*
	 * Add vertex to vertex list.
	 */

	if (vertices == NULL)
	{
	  *num_vertices = 1;
	  vertices = (SIGMET_vertex_t *)umalloc(sizeof(SIGMET_vertex_t));

	  bounding_box->min_lat = vertex_lat;
	  bounding_box->min_lon = vertex_lon;
	  bounding_box->max_lat = vertex_lat;
	  bounding_box->max_lon = vertex_lon;
	}
	else
	{
	  (*num_vertices)++;
	  vertices = (SIGMET_vertex_t *)urealloc(vertices,
						 *num_vertices *
						 sizeof(SIGMET_vertex_t));
	}
	
	curr_vertex = *num_vertices - 1;
                
	vertices[curr_vertex].lat = vertex_lat;
	vertices[curr_vertex].lon = vertex_lon;
	vertices[curr_vertex].slope = 0.0;
	vertices[curr_vertex].intercept = 0.0;
                
	/*
	 * Calculate the slope/intercept values for the line
	 * segment.
	 */

	if (curr_vertex > 0)
	{
	  double slope_denom;
                    
	  slope_denom =vertex_lon -
	    vertices[curr_vertex-1].lon;
                    
	  if (slope_denom == 0.0)
	    vertices[curr_vertex-1].slope =
	      MAX_SLOPE;
	  else
	    vertices[curr_vertex-1].slope =
	      (vertex_lat -
	       vertices[curr_vertex-1].lat) /
		 slope_denom;
	  if (vertices[curr_vertex-1].slope > MAX_SLOPE)
	    vertices[curr_vertex-1].slope = MAX_SLOPE;
                    
	  vertices[curr_vertex-1].intercept =
	    vertex_lat -
	      (vertices[curr_vertex-1].slope *
	       vertex_lon);
	}
                
	/*
	 * Update the bounding box for the polygon.
	 */
                
	if (bounding_box->min_lat > vertex_lat)
	  bounding_box->min_lat = vertex_lat;
	if (bounding_box->min_lon > vertex_lon)
	  bounding_box->min_lon = vertex_lon;
	if (bounding_box->max_lat < vertex_lat)
	  bounding_box->max_lat = vertex_lat;
	if (bounding_box->max_lon < vertex_lon)
	  bounding_box->max_lon = vertex_lon;

	station_flag = FALSE;
      }
            
    }
    else
    {
      if (!STRequal_exact(sigmet_tokens[i], "FROM") &&
	  !STRequal_exact(sigmet_tokens[i], "TO") &&
	  !STRequal_exact(sigmet_tokens[i], "-"))
      {
	fprintf(stderr,
		"WARNING: %s:%s\n",
		module_name, routine_name);
	fprintf(stderr,
		"Expecting TO, FROM or -, got \"%s\"\n",
		sigmet_tokens[i]);

	*num_vertices = 0;
		
	if (vertices != NULL)
	  ufree(vertices);
	
	return((SIGMET_vertex_t *)NULL);
      }

      station_flag = TRUE;
            
    }
        
    i++;
        
  } /* endwhile - !done */
    
  /*
   * Make sure the polygon was completed.
   */

  if (*num_vertices > 2 &&
      (vertices[0].lat != vertices[*num_vertices-1].lat ||
       vertices[0].lon != vertices[*num_vertices-1].lon))
  {
    fprintf(stderr,
	    "WARNING: %s:%s\n",
	    module_name, routine_name);
    fprintf(stderr,
	    "Beginning vertex in polygon different from ending vertex\n");

    (*num_vertices)++;
    vertices = (SIGMET_vertex_t *)urealloc(vertices,
					   *num_vertices *
					   sizeof(SIGMET_vertex_t));
    
    curr_vertex = *num_vertices - 1;

    vertices[curr_vertex].lat = vertices[0].lat;
    vertices[curr_vertex].lon = vertices[0].lon;
    vertices[curr_vertex-1].slope = (vertices[curr_vertex].lat -
				     vertices[curr_vertex-1].lat) /
				       (vertices[curr_vertex].lon -
					vertices[curr_vertex-1].lon);
    vertices[curr_vertex-1].intercept =
      vertices[curr_vertex].lat -
	(vertices[curr_vertex-1].slope *
	 vertices[curr_vertex].lon);
  }
        
  return(vertices);
}


/***********************************************************************
 * offset_vertex() - Offset a lat/lon point by the given range and bearing
 *                   string.
 */

void offset_vertex(double *lat, double *lon,
		   char *range_bearing)
{
  static char *routine_name = "offset_vertex";
  
  double range;
  double bearing;
  char   *bearing_string;
  
  /*
   * Convert the range and bearing to numerical format.
   */

  range = (double)atoi(range_bearing) * KM_PER_NM;
    
  /*
   * Skip the digits in the string which are the range to get
   * to the bearing information.
   */

  for (bearing_string = range_bearing;
       isdigit(*bearing_string);
       bearing_string++)
    /* NULL BODY */ ;
    
  /*
   * Convert the text in the bearing string to a bearing in degrees.
   */

  if (STRequal_exact(bearing_string, "N"))
    bearing = 0.0;
  else if (STRequal_exact(bearing_string, "NNE"))
    bearing = 22.5;
  else if (STRequal_exact(bearing_string, "NE"))
    bearing = 45.0;
  else if (STRequal_exact(bearing_string, "ENE"))
    bearing = 67.5;
  else if (STRequal_exact(bearing_string, "E"))
    bearing = 90.0;
  else if (STRequal_exact(bearing_string, "ESE"))
    bearing = 112.5;
  else if (STRequal_exact(bearing_string, "SE"))
    bearing = 135.0;
  else if (STRequal_exact(bearing_string, "SSE"))
    bearing = 157.5;
  else if (STRequal_exact(bearing_string, "S"))
    bearing = 180.0;
  else if (STRequal_exact(bearing_string, "SSW"))
    bearing = -157.5;
  else if (STRequal_exact(bearing_string, "SW"))
    bearing = -135.0;
  else if (STRequal_exact(bearing_string, "WSW"))
    bearing = -112.5;
  else if (STRequal_exact(bearing_string, "W"))
    bearing = -90.0;
  else if (STRequal_exact(bearing_string, "WNW"))
    bearing = -67.5;
  else if (STRequal_exact(bearing_string, "NW"))
    bearing = -45.0;
  else if (STRequal_exact(bearing_string, "NNW"))
    bearing = -22.5;
  else
  {
    fprintf(stderr,
	    "WARNING: %s:%s\n", module_name, routine_name);
    fprintf(stderr,
	    "Invalid bearing %s in polygon list, using N\n",
	    bearing_string);
    bearing = 0.0;
  }

  /*
   * Offset the lat/lon values of the vertex.
   */

  PJGLatLonPlusRTheta(*lat, *lon, range, bearing,
		      lat, lon);
    
  return;
}
