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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * NrlLtg2Spdb.cc: Program to convert NRL lightning data into
 *                 SPDB format.
 *
 * RAP, NCAR, Boulder CO
 *
 * Aug 2005
 *
 * Dan Megenhardt
 *
 *********************************************************************/

#include <cassert>
#include <signal.h>
#include <unistd.h>
#include <cstdlib>

#include <rapformats/ltg.h>
#include <toolsa/ldata_info.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>

#include "NrlLtg2Spdb.hh"
using namespace std;

// Constructor

NrlLtg2Spdb::NrlLtg2Spdb(int argc, char **argv)
{

  isOK = true;
  _inputPath = NULL;
  
  // set programe name
  
  _progName = "NrlLtg2Spdb";
  ucopyright((char *) _progName.c_str());
  
  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }
  
  // input path object
  
  if (_params.mode == Params::ARCHIVE) {
    
    if (_args.startTime != 0 && _args.endTime != 0) {
      
      _inputPath = new DsInputPath((char *) _progName.c_str(),
				   _params.debug >= Params::DEBUG_VERBOSE,
				   _params.input_dir,
				   _args.startTime,
				   _args.endTime);
      
    } else {
      
      cerr << "ERROR: " << _progName << endl;
      cerr << "In ARCHIVE mode, you must set start and end times." << endl;
      _args.usage(_progName, cerr);
      isOK = false;

    }

  } else {

    _inputPath = new DsInputPath((char *) _progName.c_str(),
				 _params.debug >= Params::DEBUG_VERBOSE,
				 _params.input_dir,
				 _params.max_realtime_valid_age,
				 PMU_auto_register);
    
  }
  
  // Create the output objects
  
  _spdb.setPutMode(Spdb::putModeOver);
  _spdb.setAppName(_progName);

  for(int i = 0; i < _params.spdb_urls_n; i++) {
    _spdb.addUrl(_params._spdb_urls[i]);
  }

  // initialize process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
}


///////////////////////////////////////
// Destructor

NrlLtg2Spdb::~NrlLtg2Spdb()
{

  // unregister process

  PMU_auto_unregister();

  // free up

  if (_inputPath) {
    delete _inputPath;
  }

}


//////////////////////////////////////////
// Run()

int NrlLtg2Spdb::Run()

{

  char *input_filename;

  _spdb.clearPutChunks();

  // Process any new files

  while ((input_filename = _inputPath->next()) != NULL)
  {
    time_t buffer_time = 0;

    buffer_time = _inputPath->getDataTime(input_filename);
    if(buffer_time < 0)
    {
      fprintf(stderr,
                "%s: ERROR: input file name <%s> must be hhmmss.aftn\n",
                _progName.c_str(), input_filename);
      continue;
    }
    input_file_time.unix_time = buffer_time;
    uconvert_from_utime(&input_file_time);
      
    // Register with the process mapper

    char procmap_string[BUFSIZ];
    Path path(input_filename);
    sprintf(procmap_string, "Processing file <%s>", path.getFile().c_str());
    PMU_force_register(procmap_string);
    
    FILE *input_file;
      
    // Allow the file activity to finish before processing the file

    if (_params.debug)
      fprintf(stderr,
		"New data in file <%s>\n", input_filename);

    // Open the input file.

    if ((input_file = fopen(input_filename, "rb")) == NULL)
    {
      fprintf(stderr,
		"%s: ERROR: Error opening input file <%s>\n",
		_progName.c_str(), input_filename);
      continue;
    }
      
    // Process the data.  Note that NRLLTG_read_strike() returns
    // a pointer to static memory that should NOT be freed.

    LTG_strike_t *strike;
    int strike_count = 0;
    _spdbBuf.free();

    Path ifName(input_filename);
    
    while ((strike =
	    NRLLTG_read_strike(input_file,
			       (char *) ifName.getFile().c_str())) != NULL) {

      // Register with the process mapper again just in case this
      // is a long file
      
      PMU_auto_register("Read file completed");
      
      // Add the strike to the buffer
      
      _spdbBuf.add(strike, sizeof(LTG_strike_t));
      strike_count++;
      
      
    } /* endwhile - strike != NULL */
      
    // Send the strikes in the file to the SPDB destinations
    _sendSpdbData((LTG_strike_t *) _spdbBuf.getPtr(), strike_count);

    if (_params.debug) {
      fprintf(stderr, "Found %d strikes in input file <%s>\n",
	      strike_count, input_filename);
    }
      
    // Close the input file.

    fclose(input_file);

  } /* endwhile - input_filename != NULL */

  return 0;
    
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _sendSpdbData()
 */

void NrlLtg2Spdb::_sendSpdbData(LTG_strike_t *strike_buffer,
				int num_strikes)

{
  time_t strike_time = input_file_time.unix_time;
  
  if (_params.debug)
    fprintf(stderr,
	    "    %d strikes in SPDB buffer for %s\n",
	    num_strikes, utimstr(strike_time));

  // Make sure the lightning data is in big-endian format
  
  for (int strike = 0; strike < num_strikes; strike++) {
    if (_params.debug) {
      LTG_print_strike(stderr, &(strike_buffer[strike]));
    }
    LTG_to_BE(&(strike_buffer[strike]));
  }
  
  _spdb.put(SPDB_LTG_ID, SPDB_LTG_LABEL, 0,
            strike_time, strike_time + _params.expire_secs,
            num_strikes * sizeof(LTG_strike_t),
            (void *)strike_buffer);

}

/************************************************************************
 * NRLLTG_read_strike(): Read a NRL lightning strike from an input
 *                       file.  Bytes are swapped as necessary.
 *
 * Returns a pointer to a static structure (or NULL on error or EOF).
 * Do NOT free this pointer.
 */

LTG_strike_t *NrlLtg2Spdb::NRLLTG_read_strike
     (FILE *input_file, char *prog_name)
{
  static LTG_strike_t strike_info;

  #define MYBUFFSIZE 255
  char buffer[MYBUFFSIZE+1];
  char buffer2[MYBUFFSIZE+1];
  date_time_t dt;
  int strike_utime=0;
  int msec;

  memset(buffer, 0, MYBUFFSIZE+1);
  while (fgets(buffer, MYBUFFSIZE, input_file) != NULL)
  {
     if (ferror(input_file))
       return(NULL);
     if (buffer[0] != '\n')
       break;
     buffer[0] = '\0';
  }

  if(buffer[0] == '\0' ||  ferror(input_file))
    return(NULL);

  if (_params.debug) {
    fprintf(stderr, "Input line: %s", buffer);
  }

  memcpy(&dt, &input_file_time, sizeof(date_time_t));
  if(sscanf(buffer, "%d:%f:%f:%d:%d:%d:%d", &(dt.unix_time), 
	    &(strike_info.latitude), &(strike_info.longitude), 
	    buffer2,&(strike_info.amplitude), buffer2, buffer2) != 7)
    return(NULL);

  strike_info.longitude = strike_info.longitude - 360;
  
//  uconvert_to_utime(&dt);
  strike_info.time = dt.unix_time;
  strike_info.type = LTG_GROUND_STROKE;
  return(&strike_info);
}
