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
 * PirepCsv2Spdb.cc: Program to convert pirep data into
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
//#include <Spdb/Product_defines.hh>
#include <toolsa/TaXml.hh>

// Local include files
#include "PirepCsv2Spdb.hh"

using namespace std;

/////////////////////////////////////////////////////
// Constructor

PirepCsv2Spdb::PirepCsv2Spdb(int argc, char **argv)

{
  isOK = true;
  _inputPath = NULL;
  _useStdin = false;


  // set programe name
  
  _progName = "PirepCsv2Spdb";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }
  
  // get TDRP params
  
  
  if (_params.loadFromArgs(argc, argv, _args.override.list,
                           NULL)) {
	  cerr << "ERROR: " << _progName << endl;
	  cerr << "Problem with TDRP parameters" << endl;
	  isOK = false;
	  return;
  }
  
  // input path object
  if (_params.mode == Params::FILELIST) {
    
	  // FILELIST mode
    
	  _inputPath = 
		  new DsInputPath(_progName,
		                  _params.debug >= Params::DEBUG_VERBOSE,
		                  _args.inputFileList);
    
  } else if (_params.mode == Params::ARCHIVE) {
    
	  if (_args.startTime != 0 && _args.endTime != 0) {
      
		  _inputPath = 
			  new DsInputPath((char *) _progName.c_str(),
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

  } else if (_params.mode == Params::STDIN) {
    _useStdin = true;
  } else {

	  _inputPath = 
		  new DsInputPath((char *) _progName.c_str(),
		                  _params.debug >= Params::DEBUG_VERBOSE,
		                  _params.input_dir,
		                  _params.max_realtime_valid_age,
		                  PMU_auto_register);
  }

  _initializeKeys();
  //  _decoder = new PirepDecoder();

  // initialize process registration
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);  
}

///////////////////////////////
// Destructor

PirepCsv2Spdb::~PirepCsv2Spdb()
{

  // unregister process

  PMU_auto_unregister();

  // Free contained objects

  if (_inputPath) {
    delete _inputPath;
  }


}

/////////////////////////////////////////////////////////////////////////
//  Initialize the various string/int pairs needed
void PirepCsv2Spdb::_initializeKeys()
{
  // fill out _turbulenceKeys
  //NEG | SMTH-LGT | LGT | LGT-MOD | MOD | MOD-SEV | SEV | SEV-EXTM | EXTM
  _turbulenceKeys.insert(pair<string,int>("EXTM",8));
  _turbulenceKeys.insert(pair<string,int>("SEV-EXTM",7));
  _turbulenceKeys.insert(pair<string,int>("SEV_EXTM",7));
  _turbulenceKeys.insert(pair<string,int>("SEV",6));
  _turbulenceKeys.insert(pair<string,int>("MOD-SEV",5));
  _turbulenceKeys.insert(pair<string,int>("MOD_SEV",5));
  _turbulenceKeys.insert(pair<string,int>("MOD",4));
  _turbulenceKeys.insert(pair<string,int>("LGT-MOD",3));
  _turbulenceKeys.insert(pair<string,int>("LGT_MOD",3));
  _turbulenceKeys.insert(pair<string,int>("LGT",2));
  _turbulenceKeys.insert(pair<string,int>("SMTH-LGT",1));
  _turbulenceKeys.insert(pair<string,int>("SMTH_LGT",1));
  _turbulenceKeys.insert(pair<string,int>("NEG",0));

  //CAT | CHOP | LLWS | MWAVE
  _turbulenceTypes.insert(pair<string,int>("CHOP",1));
  _turbulenceTypes.insert(pair<string,int>("CAT",2));
  _turbulenceTypes.insert(pair<string,int>("LLWS",3));
  _turbulenceTypes.insert(pair<string,int>("MWAVE",4));

 //NEG | NEGclr | TRC | TRC-LGT | LGT | LGT-MOD | MOD | MOD-SEV | HVY | SEV
  _icingKeys.insert(pair<string,int>("SEV",8));
  _icingKeys.insert(pair<string,int>("HVY",7));
  _icingKeys.insert(pair<string,int>("MOD-SEV",6));
  _icingKeys.insert(pair<string,int>("MOD_SEV",6));
  _icingKeys.insert(pair<string,int>("MOD",5));
  _icingKeys.insert(pair<string,int>("LGT-MOD",4));
  _icingKeys.insert(pair<string,int>("LGT_MOD",4));
  _icingKeys.insert(pair<string,int>("LGT",3));
  _icingKeys.insert(pair<string,int>("TRC-LGT",2));
  _icingKeys.insert(pair<string,int>("TRC_LGT",2));
  _icingKeys.insert(pair<string,int>("TRC",1));
  _icingKeys.insert(pair<string,int>("NEGclr",-1));
  _icingKeys.insert(pair<string,int>("NEG",-1));
 
  // SKC | CLEAR | CAVOC | FEW | SCT | BKN | OVC | OVX
  // Not used: VMC, VFR, IFR, IMC
  _skyKeys.insert(pair<string,int>("CLEAR",0));
  _skyKeys.insert(pair<string,int>("SKC",0));
  _skyKeys.insert(pair<string,int>("CAVOC",0));
  _skyKeys.insert(pair<string,int>("FEW",1));
  _skyKeys.insert(pair<string,int>("SCT",2));
  _skyKeys.insert(pair<string,int>("BKN",3));
  _skyKeys.insert(pair<string,int>("OVC",4));
  _skyKeys.insert(pair<string,int>("OVX",5));

 // RIME | CLEAR | MIXED
  _icingTypes.insert(pair<string,int>("RIME",1));
  _icingTypes.insert(pair<string,int>("CLEAR",2));
  _icingTypes.insert(pair<string,int>("MIXED",3));

 //ISOL | OCNL | CONT
  _turbulenceFreq.insert(pair<string,int>("ISOL",1));
  _turbulenceFreq.insert(pair<string,int>("OCNL",2));
  _turbulenceFreq.insert(pair<string,int>("CONT",3));
}

/////////////////////////////////////////////////////////////////////////
// clear the indices into csv file
void PirepCsv2Spdb::_clearIndicies()
{
	_iTime = MISSING_INDEX;
	_iLat = MISSING_INDEX;
	_iLon = MISSING_INDEX;
	_iAlt = MISSING_INDEX;
	_iRaw = MISSING_INDEX;
	_iAircraft = MISSING_INDEX;
	_iType = MISSING_INDEX;
	_iIceType = MISSING_INDEX;
	_iIceInt = MISSING_INDEX;
	_iIceTop = MISSING_INDEX;
	_iIceBase = MISSING_INDEX;
	_iIceType2 = MISSING_INDEX;
	_iIceInt2 = MISSING_INDEX;
	_iIceTop2 = MISSING_INDEX;
	_iIceBase2 = MISSING_INDEX;
	_iTurbType = MISSING_INDEX;
	_iTurbInt = MISSING_INDEX;
	_iTurbFreq = MISSING_INDEX;
	_iTurbTop = MISSING_INDEX;
	_iTurbBase = MISSING_INDEX;
	_iTurbType2 = MISSING_INDEX;
	_iTurbInt2 = MISSING_INDEX;
	_iTurbFreq2 = MISSING_INDEX;
	_iTurbTop2 = MISSING_INDEX;
	_iTurbBase2 = MISSING_INDEX;
	_iSkyCover = MISSING_INDEX;
	_iCloudTop = MISSING_INDEX;
	_iCloudBase = MISSING_INDEX;
	_iSkyCover2 = MISSING_INDEX;
	_iCloudTop2 = MISSING_INDEX;
	_iCloudBase2 = MISSING_INDEX;
	_iTemp = MISSING_INDEX;
	_iWindDir = MISSING_INDEX;
	_iWindSpeed = MISSING_INDEX;
	_iVisibility = MISSING_INDEX;
}

/////////////////////////////////////////////////////////////////////////
// parse the header line of the csv file to find indices
bool PirepCsv2Spdb::_parseHeader(string line)
{
	_clearIndicies();
  istringstream ss(line);

  //putting all the tokens in the vector
  vector<string> arrayTokens;//(begin, end); 

  while(ss) {
    string s;
    if(!getline(ss, s, ',')) break;
    if(s[0]==' ') {
	    s = s.substr(1);
    }
    arrayTokens.push_back(s);
  }

  for(unsigned int i=0; i < arrayTokens.size(); i++)
	{
		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.time_n)) {
			_iTime = i;
    }
		else if(!strcmp(_params.field_names.time_n,"")) {
			_iTime = _params.field_names.time_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.lat_n)) {
			_iLat = i;
    }
		else if(!strcmp(_params.field_names.lat_n,"")) {
			_iLat = _params.field_names.lat_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.lon_n))	{
			_iLon = i;
    }
		else if(!strcmp(_params.field_names.lon_n,"")) {
			_iLon = _params.field_names.lon_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.altitude_n)) {
			_iAlt = i;
    }
		else if(!strcmp(_params.field_names.altitude_n,"")) {
			_iAlt = _params.field_names.altitude_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.raw_n)) {
			_iRaw = i;
    }
		else if(!strcmp(_params.field_names.raw_n,"")) {
			_iRaw = _params.field_names.raw_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.aircraft_n))	{
			_iAircraft = i;
    }
		else if(!strcmp(_params.field_names.aircraft_n,"")) {
			_iAircraft = _params.field_names.aircraft_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.type_n))	{
			_iType = i;
    }
		else if(!strcmp(_params.field_names.type_n,"")) {
			_iType = _params.field_names.type_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.ice_type_n))	{
			_iIceType = i;
    }
		else if(!strcmp(_params.field_names.ice_type_n,"")) {
			_iIceType = _params.field_names.ice_type_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.ice_int_n))	{
			_iIceInt = i;
    }
		else if(!strcmp(_params.field_names.ice_int_n,"")) {
			_iIceInt = _params.field_names.ice_int_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.ice_top_n))	{
			_iIceTop = i;
    }
		else if(!strcmp(_params.field_names.ice_top_n,"")) {
			_iIceTop = _params.field_names.ice_top_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.ice_base_n))	{
			_iIceBase = i;
    }
		else if(!strcmp(_params.field_names.ice_base_n,"")) {
			_iIceBase = _params.field_names.ice_base_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.ice_type2_n))	{
			_iIceType2 = i;
    }
		else if(!strcmp(_params.field_names.ice_type2_n,"")) {
			_iIceType2 = _params.field_names.ice_type2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.ice_int2_n))	{
			_iIceInt2 = i;
    }
		else if(!strcmp(_params.field_names.ice_int2_n,"")) {
			_iIceInt2 = _params.field_names.ice_int2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.ice_top2_n))	{
			_iIceTop2 = i;
    }
		else if(!strcmp(_params.field_names.ice_top2_n,"")) {
			_iIceTop2 = _params.field_names.ice_top2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.ice_base2_n))	{
			_iIceBase2 = i;
    }
		else if(!strcmp(_params.field_names.ice_base2_n,"")) {
			_iIceBase2 = _params.field_names.ice_base2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.turb_type_n))	{
			_iTurbType = i;
    }
		else if(!strcmp(_params.field_names.turb_type_n,"")) {
			_iTurbType = _params.field_names.turb_type_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.turb_int_n))	{
			_iTurbInt = i;
    }
		else if(!strcmp(_params.field_names.turb_int_n,"")) {
			_iTurbInt = _params.field_names.turb_int_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.turb_freq_n))	{
			_iTurbFreq = i;
    }
		else if(!strcmp(_params.field_names.turb_freq_n,"")) {
			_iTurbFreq = _params.field_names.turb_freq_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.turb_top_n))	{
			_iTurbTop = i;
    }
		else if(!strcmp(_params.field_names.turb_top_n,"")) {
			_iTurbTop = _params.field_names.turb_top_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.turb_base_n))	{
			_iTurbBase = i;
    }
		else if(!strcmp(_params.field_names.turb_base_n,"")) {
			_iTurbBase = _params.field_names.turb_base_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.turb_type2_n))	{
			_iTurbType2 = i;
    }
		else if(!strcmp(_params.field_names.turb_type2_n,"")) {
			_iTurbType2 = _params.field_names.turb_type2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.turb_int2_n))	{
			_iTurbInt2 = i;
    }
		else if(!strcmp(_params.field_names.turb_int2_n,"")) {
			_iTurbInt2 = _params.field_names.turb_int2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.turb_freq2_n))	{
			_iTurbFreq2 = i;
    }
		else if(!strcmp(_params.field_names.turb_freq2_n,"")) {
			_iTurbFreq2 = _params.field_names.turb_freq2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.turb_top2_n))	{
			_iTurbTop2 = i;
    }
		else if(!strcmp(_params.field_names.turb_top2_n,"")) {
			_iTurbTop2 = _params.field_names.turb_top2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.turb_base2_n))	{
			_iTurbBase2 = i;
    }
		else if(!strcmp(_params.field_names.turb_base2_n,"")) {
			_iTurbBase2 = _params.field_names.turb_base2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.sky_cover_n))	{
			_iSkyCover = i;
    }
		else if(!strcmp(_params.field_names.sky_cover_n,"")) {
			_iSkyCover = _params.field_names.sky_cover_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_top_n))	{
			_iCloudTop = i;
    }
		else if(!strcmp(_params.field_names.cloud_top_n,"")) {
			_iCloudTop = _params.field_names.cloud_top_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_base_n))	{
			_iCloudBase = i;
    }
		else if(!strcmp(_params.field_names.cloud_base_n,"")) {
			_iCloudBase = _params.field_names.cloud_base_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.sky_cover2_n))	{
			_iSkyCover2 = i;
    }
		else if(!strcmp(_params.field_names.sky_cover2_n,"")) {
			_iSkyCover2 = _params.field_names.sky_cover2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_top2_n))	{
			_iCloudTop2 = i;
    }
		else if(!strcmp(_params.field_names.cloud_top2_n,"")) {
			_iCloudTop2 = _params.field_names.cloud_top2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_base2_n))	{
			_iCloudBase2 = i;
    }
		else if(!strcmp(_params.field_names.cloud_base2_n,"")) {
			_iCloudBase2 = _params.field_names.cloud_base2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.temp_n))	{
			_iTemp = i;
    }
		else if(!strcmp(_params.field_names.temp_n,"")) {
			_iTemp = _params.field_names.temp_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.wind_dir_n))	{
			_iWindDir = i;
    }
		else if(!strcmp(_params.field_names.wind_dir_n,"")) {
			_iWindDir = _params.field_names.wind_dir_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.wind_speed_n))	{
			_iWindSpeed = i;
    }
		else if(!strcmp(_params.field_names.wind_speed_n,"")) {
			_iWindSpeed = _params.field_names.wind_speed_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.visibility_n))	{
			_iVisibility = i;
    }
		else if(!strcmp(_params.field_names.visibility_n,"")) {
			_iVisibility = _params.field_names.visibility_i;
		}
  }

  if(_iTime == MISSING_INDEX)
	{
		cerr << "Required field 'time' not found in header" << endl;
		return false;
  }

  if(_iLat == MISSING_INDEX)
	{
		cerr << "Required field 'latitude' not found in header" << endl;
		return false;
  }

  if(_iLon == MISSING_INDEX)
	{
		cerr << "Required field 'longitude' not found in header" << endl;
		return false;
  }

  if(_iAlt == MISSING_INDEX)
	{
		cerr << "Required field 'altitude' not found in header" << endl;
		return false;
  }

  return true;
}



/////////////////////////////////////
// run()
int PirepCsv2Spdb::run()

{
  int iret = 0;

  char *input_filename;
  while( _useStdin || (input_filename = _inputPath->next()) != NULL)
  {
    // create SPDB object
    DsSpdb spdbXml;
    DsSpdb spdbRaw;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
	    spdbXml.setDebug();
	    spdbRaw.setDebug();
    }
    if (_params.output_compression == Params::COMPRESSION_GZIP) {
	    spdbXml.setChunkCompressOnPut(Spdb::COMPRESSION_GZIP);
	    spdbRaw.setChunkCompressOnPut(Spdb::COMPRESSION_GZIP);
    } 
    else if (_params.output_compression == Params::COMPRESSION_BZIP2) {
	    spdbXml.setChunkCompressOnPut(Spdb::COMPRESSION_BZIP2);
	    spdbRaw.setChunkCompressOnPut(Spdb::COMPRESSION_BZIP2);
    }


    spdbXml.setAppName(_progName);
    spdbRaw.setAppName(_progName);
    //    cerr << " File is: " << input_filename << endl;


    // Process the input file
    // Register with the process mapper
    string procmap_status;
    if( _useStdin )
	  {
      procmap_status = "Input from <STDIN>";
      if (_params.debug) {
        cerr << "New data from STDIN" << endl;
      }
    }
    else
	  {
      Path path(input_filename);
      procmap_status = "File <" + path.getFile() + ">";
      if (_params.debug) {
        cerr << "New data in file: " << input_filename << endl;
      }
	  }
    PMU_auto_register(procmap_status.c_str());
      


    // 
    // open the file into a fstream
    // use get_line and END_DELIMTER to delimit pireps
    // copy lines to a vector of strings; result is a array of reports
    // use isalpha and toupper to convert all characters to upper case
    // use a filter to convert all '/015' to whitespace
    //
    ifstream in_file;
    if( _useStdin ) {
      in_file.open("/dev/stdin");
    } else {
      in_file.open(input_filename);
	  }
    if(!in_file) {
      cerr << "Unable to open file: " 
	   << input_filename << "; continuing ..." << endl;
      continue;
    }

    vector<Pirep> pireps;
    string line;
    getline(in_file, line); // get header
    if(!_parseHeader(line))
	  {
		  cerr << "ERROR: _parseHeader failed" << endl;
		  if(_useStdin) break;
		  continue;
    }

    while(getline(in_file, line)) {
      if(line.size() == 0){ 
	      continue;
      }
      istringstream ss(line);

      //putting all the tokens in the vector
      vector<string> arrayTokens;//(begin, end); 

      while(ss) {
        string s;
        if(!getline(ss, s, ',')) break;
        arrayTokens.push_back(s);
      }

      Pirep p;

        if(!fillPirepObject(arrayTokens, p)) {
          if(strlen(_params.decoded_spdb_url) != 0) {
            addXmlSpdb(&spdbXml, &p, _params.expire_secs);
    	    }
          if(strlen(_params.ascii_spdb_url) != 0) {
            addRawSpdb(&spdbRaw, &p, _params.expire_secs);
	        }
	      }
        else {
	        cerr << "WARNING: Decoder failed on line: " << line  << endl;
        }
    }  

    // Write
    if(strlen(_params.decoded_spdb_url) != 0) {
      _writeSpdb(spdbXml, _params.decoded_spdb_url);
    }

    if(strlen(_params.ascii_spdb_url) != 0) {
      _writeSpdb(spdbRaw, _params.ascii_spdb_url);
    }

    if( _useStdin ) {
	    break;
    }
  } // while((input_filename = _inputPath->next() ...
    
  return iret;

}

/////////////////////////////////////////////////////////////////////////
// adds xml pirep to spdb
void PirepCsv2Spdb::addXmlSpdb(DsSpdb* spdb, Pirep* p, int& expire_secs)
{
  time_t obsTime = p->getObsTime();
  const time_t expireTime = obsTime + expire_secs;

  string xml = p->getXml(); 
  
  spdb->addPutChunk(0,
         	    obsTime,
		    expireTime,
         	    xml.size()+1,
		    xml.c_str());  
}


/////////////////////////////////////////////////////////////////////////
// Method Name:	PirepCsv2Spdb::addRawSpdb
void PirepCsv2Spdb::addRawSpdb(DsSpdb* spdb, Pirep* p, const time_t& expire_secs)
{
  time_t obsTime = p->getObsTime();
  const time_t expireTime = obsTime + expire_secs;

  string raw = p->getRaw();  

  spdb->addPutChunk(0,
		    obsTime,
		    expireTime,
                    raw.size()+1,
                    raw.c_str());
}


////////////////////////////////
// write ASCII airep to database
int PirepCsv2Spdb::_writeSpdb(DsSpdb &spdb, string url)
{
  
  if (spdb.nPutChunks() > 0) {
    if (_params.debug) {
      cerr << "Putting ascii pireps to URL: " << url << endl;
    }
    spdb.setPutMode(Spdb::putModeAddUnique);
    if (spdb.put(url,
		 SPDB_XML_ID, SPDB_XML_LABEL)) {
      cerr << "ERROR - PirepCsv2Spdb::_writeSpdb" << endl;
      cerr << spdb.getErrStr() << endl;
      return -1;
    }
  }
  
  return 0;

}

/////////////////////////////////////////////////////////////////////////
// addAssumptions
void PirepCsv2Spdb::addAssumptions(Pirep& out)
{
  Pirep::sky_cond_t sky1 = out.getSkyCondition1();
  Pirep::sky_cond_t sky2 = out.getSkyCondition2();
  Pirep::ice_obs_t ice2 = out.getIceObs2();



  
  if(ice2.intensity == Pirep::FILL_II) {
    if(sky1.coverage == Pirep::CLEAR_SKY) {
      ice2.intensity = Pirep::NONE_II;
      ice2.base = sky1.base;
      ice2.top = sky1.top;
      out.setIceObs2(ice2);
    }
    else if(sky2.coverage == Pirep::CLEAR_SKY) {
      ice2.intensity = Pirep::NONE_II;
      ice2.base = sky2.base;
      ice2.top = sky2.top;
      out.setIceObs2(ice2);
    }
  }
}

/////////////////////////////////////////////////////////////////////////
// fillPirepObject
int PirepCsv2Spdb::fillPirepObject(const vector<string>& in, Pirep& out)
{

  float as = _params.altitude_conversions.scale;
  float ab = _params.altitude_conversions.bias;

  float ts = _params.temperature_conversion.scale;
  float tb = _params.temperature_conversion.bias;

  float ws = _params.wind_speed_conversion.scale;
  float wb = _params.wind_speed_conversion.bias;

  float vs = _params.visibility_conversion.scale;
  float vb = _params.visibility_conversion.bias;

  int maxI = in.size();

  // check if item exists before entering it
  int index = _iTime;
  if(index < 0 || index >= maxI || in[index].size() == 0)
  {
    return -1;
  }

  time_t obsT = convertStringToTimeT(in[index]);
  if(obsT != 0) {
    out.setObsTime(obsT);
    cout << "Setting obs time to " << obsT << endl;
  }
  else {
    return -1;
  }

  index = _iLat;
  if(index < 0  || index >= maxI || in[index].size() == 0)
  {
    return -1;
  }
  out.setLatitude(atof(in[index].c_str()));

  index = _iLon;
  if(index < 0  || index >= maxI || in[index].size() == 0)
  {
    return -1;
  }
  out.setLongitude(atof(in[index].c_str()));


  index = _iAlt;
  if(index < 0 || index >= maxI  || in[index].size() == 0)
  {
    return -1;
  }
  out.setAltitude(atoi(in[index].c_str())*as+ab);

  // is valid if contains above fields
  out.setType(Pirep::VALID_PIREP);

  if(_iRaw >= 0 && _iRaw < maxI) {
    out.setRaw(trimWhitespace(in[_iRaw]));
  }

  if(_iAircraft >= 0 && _iAircraft < maxI) {
    out.setAircraftId(in[_iAircraft]);
  }

  if(_iType >= 0 && _iType < maxI) {
    out.setReportType(in[_iType]);
  }

  // get icing obs
  index = _iIceInt;
  if(index >= 0 && index < maxI && in[index].size() != 0)
  {
    Pirep::ice_obs_t ice;
    ice.type = Pirep::FILL_IT;
    ice.intensity = Pirep::FILL_II;
    ice.top = Pirep::MISSING_VALUE;
    ice.base = Pirep::MISSING_VALUE;
    out.setType(Pirep::ICING_PIREP);
   
    map<string,int>::const_iterator it;
    if((it = _icingKeys.find(in[index])) != _icingKeys.end())
    {
      ice.intensity = (*it).second;
    }
    
    index = _iIceType;
    if(index >= 0 && in[index].size() != 0)
    {
      if((it = _icingTypes.find(in[index])) != _icingTypes.end())
      {
        ice.type = (*it).second;
      }
    }

    index = _iIceTop;
    if(index >= 0 && in[index].size() != 0)
    {
      ice.top = atoi(in[index].c_str())*as+ab;
    }

    index = _iIceBase;
    if(index >= 0 && in[index].size() != 0)
    {
      ice.base = atoi(in[index].c_str())*as+ab;
    }
    
    out.setIceObs1(ice);
  }

  index = _iIceInt2;
  if(index >= 0 && index < maxI && in[index].size() != 0)
  {
    Pirep::ice_obs_t ice;
    ice.type = Pirep::FILL_IT;
    ice.intensity = Pirep::FILL_II;
    ice.top = Pirep::MISSING_VALUE;
    ice.base = Pirep::MISSING_VALUE;
    out.setType(Pirep::ICING_PIREP);
   
    map<string,int>::const_iterator it;
    if((it = _icingKeys.find(in[index])) != _icingKeys.end())
    {
      ice.intensity = (*it).second;
    }
    
    index = _iIceType2;
    if(index >= 0 && index < maxI  && in[index].size() != 0)
    {
      if((it = _icingTypes.find(in[index])) != _icingTypes.end())
      {
        ice.type = (*it).second;
      }
    }

    index = _iIceTop2;
    if(index >= 0 && index < maxI  && in[index].size() != 0)
    {
      ice.top = atoi(in[index].c_str())*as+ab;
    }

    index = _iIceBase2;
    if(index >= 0 && index < maxI  && in[index].size() != 0)
    {
      ice.base = atoi(in[index].c_str())*as+ab;
    }
    
    out.setIceObs2(ice);
  }

  // get turb obs
  index = _iTurbInt;
  if(index >= 0 && index < maxI  && in[index].size() != 0)
  {
    Pirep::turb_obs_t turb;
    turb.type = Pirep::FILL_TT;
    turb.intensity = Pirep::FILL_TI;
    turb.top = Pirep::MISSING_VALUE;
    turb.base = Pirep::MISSING_VALUE;

    if(out.getType() == Pirep::ICING_PIREP)
    {
      out.setType(Pirep::BOTH_PIREP);
    }
    else if(out.getType() == Pirep::VALID_PIREP)
    {
      out.setType(Pirep::TURB_PIREP);
    }

    map<string,int>::const_iterator it;
    if((it = _turbulenceKeys.find(in[index])) != _turbulenceKeys.end()) {
      turb.intensity = (*it).second;
    }

    index = _iTurbType;
    if(index >= 0 && index < maxI ) {
      if((it = _turbulenceTypes.find(in[index])) != _turbulenceTypes.end()) {
	  turb.type = (*it).second;
	}
    }

    index = _iTurbTop;
    if(index >= 0 && index < maxI ) {
      turb.top = atoi(in[index].c_str())*as+ab;
    }

    index = _iTurbBase;
    if(index >= 0 && index < maxI ) {
      turb.base = atoi(in[index].c_str())*as+ab;
    }
      out.setTurbObs1(turb);
  }

  index = _iTurbInt2;
  if(index >= 0 && index < maxI  && in[index].size() != 0)
    {
      Pirep::turb_obs_t turb;
      turb.type = Pirep::FILL_TT;
      turb.intensity = Pirep::FILL_TI;
      turb.top = Pirep::MISSING_VALUE;
      turb.base = Pirep::MISSING_VALUE;

      if(out.getType() == Pirep::ICING_PIREP)
	{
	  out.setType(Pirep::BOTH_PIREP);
	}
      else if(out.getType() == Pirep::VALID_PIREP)
	{
	  out.setType(Pirep::TURB_PIREP);
	}

      map<string,int>::const_iterator it;
      if((it = _turbulenceKeys.find(in[index])) != _turbulenceKeys.end())
	{
	  turb.intensity = (*it).second;
	}

      index = _iTurbType2;
      if(index >= 0 && index < maxI) {
	if((it = _turbulenceTypes.find(in[index])) != _turbulenceTypes.end())
	  {
	    turb.type = (*it).second;
	  }
      }
      index = _iTurbTop2;
      if(index >= 0 && index < maxI) {
	turb.top = atoi(in[index].c_str())*as+ab;
      }

      index = _iTurbBase2;
      if(index >= 0 && index < maxI) {
	turb.base = atoi(in[index].c_str())*as+ab;
      }

      out.setTurbObs2(turb);
    }


  // get sky obs
  index = _iSkyCover;
  if(index >= 0 && index < maxI  && in[index].size() != 0)
  {
    Pirep::sky_cond_t sky;
    sky.top = Pirep::MISSING_VALUE;
    sky.base = Pirep::MISSING_VALUE;
    sky.coverage = Pirep::NO_REPORT_SKY;

    map<string,int>::const_iterator it;
    if((it = _skyKeys.find(in[index])) != _skyKeys.end())
    {
      sky.coverage = (*it).second;
    }

    if(sky.coverage == Pirep::CLEAR_SKY && out.getType() == Pirep::VALID_PIREP)
    {
      out.setType(Pirep::CLEAR_PIREP);
    }

    index = _iCloudTop;
    if(index >= 0 && index < maxI  && in[index].size() != 0)
    {
      sky.top = atoi(in[index].c_str())*as+ab;
    }

    index = _iCloudBase;
    if(index >= 0 && index < maxI  && in[index].size() != 0)
    {
      sky.base = atoi(in[index].c_str())*as+ab;
    }

    out.setSkyCondition1(sky);    
  }

  index = _iSkyCover2;
  if(index >= 0 && index < maxI  && in[index].size() != 0)
  {
    Pirep::sky_cond_t sky;
    sky.top = Pirep::MISSING_VALUE;
    sky.base = Pirep::MISSING_VALUE;
    sky.coverage = Pirep::NO_REPORT_SKY;

    map<string,int>::const_iterator it;
    if((it = _skyKeys.find(in[index])) != _skyKeys.end())
    {
      sky.coverage = (*it).second;
    }

    if(sky.coverage == Pirep::CLEAR_SKY && out.getType() == Pirep::VALID_PIREP)
    {
      out.setType(Pirep::CLEAR_PIREP);
    }

    index = _iCloudTop2;
    if(index >= 0 && index < maxI  && in[index].size() != 0)
    {
      sky.top = atoi(in[index].c_str())*as+ab;
    }

    index = _iCloudBase2;
    if(index >= 0 && index < maxI  && in[index].size() != 0)
    {
      sky.base = atoi(in[index].c_str())*as+ab;
    }

    out.setSkyCondition2(sky);    
  }


  // get weather obs
  Pirep::wx_obs_t weather;
  weather.visibility = Pirep::MISSING_VALUE;
  weather.temperature = Pirep::MISSING_VALUE;
  weather.wind_dir = Pirep::MISSING_VALUE;
  weather.wind_speed = Pirep::MISSING_VALUE;
  weather.clear_above = Pirep::MISSING_VALUE;
  weather.observation = Pirep::MISSING_VALUE;
  
  bool hasWeather = false;

  index = _iVisibility;
  if(index >= 0 && index < maxI  && in[index].size() != 0)
  {
    weather.visibility = atoi(in[index].c_str())*vs+vb;
    hasWeather = true;
  }

  index = _iTemp;
  if(index >= 0 && index < maxI  && in[index].size() != 0)
  {
    weather.temperature = atoi(in[index].c_str())*ts+tb;
    hasWeather = true;
  }

  index = _iWindDir;
  if(index >= 0 && index < maxI  && in[index].size() != 0)
  {
    weather.wind_dir = atoi(in[index].c_str());
    hasWeather = true;
  }

  index = _iWindSpeed;
  if(index >= 0 && index < maxI  && in[index].size() != 0)
  {
    weather.wind_speed = atoi(in[index].c_str())*ws+wb;
    hasWeather = true;
  }
  // no clear above field in csv files?

  if(hasWeather)
  {
    out.setWeatherObs(weather);
  }

  if(_params.assumptions) {
    addAssumptions(out);
  }

  out.toXml();

  return 0;
}
/////////////////////////////////////////////////////////////////////////
// convertStringToTimeT
time_t PirepCsv2Spdb::convertStringToTimeT(string in)
{
  int year, month, day, hour, min, sec;
  if (sscanf(in.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d",
         &year, &month, &day, &hour, &min, &sec) == 6) {
    DateTime dt(year, month, day, hour, min, sec);
    return dt.utime();
  }
  time_t tval;
  if (sscanf(in.c_str(), "%ld", &tval) == 1) {
    return tval;
  }

  return 0;
}

string PirepCsv2Spdb::trimWhitespace(string s)
{
   string temp = s;
   size_t p = temp.find_first_not_of(" \t");
   temp.erase(0, p);

   p = temp.find_last_not_of(" \t");
   if (string::npos != p)
      temp.erase(p+1);
   return temp;
}
