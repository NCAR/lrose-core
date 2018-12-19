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
 * Args.cc: class controlling the command line arguments for the
 *          program.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstring>

#include <toolsa/os_config.h>
#include <toolsa/udatetime.h>
#include <tdrp/tdrp.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Args.hh"

/**********************************************************************
 * Constructor
 */

Args::Args (int argc, char **argv, char *prog_name)
{
  char tmp_str[BUFSIZ];

  // Intialize

  okay = true;
  _startTime = -1;
  _endTime = -1;
  _inputFileList.clear();

  TDRP_init_override(&override);
  
  // search for command options
  
  for (int i =  1; i < argc; i++) {

    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man")) {

      _usage(prog_name, stdout);
      exit(0);

    } else if (STRequal_exact(argv[i], "-debug")) {

      snprintf(tmp_str, BUFSIZ, "debug = true;");
      TDRP_add_override(&override, tmp_str);

    } else if (STRequal_exact(argv[i], "-endtime")) {

      if (i < argc - 1) {
        if ((_endTime = _parseTime(argv[i+1], prog_name,
                                   (char *) "endtime")) < 0) {
          okay = FALSE;
        }
        snprintf(tmp_str, BUFSIZ, "mode = ARCHIVE_MODE;");
        TDRP_add_override(&override, tmp_str);
      } else {
        okay = FALSE;
      }
      
    } else if (STRequal_exact(argv[i], "-mode")) {

      if (i < argc - 1) {
        snprintf(tmp_str, BUFSIZ, "mode = %s_MODE;", argv[++i]);
        TDRP_add_override(&override, tmp_str);
      } else {
        okay = FALSE;
      }

    } else if (STRequal_exact(argv[i], "-starttime")) {

      if (i < argc - 1) {
        if ((_startTime = _parseTime(argv[i+1], prog_name,
                                     (char *) "starttime")) < 0) {
          okay = FALSE;
        }
        snprintf(tmp_str, BUFSIZ, "mode = ARCHIVE_MODE;");
        TDRP_add_override(&override, tmp_str);
      } else {
        okay = FALSE;
      }
      
    } else if (STRequal_exact(argv[i], "-in_url")) {

      if (i < argc - 1) {
        snprintf(tmp_str, BUFSIZ, "input_url = \"%s\";", argv[i+1]);
        TDRP_add_override(&override, tmp_str);
      } else {
        okay = FALSE;
      }
      
    } else if (!strcmp(argv[i], "-f")) {
      
      if (i < argc - 1) {
	// load up file list vector. Break at next arg which
	// start with -
	for (int j = i + 1; j < argc; j++) {
	  if (argv[j][0] == '-') {
	    break;
	  } else {
	    _inputFileList.push_back(argv[j]);
	  }
	}
	sprintf(tmp_str, "mode = FILELIST_MODE;");
	TDRP_add_override(&override, tmp_str);
      } else {
	okay = false;
      }
      
    }
    
  } /* i */

  if (!okay)
  {
    _usage(prog_name, stderr);
  }
    
}


/**********************************************************************
 * Destructor
 */

Args::~Args(void)
{
  TDRP_free_override(&override);
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/


/**************************************************************
 * _parseTime()
 */

time_t Args::_parseTime(char *time_str, char *prog_name, char *label)
{
  date_time_t dtime;

  if (sscanf(time_str, "%d/%d/%d_%d:%d:%d",
             &dtime.year,
             &dtime.month,
             &dtime.day,
             &dtime.hour,
             &dtime.min,
             &dtime.sec) != 6)
  {
    fprintf(stderr, "ERROR - %s:_parseTime()\n", prog_name);
    fprintf(stderr, "Incorrect format for %s time.\n", label);
    fprintf(stderr, "Trying to decode '%s'\n", time_str);
    fprintf(stderr, "Format is yyyy/mm/dd_hh:mm:ss\n");
    return (-1);
  }
    
  /*
   * check that time is valid
   */
  
  if (!uvalid_datetime(&dtime))
  {
    fprintf(stderr, "ERROR - %s:parse_args\n", prog_name);
    fprintf(stderr, "Invalid reference date and time.\n");
    fprintf(stderr, "Trying to decode '%s'\n", time_str);
    return (-1);
  }
    
  uconvert_to_utime(&dtime);

  return (dtime.unix_time);
}


/**********************************************************************
 * _usage() - Print the usage for this program.
 */

void Args::_usage(char *prog_name, FILE *stream)
{
  fprintf(stream, "%s%s%s",
	  "Usage:\n\n", prog_name, " [options] as below:\n\n"
	  "       [ --, -help, -man ] produce this list.\n"
	  "       [ -debug ] debugging on\n"
          "       [ -endtime yyyy/mm/dd_hh:mm:ss ] end time, sets to ARCHIVE_MODE\n"
          "       [ -f ? ] set file paths, sets mode to FILELIST\n"
          "       [ -f ? ] set file paths, sets mode to FILELIST\n"
          "       [ -mode ?] ARCHIVE or REALTIME\n"
          "       [ -starttime yyyy/mm/dd_hh:mm:ss ] start time, sets to ARCHIVE_MODE\n"
          "       [ -in_url ? ] set file paths, sets mode to FILELIST\n"
	  "\n");


  TDRP_usage(stream);
}






