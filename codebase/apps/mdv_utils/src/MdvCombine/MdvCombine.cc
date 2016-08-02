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
/*
 *  $Id: MdvCombine.cc,v 1.16 2016/03/04 02:22:10 dixon Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	MdvCombine
//
// Author:	G M Cunning
//
// Date:	Tue Jan 16 11:42:17 2001
//
// Description: This program collects MDV data from several input 
//		directories and copies data to a single output directory
//
//
//
//
//
//


// C++ include files
#include <cassert>

// System/RAP include files
#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsMultipleTrigger.hh>
#include <dsdata/DsSpecificFcstLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/pmu.h>

// Local include files
#include "MdvCombine.hh"
#include "Args.hh"
#include "Params.hh"
#include "InputUrl.hh"
#include "OutputUrl.hh"
using namespace std;


// the singleton itself
MdvCombine *MdvCombine::_instance = 0;

// define any constants
const string MdvCombine::_className    = "MdvCombine";


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

MdvCombine::MdvCombine(int argc, char **argv) :
  _debug(false),
  _isOk(true),
  _progName(""),
  _errStr(""),
  _args(0),
  _params(0)
{
  const string methodName = _className + string("::Constructor");
  _errStr = string("ERROR: ") + methodName;


 // Make sure the singleton wasn't already created.
  assert(_instance == 0);

  // Set the singleton instance pointer
  _instance = this;

  // set programe name
  Path pathParts(argv[0]);
  _progName = pathParts.getBase();

  // display ucopyright message and RCSid
  ucopyright(const_cast<char*>(_progName.c_str()));

  // get command line args
  _args = new Args(argc, const_cast<char**>(argv), _progName);
  if (!_args->isOK) {
    _errStr += "\tProblem with command line arguments.\n";
    _isOk = false;
  }

  // get TDRP params
  _params = new Params();
  char *paramsPath = (char *)"unknown";
  if (_params->loadFromArgs(argc, argv, _args->override.list, 
			    &paramsPath)) {
    _errStr += "\tProblem with TDRP parameters.\n";
    _isOk = false;
    return;
  }


  if (_params->mode == Params::ARCHIVE &&
      (_args->getArchiveStartTime() == 0 ||
       _args->getArchiveEndTime() == 0))
  {
    _errStr += "\tError in command line\n";
    _errStr += 
      "\t-start and -end must be specified on the command line in ARCHIVE mode\n";
    _isOk = false;
    return;
  }
  
   // init process mapper registration
   PMU_auto_init((char *) _progName.c_str(),
		 _params->instance,
		 PROCMAP_REGISTER_INTERVAL);
   

  return;

}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
MdvCombine::~MdvCombine()
{
  // unregister process
  PMU_auto_unregister();

  // free contained objects
  delete _args;
  delete _params;


}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MdvCombine::instance
//
// Description:	Retrieves the singleton instance.
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

MdvCombine* 
MdvCombine::instance( int argc, char **argv )
{
  if ( _instance == 0 ) {
    _instance = new MdvCombine(argc, argv);
  }
  
  return(_instance);
}

MdvCombine* 
MdvCombine::instance()
{
  assert(_instance != 0 );
  return( _instance );
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MdvCombine::execute
//
// Description:	this method is responbile for doing the work.
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

bool 
MdvCombine::execute()
{
  static const string methodName = _className + string( "::execute" );

  PMU_auto_register((char*)methodName.c_str());
  
  // create the trigger
  
  DsTrigger *trigger = _createTrigger();
  
  // check that we have a good trigger
  if(!trigger) {
    return false;
  }

  // setup the input URLs to get data
  int nInput = _params->field_info_n;
  vector<InputUrl*> inputUrl;
  for(int i=0; i<_params->field_info_n; i++) {
    inputUrl.push_back(new InputUrl(_params->_field_info[i].url,
				    _params->_field_info[i].field_num,
				    _params->_field_info[i].field_name,
				    _params->time_trigger_interval,
				    _params->debug));
  }

  // create output file object

  OutputUrl *outputUrl = new OutputUrl(_params);

  // loop through times

  DateTime triggerTime;
  
  while (true) {
    
    if (trigger->endOfData())
      break;
    
    if (trigger->nextIssueTime(triggerTime) != 0)
    {
      cerr << "ERROR: " << methodName << endl;
      cerr << "Error getting next issue time from trigger" << endl;
      
      break;
    }
    
    if (_params->debug)
      cerr <<  "----> Trigger time: "<< triggerTime << endl;

    // Clean out the old output file

    outputUrl->clear();
    
    // read relevant input files
    
    outputUrl->addToInfo("Combined data set from the following files:\n");

    time_t startTime = -1;
    time_t endTime = -1;

    bool dataAvail = false;
    
    bool outInit = false;

    if( _params->sleepAfterTrigger > 0  && 
	( _params->mode == Params::REALTIME ||
	  _params->mode == Params::SPEC_FCAST_REALTIME ) )
    {
      if(_params->debug)
      {
	cerr << "Sleeping for " << _params->sleepAfterTrigger << " seconds before processing data.\n";
      }

      for (int iSleep=0; iSleep < _params->sleepAfterTrigger; iSleep++) 
      {
	PMU_auto_register("Sleeping before processing data");
	sleep(1);
      }      
    }
    
    int read_field = -1;
    for(int i=0; i<nInput; i++)
    {
      inputUrl[i]->getNext(triggerTime.utime());
      if (inputUrl[i]->readSuccess())
      {
	dataAvail = TRUE;
	read_field = i;
	// on first successful read, initialize the output file headers
	if (!outInit)
	{
	  Mdvx::master_header_t *in_mhdr = inputUrl[i]->getMasterHeader();
	  outputUrl->initMasterHeader(in_mhdr);
	  outInit = TRUE;
	}

	// set info string
	outputUrl->addToInfo(inputUrl[i]->path());
	outputUrl->addToInfo("\n");

	// set the times
	inputUrl[i]->updateTimes(&startTime, &endTime);

	// add the field
	outputUrl->addField(inputUrl[i]->getField());
      
      } // if (inputUrl[i]->readSuccess())
    } // i

    if(_params->write_blank_if_missing && read_field >= 0) 
    {
      for(int i=0; i<nInput; i++)
      {
	if(! inputUrl[i]->readSuccess())
        {

	  //	  inputUrl[read_field]->getNext(triggerTime.utime());
	  MdvxField *input_field = inputUrl[read_field]->getField();
	  Mdvx::field_header_t input_field_hdr = input_field->getFieldHeader();
	
	  Mdvx::field_header_t new_field_hdr;
	  
	  memset(&new_field_hdr, 0, sizeof(new_field_hdr));
  
	  new_field_hdr.field_code = 1;
	  new_field_hdr.forecast_delta = input_field_hdr.forecast_delta;
	  new_field_hdr.forecast_time = input_field_hdr.forecast_time;
	  new_field_hdr.nx = input_field_hdr.nx;
	  new_field_hdr.ny = input_field_hdr.ny;
	  new_field_hdr.nz = input_field_hdr.nz;
	  new_field_hdr.proj_type = input_field_hdr.proj_type;
	  new_field_hdr.encoding_type = Mdvx::ENCODING_INT8;
	  new_field_hdr.data_element_nbytes = 4;
	  new_field_hdr.volume_size =
	    new_field_hdr.nx * new_field_hdr.ny * new_field_hdr.nz *
	    new_field_hdr.data_element_nbytes;
	  new_field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
	  new_field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
	  new_field_hdr.scaling_type = Mdvx::SCALING_NONE;
	  //  new_field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_NEW;
	  //  new_field_hdr.vlevel_type = Mdvx::VERT_TYPE_NEW;
	  new_field_hdr.dz_constant = input_field_hdr.dz_constant;
	  new_field_hdr.proj_origin_lat = input_field_hdr.proj_origin_lat;
	  new_field_hdr.proj_origin_lon = input_field_hdr.proj_origin_lon;
	  for (int j = 0; j < MDV_MAX_PROJ_PARAMS; ++j)
	    new_field_hdr.proj_param[j] = input_field_hdr.proj_param[j];
	  new_field_hdr.vert_reference = input_field_hdr.vert_reference;
	  new_field_hdr.grid_dx = input_field_hdr.grid_dx;
	  new_field_hdr.grid_dy = input_field_hdr.grid_dy;
	  new_field_hdr.grid_dz = input_field_hdr.grid_dz;
	  new_field_hdr.grid_minx = input_field_hdr.grid_minx;
	  new_field_hdr.grid_miny = input_field_hdr.grid_miny;
	  new_field_hdr.grid_minz = input_field_hdr.grid_minz;
	  new_field_hdr.scale  = 1;
	  new_field_hdr.bias = 0;

	  new_field_hdr.proj_rotation = input_field_hdr.proj_rotation;

	  string output_field_name;
	  if(_params->_field_info[i].field_name != "")
          {
	    output_field_name = _params->_field_info[i].field_name;
	  }
	  else
          {
	    output_field_name = _params->_field_info[i].field_num;
	  }

	  STRcopy(new_field_hdr.field_name_long, output_field_name.c_str(), MDV_LONG_FIELD_LEN);
	  STRcopy(new_field_hdr.field_name, output_field_name.c_str(), MDV_SHORT_FIELD_LEN);
	  STRcopy(new_field_hdr.units, input_field_hdr.units, MDV_UNITS_LEN);

	  Mdvx::vlevel_header_t new_vlevel_hdr =
	    input_field->getVlevelHeader();
	  
	  MdvxField *output_field;

	  if ((output_field =
	       new MdvxField(new_field_hdr,new_vlevel_hdr,(void *)0,false,false)) == 0)
          {
	    cerr << "Error creating output field " << output_field_name << endl;
	    return -1;
	  }

	  if (_params->debug)
          {
	    cerr << "Missing field " << output_field_name
		 << " from path " << inputUrl[i]->path() << endl;
	    cerr << "Producing blank field with name " << output_field_name << endl;
	  }

	  output_field->convertType(Mdvx::ENCODING_ASIS,
				    Mdvx::COMPRESSION_BZIP,
				    Mdvx::SCALING_DYNAMIC);                                                                                             
	  outputUrl->addField(output_field);
	} // if(! inputUrl[i]->readSuccess())
      } // for(int i=0; i<nInput; i++)
    } // if(_params->write_blank_if_missing && read_field >= 0)
      

    if (!dataAvail)
      continue;

    
    // write out volume
    if(!outputUrl->writeVol(triggerTime.utime(), startTime, endTime))
      return false;

    if(_args->runOnce()) {
      break;
    }    

  } // while

  return true;

}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

DsTrigger *MdvCombine::_createTrigger()
{

  DsTrigger *trigger;

  // create the trigger

  switch (_params->mode)
  {
  case Params::REALTIME :
  {
    if (_params->debug)
      cerr << "---> Creating REALTIME trigger" << endl;
    
    // REALTIME mode
    DsLdataTrigger *ldata_trigger = new DsLdataTrigger();
    if (ldata_trigger->init(_params->trigger_url,
				-1, PMU_auto_register) != 0) 
    {
      cerr << ldata_trigger->getErrStr();
      trigger = 0;
    }
    else
    {
      trigger = ldata_trigger;
    }
    break;
  }
 
  case Params::MULTIPLE_URL :
  {
    if (_params->debug)
    {
      cerr << "Initializing MULTIPLE_URL trigger using urls: " << endl;
      for (int i = 0; i < _params->multiple_url_trigger_n; ++i)
	cerr << "    " << _params->_multiple_url_trigger[i] << endl;
    }
    
    DsMultipleTrigger *MURL_trigger = new DsMultipleTrigger();

    if (!MURL_trigger->initRealtime(-1,
				    PMU_auto_register))
    {
      cerr << "Error initializing MULTIPLE_URL trigger using urls: " << endl;
      for (int i = 0; i < _params->multiple_url_trigger_n; ++i)
	cerr << "    " << _params->_multiple_url_trigger[i] << endl;
      
      trigger = 0;
    }
    else
    {

      for (int i = 0; i < _params->multiple_url_trigger_n; ++i)
	MURL_trigger->add(_params->_multiple_url_trigger[i]);
      MURL_trigger->set_debug(_params->debug);

      trigger = MURL_trigger;
    }
    break;
  }

  case Params:: SPEC_FCAST_REALTIME :
  {
    if (_params->debug)
    {
	cerr << "---> Creating  SPEC_FCAST_REALTIME trigger" << endl;
    }
    DsSpecificFcstLdataTrigger *spec_trigger =
      new DsSpecificFcstLdataTrigger();
    vector< int > fcast_lead_times;
    fcast_lead_times.push_back(_params->fcast_lead_time.lead_time_secs);

    if (spec_trigger->init(_params->trigger_url,
                      fcast_lead_times,
                      _params->fcast_lead_time.use_gen_time,
                      7200, PMU_auto_register) != 0)
    {
      cerr << spec_trigger->getErrStr() << endl;
      trigger = 0;
    }
    else
    {
      trigger = spec_trigger;
    }
    break; 
  }

  case Params::ARCHIVE :
  {
    if (_params->debug)
    {
      cerr << "---> Creating ARCHIVE trigger" << endl;
      cerr << "     start time: "
	   << DateTime::str(_args->getArchiveStartTime()) << endl;
      cerr << "     end time: "
	   << DateTime::str(_args->getArchiveEndTime()) << endl;
    }
    
    DsTimeListTrigger *archive_trigger = new DsTimeListTrigger();

    if (archive_trigger->init(_params->trigger_url,
			  _args->getArchiveStartTime(),  
			      _args->getArchiveEndTime()) != 0) 
    {
      cerr << archive_trigger->getErrStr();
      trigger = 0;
    } 
    else 
    {
      trigger = archive_trigger;
    }
    break;
  }
  }
  
  return (trigger);

}
