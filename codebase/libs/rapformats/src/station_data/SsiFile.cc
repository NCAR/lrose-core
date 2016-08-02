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
 * SsiFile.cc: Class for collecting data from a raw SSI snow gauge file.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1996
 *
 * Nancy Rehak
 *
 *********************************************************************/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:45:40 $
//   $Id: SsiFile.cc,v 1.5 2016/03/03 18:45:40 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <toolsa/str.h>
#include <toolsa/utim.h>
#include <toolsa/file_io.h>

#include <rapformats/SsiFile.h>

/*
 * Global variables
 */

/**********************************************************************
 * Constructor
 */

SsiFile::SsiFile(const char *input_filename,
		 const char *output_path,
		 const char *output_ext,
		 SsiFileOutputProc map_input_to_output,
		 const int station_id)
{
#ifdef DEBUG
  fprintf(stdout,
	  "Creating SsiFile object for file <%s>\n",
	  input_filename);
#endif

  // Save the file information

  _inputFilename = STRdup(input_filename);
  _outputPath = STRdup(output_path);
  _outputExtension = STRdup(output_ext);
  
  _outputFile = (FILE *)NULL;
  
  // Save the station information

  _stationId = station_id;
  
  // Save the function pointer for mapping input to output

  _mapInputToOutput = map_input_to_output;
  
  // Keep track of when we last received data

  _lastFileUpdateTime = 0;
  _lastDataTime = 0;

  _lastDataTimeStruct.year = 0;
  _lastDataTimeStruct.month = 0;
  _lastDataTimeStruct.day = 0;
  
}

/**********************************************************************
 * Destructor
 */

SsiFile::~SsiFile(void)
{
#ifdef DEBUG
  fprintf(stdout, "Destroying SsiFile object for file <%s>\n",
	  _inputFilename);
#endif

  // Free space used for strings

  STRfree(_inputFilename);
  STRfree(_outputPath);
  STRfree(_outputExtension);
  
  // Close the output file

  if (_outputFile != NULL)
    fclose(_outputFile);
}


/**********************************************************************
 * getNewData() - Get new data from a data file.  The first version gets
 *                data from the input file specified in the constructor.
 *                The second version looks for data with the correct
 *                station id in the given file.  This allows us to get
 *                data from the SSI "current" file, also.
 */

void SsiFile::getNewData(const long input_header_lines)
{
  // Make sure the input data file was really updated

  struct stat file_stat;
  
  if (ta_stat(_inputFilename, &file_stat) != 0)
  {
    fprintf(stderr,
	    "Error %d stating input file <%s>\n",
	    errno, _inputFilename);
    return;
  }
  
  if (_lastFileUpdateTime >= file_stat.st_ctime)
  {
    fprintf(stderr,
	    "Input file <%s> not updated.\n",
	    _inputFilename);
    return;
  }
  
  _lastFileUpdateTime = file_stat.st_ctime;
  
  // Get the data from the file

  getNewData(_inputFilename,
	     input_header_lines);
  
  return;
}

void SsiFile::getNewData(const char *input_filename,
			 const long input_header_lines)
{
#ifdef DEBUG
  fprintf(stdout, "Getting new data from file <%s>.\n",
	  input_filename);
#endif

  // Make sure the input file exists

  struct stat file_stat;
  
  if (ta_stat(input_filename, &file_stat) != 0)
  {
    fprintf(stderr,
	    "Error %d stating input file <%s>\n",
	    errno, input_filename);
    return;
  }
  
  // Open the input file

  FILE *input_file;
  char input_buffer[MAX_BUFFER_LEN];
  char output_buffer[MAX_BUFFER_LEN];
  
  if ((input_file = fopen(input_filename, "r")) == (FILE *)NULL)
  {
    fprintf(stderr,
	    "Error opening input file <%s>\n",
	    input_filename);
    return;
  }
  
  // Skip the header lines in the file

  int line;
  
  for (line = 0; line < input_header_lines; line ++)
    fgets(input_buffer, MAX_BUFFER_LEN, input_file);
  
  // Now read all of the data lines

  time_t data_time;
  int station_id;
  
  while (fgets(input_buffer, MAX_BUFFER_LEN, input_file) != NULL)
  {
    // Remove the carriage return from the end of the line

    input_buffer[strlen(input_buffer)-1] = '\0';
    
    // Map the input line to the desired output line

    _mapInputToOutput(input_buffer, output_buffer, MAX_BUFFER_LEN,
		      &data_time, &station_id);
    
    // See if we really want to output this line

    if (station_id != _stationId)
    {
#ifdef DEBUG
      fprintf(stdout,
	      "Wrong station id: expected %d, got %d\n",
	      _stationId, station_id);
#endif

      continue;
    }
    
    if (data_time <= _lastDataTime)
    {
#ifdef DEBUG
      fprintf(stdout,
	      "Old data: data time %ld, last data %ld\n",
	      data_time, _lastDataTime);
#endif

      continue;
    }
    
    // Now output the line

    _outputDataLine(data_time, output_buffer);
    
    // Update the last data time

    _lastDataTime = data_time;
    UTIMunix_to_date(data_time, &_lastDataTimeStruct);
    
  } /* endwhile - more data in file */
  
  // Close the input file

  fclose(input_file);
  
  return;
}


/**********************************************************************
 * _outputDataLine() - Output the given data line in the output file
 *                     indicated by the data time.
 */

void SsiFile::_outputDataLine(time_t data_time,
			      char *output_buffer)
{
  UTIMstruct data_time_struct;
  char output_filename[MAX_PATH_LEN];
  
  // See if we need to open a new file.  We need to do this if
  // the date of the data changes.  Since we assume that data is
  // received in chronological order and we don't process old data,
  // we know we have to open a new output file if the date changes.

  UTIMunix_to_date(data_time, &data_time_struct);
  
  if (data_time_struct.day != _lastDataTimeStruct.day ||
      data_time_struct.month != _lastDataTimeStruct.month ||
      data_time_struct.year != _lastDataTimeStruct.year ||
      _outputFile == NULL)
  {
    // Close the old data file
    if (_outputFile != NULL)
      fclose(_outputFile);
    
    // Determine the output file name

    sprintf(output_filename,
	    "%s/%04ld%02ld%02ld.%s",
	    _outputPath,
	    data_time_struct.year,
	    data_time_struct.month,
	    data_time_struct.day,
	    _outputExtension);
    
#ifdef DEBUG
    fprintf(stdout,
	    "Opening new output file: <%s>\n",
	    output_filename);
#endif

    // Open the new output file

    if ((_outputFile = fopen(output_filename, "a")) == (FILE *)NULL)
    {
      fprintf(stderr,
	      "Error opening output file <%s>\n",
	      output_filename);
      
      return;
    }
    
  }
  
  // Write the data to the output file

  fprintf(_outputFile, "%s\n", output_buffer);
  fflush(_outputFile);
	  
  return;
}
