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
/*
 * Lowest level file i.o.  Read and interpret ascii product files.
 *                         Write ascii product records.
 *                         Open ascii product file.
 *
 * Contents:  SIO_write_data(), SIO_file_name(), SIO_file_name_full(), 
 *            SIO_open_data_file(), SIO_open_named_data_file(), 
 *            SIO_clear_read_buf(), SIO_read_record()
 *
 * Static routines:  is_a_blank_line(),
 *                   parse_newprod(), parse_gentime(), 
 *                   parse_motion(), parse_time(), parse_polyhead(), 
 *                   prepare_read_buf(), parse_product_description(), 
 *                   parse_point_values(), parse_detection_attr(), 
 *                   examine_read_status(), read_first_line(),
 *            
 */

#include <cstdio>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <iostream>
#include <math.h>

#include <rapformats/ascii_shapeio.h>
#include <toolsa/globals.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>

#include "shapeio_int.h"

using namespace std;

/*
 * fixed keywords found in the data files.
 */

#define KEY_NEWPROD    "PRODUCT "
#define KEY_PRODTIME   "PRODUCT_TIME "
#define KEY_POLYLINE   "POLYLINE"
#define KEY_PRODDESC   "PRODUCT_DESCRIPTION"
#define KEY_GENTIME    "GENERATE_TIME "
#define KEY_PRODMOTION "PRODUCT_MOTION"
#define KEY_DETECTATTR "DETECTION_ATTR"

/*
 * Work buffer size...one line of product files.
 */

#define MAXWORK  200

/*
 * Storage for lines of file read in.
 */

static char Read_buf[MAXWORK];

/*----------------------------------------------------------------*
 *
 * STATIC FUNCTIONS
 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

static int is_a_blank_line(char *line)
{
  size_t i;
    
  for (i = 0; i < strlen(line); ++i)
  {
    if (!isspace(line[i]))
      return FALSE;
  }

  return TRUE;
}

/*----------------------------------------------------------------*/
/*
 * Work buf has the new product line..pull out the
 * rest of that line into the shape structure..  If successful, return 1.
 */

static int parse_newprod(SIO_shape_data_t *shape)
{
  char *parse_buf;
  char typebuf[20], subtypebuf[20];
  SIO_polyline_object_t *polyline;
  int i;
    
  /*
   * Free the old shape information so we can start creating a new
   * shape.
   */

  SIO_free_shapes(shape, 1);
  
  /*
   * Parse the new product line
   */

  parse_buf = Read_buf + strlen(KEY_NEWPROD);
  if (sscanf(parse_buf, "%s %s %d %d %d",
	     typebuf, subtypebuf,
	     &shape->num_objects, &shape->group_id,
	     &shape->sequence_number) != 5)
  {
    printf("ERROR scanning line '%s'\n", Read_buf);
    return 0;
  }

  shape->type = known_type_subtype_list(typebuf);
  shape->sub_type = known_type_subtype_list(subtypebuf);

  /*
   * We've now filled in some of the fields...initialize all the
   * rest.
   */

  shape->data_time = 0;
  shape->valid_time = 0;
  shape->expire_time = 0;
  shape->description[0] = 0;
  shape->motion_dir = 0.0;
  shape->motion_speed = 0.0;
  shape->line_type = NULL;
  shape->qual_value = 0.0;
  shape->qual_thresh = 0.0;
  
  shape->P = (SIO_polyline_object_t *)
    umalloc(shape->num_objects * sizeof(SIO_polyline_object_t));

  for (i = 0; i < shape->num_objects; ++i)
  {
    polyline = &shape->P[i];
    polyline->object_label[0] = 0;
    polyline->npoints = 0;
    polyline->lat = NULL;
    polyline->lon = NULL;
    polyline->u_comp = NULL;
    polyline->v_comp = NULL;
    polyline->value = NULL;
  } // for (i = 0; i < shape->num_objects; ++i)

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Input is a line that has the time generated...
 */

static int parse_gentime(SIO_shape_data_t *shape)
{
  char *parse_buf;
  UTIMstruct gen_time;

  //
  // Skip past the field, then look for 1 6 int fields (times)
  //

  parse_buf = Read_buf + strlen(KEY_GENTIME);
  if (sscanf(parse_buf, " %ld %ld %ld %ld %ld %ld",
	     &gen_time.year, &gen_time.month, &gen_time.day,
	     &gen_time.hour, &gen_time.min, &gen_time.sec) != 6)
  {
    printf("ERROR parsing '%s'\n", Read_buf);
    return 0;
  }
  
  shape->gen_time = UTIMdate_to_unix(&gen_time);

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Parse out motion line
 */

static int parse_motion(SIO_shape_data_t *shape)
{
  char   *parse_buf;
  parse_buf = Read_buf + strlen(KEY_PRODMOTION);
  if (sscanf(parse_buf, "%f %f",
	     &shape->motion_dir, &shape->motion_speed) != 2)
  {
    printf("ERROR parsing '%s'\n", Read_buf);
    return 0;
  } // if (sscanf...

#ifdef NOT_ANY_MORE

  {
  double degrees;
    
  // The following code is commented out as of Aug.7,1997
  // Colide no longer puts out motion directions in polar coords.
  // they are now in radar coords.  The following comments are
  // also, therefore, obsolete, but are left in for clarity (??!??!)
  // (t.betancourt)
  //

  //
  // If product type is generated from COLIDE, (SIO_PRODTYPE_BOUNDARY_COLIDE 
  // (BDC) or SIO_PRODTYPE_EXTRAP_ISSUE_COLIDE (X??)) then convert the motion
  // dir to radar coords.  COLIDE products' motion dirs are in polar coords.
  //

  if (STRequal_exact(shape->type, SIO_PRODTYPE_BOUNDARY_COLIDE) ||
      SIO_IS_EXTRAP_ISSUE_COLIDE(shape->type))
  {
    // Convert motion dir from polar to radar coords.
    degrees = 90.0 - shape->motion_dir;     // polar to radar
    while (degrees < 0.0)
      degrees += 360.0;
    while (degrees > 360.0)
      degrees -= 360.0;
    shape->motion_dir = degrees;
  } /* endif - COLIDE product */
  }

#endif

  return 1;
}

/*----------------------------------------------------------------*/

static int parse_time(SIO_shape_data_t *shape)
{
  char *parse_buf;
  UTIMstruct data_time;
  UTIMstruct valid_time;
  UTIMstruct expire_time;
    
  /*
   * Skip past the field, then look for 3 6 int fields (times)
   */

  parse_buf = Read_buf + strlen(KEY_PRODTIME);
  if (sscanf(parse_buf,
	     "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
	     &data_time.year, &data_time.month, &data_time.day,
	     &data_time.hour, &data_time.min, &data_time.sec,
	     &valid_time.year, &valid_time.month, &valid_time.day,
	     &valid_time.hour, &valid_time.min, &valid_time.sec,
	     &expire_time.year, &expire_time.month, &expire_time.day,
	     &expire_time.hour, &expire_time.min, &expire_time.sec) != 18)
  {
    printf("ERROR scanning line '%s'\n", Read_buf);
    return 0;
  }

  shape->data_time = UTIMdate_to_unix(&data_time);

  shape->valid_time = UTIMdate_to_unix(&valid_time);

  shape->expire_time = UTIMdate_to_unix(&expire_time);

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Parse out header line of polygon ..store into polyline.
 * Note that the seconds field is optional.
 */

static int parse_polyhead(SIO_polyline_object_t *polyline)
{
  char *parse_buf;
  int num;
    
  /*
   * SKip past the keyword
   */

  parse_buf = Read_buf + strlen(KEY_POLYLINE);
  num = sscanf(parse_buf, "%s %d %d",
	       polyline->object_label, &polyline->npoints,
	       &polyline->nseconds);

  /*
   * See if all fields were included on the line.
   */

  if (num == 3)
    return 1;

  /*
   * Now see if everything but the seconds was included, since
   * the seconds is optional.
   */

  if (num == 2)
  {
    polyline->nseconds = 0;
    return 1;
  }

  /*
   * If we get here, there was an error.
   */

  printf("ERROR scanning line '%s'\n", Read_buf);
  return 0;
}

/*----------------------------------------------------------------*/
/*
 * On entry either Read_buf should have a new product line in it,
 * or else need to load in records till get such a line.
 *
 * return 1 if succesful and new product line now loaded.
 * Memory for the shape is also allocated properly.
 */

static int prepare_read_buf(FILE *fp, SIO_shape_data_t *shape, int harsh)
{
  int count;
    
  if (Read_buf[0] == 0)
  {
    /*
     * First time through....special case!
     * Read in some data.
     */

    if (fgets(Read_buf, MAXWORK, fp) == 0)
      /*
       * EOF
       */
      return 0;

    /*
     * Now scan ahead in "nonharsh" fashion.
     */

    return(prepare_read_buf(fp, shape, 0));
  }
    
  /*
   * Work buf has something in it...see if it is what we want.
   */

  count = 0;
  while (1)
  {
    if (strncmp(KEY_NEWPROD, Read_buf, strlen(KEY_NEWPROD)) == 0)
    {
      /*
       * Yes..is it the right product, etc?
       */

      if (parse_newprod(shape) != 0)
	/*
	 * Yes.
	 */
	break;
    }

    /*
     * Not yet..read something else.
     */

    if (fgets(Read_buf, MAXWORK, fp) == 0)
      /*
       * Well...o.k. it was EOF..thats o.k.
       */
      return 0;
    else
    {
      if (harsh == 1)
      {
	printf("UNusual alignment in reading prod data, '%s'\n",
	       Read_buf);
	return 0;
      }
    }	

    /*
     * Note .. if not harsh, keep reading ahead looking for the
     *         NEWPROD line...within reason.
     */

    if (count++ > 1000)
    {
      printf("ERROR this file seems bad\n");
      return 0;
    }
  }

  /*
   * Cool.
   */

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Parse into a variable the product description.
 */

static int parse_product_description(SIO_shape_data_t *shape)
{
  char *parse_buf;
    
  parse_buf = Read_buf + strlen(KEY_PRODDESC);

  STRcopy(shape->description, parse_buf, SIO_LABEL_LEN);
  STRblnk(shape->description);
  
  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Input is just prior to reading in list of lat/lon values.
 * read them into the struct if you can.
 * (allocates space).
 */

static int parse_point_values(SIO_polyline_object_t *polyline, FILE *fp)
{	    
  int i;
  int free_comp_vectors = FALSE;
  
  /*
   * Clear space as needed.
   */

  if (polyline->lat != NULL)
    ufree(polyline->lat);

  if (polyline->lon != NULL)
    ufree(polyline->lon);

  if (polyline->u_comp != NULL)
    ufree(polyline->u_comp);

  if (polyline->v_comp != NULL)
    ufree(polyline->v_comp);

  if (polyline->value != NULL)
    ufree(polyline->value);

  /*
   * Allocate space for the points in the polyline
   */

  if (polyline->npoints > 0)
  {
    polyline->lat = (float *)umalloc(polyline->npoints * sizeof(float));
    polyline->lon = (float *)umalloc(polyline->npoints * sizeof(float));
    polyline->u_comp = (float *)umalloc(polyline->npoints * sizeof(float));
    polyline->v_comp = (float *)umalloc(polyline->npoints * sizeof(float));
    polyline->value = NULL;
  }
  else
  {
    polyline->lat = NULL;
    polyline->lon = NULL;
    polyline->u_comp = NULL;
    polyline->v_comp = NULL;
    polyline->value = NULL;
    
    /*
     * No points to read so we might as well return.
     */

    return 1;
    
  } /* endif */

  /*
   * Read each point in from the input file
   */

  for (i = 0; i < polyline->npoints; ++i)
  {
    /*
     * Get a line.  The line should have either 2 or 4 floats (u_comp
     * and v_comp are optional).
     */

    if (fgets(Read_buf, MAXWORK, fp) == 0)
    {
      printf("ERROR premature end of file\n");
      return 0;
    }

    int num_tokens = sscanf(Read_buf, "%f %f %f %f",
			    &polyline->lat[i], &polyline->lon[i], 
			    &polyline->u_comp[i], &polyline->v_comp[i]);
    
    /*
     * Check to see if all of the values were included on the 
     * line.
     */

    if (num_tokens == 4)
    {
      //
      // Check if u and v are within a range considered to be valid.
      // If not, then set them to a missing value.
      //

      if (fabs(polyline->u_comp[i]) > SIO_UV_LIMIT)
	polyline->u_comp[i] = SIO_MISSING_UV;

      if (fabs(polyline->v_comp[i]) > SIO_UV_LIMIT)
	polyline->v_comp[i] = SIO_MISSING_UV;
    }

    /*
     * Now see if just the lat and lon were included on the line.
     * This is okay, u_comp and v_comp are optional.  If any line
     * doesn't contain the u_comp and v_comp values, we'll free
     * these arrays at the end of processing.
     */

    else if (num_tokens == 2)
    {
      /*
       * Don't free the vectors until after reading in all of the
       * points just in case there are some lines with extra values.
       */

      free_comp_vectors = TRUE;
    }
    else
    {
      printf("ERROR scanning 2 floats out of '%s'\n", Read_buf);

      /*
       * Free the polyline arrays since we weren't able to fill them.
       */

      ufree(polyline->lat);
      polyline->lat = NULL;
      
      ufree(polyline->lon);
      polyline->lon = NULL;
      
      ufree(polyline->u_comp);
      polyline->u_comp = NULL;
      
      ufree(polyline->v_comp);
      polyline->v_comp = NULL;
      
      polyline->npoints = 0;
      
      return 0;
    }

  } /* endfor - i */

  // free space for u,v components

  if (free_comp_vectors)
  {
    ufree(polyline->u_comp);
    polyline->u_comp = NULL;

    ufree(polyline->v_comp);
    polyline->v_comp = NULL;
  } /* endif - free_comp_vectors */
  
  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Parse out detection attr line.
*/

static int parse_detection_attr(SIO_shape_data_t *shape)
{
  char *parse_buf;
  char line_type_buf[20];
    
  parse_buf = Read_buf + strlen(KEY_DETECTATTR);
  if (sscanf(parse_buf, "%s %f %f",
	     line_type_buf,
	     &shape->qual_value, &shape->qual_thresh) != 3)
  {
    printf("ERROR parsing '%s'\n", Read_buf);
    return 0;
  } // if (sscanf() != 3)

  //
  // Make sure this line type is one which Colide outputs.
  //

  shape->line_type = known_line_type(line_type_buf);

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Look at status (see read routine below) and print out what
 * went wrong.
 * Return 1 if nothing wrong.
 */

static int examine_read_status(int status, SIO_shape_data_t *shape,
			       int num_objects)
{
  int retstat = 1;

  if (num_objects != shape->num_objects)
  {
    printf("ERROR wrong # of objects %d expected %d\n",
	   num_objects, shape->num_objects);
    retstat = 0;
  }

  if ((status & 4) == 0)
  {
    /*
     * Just a warning for this
     */

    printf("Warning..NO motion vector\n");
    shape->motion_dir = 0;
    shape->motion_speed = 0;
  }

  if ((status & 2) == 0)
  {
    printf("ERROR no polyline line\n");
    retstat = 0;
  }

  if ((status & 1) == 0)
  {
    printf("ERROR no time line\n");
    retstat = 0;
  }

  return retstat;
}

/*----------------------------------------------------------------*
 *
 * LOCALLY EXPORTED FUNCTIONS
 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*
 * Read 1 line of a file, hoping for 1st line of a new product..
 * Return 1 if it is aligned to this.
 * Return 0 otherwise.
 */

int read_first_line(FILE *fp, int verbose)
{
  if (fgets(Read_buf, MAXWORK, fp) == 0)
  {
    if (verbose)
      printf("ERROR EOF encountered when reading file\n");
    return 0;
  }    

  if (strncmp(KEY_NEWPROD, Read_buf, strlen(KEY_NEWPROD)) != 0)
  {
    if (verbose)
      printf("ERROR in alignment expected '%s' saw '%s'\n",
	     KEY_NEWPROD, Read_buf);
    return 0;
  }

  return 1;
}

/*----------------------------------------------------------------*
 *
 * GLOBALLY EXPORTED FUNCTIONS
 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

int SIO_read_record(FILE *fp,	       /* opened file */
		    SIO_shape_data_t *shape, /* returned data */
		    int *pos)	       /* returned bytes to start of
					* next record */
{
  int status;
  int num_objects;
    
  /*
   * Try to get work buf to be a new product that we want...
   * this should be what is in the buffer right now..
   */

  if (prepare_read_buf(fp, shape, 1) == 0)
    /*
     * Couldn't do it.
     */
    return 0;
    
  /*
   * Now read lines till get another one that says its a new product.
   * interpret each intermediate line appropriately.
   * keep track of file position for return.
   */

  status = 0;
  num_objects = 0;

  while (1)
  {
    /*
     * read a line.
     */

    *pos = ftell(fp);
    
    if (fgets(Read_buf, MAXWORK, fp) == 0)
      /*
       * End of file or error.
       */
      return(examine_read_status(status, shape, num_objects));
	
    /*
     * Compare to known strings, building up status as we go..
     */

    if (strncmp(KEY_PRODTIME, Read_buf, strlen(KEY_PRODTIME)) == 0)
    {
      if (parse_time(shape) == 1)
	status |= 1;
    }
    else if (strncmp(KEY_GENTIME, Read_buf, strlen(KEY_GENTIME)) == 0)
    {
      parse_gentime(shape);
    }
    else if (strncmp(KEY_PRODMOTION, Read_buf, strlen(KEY_PRODMOTION))
	     == 0)
    {
      if (parse_motion(shape) == 1)
	status |= 4;
    }
    else if (strncmp(KEY_POLYLINE, Read_buf, strlen(KEY_POLYLINE)) 
	     == 0)
    {
      SIO_polyline_object_t *polyline;
	    
      if (num_objects >= shape->num_objects)
      {
	printf("ERROR extra objects in prod\n");
	return 0;
      }

      /*
       * Here set pointer to proper object
       */

      polyline = &(shape->P[num_objects++]);
	    
      /*
       * Parse the polyline line.
       */

      if (parse_polyhead(polyline) == 1)
	status |= 2;

      /*
       * Parse the locations.
       */

      if (parse_point_values(polyline, fp) == 0)
	return 0;
    }
    else if (strncmp(KEY_PRODDESC, Read_buf, strlen(KEY_PRODDESC)) == 0)
    {
      /*
       * This is optional, so no status update is made.
       */

      parse_product_description(shape);
    }
    else if (strncmp(KEY_DETECTATTR, Read_buf, strlen(KEY_DETECTATTR)) == 0)
    {
      //
      // No status update is made because this only occurs in Colide
      // generated files.
      //

      parse_detection_attr(shape);
    }
    else if (strncmp(KEY_NEWPROD, Read_buf, strlen(KEY_NEWPROD)) == 0)
    {
      /*
       * End of this product, start of a new one...should do it.
       */

      break;
    }
    else if (!is_a_blank_line(Read_buf))
      printf("UNKNOWN LINE '%s'\n", Read_buf);

  } // while (1)
	
  return(examine_read_status(status, shape, num_objects));
}

/*----------------------------------------------------------------*/
/*
 * Write input data to input file as one product.
 */

int SIO_write_data(FILE *fp,            /* opened file */
		   SIO_shape_data_t *shape,   /* some data */
		   int print_shapes)    /* print shape info or not */
{
  int obj, point;
  time_t now;
  UTIMstruct data_time, valid_time, expire_time, gen_time;
  SIO_polyline_object_t *polyline;

  /*
   * FIgure out times.
   */

  now = time((time_t *)NULL);
  UTIMunix_to_date(now, &gen_time);
  UTIMunix_to_date(shape->data_time, &data_time);
  UTIMunix_to_date(shape->valid_time, &valid_time);
  UTIMunix_to_date(shape->expire_time, &expire_time);

  /*
   * First form and write out the header line
   */

  fprintf(fp, "%s %s %s  %d %d %d\n",
	  KEY_NEWPROD, shape->type, shape->sub_type, shape->num_objects,
	  shape->group_id, shape->sequence_number);

  if (print_shapes)
    printf("  Data time: %d %2d %02d %02d %02d %02d\n",
	   (int)data_time.year, (int)data_time.month, (int)data_time.day, 
	   (int)data_time.hour, (int)data_time.min, (int)data_time.sec);

  /*
   * Now the "generate" line
   */

  fprintf(fp, "%s  %d %d %d %d %d %d\n",
	  KEY_GENTIME,
	  (int)gen_time.year, (int)gen_time.month, (int)gen_time.day,
	  (int)gen_time.hour, (int)gen_time.min, (int)gen_time.sec);
    
  /*
   * Now the time line..
   */

  fprintf(fp,
	  "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	  KEY_PRODTIME,
	  (int)data_time.year,
	  (int)data_time.month,
	  (int)data_time.day,
	  (int)data_time.hour,
	  (int)data_time.min,
	  (int)data_time.sec,
	  (int)valid_time.year,
	  (int)valid_time.month,
	  (int)valid_time.day,
	  (int)valid_time.hour,
	  (int)valid_time.min,
	  (int)valid_time.sec,
	  (int)expire_time.year,
	  (int)expire_time.month,
	  (int)expire_time.day,
	  (int)expire_time.hour,
	  (int)expire_time.min,
	  (int)expire_time.sec);
    
  /*
   * Now the description line
   */

  fprintf(fp, "%s  %s\n",
	  KEY_PRODDESC, shape->description);

  /*
   * The motion line.
   */

  fprintf(fp, "%s %f %f\n",
	  KEY_PRODMOTION, shape->motion_dir, shape->motion_speed);
    
  //
  // the detection attr line
  //

  if (shape->line_type != NULL)
  {
    fprintf(fp, "%s %s %f %f\n",
	    KEY_DETECTATTR, shape->line_type, 
	    shape->qual_value, shape->qual_thresh);
  } // if (shape->line_type != NULL)

  /*
   * for each object
   */

  for (obj = 0; obj < shape->num_objects; ++obj)
  {
    polyline = &shape->P[obj];
	
    /*
     * Form the polyline header line.
     */

    fprintf(fp, "%s %s %d %d\n",
	    KEY_POLYLINE, polyline->object_label,
	    polyline->npoints, polyline->nseconds);

    for (point = 0; point < polyline->npoints; ++point)
    {
      /*
       * each line of 2 values.
       * Note: Not writing out SIO_polyline_object_t->value.
       */

      if (polyline->u_comp == NULL)
	fprintf(fp, "  %16.6f %16.6f\n",
		polyline->lat[point], polyline->lon[point]);
      else
	fprintf(fp, "  %16.6f %16.6f %16.6f %16.6f\n", 
		polyline->lat[point], polyline->lon[point],
		polyline->u_comp[point], polyline->v_comp[point]);
    }
  }
    
  /*
   * Blank line.
   */

  fprintf(fp, "\n");

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Return standard sio file name for the inputs
 */

char *SIO_file_name(const char *suffix, /* file suffix */
		    time_t time)	/* time of data*/
{
  static char filename[MAX_PATH_LEN];
  UTIMstruct time_struct;
    
  UTIMunix_to_date(time, &time_struct);

  sprintf(filename, "%d%02d%02d.%s",
	  (int)time_struct.year, (int)time_struct.month, (int)time_struct.day, 
	  suffix);

  return filename;
}

/*----------------------------------------------------------------*/
/*
 * Return the standard sio file name for the inputs.
 */

char *SIO_file_name_full(const char   *directory, /* where */
			 const char   *suffix,    /* file suffix */
			 time_t time,             /* time of data*/
			 int    minutes_offset)
/* minutes_offset: number of minutes to offset the time; effectively
 *                 adjusting the time when a new file is opened;
 *                 default is 0
 */
{
  static char filename[MAX_PATH_LEN];
  UTIMstruct time_struct;
    
  time += minutes_offset * 60;
  UTIMunix_to_date(time, &time_struct);

  sprintf(filename, "%s/%d%02d%02d.%s",
	  directory,
	  (int)time_struct.year, (int)time_struct.month, (int)time_struct.day,
	  suffix);

  return filename;
}

/*----------------------------------------------------------------*/

FILE *SIO_open_data_file(const char *directory, const char *suffix, time_t file_time,
			 const char *mode, int minutes_offset)
{
  char *filename;
  FILE *fp;
    
  /*
   * Form the name.
   */

  filename = SIO_file_name_full(directory, suffix,
				file_time, minutes_offset);

  /*
   * Open the file.
   */

  if ((fp = fopen(filename, mode)) == NULL)
    printf("ERROR opening %s in mode %s\n", filename, mode);

  return fp;
}

/*----------------------------------------------------------------*/

FILE *SIO_open_named_data_file(const char *filename, const char *mode)
{
  FILE *fp;
    
  /*
   * Open the file.
   */

  if ((fp = fopen(filename, mode)) == NULL)
    printf("ERROR opening %s in mode %s\n", filename, mode);

  return fp;
}

/*----------------------------------------------------------------*/

void SIO_clear_read_buf()
{
  Read_buf[0] = 0;
}

/*----------------------------------------------------------------*/
