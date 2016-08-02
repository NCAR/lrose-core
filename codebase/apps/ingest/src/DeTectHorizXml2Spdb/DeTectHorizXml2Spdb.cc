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
/**
 *
 * @file DeTectHorizXml2Spdb.cc
 *
 * @class DeTectHorizXml2Spdb
 *
 * DeTectHorizXml2Spdb program object.
 *  
 * @date 9/6/2011
 *
 */

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <DeTect/DeTectHorizontalSpdb.hh>
#include <DeTect/DeTectHorizontalTable.hh>
#include <DeTect/DeTectMetadataTable.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Params.hh"
#include "DeTectHorizXml2Spdb.hh"

using namespace std;

// Global variables

DeTectHorizXml2Spdb *DeTectHorizXml2Spdb::_instance =
     (DeTectHorizXml2Spdb *)NULL;


/*********************************************************************
 * Constructors
 */

DeTectHorizXml2Spdb::DeTectHorizXml2Spdb(int argc, char **argv)
{
  static const string method_name = "DeTectHorizXml2Spdb::DeTectHorizXml2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (DeTectHorizXml2Spdb *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = new char(strlen("unknown") + 1);
  strcpy(params_path, "unknown");
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
  }

}


/*********************************************************************
 * Destructor
 */

DeTectHorizXml2Spdb::~DeTectHorizXml2Spdb()
{
  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst()
 */

DeTectHorizXml2Spdb *DeTectHorizXml2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (DeTectHorizXml2Spdb *)NULL)
    new DeTectHorizXml2Spdb(argc, argv);
  
  return(_instance);
}

DeTectHorizXml2Spdb *DeTectHorizXml2Spdb::Inst()
{
  assert(_instance != (DeTectHorizXml2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool DeTectHorizXml2Spdb::init()
{
  static const string method_name = "DeTectHorizXml2Spdb::init()";
  
  if (_params->verbose)
    _params->debug = pTRUE;
  
  return true;
}


/*********************************************************************
 * run()
 */

void DeTectHorizXml2Spdb::run()
{
  static const string method_name = "DeTectHorizXml2Spdb::run()";
  
  _processFiles();
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processFiles()
 */

bool DeTectHorizXml2Spdb::_processFiles()
{
  static const string method_name = "DeTectHorizXml2Spdb::_processFiles()";
  
  if (_params->debug || _params->verbose)
  {
    cerr << endl;
    cerr << "*** Horizontal table XML file path: "
	 << _params->horizontal_xml_path << endl;
    cerr << "    Metadata table XML file path: "
	 << _params->metadata_xml_path << endl;
  }
  
  // Read the Metadata table

  DeTectMetadataTable metadata_table(_params->debug || _params->verbose);

  if (!metadata_table.init(_params->metadata_xml_path))
    return false;
  
  // Read the Horizontal table

  DeTectHorizontalTable horiz_table(_params->debug || _params->verbose);
  
  if (!horiz_table.init(_params->horizontal_xml_path, metadata_table))
    return false;
  
  // Loop through the entries in the horizontal table, creating SPDB objects
  // and adding them to the database

  DsSpdb spdb;
  spdb.setPutMode(Spdb::putModeAdd);

  const vector< DeTectHorizontal > &entries = horiz_table.getEntries();
  
  for (vector< DeTectHorizontal >::const_iterator entry = entries.begin();
       entry != entries.end(); ++entry)
  {
    DeTectHorizontal horiz(*entry);
    
    // If requested, shift the locations of the track vertices to assume the
    // new radar location.

    if (_params->shift_radar_location)
      horiz.shiftTrack(_params->radar_location.lat,
		       _params->radar_location.lon);
    
    if (_params->verbose)
      entry->print(cerr);
    
    DeTectHorizontalSpdb detect_spdb;

    if (!detect_spdb.init(horiz, _params->use_pixel_grid_track))
      continue;
    
    if (_params->verbose)
      detect_spdb.print(cerr);
    
    // Add the record to the SPDB database

    if (!detect_spdb.assemble())
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error assembling record for SPDB database" << endl;
      cerr << detect_spdb.getErrStr() << endl;
      
      continue;
    }
    
    spdb.addPutChunk(0,
		     entry->getDate().utime(),
		     entry->getDate().utime() + _params->entry_expire_secs,
		     detect_spdb.getBufLen(), (void *)detect_spdb.getBufPtr());
    
  } /* endfor - entry */
  
  // Write the records to the output database

  if (spdb.put(_params->output_url, SPDB_GENERIC_POLYLINE_ID,
	       SPDB_GENERIC_POLYLINE_LABEL) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing data to output URL: " << _params->output_url << endl;
    cerr << spdb.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}
