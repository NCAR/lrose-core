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
///////////////////////////////////////////////////////////
// MdvFlagData.cc
//
// MdvFlagData object
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2007
//

#include "MdvFlagData.hh"
#include <toolsa/Path.hh>
using namespace std;

//
// Constructor
//
MdvFlagData::MdvFlagData(int argc, char **argv)
{

  // initialize

  isOK = true;

  _trigger = NULL;

  //
  // set programe name
  //
  _progName = "MdvFlagData";

  ucopyright((char *) _progName.c_str());

  //
  // get command line args
  //
  if (_args.parse(argc, argv, _progName))
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
  // If an additional TDRP file was specified, load that
  // over the existing params.
  //
  if (NULL != _args.additional_tdrp_file){

    if (_params.debug){
      cerr << "Attempting to load additional param file " << _args.additional_tdrp_file << endl;
    }

    if (_params.load(_args.additional_tdrp_file, NULL, TRUE, FALSE)){
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters in file: " << _args.additional_tdrp_file << endl;
      isOK = false;
      return;
    }
  }

  //
  // Initialize variables
  // 
  _fieldData  = NULL;

  _flagsFieldData = NULL;


  //
  // set up trigger
  //
  if (_setUpTrigger()) {
    isOK = FALSE;
    return;
  }

  //
  // Init process mapper registration
  //
  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);

}

//////////////////////////////////////////////////////
//
// destructor
//
MdvFlagData::~MdvFlagData()

{

  //
  // unregister process
  //
  PMU_auto_unregister();

  if( _trigger) {
    delete _trigger;
  }
}


//////////////////////////////////////////////////
//
// Run
//
int MdvFlagData::Run ()
{

  int iret = 0;

  //
  // register with procmap
  //
  PMU_auto_register("Run");
  
  //
  // process data: 
  //
  
  time_t inputTime;

  while (!_trigger->endOfData())
    {
      TriggerInfo triggerInfo;
      inputTime = _trigger->next(triggerInfo);
      if (_processData(triggerInfo.getIssueTime(), 
		       triggerInfo.getForecastTime() - triggerInfo.getIssueTime(),
		       triggerInfo.getFilePath()))
	{
	  cerr << "MdvFlagData::Run" <<"  Errors in processing time: "
	       <<  triggerInfo.getIssueTime()
	       << " input file: " << triggerInfo.getFilePath() << endl;
	  
	  iret = 1;
	}
    } // while
  
  return iret;
}


/////////////////////////////////////////
// Set up triggering mechanisme
//
// Returns 0 on success, 1 on failure

int MdvFlagData::_setUpTrigger()

{ 
 if ( _params.mode == Params::FILELIST )
  {
    //
    // Archive filelist mode.
    //
    if ( _args.inputFileList.size() )
    {
      if (_params.debug)
        cerr << "FileInput::init: Initializing archive FILELIST mode." << endl;

      DsFileListTrigger *file_trigger = new DsFileListTrigger();

      if  (file_trigger->init( _args.inputFileList ) )
      {
        cerr << file_trigger->getErrStr();
      
	_trigger = 0;
        
	return 1;
      }
      else
        _trigger = file_trigger;
    }
    else
      {
	if (_params.debug)
	  cerr << "MdvFlagData::setUpTrigger(): FILELIST mode called but no files." << endl;
	
	_trigger = 0;
        
	return 1;

      }
  }
  else if (_params.mode == Params::REALTIME ||_params.mode == Params::REALTIME_FCST )
  {
    if (_params.debug)
    {
      cerr << "MdvFlagData::Run(): Creating REALTIME trigger" << endl;
    }

    //
    // realtime mode
    //
    DsLdataTrigger *realtime_trigger = new DsLdataTrigger();

    if (realtime_trigger->init(_params.mdv_url,
                               _params.max_valid_realtime_age,
                               PMU_auto_register))
    {
      cerr << realtime_trigger->getErrStr();
      
      _trigger = 0;
      
      return 1;
    }
    else
    {
      _trigger = realtime_trigger;
    }
  }
  else
    {
      _trigger = 0;
      
      return 1;
    }
 
  return 0;
  
}

///////////////////////////////////
//
//  process data at trigger time
//
int MdvFlagData::_processData(const time_t inputTime, const int leadTime, const string filepath)
{
  //
  // registration with procmap
  //
  PMU_force_register("Processing data");

  if (_params.debug)
    {
      cerr << "MdvFlagData::_processData: Processing time: issue: " << inputTime 
	   << " lead time: "  << leadTime << " file trigger: " << filepath << endl;
    }
  
  if (_readMdv(inputTime, leadTime, filepath))
    {
      cerr << "ERROR - MdvFlagData::_processData()" << endl;
      return 1;
    }
  
  if ( _applyFlagMods())
    {
      cerr << "ERROR - MdvFlagData::_processData()" << endl;
      
      return 1;
    }

 
  if (_writeMdv())
    {
      cerr << "ERROR - MdvFlagData::_processData()" << endl;
      return 1;
    }

  return 0;
}

//
// Read mdv file.
//
int MdvFlagData::_readMdv(const time_t requestTime, const int leadTime, const string filepath)
{
  //
  // Set up for reading mdv data, reinitialize DsMdvx object
  //
  _mdvx.clearRead();


  if ( _params.mode == Params::FILELIST )
    {
      _mdvx.setReadPath(filepath);
    }
  else if (_params.mode == Params::REALTIME_FCST)
    {
      _mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _params.mdv_url, 300, requestTime, leadTime);
    }
  else if (_params.mode == Params::REALTIME)
    {
      _mdvx.setReadTime(Mdvx::READ_CLOSEST, _params.mdv_url, 300, requestTime, leadTime);
    }
 
  if (_params.debug == Params::DEBUG_VERBOSE) 
    {
      cerr << "MdvFlagData::_readMdv() : Reading data for URL: "
	   << _params.mdv_url << endl;
      
      _mdvx.printReadRequest(cerr);
    }

  //
  // perform the read
  //
  if (_mdvx.readVolume()) 
    {
      cerr << "MdvFlagData::readMdv():"
	   << "  " << _mdvx.getErrStr() << endl;
    }

  //
  // Get master header
  //
  _masterHdr = _mdvx.getMasterHeader();

  //
  // Get Field data and other info from field header
  //
  MdvxField *field = _mdvx.getField(_params.field_name);
  
  field->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  
  _fHdr  =  field->getFieldHeader();

  _fieldData = (fl32*)field->getVol();

  _vHdr = field->getVlevelHeader();

  _fieldMissing = _fHdr.missing_data_value;


  MdvxField *flagsField = _mdvx.getField(_params.field_flags_name);
   
  flagsField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  
  _flagsFhdr = flagsField->getFieldHeader();

  _flagsFieldData =  (fl32*)flagsField->getVol();

  _flagsVhdr =  flagsField->getVlevelHeader();

  _flagsMissing = _flagsFhdr.missing_data_value;

  _nx = _fHdr.nx;

  _ny = _fHdr.ny;

  _nz = _fHdr.nz;
  
  return 0;
}


int MdvFlagData::_applyFlagMods()
{
  if (_params.debug)
    cerr << "MdvFlagData::_ApplyFlagMods()" << endl;

  int num_pts = _nx * _ny * _nz;
 
  for(int i = 0; i < num_pts; i++) 
    {
      if (_flagsFieldData[i] != _flagsMissing )
	{
	  for (int j = 0; j < _params.flag_maps_n; j++)
	    {
	      if ( fabs( _flagsFieldData[i] -  _params._flag_maps[j].flagVal) < .0001)
		{
		  if(_params._flag_maps[j].setToMissing)
		    _fieldData[i] = _fieldMissing;
		  else
		    _fieldData[i] = _params._flag_maps[j].newVal; 
		}
	    }// end for each mask map
	} // end if flags data not missing
      else
	{
	  //
	  // Flag data is missing. If field data is missing, set val to zero
	  // (Hardwire by request!)
	  //
	  if (_fieldData[i] == _fieldMissing)
	    _fieldData[i] = 0;
	 
	}
    }// end for int i < numpts
  
  return 0;
}


int MdvFlagData::_writeMdv()
{

  if (_params.debug)
    cerr << "MdvFlagData::_writeMdv(): Writing data to " << _params.output_url << endl;
  
  PMU_auto_register("Writing data to output url");
  
  DsMdvx mdvFile;

  //
  // Set master header
  //
  mdvFile.setMasterHeader(_masterHdr);

  MdvxField *field;

  //
  // Add modifield field
  //
  field = new MdvxField(_fHdr, _vHdr, (void*)_fieldData);
 
  field->convertType(Mdvx::ENCODING_INT16,
		     Mdvx::COMPRESSION_GZIP,
		     Mdvx::SCALING_DYNAMIC);
      
  mdvFile.addField(field); 
 
  //
  // Add flag data
  //
  field = new MdvxField(_flagsFhdr, _flagsVhdr, (void*)_flagsFieldData);

  field->convertType(Mdvx::ENCODING_INT16,
		     Mdvx::COMPRESSION_GZIP,
		     Mdvx::SCALING_DYNAMIC);
      

  mdvFile.addField(field); 

  if (_params.mode == Params::REALTIME_FCST || _params.output_fcst_dir)
    {
      mdvFile.setWriteAsForecast();
    }
  
  mdvFile.writeToDir(_params.output_url);
  
  return 0;
}
