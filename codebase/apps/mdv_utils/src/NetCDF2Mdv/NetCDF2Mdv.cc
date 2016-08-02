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
/////////////////////////////////////////////////////////////
// NetCDF2Mdv.cc
//
// NetCDF2Mdv object
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2007
//
///////////////////////////////////////////////////////////////
//
// NetCDF2Mdv reads netCDF data and writes Mdv files
//
///////////////////////////////////////////////////////////////////////

#include "NetCDF2Mdv.hh"

#include <Mdv/DsMdvx.hh>
#include <Mdv/Ncf2MdvTrans.hh>
#include <toolsa/pmu.h>

using namespace std;

//
// Constructor
//
NetCDF2Mdv::NetCDF2Mdv(int argc, char **argv):
_progName("NetCDF2Mdv"),
_args(_progName)

{
  isOK = true;

  //
  // set programe name
  //
  ucopyright((char *) _progName.c_str());

  //
  // get command line args
  //
  if (_args.parse(argc, argv)) 
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with command line args" << endl;
      isOK = FALSE;
      return;
    }

  //
  // get TDRP params
  //
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) 
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters" << endl;
      isOK = FALSE;
      return;
    }

  //
  // init process mapper registration
  //
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		300);

  return;

}

//////////////////////////////////////////////////////
//
// destructor
//
NetCDF2Mdv::~NetCDF2Mdv()

{
  //
  // unregister process
  //
  PMU_auto_unregister();
  
}



//////////////////////////////////////////////////////
//
// destructor
//
void NetCDF2Mdv::_clear()

{

}



//////////////////////////////////////////////////
// 
// Run
//
int NetCDF2Mdv::Run ()
{

  //
  // register with procmap
  //
  PMU_auto_register("Run");

  //
  // Initialize file trigger
  //
  if ( _params.mode == Params::REALTIME ) 
    {
      if (_params.debug)
	cerr << "FileInput::init: Initializing realtime input from " 
	     << _params.input_dir << endl;
      
      fileTrigger = new DsInputPath( _progName, _params.debug,
				     _params.input_dir,
				     _params.max_valid_realtime_age_min*60,
				     PMU_auto_register,
				     _params.ldata_info_avail,
				     false );
      //
      // Set wait for file to be written to disk before being served out.
      //
      fileTrigger->setFileQuiescence( _params.file_quiescence_sec );
      
      //
      // Set time to wait before looking for new files.
      // 
      fileTrigger->setDirScanSleep( _params.check_input_sec );
      
    }

   if ( _params.mode == Params::FILELIST ) 
    {    
      //
      // Archive mode.
      //
      const vector<string> &fileList =  _args.getInputFileList();
      if ( fileList.size() ) 
	{
	  if (_params.debug)
	    cerr << "FileInput::init: Initializing archive FILELIST mode." << endl;
	  
	  fileTrigger = new DsInputPath( _progName, _params.debug , fileList );
	}
    }
    
   if ( _params.mode == Params::TIME_INTERVAL ) 
     { 
       //
       // Set up the time interval
       //
       DateTime start( _params.start_time);
      
       DateTime end( _params.end_time);
       
       time_t time_begin = start.utime();
       
       time_t time_end = end.utime();
       
       if (_params.debug)
	 {
	   cerr << "FileInput::init: Initializing file input for time interval [" 
		<< time_begin << " , " << time_end << "]." << endl;
	   
	 }
       
       fileTrigger = new DsInputPath( _progName, _params.debug,
				      _params.input_dir,
				      time_begin,
				      time_end);
     }


  //
  //
  // process data
  //
  char *inputPath;

  while ( (inputPath = fileTrigger->next()) != NULL)
    {
     
      if (_processData(inputPath))
	{
          cerr << "Error - NetCDF2Mdv::Run" << endl;
          cerr << "  Errors in processing time: " <<  inputPath << endl;
	  return 1;
        }
    } // while
  
  delete fileTrigger;

  return 0;
  

}


///////////////////////////////////
//
//  process data at trigger time
//
int NetCDF2Mdv::_processData(char *inputPath)

{
  if (_params.debug)
  {
      cerr << "NetCDF2Mdv::_processData: Processing file : " << inputPath << endl;
  }
  
  // registration with procmap

  PMU_force_register("Processing data");

  int ntimes = _determineTimes(inputPath);
  if (ntimes > 0)
  {
    // do each time individually, write out as forecasts
    for (int itime = 0; itime < ntimes; ++itime)
    {
      Ncf2MdvTrans trans;
      trans.setDebug(_params.debug >= Params::DEBUG_VERBOSE);
      trans.setExpectedNumTimes(ntimes);
      trans.setTimeIndex(itime);

      DsMdvx mdv;

      for (int i = 0; i < _params.field_names_n; i++)
	mdv.addReadField(_params._field_names[i]);

      // translate into MDV
      if (trans.translate(inputPath, mdv)) {
	cerr << "ERROR - NetCDF2Mdv::_processData()" << endl;
	cerr << "  Cannot translate file: " <<  inputPath << endl; 
	cerr << trans.getErrStr() << endl;
	return -1;
      }
      // write to file
      if (_writeData( mdv, ntimes > 1) ) {
	cerr << "ERROR - NetCDF2Mdv::_writeData()" << endl;
	cerr << "  Cannot translate file: " <<  inputPath << endl;
	return -1;
      }
    }
  }
  else
  {
    // no loop, just do it all
    DsMdvx mdv;

    for (int i = 0; i < _params.field_names_n; i++)
      mdv.addReadField(_params._field_names[i]);

    // create translator
    Ncf2MdvTrans trans;
    trans.setDebug(_params.debug >= Params::DEBUG_VERBOSE);

    if (trans.translate(inputPath, mdv)) {
      cerr << "ERROR - NetCDF2Mdv::_processData()" << endl;
      cerr << "  Cannot translate file: " <<  inputPath << endl;
      cerr << trans.getErrStr() << endl;
      return -1;
    }

    // write to file
    if (_writeData( mdv, false) ) {
      cerr << "ERROR - NetCDF2Mdv::_writeData()" << endl;
      cerr << "  Cannot translate file: " <<  inputPath << endl;
      return -1;
    }
  }
  return 0;
}

int NetCDF2Mdv::_writeData( DsMdvx &mdv, bool mustBeForecast)
{
  
  const Mdvx::master_header_t &mhdr = mdv.getMasterHeader();

  if (mhdr.num_data_times <= 1){
    if (mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED ||
	mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
	mhdr.forecast_time != 0) {
      mdv.setWriteAsForecast();
    }
    else {
      if (mustBeForecast) {
	cerr << "ERROR - NetCDF2Mdv::_writeData()" << endl;
	cerr << "expect forecast but collection type " <<
	  mhdr.data_collection_type << " is wrong , forecast time = " <<
	  mhdr.forecast_time << endl;
	return -1;
      }
    }
    
    if (mdv.writeToDir(_params.output_url)) {
      cerr << "ERROR - NetCDF2Mdv::_writeData()" << endl;
      cerr << "  Cannot write mdv file" << endl;
      cerr << mdv.getErrStr() << endl;
      return -1;
    }
    
    if (_params.debug) {
      cerr << "Wrote file: " << mdv.getPathInUse() << endl;
    }

  } else {

    if (_params.debug) {
      mdv.setDebug(true);
    }
    
    if (mdv.writeMultForecasts(_params.output_url)) {
      cerr << "ERROR - NetCDF2Mdv::_writeData()" << endl;
      cerr << "  Cannot write mult forecasts to url: "
           << _params.output_url << endl;
      cerr << mdv.getErrStr() << endl;
      return -1;
    }
    
  }

  return 0;

}

int NetCDF2Mdv::_determineTimes(char *inputPath)
{
  DsMdvx mdv;

  for (int i = 0; i < _params.field_names_n; i++)
    mdv.addReadField(_params._field_names[i]);

  Ncf2MdvTrans trans;
  trans.setDebug(_params.debug >= Params::DEBUG_VERBOSE);

  // check time dimension in netCDF data.
  return trans.inspectTimes(inputPath, mdv);
}

