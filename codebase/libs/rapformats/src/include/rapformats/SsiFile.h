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
/************************************************************************
 * SsiFile.h : header file for SsiFile object
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1996
 *
 * Nancy Rehak
 *
 ************************************************************************/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 19:23:53 $
 *   $Id: SsiFile.h,v 1.3 2016/03/03 19:23:53 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 *
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

#ifndef SsiFile_H
#define SsiFile_H

/*
 **************************** includes ***********************************
 */

#include <stdio.h>
#include <sys/time.h>

#include <toolsa/utim.h>

/*
 ************************* defines/constants *****************************
 */

const long MAX_BUFFER_LEN = 1024;

/*
 ******************************** types **********************************
 */

typedef void (*SsiFileOutputProc)(const char *input_line,
				  char *output_line,
				  const int output_line_len,
				  time_t *data_time,
				  int *station_id);

/*
 ************************* class definitions *****************************
 */

class SsiFile
{
 public:
  
  // Constructor

  SsiFile(const char *input_filename,
	  const char *output_path,
	  const char *output_ext,
	  SsiFileOutputProc map_input_to_output,
	  const int station_id);

  // Destructor

  ~SsiFile(void);
  
  // Get new data from an input file.  The first version gets the
  // data from the station specific data file specified in the
  // constructor.  The second version looks for data with the
  // correct station id in the given file.  This allows us to get
  // data from the SSI "current" file, also.

  void getNewData(const long input_header_lines);
  void getNewData(const char *input_filename,
		  const long input_header_lines);
  
 private:
  
  char *_inputFilename;
  char *_outputPath;
  char *_outputExtension;
  
  FILE *_outputFile;
  
  SsiFileOutputProc _mapInputToOutput;
  
  int _stationId;
  
  time_t _lastFileUpdateTime;
  time_t _lastDataTime;
  UTIMstruct _lastDataTimeStruct;
  
  // Private member functions

  void _outputDataLine(time_t data_time,
		       char *output_buffer);
  
};


#endif
