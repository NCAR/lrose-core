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
/******************************************************************************
 *  MDV_THRESH_CNT.C Counting Utility Program for MDV files. 
 *
 *  Modified from PRINT_MDV.C  M. Petach 17-DEC-1998.
 *
 *  The program is intended to count the number of data points
 *  in all MDV files in a specified directory whose data value
 *  exceeds a given threshold and fall within a specified time range.
 *
 *  The command line might be:
 *
 *  mdv_thresh_cnt -input_dir $AWC_DATA_DIR/mdv_data/wsi_4km_vil \
 *	-field_num 0 -threshold 3.4 -starttime "1998/08/16_00:00:00" \
 *	-endtime "1998/08/16_23:59:59"
 *
 *	and the result:
 *
 *	23451 data points exceed threshhold of 3.4 for field 0
 *
 *	Defaults to input_dir "." and field_num 0.
 *
 *  Does not use bad or missing data values in counts.
 *
 */

#include <stdio.h>
#include <string.h>

#include <toolsa/udatetime.h> /* For time structure date_time_t */
#include <toolsa/umisc.h>
#include <mdv/mdv_print.h>
#include <mdv/mdv_read.h>
#include <mdv/mdv_user.h>
#include <mdv/mdv_handle.h>

#include <didss/ds_input_path.h>

#include "mdv_thresh_cnt.h"
#include "_tdrp.h"

/*
 * Forward function declarations.
 */

void parse_command_line(int argc, char **argv);
void print_usage(void);
void print_using_handle(void);

/*
 * TDRP basic declarations
 */

char prog_name[MAX_PATH_LEN];
char *params_file_path = NULL;
tdrp_override_t override;
_tdrp_struct params;

/*
 * Flags for checking command line args
 */

int Inputdir_flag = FALSE;
int Fieldnum_flag = FALSE;
int Starttime_flag = FALSE;
int Endtime_flag = FALSE;
int Threshold_flag = FALSE;
int Error_flag = FALSE;
int Data_flag = FALSE;

/*
 * Global variables from the command line.
 */

/* Threshold value to use for counting data values */
double threshold;

/* Field number from command line */
int fieldnum;

/* Directory name from command line */
char *inputdir = NULL;

date_time_t DateTime;
time_t begtime = -1;
time_t endtime = -1;

/* total number of data values exceeding threshold for all files */
int tot_count = 0;
   
/* text string describing threshold comparison type */
char *comp_desc;

/* Current input file */
char *in_fname;

/******************************************************************************
 * MAIN :   Open file and send the output to STDOUT
 *
 */

int main(int argc, char **argv)
{

  int numfiles;

  /* We may be using the TDRP debug information here... */
  int debug = 0;

  /* dataset handle */
  DSINP_handle_t ds_hand;

  /* initialize before parsing the command line */
  TDRP_init_override(&override);

  /*
   * Parse the command line.
   */

  parse_command_line(argc, argv);

  strcpy(prog_name, *argv);

  /*
   * Load the TDRP parameters
   */

  if (_tdrp_load_from_args(argc, argv, &params,
			   override.list, &params_file_path)) {
    fprintf(stderr, "ERROR - Problems with params file '%s'\n",
	    params_file_path);

    exit(-1);
  }

  /* free override list */
  TDRP_free_override(&override);

  /* Test the thresh_comp mode */

  switch (params.thresh_comp) {
  case LESS_THAN :
    comp_desc = "<";
    break;
  case LESS_THAN_OR_EQUAL :
    comp_desc = "<=";
    break;
  case GREATER_THAN :
    comp_desc = ">";
    break;
  case GREATER_THAN_OR_EQUAL :
    comp_desc = ">=";
    break;
  default :
    fprintf(stderr, "ERROR - %s\n", prog_name); 
    fprintf(stderr, "Unknown thresh_comp mode %d\n", params.thresh_comp); 
  }

  fprintf(stdout, "Threshold comparison mode: %s %.3f\n\n",
	  comp_desc, threshold);

  /* Get list of files via create_archive_time */
  DSINP_create_archive_time(&ds_hand, prog_name, debug,
			    inputdir, begtime, endtime);

  numfiles = 0;
  while ((in_fname = DSINP_next(&ds_hand)) != NULL) {
    printf("%s\n", in_fname);
    numfiles++;

    /*
     * check that file is an MDV file
     */

    if (!MDV_verify(in_fname)) {
      fprintf(stderr, "ERROR - %s\n", prog_name);
      fprintf(stderr, "File %s is not MDV format\n", in_fname);
      exit (-1);
    }
    
    print_using_handle();
  }
  printf("%s: %d files found.\n", prog_name, numfiles);

  fprintf(stdout, "%s%s%s%d%s%s%s%.3f%s%d%s%d%s",
	  "\n", prog_name, " Summary:\n", tot_count,
	  " points are ", comp_desc,
	  " the threshold of ", threshold, 
	  " in field ", fieldnum,
	  " for ", numfiles, " files.\n\n");

  DSINP_free(&ds_hand);
  exit(0);
}

/*****************************************************
 * parse_command_line()
 */

void parse_command_line(int argc, char **argv)
{
  int i;
  char *tmpstr;
  int dt_cnt;
  
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 ||
	strcmp(argv[i], "-help") == 0) {
      print_usage();
      exit(0);
    }
    
    if (strcmp(argv[i], "-inputdir") == 0) {
      Inputdir_flag = TRUE;
      
      if (i < argc - 1) {
	inputdir = argv[i+1];
      }
      else {
	Error_flag = TRUE;
      }
    }
    else if (strcmp(argv[i], "-fieldnum") == 0) {
      Fieldnum_flag = TRUE;
      if (i < argc - 1) {
	fieldnum = atoi(argv[i+1]);

	if (fieldnum < 0) {
	  Error_flag = TRUE;
	}
      }
      else {
	Error_flag = TRUE;
      } /* endif - (i < argc - 1) */

    }
    else if (strcmp(argv[i], "-starttime") == 0) {
      Starttime_flag = TRUE;
      
      /* check if arg is actually available */
      if (i < argc - 1) {
	/* parse the starttime information */
	/* See if we had the "YYYY/MM/DD_hh:mm:ss" format. */

	tmpstr = argv[i+1];
	dt_cnt = sscanf(tmpstr, "%4d/%2d/%2d_%2d:%2d:%2d",
			&DateTime.year, &DateTime.month, &DateTime.day,
			&DateTime.hour, &DateTime.min, &DateTime.sec);

	/* dt_cnt should equal 6 when done for reading in 6 values */
	if (dt_cnt != 6) {
	  Error_flag = TRUE;
	} 

	/* conversion takes time struct values and returns a unix time */
	begtime = uconvert_to_utime(&DateTime);
	fprintf(stdout, "BEG_TIME %s\n", utimstr(begtime));
	      
      }
      else {
	Error_flag = TRUE;
      } /* endif - argc */

    }
    else if (strcmp(argv[i], "-endtime") == 0) {
      Endtime_flag = TRUE;
	  
      /* check if arg is actually available */
      if (i < argc - 1) {
	      
	/* parse the endtime information */
	/* See if we had the "YYYY/MM/DD_hh:mm:ss" format. */
	
	tmpstr = argv[i+1];
	dt_cnt = sscanf(tmpstr, "%4d/%2d/%2d_%2d:%2d:%2d",
			&DateTime.year, &DateTime.month, &DateTime.day,
			&DateTime.hour, &DateTime.min, &DateTime.sec);
	      
	/* dt_cnt should equal 6 when done for reading in 6 values */
	if (dt_cnt != 6) {
	  Error_flag = TRUE;
	} 

	/* conversion takes time struct values and returns a unix time */
	endtime = uconvert_to_utime(&DateTime);
	fprintf(stdout, "END_TIME %s\n", utimstr(endtime));
      }
      else {
	Error_flag = TRUE;
      } /* endif - (i < argc - 1) */
	  
    }
    else if (strcmp(argv[i], "-data") == 0) {
      Data_flag = TRUE;
    }
    else if (strcmp(argv[i], "-threshold") == 0) {
      Threshold_flag = TRUE;
      if (i < argc - 1) {
	threshold = atof(argv[i+1]);
	
	/* Check that a positive threshold value was entered */
	if (threshold <= 0) {
	  fprintf(stdout, "Warning, threshold is: %f\n", threshold);
	} /* endif - threshold */
      } /* endif - (i < argc - 1) */
      else {
	Error_flag = TRUE;
      } 
    }
    else {
      /* Arg is not a keyword */
    }
  }
  
  /* Default directory is . when it is ready */
  if (!Inputdir_flag) {
    inputdir = ".";
  }
  
  /* Default field number is 0 */
  if (!Fieldnum_flag) {
    fieldnum = 0;
  }

  if (Error_flag) {
    print_usage();
    exit(1);
  }
  
  return;
}

/*****************************************************
 * print_usage()
 */

void print_usage(void)
{
  fprintf(stderr, "Usage: mdv_thresh_cnt [options]\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options as follows:\n");
  fprintf(stderr, "  [ -inputdir path ]   directory for MDV files\n");
  fprintf(stderr, "  [ -fieldnum number ] field number for threshold count\n");
  fprintf(stderr, "  [ -threshold level ] threshold value for data counts\n");
  fprintf(stderr, "  [ -starttime start ] first time to count, eg \"1998/08/16_23:59:59\"\n");
  fprintf(stderr, "  [ -endtime end ]     last time to count,  eg \"1998/08/19_00:00:00\"\n");
  fprintf(stderr, "  [ -help, -h ]        prints help or usage\n");
  fprintf(stderr, "  [ -data ]            prints data that meet threshold criteria\n");
  fprintf(stderr, "                       (use params file to change threshold comparison type,\n");
  fprintf(stderr, "                        default comparison is \"greater than or equal to\").\n");
  fprintf(stderr, "\n");
  
  TDRP_usage(stderr);
  return;
}

/**********************
 * print_using_handle()
 */

void print_using_handle(void)
     
{
  
  ui08 *bval;
  int iz, i;
  int ifield;
  int prev;

  /* A count of how many data values exceed >= the threshold */   
  int th_count;

  int bad_miss_cnt;
  
  /* The data value converted to appropriately scaled data units */
  double scaled_data;
  
  MDV_field_header_t  *fhdr;
  MDV_handle_t mdv;
  
  MDV_init_handle(&mdv);
  
  /*
   * read input mdv file
   */
  
  if (MDV_read_all(&mdv, in_fname, MDV_INT8)) {
    fprintf(stderr, "ERROR - mdv_thresh_cnt\n");
    fprintf(stderr, "Cannot read mdv file %s\n", in_fname);
    perror(in_fname);
    exit (-1);
  }
  
  /*
   * Print the field data.
   */
  
  /* Changed this to use the field specified on the command line only, */
  /* Although there may be some interest in checking multiple fields as well */
  /* for (ifield = 0; ifield < mdv.master_hdr.n_fields; ifield++) { */
  
  if (fieldnum >= mdv.master_hdr.n_fields) {
    fprintf(stderr, "Warning, field %d not found.\n", fieldnum);
  }
  else {
    ifield = fieldnum;
    
    /* fprintf(stdout, "------>Field: %d\n", ifield); */
    fhdr = mdv.fld_hdrs + ifield;
    
    for (iz = 0; iz < fhdr->nz; iz++) {
      
      /* fprintf(stdout, "------->Plane: %d\n", iz); */
      
      th_count = 0;	/* Initailize threshold count for current plane */
      bad_miss_cnt = 0; /* Iniatialze bad and missing data count */
            
      prev = -999999;
      bval = mdv.field_plane[ifield][iz];
      for (i = 0; i < fhdr->ny * fhdr->nx; i++, bval++) {
	
	/*
	 * Don't count bad or missing values in threshold counts.
	 */

	if (*bval != fhdr->bad_data_value &&
	    *bval != fhdr->missing_data_value) {
          scaled_data = ( *bval * fhdr->scale) + fhdr->bias;

	  /*
	   * Count how many values meet the threshold criteria.
	   */

	  if ( ((scaled_data >= threshold) &&
		(params.thresh_comp == GREATER_THAN_OR_EQUAL)) ||
	       ((scaled_data >  threshold) &&
		(params.thresh_comp == GREATER_THAN)) ||
	       ((scaled_data <  threshold) &&
		(params.thresh_comp == LESS_THAN)) ||
	       ((scaled_data <= threshold) &&
		(params.thresh_comp == LESS_THAN_OR_EQUAL)) ) {
	    th_count++;
	    tot_count++;

            if (Data_flag) {
	      fprintf(stdout, " %.1f", scaled_data);
            } /* if (Data_flag) */

	  } /* endif - scaled_data */
	  
	}
	else {
	  bad_miss_cnt++;
	} /* endif - *bval */
	
      } /* i */
      
      /*
       * Print threshold counts
       */

      if (Data_flag) {
	fprintf(stdout, "\n");
	fprintf(stdout, "%d bad or missing data values\n", bad_miss_cnt);
      } /* if (Data_flag) */
      
      fprintf(stdout, "%d %s %s %s %.3f %s %d%s",
	      th_count, "data points",
	      comp_desc, "the threshold of", threshold,
	      "for field", ifield, "\n");
      
    } /* iz */
    
  } /* ifield */
  
  MDV_free_handle(&mdv);
}




