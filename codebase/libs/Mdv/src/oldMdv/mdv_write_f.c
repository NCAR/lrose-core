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
 *  MDV_WRITE_F.C Fortran Wrappers for Subroutines to write data to MDV files.
 *  Nancy Rehak, June 1997
 */

#include <errno.h>
#include <stdio.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_write.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <unistd.h> 

#include "mdv_private_f.h"

#if defined(F_UNDERSCORE2)
  #define mf_wof_write_open        mf_wof_write_open__
  #define mf_wcf_write_close       mf_wcf_write_close__
  #define mf_wm_write_master_hdr   mf_wm_write_master_hdr__
  #define mf_wf_write_field        mf_wf_write_field__
#elif defined(F_UNDERSCORE)
  #define mf_wof_write_open        mf_wof_write_open_
  #define mf_wcf_write_close       mf_wcf_write_close_
  #define mf_wm_write_master_hdr   mf_wm_write_master_hdr_
  #define mf_wf_write_field        mf_wf_write_field_
#endif

/*
 * Status values returned by the write routines.  These must match
 * the corresponding values in mdv/mf_mdv.inc.
 */

#define MDV_WRITE_SUCCESSFUL             0
#define MDV_WRITE_OPEN_FAILURE           1
#define MDV_WRITE_MASTER_HDR_FAILED      2
#define MDV_WRITE_FIELD_FAILED           3


/*
 * Prototypes for static functions
 */

static int open_output_file(char *filename);
static void close_output_file(void);

/*
 * Global variables
 */

FILE *Output_file = NULL;
char *Output_filename = NULL;


/***********************************************************************
* MF_WOF_WRITE_OPEN: Open a file to be used for writing the MDV information.
*                    This routine must be called before calling the 
*                    MF_xx_WRITE_xxx routines.
*
* Use the following to call the subroutine:
*
*    CHARACTER*1024 FNAME
*    INTEGER        RETURN_STATUS
*
*   CALL MF_WOF_WRITE_OPEN(FNAME, RETURN_STATUS)
*
* This function will open the file named FNAME.  This file will then be
* used by all subsequent write routine calls.  Remember to call
* MF_WCF_WRITE_CLOSE after writing all of the MDV information to the 
* file.
* 
* When finished, RETURN_STATUS can be MDV_WRITE_SUCCESSFUL
*                                     MDV_WRITE_OPEN_FAILURE
*/

void mf_wof_write_open( char *fname,
			int   *return_status)
{
  *return_status = open_output_file(fname);
  return;
}


/***********************************************************************
* MF_COF_WRITE_CLOSE: Close the file being used for writing the MDV
*                     information.  This routine must be called after
*                     calling MF_WOF_WRITE_OPEN and the MF_xx_WRITE_xxx 
*                     routines.
*
* Use the following to call the subroutine:
*
*   CALL MF_WCF_WRITE_CLOSE()
*
* This function will close the file currently being used for MDV output.
*/

void mf_wcf_write_close(void)
{
  close_output_file();
  return;
}


/***********************************************************************
* MF_WM_WRITE_MASTER_HDR: Write the master header to a data file.
*
* Use the following to call the subroutine:
*
*    INTEGER*4      MASTER_HDR_INTS(MDV_NUM_MASTER_HEADER_SI32)
*    REAL*4         MASTER_HDR_REALS(MDV_NUM_MASTER_HEADER_FL32)
*    CHARACTER*(MDV_INFO_LEN)  DATASET_INFO
*    CHARACTER*(MDV_NAME_LEN)  DATASET_NAME
*    CHARACTER*(MDV_NAME_LEN)  DATASET_SOURCE
*    INTEGER        RETURN_STATUS
*
*   CALL MF_WM_WRITE_MASTER_HDR(MASTER_HDR_INTS, MASTER_HDR_REALS,
*                               DATASET_INFO, DATASET_NAME, DATASET_SOURCE,
*		                RETURN_STATUS)
*
* This function will open the file named FNAME and write the master header
* in the correct location.
* 
* When finished, RETURN_STATUS can be MDV_WRITE_SUCCESSFUL
*                                     MDV_WRITE_MASTER_HDR_FAILED
*/

void mf_wm_write_master_hdr(si32 *master_hdr_ints,
			    fl32 *master_hdr_reals,
			    char  *dataset_info,
			    char  *dataset_name,
			    char  *dataset_source,
			    int   *return_status)
{
  MDV_master_header_t master_hdr;
  
  /*
   * Copy the master header info
   */

  mf_master_hdr_from_fortran(master_hdr_ints,
                             master_hdr_reals,
                             dataset_info,
                             dataset_name,
                             dataset_source,
                             &master_hdr);
  
  /*
   * Write the master header
   */

  if (MDV_write_master_header(Output_file, &master_hdr) == MDV_FAILURE)
  {
    fprintf(stderr, "Error writing master header to file <%s>\n",
	    Output_filename);
    close_output_file();
    *return_status = MDV_WRITE_MASTER_HDR_FAILED;
    return;
  }

  *return_status = MDV_WRITE_SUCCESSFUL;
     
  return;
}

/***********************************************************************
* MF_WF_WRITE_FIELD: Write the indicated field header and data to a
* data file.
*
* Use the following to call the subroutine:
*
*    INTEGER        FIELD_NUM
*    INTEGER*4      FIELD_HDR_INTS(MDV_NUM_FIELD_HEADER_SI32)
*    REAL*4         FIELD_HDR_REALS(MDV_NUM_FIELD_HEADER_FL32)
*    CHARACTER*(MDV_LONG_FIELD_LEN) FIELD_NAME_LONG
*    CHARACTER*(MDV_SHORT_FIELD_LEN) FIELD_NAME_SHORT
*    CHARACTER*(MDV_UNITS_LEN) FIELD_UNITS
*    CHARACTER*(MDV_TRANSFORM_LEN) FIELD_TRANSFORM
*    CHARACTER*(MDV_UNITS_LEN) FIELD_UNUSED_CHAR
*    INTEGER        DATA_SIZE
*    BYTE           FIELD_DATA_ARRAY(DATA_SIZE)
*    INTEGER        OUTPUT_ENCODING_TYPE
*    INTEGER        BYTES_WRITTEN
*    INTEGER        RETURN_STATUS
*
*   CALL MF_WF_WRITE_FIELD(FIELD_NUM,
*                          FIELD_HDR_INTS, FIELD_HDR_REALS,
*                          FIELD_NAME_LONG, FIELD_NAME_SHORT,
*                          FIELD_UNITS, FIELD_TRANSFORM, FIELD_UNUSED_CHAR,
*                          DATA_SIZE, FIELD_DATA_ARRAY,
*                          OUTPUT_ENCODING_TYPE, BYTES_WRITTEN,
*		           RETURN_STATUS)
*
* This function will open the file named FNAME and write the indicated field
* header and data in the proper position.
* 
* When finished, RETURN_STATUS can be MDV_WRITE_SUCCESSFUL,
*                                     MDV_WRITE_FIELD_FAILED
*/

void mf_wf_write_field( int *field_num,
                        si32 *field_hdr_ints,
                        fl32 *field_hdr_reals,
                        char *field_name_long,
                        char *field_name_short,
                        char *field_units,
                        char *field_transform,
                        char *field_unused_char,
                        int *data_size,
                        char *field_data_array,
                        int *output_encoding_type,
                        int *bytes_written,
                        int  *return_status)
{

  MDV_field_header_t field_hdr;
  
  /*
   * Convert the FORTRAN field header information to the C structure
   */

  mf_field_hdr_from_fortran(field_hdr_ints,
			    field_hdr_reals,
			    field_name_long,
			    field_name_short,
			    field_units,
			    field_transform,
			    field_unused_char,
			    &field_hdr);
  
  /*
   * Write the field header and data
   */

  if ((*bytes_written =
       MDV_write_field(Output_file, &field_hdr, field_data_array,
		       *field_num, field_hdr.field_data_offset,
		       *output_encoding_type)) <= 0)
  {
    fprintf(stderr, "Error loading field %d header from file\n", *field_num);
    close_output_file();
    *bytes_written = 0;
    *return_status = MDV_WRITE_FIELD_FAILED;
    return;
  }
  
  *return_status = MDV_WRITE_SUCCESSFUL;
     
  return;
}

/***********************************************************************
 * STATIC FUNCTIONS
 ***********************************************************************/

/***********************************************************************
* open_output_file()
*/

static int open_output_file(char *filename)
{
  int i;
  
  /*
   * Strip off trailing spaces from the file name
   */

  i = 0;
  while (filename[i] != ' ')
    i++;
  filename[i] = '\0';
	 
  /*
   * Open the output file
   */

  if ((Output_file = fopen(filename,"w+")) == NULL)
  {
    fprintf(stderr, "Error opening output file\n");
    perror(filename);
    return(MDV_WRITE_OPEN_FAILURE);
  }

  Output_filename = STRdup(filename);
  
  return(MDV_WRITE_SUCCESSFUL);
}


/***********************************************************************
* close_output_file()
*/

static void close_output_file(void)
{

  /* These variables added by Niles for latest_data_info file writing. */
  FILE *lfp;
  date_time_t T;
  char *EncodedFilename;
  char Dir[ MAX_PATH_LEN ];
  char ldataFilename[ MAX_PATH_LEN ];

  /*
   * Close the output file
   */

  fclose(Output_file);
  /*
   * Write an ldatainfo file if we can. Added by Niles,
   * August 2001. Assumes that the Output_filename
   * string is of the form /Some/Directory/Structure/YYYYMMDD/hhmmss.mdv
   * and will fail to do the ldata file write if this
   * assumption is found to fail.
   *
   *
   */

  /*
   * First, make sure we have enough characters to even consider
   * this decoding.
   *
   */
  if (strlen(Output_filename) >= strlen("YYYYMMDD/hhmmss.mdv")){

    EncodedFilename = Output_filename +
      strlen(Output_filename) - 
      strlen("YYYYMMDD/hhmmss.mdv");
    /*
     *
     * Make sure the characters we do have conform to this
     * naming convention.
     *
     */
    if (6 == sscanf(EncodedFilename,"%4d%2d%2d/%2d%2d%2d.mdv",
		    &T.year, &T.month, &T.day,
		    &T.hour, &T.min, &T.sec)
	){
      
      uconvert_to_utime( &T );
      /*
       *
       * Extract the directory name from the file name.
       *
       */
      STRcopy(Dir, Output_filename,
	     strlen(Output_filename) -  strlen("YYYYMMDD/hhmmss.mdv" ));
      /*
       *
       * Contrue the ldatainfo filename.
       *
       */
      sprintf(ldataFilename,"%s/_latest_data_info",Dir);

      /*
       *
       * If the file exists, remove it.
       * This alleviates cross-mount problems.
       *
       */
      unlink(ldataFilename);
      
      lfp = fopen(ldataFilename,"wt");
      if (lfp != NULL){

	fprintf(lfp,"%ld %d %02d %02d %02d %02d %02d\n",
		(unsigned long)T.unix_time, T.year, T.month, T.day,
		T.hour, T.min, T.sec);

	fprintf(lfp,"mdv\n");

	fprintf(lfp,"%4d%02d%02d/%02d%02d%02d\n",
		    T.year, T.month, T.day,
		    T.hour, T.min, T.sec);

	fprintf(lfp,"%4d%02d%02d/%02d%02d%02d.mdv\n",
		    T.year, T.month, T.day,
		    T.hour, T.min, T.sec);

	  fprintf(lfp,"0\n");

      }
      fclose(lfp);

    }
    /*
     *
     * End of additions to create latest_data_info file.
     *
     */	      


  }

  Output_file = NULL;
  STRfree(Output_filename);
  
  return;
}



