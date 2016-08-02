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
 * Pirep2Spdb.cc: Program to convert pirep data into
 *                 SPDB format.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2013
 *
 * George McCabe
 *
 *********************************************************************/

// C++ include files
#include <cstdio>
#include <cerrno>
#include <toolsa/os_config.h>
#include <fstream>
#include <streambuf>

#include<sstream>
#include<iterator>
#include<vector>

// System/RAP include files
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <didss/DsInputPath.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/TaXml.hh>

#include <boost/regex.hpp>

// Local include files
#include "Pirep2Spdb.hh"
#include "PirepDecoder.hh"

using namespace std;

/////////////////////////////////////////////////////
// Constructor

Pirep2Spdb::Pirep2Spdb(int argc, char **argv)

{
  isOK = true;
  _inputPath = NULL;

  // set programe name
  
  _progName = "Pirep2Spdb";
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
  
  _decoder = new PirepDecoder();

  // initialize process registration
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);  
}

///////////////////////////////
// Destructor

Pirep2Spdb::~Pirep2Spdb()
{

  // unregister process

  PMU_auto_unregister();

  // Free contained objects

  if (_inputPath) {
    delete _inputPath;
  }

  if (_decoder) {
    delete _decoder;
  }

}

/////////////////////////////////////
// Run()

int Pirep2Spdb::Run()

{
  int iret = 0;

  char *input_filename;
  while((input_filename = _inputPath->next()) != NULL)
  {
    // create SPDB object
    DsSpdb spdbXml;
    DsSpdb spdbRaw;
    spdbXml.setAppName(_progName);
    spdbRaw.setAppName(_progName);
    cout << " File is: " << input_filename << endl;
    // try to get time from input file name, since this will be used
    // to compute airep time
    
    _inputFileTime.unix_time = _inputPath->getDataTime(input_filename); 
    if(_inputFileTime.unix_time < 0)
    {
      _inputFileTime.unix_time = time(NULL);
    }
    uconvert_from_utime(&_inputFileTime);
    _airepTime = _inputFileTime;
    
    // Process the input file
    // Register with the process mapper

    char procmap_string[BUFSIZ];
    Path path(input_filename);
    sprintf(procmap_string, "File <%s>", path.getFile().c_str());
    PMU_auto_register(procmap_string);
      
    if (_params.debug) {
      cerr << "New data in file: " << input_filename << endl;
    }

    // 
    // open the file into a fstream
    // use get_line and END_DELIMTER to delimit pireps
    // copy lines to a vector of strings; result is a array of reports
    // use isalpha and toupper to convert all characters to upper case
    // use a filter to convert all '/015' to whitespace
    //
    ifstream in_file;
    in_file.open(input_filename);
    if(!in_file) {
      cerr << "Unable to open file: " 
	   << input_filename << "; continuing ..." << endl;
      continue;
    }

    if(_params.input_type == Params::INPUT_CSV)
    {
      vector<Pirep> pireps;
      string line;
      getline(in_file, line); // skip header
      while(getline(in_file, line)) {
        if(line.size() == 0){ 
	  continue;
        }
        istringstream ss(line);

        //putting all the tokens in the vector
        vector<string> arrayTokens;//(begin, end); 

        while(ss)
	{
          string s;
          if(!getline(ss, s, ',')) break;
          arrayTokens.push_back(s);
        }

	if(arrayTokens[0].compare("PIREP") == 0)
	{
	  Pirep p;
          if(!_decoder->createPirepObject(arrayTokens, p))
	  {
	    cout << "adding chunk: " << endl;
            cout << p.getXml() << endl;
            if(_params.write_decoded_spdb)
	    {
              _decoder->addXmlSpdb(&spdbXml, &p, _params.expire_secs);
	    }
            if(_params.write_ascii_spdb)
	    {
              _decoder->addRawSpdb(&spdbRaw, &p, _params.expire_secs);
	    }
	  }
        }
        

      }
    }
    else// if input is Params::INPUT_ADDS_XML
    {
      vector<Pirep> pireps;
      stringstream buffer;
      buffer << in_file.rdbuf();
      vector<string> xmlVec;
      if (TaXml::readStringArray(buffer.str(), "AircraftReport", xmlVec) == 0)
      {
        for (size_t ii = 0; ii < xmlVec.size(); ii++)
        {
          const string &xml = xmlVec[ii];      
          if(_params.input_type == Params::INPUT_XML_OTHER)
          {
            cout << "XML_OTHER input format not implemented" << endl;
            return -3;
          }
	  else // INPUT_XML_ADDS
          {
            Pirep p;
            if(!_decoder->createPirepObject(xml,p,_params.xml_names))
            {
              time_t obs = p.getObsTime();
              cout << "obs: " << obs << endl;

	    cout << "adding chunk " << endl;
            if(_params.write_decoded_spdb)
	    {
              _decoder->addXmlSpdb(&spdbXml, &p, _params.expire_secs);
	    }
            if(_params.write_ascii_spdb)
	    {
              _decoder->addRawSpdb(&spdbRaw, &p, _params.expire_secs);
	    }
	  }
	  
          }	
        }     
      }
    }    
    // Write
    if(_params.write_decoded_spdb)
    {
      _writeSpdb(spdbXml, _params.decoded_spdb_url);
    }

    if(_params.write_ascii_spdb)
    {
      _writeSpdb(spdbRaw, _params.ascii_spdb_url);
    }

  } // while((input_filename = _inputPath->next() ...
    
  return iret;

}

////////////////////////////////
// write ASCII airep to database

int Pirep2Spdb::_writeSpdb(DsSpdb &spdb, string url)
{
  
  if (spdb.nPutChunks() > 0) {
    if (_params.debug) {
      cerr << "Putting ascii pireps to URL: " << url << endl;
    }
    spdb.setPutMode(Spdb::putModeAddUnique);
    if (spdb.put(url,
		 SPDB_ASCII_ID, SPDB_ASCII_LABEL)) {
      cerr << "ERROR - Pirep2Spdb::_writeSpdb" << endl;
      cerr << spdb.getErrStr() << endl;
      return -1;
    }
  }
  
  return 0;

}
