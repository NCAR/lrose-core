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
/*****************************************************************************
*                             wsiIngest.c
*
*      This program is designed to receive WSI NOWRAD wx data which
* is a binary run length uncoded data stream.
*
*      TCP/IP routine development is based on a code sample obtained
* from Arthur Person of Penn State University.
*
* original: K. E. Stringham, Jr. | 31 JAN 1996
*
* Revisions:
*
*    9 APR 1997  S.Maloney  Created more general version 
****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/time.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <toolsa/udatetime.h>
#include <tdrp/tdrp.h>
#include <toolsa/file_io.h>
#include <toolsa/ldata_info.h>
#include <toolsa/membuf.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "fs.h"
#include "Params.hh"

/******************************
   WSI Weather Header STructure
******************************/

struct pType
{
  int  hdrCount;
  char coverage[5];
  char hour[3];
  char mins[3];
  char type[9];
  char day[3];
  char month[4];
  char year[3];
} gProduct;


/******************************
   Global Variable Definitions
******************************/

ErrBlk gErrBlk;

enum
{
  iNone,
  iTooShort,
  iTooLong,
  iNullLines,
  iDumpedImages,
  iUpdate
} gErrIDs;

/**********************
   Function Prototypes
**********************/

int    FlgDetect(ui08 *pBuf,
		 ui08 flg,
		 int bufLen);

ui08  *SrchChr(ui08 *pBuf,
	       int count,
	       ui08 ch);

void   HandleErr(int errType);

void parse_args(int argc,
		char **argv,
		int *check_params_p,
		int *print_params_p,
                char **params_file_path_p,
		tdrp_override_t *override);

void tidy_and_exit(int sig);

/*
 * GLOBALS
 */

char *Program_name;
Params _params;

ui08 wxBuf[kSerIOBufSz];

/********************** main *****************************************/

int main(int argc, char **argv)
{
  ui08         wsiBuffer, wxHdr[kOutBufSz], *pBuf;
  struct pType product;
  int          count, status, i;
  MEMbuf       *header_buffer;
  
  time_t       data_time;
  date_time_t  data_time_struct;
  
  path_parts_t progname_parts;
  int check_params;
  int print_params;
  tdrp_override_t override;
  
  int output_type;

  char         output_subdir[MAX_PATH_LEN];
  char         output_filename[MAX_PATH_LEN];
  char         output_path[MAX_PATH_LEN];
  char         temp_filename[MAX_PATH_LEN];
  FILE         *updateFD;
  LDATA_handle_t ldata_handle;
  

  char cmon[][6]={"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  char wsitime[20], tempstr[10];

  /*
   * set signal traps
   */

  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);

  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Program_name = STRdup(progname_parts.base);
  
  /*
   * display ucopright message
   */

  ucopyright(Program_name);

  /*
   * parse command line arguments
   */

  char *params_path = (char *) "unknown";
  parse_args(argc, argv, &check_params, &print_params,
	     &params_path, &override);
  
  /*
   * load up parameters
   */
  
  
  if (_params.loadFromArgs(argc, argv,
                           override.list,
                           &params_path))
  {
    fprintf(stderr, "ERROR - %s:main\n", Program_name);
    fprintf(stderr, "Problems with params file '%s'\n",
	    params_path);
    tidy_and_exit(-1);
  }
  
  /*
   * Initialize the memory buffer used for collecting the header information for
   * the file.
   */

  header_buffer = MEMbufCreate();
  
  /*
   * Initialize the LDATA handle.
   */

  LDATA_init_handle(&ldata_handle,
		    Program_name,
		    _params.debug_level >= Params::DEBUG_NORM);
  
  /*
   * Initialize process registration
   */

  PMU_auto_init(Program_name, _params.instance, PROCMAP_REGISTER_INTERVAL);
  
  /**************************** Main Processing Begins ************************/


  if (_params.debug_level >= Params::DEBUG_NORM)
    fprintf(stderr, "\nInitializing wsiIngest\n");

  /***** Open the network port for communications *****/

  PMU_auto_register("Openning network port");
  
  if (open_net(_params.wsi_ip_address, _params.wsi_port) < 0)
  {
    fprintf(stderr, "Error Opening Network Port\n");
    exit(-1);
  }
  else
  {
    if (_params.debug_level >= Params::DEBUG_NORM)
      fprintf(stderr, "Opened Network Port\n");
  }

  /* start the data flowing */
  if (_params.debug_level >= Params::DEBUG_NORM)
  {
    fprintf(stderr, "Starting wsiIngest Processing...\n");
    fprintf(stderr, "Waiting for image data header\n");
  }
  
  /****** Main Decoder Loop *****/

  FOREVER
  {
    int byte_count;
    ui08 mem_buf_byte;
    
    PMU_auto_register("Checking WSI data feed for new files");
    
    gErrBlk.errCount = 0;
    memset(wxBuf, 0, sizeof(wxBuf));
    memset(wxHdr, 0, sizeof(wxHdr));

    if (_params.debug_level >= Params::DEBUG_EXTRA)
      fprintf(stderr, "Waiting for kHdrSg flag\n");
    
    byte_count = FlgDetect(wxBuf, kHdrSg, sizeof(wxBuf));
    
    if (_params.debug_level >= Params::DEBUG_EXTRA)
      fprintf(stderr, "Got kHdrSg flag, byte_count = %d\n", byte_count);
    
    if (byte_count == -1)   /* Wait For Header Flag */
    {
      fprintf(stderr, "\nFlag Detection Routine Returned an Error\n");
      exit( -1);
    }

    /*
     * The header flag was found.  Reset the header memory buffer and add
     * the flag to it.
     */

    MEMbufReset(header_buffer);
    
    mem_buf_byte = 0x00;
    MEMbufAdd(header_buffer, &mem_buf_byte, 1);
    
    mem_buf_byte = 0xF0;
    MEMbufAdd(header_buffer, &mem_buf_byte, 1);
    
    mem_buf_byte = kHdrSg;
    MEMbufAdd(header_buffer, &mem_buf_byte, 1);
    
    /*
     * Read the header size from the stream.
     */

    if (ReadBuf(Program_name, (ui08 *)&wsiBuffer, 1) == -1)
    {
      memset(gErrBlk.errArray, '\0', sizeof(gErrBlk.errArray));
      sprintf(gErrBlk.errArray[0], "\n%s Error:\n", Program_name);
      sprintf(gErrBlk.errArray[1],"\nReadBuf: Header Byte Count\n");

      HandleErr(iNone);
      exit(-1);
    }

    MEMbufAdd(header_buffer, &wsiBuffer, 1);
    
    product.hdrCount = wsiBuffer;

    if (product.hdrCount != 0x15)
    {
      memset(gErrBlk.errArray, '\0', sizeof(gErrBlk.errArray));
      sprintf(gErrBlk.errArray[0], "\n%s Error:\n", Program_name);
      sprintf(gErrBlk.errArray[1],
	      "\nImage Header Size Error: %02X!",
	      product.hdrCount);
                           
      HandleErr(iNone);
      continue;
    }

    /*
     * Now read the header information
     */

    if (ReadBuf(Program_name, wxHdr, product.hdrCount) == EOF)
    {
      memset(gErrBlk.errArray, '\0', sizeof(gErrBlk.errArray));
      sprintf(gErrBlk.errArray[0], "\n%s Error:\n", Program_name);
      sprintf(gErrBlk.errArray[1], 
	      "\nReadBuf: Header Read Error\n");

      HandleErr(iNone);
      exit(-1);
    }

    MEMbufAdd(header_buffer, wxHdr, product.hdrCount);
    
    /*
     * Retrieve the coverage code and time from the header
     * information.
     */

    sscanf((char *)wxHdr, "%s%2s:%2s%s",
	   product.coverage,
	   product.hour,
	   product.mins,
	   product.type);

    if (!isdigit(product.hour[0]) || !isdigit(product.hour[1]) ||
	!isdigit(product.mins[0]) || !isdigit(product.mins[1]))
    {
      memset(gErrBlk.errArray, '\0', sizeof(gErrBlk.errArray));
      sprintf(gErrBlk.errArray[0], "\n%s Error:\n", Program_name);
      sprintf(gErrBlk.errArray[1],
	      "\nError in product time: %s", product.hour);
      sprintf(gErrBlk.errArray[2],
	      "\nError in product time: %s", product.mins);

      HandleErr(iNone);
      continue;
    }

    if (_params.debug_level >= Params::DEBUG_NORM)
    {
      fprintf(stderr, "\nWSI Wx Product Header Detected");
      fprintf(stderr, "   Data time: %s:%sGMT",  product.hour, product.mins);
      fprintf(stderr, " Type: %s", product.type);
      fprintf(stderr, " hdrCount: %d  coverage: %s\n",
	      product.hdrCount, product.coverage);
    }
    
    /*
     * Check for desired product type and coverage.
     *
     * Just to record some of what is available, Lincoln was using the
     * following types and coverages:
     *
     *      product type             coverage
     *      ------------             --------
     *      NOWRADHD                 NA
     *      NOWRADHF                 NA
     *      PRECIPLS                 NA
     *      NEXVI                    MS
     *      NEXET                    MS
     *      NEXCR                    MS
     *      NEXLL                    MS
     *      NEXLM                    MS
     *      NEXLH                    MS
     *      SPECIAL                  NA
     *
     * The NOWrad documentation lists the following types:
     *
     *      product type             coverage
     *      ------------             --------
     *      NOWRADHD                 NA           (NOWradPLUS Master)
     *      USRADHD                  NA           (NOWradPLUS National)
     *      NOWRADHF                 NA           (NOWradPLUS Master-5 min)
     *      USRADHF                  NA           (NOWradPLUS National-5 min)
     *      NEXET                    MS           (Echo Tops Mosaic)
     *      NEXET                    US           (Echo Tops Mosaic)
     *      NEXVI                    MS           (VIL Mosaic)
     *      NEXVI                    US           (VIL Mosaic)
     */

    output_type = -1;
    
    for (i = 0; i < _params.output_data_n; i++)
    {
      if ((STRequal_exact(product.type,     _params._output_data[i].product_type) &&
	   STRequal_exact(product.coverage, _params._output_data[i].product_coverage)))
      {
	if (_params.debug_level >= Params::DEBUG_NORM)
	  fprintf(stderr, "Header selected for archive, starting processing...\n");

	output_type = i;
	break;
      }
    } /* endfor - i */

    if (output_type < 0)
      continue;
    
    /*
     * Get the rest of the header information from the
     * stream.
     */

    if (_params.debug_level >= Params::DEBUG_NORM)
      fprintf(stderr, "Accumulating Data...\n");

    status = FlgDetect(wxBuf, kLinSg, sizeof(wxBuf));
    
    if (status < 0 )
    {
      memset(gErrBlk.errArray, '\0', sizeof(gErrBlk.errArray));
      sprintf(gErrBlk.errArray[0], "\n%s Error:\n", Program_name);
      sprintf(gErrBlk.errArray[0],
	      "\nLine Segment Size Error: %0X!", status);
      HandleErr(iNone);
      exit(-1);
    }

    MEMbufAdd(header_buffer, wxBuf, status);
    
    /*
     * Retrieve the date information from the rest of the header.
     */

    pBuf = SrchChr(wxBuf, status, ':') + 3;

    sscanf((char *)pBuf, "%2s-%3s-%2s",
	   product.day,
	   product.month,
	   product.year);

    /*
     * Get time6 equiv data time
     */

    sprintf(wsitime, "%s%s_%s_%s%s",
	    product.month,
	    product.day,
	    product.year,
	    product.hour,
	    product.mins);

    /*
     * Get integer month from month string
     */

    for (i = 0; i < 12; i++)
    {
      sprintf(tempstr, "%c%c%c", wsitime[0], wsitime[1], wsitime[2]);
      if (strstr(tempstr, cmon[i]) != (char *)NULL)
      {
	data_time_struct.month = i + 1;
	break;
      }
    }  
 
    sprintf(tempstr, "%c%c", wsitime[3], wsitime[4]);
    data_time_struct.day = atoi(tempstr);
    sprintf(tempstr, "%c%c", wsitime[6], wsitime[7]);
    data_time_struct.year = atoi(tempstr);
    sprintf(tempstr, "%c%c", wsitime[9], wsitime[10]);
    data_time_struct.hour = atoi(tempstr);
    sprintf(tempstr, "%c%c", wsitime[11], wsitime[12]);
    data_time_struct.min = atoi(tempstr);
    data_time_struct.sec = 0;
    
    /*
     * Set the filename base time string to match itws archive format
     */

    sprintf(output_filename, "%02d%02d%02d.%s", 
	    data_time_struct.hour, data_time_struct.min, data_time_struct.sec,
	    _params._output_data[output_type].output_ext);

    count = FlgDetect(wxBuf, kEndSg, sizeof(wxBuf));
    
    if (count < 0)
    {
      memset(gErrBlk.errArray, '\0', sizeof(gErrBlk.errArray));
      sprintf(gErrBlk.errArray[0], "\nGWSWX Error:\n");
      sprintf(gErrBlk.errArray[1], "\nBuffer Length Error\n");

      HandleErr(iNone);
      exit(-1);
    }

    MEMbufAdd(header_buffer, wxBuf, count);
    
    gErrBlk.imagesRcvD++;

    /*
     * Set the path for the archived image file
     */

    if (data_time_struct.year > 50)
      data_time_struct.year += 1900;
    else
      data_time_struct.year += 2000;
    
    sprintf(output_subdir, "%04d%02d%02d",
	    data_time_struct.year, data_time_struct.month,
	    data_time_struct.day);
    
    sprintf(temp_filename, "%s/%s",
            _params._output_data[output_type].output_dir, output_subdir);
    
    if (_params.debug_level >= Params::DEBUG_NORM)
      fprintf(stderr, "Making output subdirectory <%s>\n", temp_filename);
    
    if (makedir(temp_filename) != 0)
    {
      memset(gErrBlk.errArray, '\0', sizeof(gErrBlk.errArray));
      sprintf(gErrBlk.errArray[0], "\n%s Error:\n", Program_name);
      sprintf(gErrBlk.errArray[1], 
	      "\nUnable to make directory: %s\n", temp_filename);

      HandleErr(iNone);
      continue;
    }

    sprintf(output_path, "%s/%s/%s",
	    _params._output_data[output_type].output_dir,
	    output_subdir, output_filename);

    if (_params.debug_level >= Params::DEBUG_NORM)
      fprintf(stderr, "Writing wsi raw image data to %s\n", output_path);

    /*
     * Save the Image to File
     */

    if ((updateFD = fopen(output_path, "w")) == (FILE *)NULL)
    {
      memset(gErrBlk.errArray, '\0', sizeof(gErrBlk.errArray));
      sprintf(gErrBlk.errArray[0], "\n%s Error:\n", Program_name);
      sprintf(gErrBlk.errArray[1], 
	      "\nUnable to Open: %s for Write\n", output_path);

      HandleErr(iNone);
    }
    else
    {
      fwrite(MEMbufPtr(header_buffer), 1, MEMbufLen(header_buffer), updateFD);
      fclose(updateFD);
    }

    /*
     * Update the LDATA information.
     */

    sprintf(temp_filename, "%s/%s",
	    output_subdir, output_filename);
    
    data_time = uunix_time(&data_time_struct);
    
    if (LDATA_info_write(&ldata_handle,
			 _params._output_data[output_type].output_dir,
			 data_time,
			 _params._output_data[output_type].output_ext,
			 temp_filename,
			 (char *)NULL,
			 0,
			 (int *)NULL) != 0)
    {
      memset(gErrBlk.errArray, '\0', sizeof(gErrBlk.errArray));
      sprintf(gErrBlk.errArray[0], "\n%sError:\n", Program_name);
      sprintf(gErrBlk.errArray[1],
	      "Error returned from LDATA_info_write()\n");
      
      HandleErr(iNone);
    }
    
    if (_params.debug_level >= Params::DEBUG_NORM)
    {
      fprintf(stderr, "Done with write\n");
      fprintf(stderr, "Waiting for next data header\n");
    }
    
  } /* END FOREVER LOOP */
}

/********************************************************************

    HandleErr()

    Function designed to process error messages to an error file and
    to display them to the stdout.

********************************************************************/

void HandleErr(int errType)
{
  time_t      curTime;
  char        *timeDate;
  int         i;

  curTime  = time( NULL);
  timeDate = ctime( &curTime);

  fprintf(stderr, "\n%s", timeDate);

  for (i = 0; gErrBlk.errArray[i][0]; i++)
    fprintf(stderr, "%s", gErrBlk.errArray[i]);

  fprintf(stderr, "\n");

  switch( errType)
  {
  case iTooShort:
    gErrBlk.shortLines++;
    break;
  case iTooLong:
    gErrBlk.longLines++;
    break;
  case iNullLines:
    gErrBlk.nulledLines++;
    break;
  case iDumpedImages:
    gErrBlk.imagesDumped++;
    break;
  }

  return;
}



/*********** Flag Detector Function ******************/

int FlgDetect( ui08 *read_buffer, ui08 flg, int bufLen)
{
  ui08 flg0  = FALSE;
  ui08 flgF0 = FALSE;
  int    count = 0;

  FOREVER
  {
    PMU_auto_register("Inside FlgDetect");
    
    if (ReadBuf(Program_name, read_buffer, 1) == -1)
      return(-1);

    if (!flg0 && flgF0 == 0xF0 && *read_buffer == flg)
    {
      if (flg == kLinSg && _params.debug_level >= Params::DEBUG_NORM)
	fprintf(stderr, "kLinSg flag found at position %d\n", count);
      
      return(count+1);
    }
    
    flg0  = flgF0;
    flgF0 = *read_buffer;

    if (count++ > bufLen)
    {
      switch(flg)
      {
      case kEndSg:
	if (_params.debug_level >= Params::DEBUG_NORM)
	  fprintf(stderr, "\nBuffer Length Exceeded: End Segment Flag");
	break;

      case kHdrSg:
	if (_params.debug_level >= Params::DEBUG_NORM)
	  fprintf(stderr, "\nBuffer Length Exceeded: Header Segment Flag");
	break;

      case kTagSg:
	if (_params.debug_level >= Params::DEBUG_NORM)
	  fprintf(stderr, "\nBuffer Length Exceeded: Tag Segment Flag");
	break;

      case kNavSg:
	if (_params.debug_level >= Params::DEBUG_NORM)
	  fprintf(stderr, "\nBuffer Length Exceeded: Navigation Segment Flag");
	break;
      }
           
      if (_params.debug_level >= Params::DEBUG_NORM)
	fprintf(stderr, "Count        : %d\n", count);
      return( -1);
    }

    read_buffer++;
  }
}

/*********** Byte Search within Binary Buffer *****/

ui08 *SrchChr( ui08 *pBuf, int count, ui08 ch)
{
  int inx;

  for (inx = count - 1; inx; inx--, pBuf++)
    if (*pBuf == ch)
      return(pBuf);

  return((ui08 *)NULL);
}

/*********** Parse command line arguments *****/

void parse_args(int argc,
		char **argv,
		int *check_params_p,
		int *print_params_p,
                char **params_file_path_p,
                tdrp_override_t *override)
{
  int error_flag = 0;
  int i;

  char usage[BUFSIZ];
  char tmp_str[BUFSIZ];

  /*
   * set usage string
   */

  sprintf(usage, "%s%s%s",
	  "Usage: ",
	  Program_name,
	  " [options as below] [-if input_file_list (ARCHIVE mode)]\n"
	  "options:\n"
	  "       [ --, -h, -help, -man, -usage] produce this list.\n"
	  "       [ -check_params ] check parameter usage\n"
	  "       [ -debug ] print debug messages\n"
	  "       [ -mdebug level ] malloc debug level\n"
	  "       [ -params path ] specify params file\n"
	  "       [ -print_params ] print parameter usage\n"
	  "\n");

  /*
   * initialize
   */

  *check_params_p = FALSE;
  *print_params_p = FALSE;
  TDRP_init_override(override);

  /*
   * look for command options
   */

  for (i =  1; i < argc; i++)
  {
    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man") ||
	STRequal_exact(argv[i], "-usage"))
    {
      fprintf(stderr, "%s", usage);
      exit(0);
    }
    else if (STRequal_exact(argv[i], "-check_params"))
    {
      *check_params_p = TRUE;
    }
    else if (STRequal_exact(argv[i], "-debug"))
    {
      sprintf(tmp_str, "debug = Params::DEBUG_NORM;");
      TDRP_add_override(override, tmp_str);
    }
    else if (STRequal_exact(argv[i], "-mdebug"))
    {
      if (i < argc - 1)
      {
	sprintf(tmp_str, "malloc_debug_level = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
      }
      else
      {
	error_flag = TRUE;
      }
	
    }
    else if (STRequal_exact(argv[i], "-params"))
    {
      if (i < argc - 1)
      {
        *params_file_path_p = argv[i+1];
      }
      else
      {
        error_flag = TRUE;
      }
    }
    else if (STRequal_exact(argv[i], "-print_params"))
    {
      *print_params_p = TRUE;
    } /* if */
    
  } /* i */

  /*
   * print message if error flag set
   */

  if(error_flag)
  {
    fprintf(stderr, "%s", usage);
    exit(-1);
  }

  return;

}

/*********** Clean exit from program *****/

void tidy_and_exit(int sig)
{
  /*
   * unregister process
   */

  PMU_auto_unregister();

  /*
   * check memory allocation
   */

  umalloc_map();
  umalloc_verify();
  
  /*
   * exit with code sig
   */

  exit(sig);
}
