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
 *  @file MdvBlender.cc
 *
 *  @class MdvBlender
 *
 *  Blends two MDV files
 *
 *  @author G. P. McCabe
 * 
 *  @date August, 2014
 *
 *  @version $Id: MdvBlender.cc,v 1.19 2016/03/04 02:22:10 dixon Exp $
 *
 */

// C++ include files
#include <algorithm>
#include <cassert>

// System/RAP include files
#include <Mdv/DsMdvxInput.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <iomanip>      // std::setprecision

// Local include files
#include "MdvBlender.hh"
#include "Params.hh"
#include "Args.hh"

using namespace std;

// the singleton itself
MdvBlender *MdvBlender::_instance = 0;

// define any constants
const string MdvBlender::_className = "MdvBlender";

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

MdvBlender::MdvBlender() : 
  _isOK(true),
  _mdvx1(NULL),
  _mdvx2(NULL)
{

  // Make sure the singleton wasn't already created.
  assert(_instance == 0);

  // Set the singleton instance pointer
  _instance = this;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

MdvBlender::~MdvBlender()
{
  // unregister process

  PMU_auto_unregister();

  // Free contained objects
  delete _params;
  delete _args;

  delete _inputTrigger;
  for(unsigned int i=0; i< _blenderFieldNames.size(); i++) {
    delete _blenderFieldNames[i];
  }
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
// Method Name:	MdvBlender::instance
//
// Description:	this method creates instance of MdvBlender object
//
// Returns:	returns pointer to self
//
// Notes:	this method implements the singleton pattern
//
//

MdvBlender* MdvBlender::instance(int argc, char **argv)
{
  if (_instance == 0) {
    new MdvBlender();

    if(_instance->_isOK) {
      _instance->_initialize(argc, argv);
    }
  }
  return(_instance);
}

MdvBlender* MdvBlender::instance()
{
  assert(_instance != 0);
  return(_instance);
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MdvBlender::run
//
// Description:	runs the object
//
// Returns:	returns 0
//
// Notes:
//
//

int MdvBlender::run()
{
  const string methodName = _className + string( "::run" );

  // register with procmap
  PMU_auto_register("Running");

  if(_params->debug != Params::DEBUG_OFF) {
    cerr << "Running:" << endl;
  } 

  bool moreDataTimes = true;

  while(moreDataTimes){
    PMU_auto_register("In main loop");

    // GET NEXT RUN TIME
    if((_params->run_mode == Params::REALTIME) ||
      (_params->run_mode == Params::ARCHIVE)) {
    
      if(_inputTrigger->getTimeNext(_runTime) == -1) {
        cerr << "getTimeNext failed." << endl;
        cerr << _inputTrigger->getErrStr() << endl;
        return false;
      }
    }    
    if (_params->run_mode == Params::REALTIME){
      int totalSleep = 0;
      if (_params->trigger_sleep> 0){
        if(_params->debug >= Params::DEBUG_NORM) {
          cerr << "\nNew Input Triggered with runtime: " 
               << DateTime::strm(_runTime) << endl
               << "Sleeping (seconds): " << _params->trigger_sleep << endl;
        }
      }
      while (totalSleep < _params->trigger_sleep){
        string status ("Input Triggered: Sleeping");
        PMU_auto_register(status.c_str());
        int sleepTime = 10;
        if (sleepTime > _params->trigger_sleep - totalSleep){
          sleepTime =  _params->trigger_sleep - totalSleep;
        }
        sleep(sleepTime);
        totalSleep += sleepTime;
      }
    }

    /* Do blending */
    for (int dix = 0; dix < _params->blender_info_n; dix++){
      if(!_readInputFiles(dix)) {
        return -1;
      }

      if(!_doBlend(dix)) {
        return -3;
      }
    }

    if(!_addPassThroughFields()) {
      cerr << "ERROR: Could not add pass through fields" << endl;
      return -4;
    } 

    _writeOutput();

    _cleanupOutputData();

    //if given a specific runtime, just run once & exit.
    if(_params->run_mode == Params::RUNTIME){
      moreDataTimes  = false;
    }		 
    else{
      moreDataTimes = !_inputTrigger->endOfData();
    }
  }
  delete _mdvx1;
  delete _mdvx2;
  delete _out;

  return 0;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MdvBlender::_initialize
//
// Description:	Initializes the class.
//
// Returns:	none
//
// Notes:	use isOK method to test success
//
//

bool MdvBlender::_initialize(int argc, char **argv)
{
  const string methodName = _className + "::_initialize";

  // set programe name
  Path pathParts(argv[0]);
  _progName = pathParts.getBase();

  ucopyright(_progName.c_str());

  // get command line args
  _args = new Args(argc, argv, _progName);

  // get TDRP params
  _params = new Params();

  if(_params->loadFromArgs(argc, argv, _args->getOverride().list, NULL)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    _isOK = false;
  } 

 
  // init process mapper registration
  if(_params->run_mode == Params::REALTIME) {
    PMU_auto_init(const_cast<char *>(_progName.c_str()),
    _params->instance,
    PROCMAP_REGISTER_INTERVAL);
  }

  // initialize the input trigger
  _inputTrigger = new DsMdvxInput();

  if(_params->run_mode == Params::REALTIME) {
    if(_inputTrigger->setRealtime(_params->trigger_url, 
                                  _params->trigger_max_valid_age,
	                                (DsMdvxInput::heartbeat_t)PMU_auto_register)) {
      cerr << "ERROR - " << endl;
      cerr << "  setup for realtime mode failed." << endl;
      cerr << _inputTrigger->getErrStr() << endl;
      return false;
    }  
  } else if(_params->run_mode == Params::ARCHIVE) {
    time_t startTime = DateTime::parseDateTime(_params->start_time);
    time_t endTime = DateTime::parseDateTime(_params->end_time);
    
    if(_inputTrigger->setArchive(_params->trigger_url, startTime, endTime)) {
      cerr << "ERROR - " << endl;
      cerr << "  setup for archive mode failed." << endl;
      cerr << _inputTrigger->getErrStr() << endl;
      return false;
    }  
  } else if(_params->run_mode == Params::RUNTIME) {
    _runTime = DateTime::parseDateTime(_params->run_time);
  }

  //initialize the two inputs

  _mdvx1 = new DsMdvx();
  if(_params->debug >= Params::DEBUG_GARRULOUS) {
    _mdvx1->setDebug(true);
  }
  else {
    _mdvx1->setDebug(false);
  }
  _mdvx1->clearRead();

  _mdvx2 = new DsMdvx();
  if(_params->debug >= Params::DEBUG_GARRULOUS) {
    _mdvx2->setDebug(true);
  }
  else {
    _mdvx2->setDebug(false);
  }
  _mdvx2->clearRead();

  _out = new DsMdvx();

  if(_params->random_seed < 0) {
    srand(time(NULL));
  }
  else {
    srand(_params->random_seed);
  }

  //parse the param strings & store in more sensible data structures
  return _parseParams();
}
/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MdvBlender::_parseParams()
//
// Description:	Parse the param strings
//
// Returns:	none
//
// Notes:	use isOK method to test success
//
//
bool MdvBlender::_parseParams()
{
  //output field names
  for (int ix=0; ix < _params->blender_info_n; ix++){
    blenderFieldNames_t* bft = new blenderFieldNames_t();
    std::vector<std::string> roundingTemp;

    // output file names
    _fillVector(_params->_blender_info[ix].outputFieldNames, bft->outputFieldNames);
    _fillVector(_params->_blender_info[ix].outputFieldLongNames, bft->outputFieldLongNames,false);
    _fillVector(_params->_blender_info[ix].outputFieldUnits, bft->outputFieldUnits,false);
    _fillVector(_params->_blender_info[ix].input1FieldNames, bft->input1FieldNames);
    _fillVector(_params->_blender_info[ix].input2FieldNames, bft->input2FieldNames);
    _fillVector(_params->_blender_info[ix].outputRounding, roundingTemp);
    bft->input1WeightName = _params->_blender_info[ix].input1WeightField;
    bft->input1ConstantWeight = _params->_blender_info[ix].input1ConstantWeight;
    bft->input2WeightName = _params->_blender_info[ix].input2WeightField;
    bft->input2ConstantWeight = _params->_blender_info[ix].input2ConstantWeight;
		
    for(unsigned int rix = 0; rix < roundingTemp.size(); rix++) {
	    if(roundingTemp[rix] == "ROUND_NONE") { 
		    bft->outputRounding.push_back(Params::ROUND_NONE);
	    }
	    else if(roundingTemp[rix] == "ROUND_UP") { 
		    bft->outputRounding.push_back(Params::ROUND_UP);
	    }
	    else if(roundingTemp[rix] == "ROUND_DOWN") { 
		    bft->outputRounding.push_back(Params::ROUND_DOWN);
	    }
	    else if(roundingTemp[rix] == "ROUND_CLOSEST") { 
		    bft->outputRounding.push_back(Params::ROUND_CLOSEST);
	    }
	    else {
		    cerr << "Invalid parameter: outputRounding"  << endl;
		    return false;
	    }
    }

    if(bft->outputFieldNames.size() !=  bft->outputFieldLongNames.size() ||
       bft->outputFieldNames.size() !=  bft->outputFieldUnits.size() ||
       bft->outputFieldNames.size() !=  bft->input1FieldNames.size() ||
       bft->outputFieldNames.size() !=  bft->input1FieldNames.size() ||
       bft->outputFieldNames.size() !=  bft->outputRounding.size() ) {
      cerr << "ERROR: blender_info[" << ix << "] parameter's lists must contain the same number of items.  Exiting..." << endl;
      _isOK = false;
      return false;
    }

    _blenderFieldNames.push_back(bft);
    printBlenderField(bft);
  }

  return true;

}
/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MdvBlender::_fillVector()
//
// Description:	Parse the param strings
//
// Returns:	none
//
// Notes:	use isOK method to test success
//
//
void MdvBlender::_fillVector(string commaSep, vector<string>& vec, bool removeWhite/*=true*/)
{

  // add an extra comma at the end to simplify later processing
  string tmpString(commaSep);
  tmpString += ",";

  size_t commaPos;
  while (tmpString.length() > 0) {
    commaPos = tmpString.find(","); 
    string element = tmpString.substr(0,commaPos);
    //trim white space
    if(removeWhite) {
      element.erase(remove(element.begin(), element.end(), ' '), element.end());
    }
    vec.push_back(element);
    tmpString.erase(0,commaPos+1);
  }
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MdvBlender::_readInputFiles
//
// Description:	Initializes the class.
//
// Returns:	none
//
// Notes:	use isOK method to test success
//
//
bool MdvBlender::_readInputFiles(int bix)
{
  PMU_force_register("Reading Inputs");
  if(_params->debug >= Params::DEBUG_NORM) {
    cout << "Reading Inputs at " << ctime(&_runTime) << endl;
  }

  _mdvx1->clearReadFields();
  _mdvx1->setReadTime(Mdvx::READ_FIRST_BEFORE, _params->input1_url,  _params->input1_max_valid_age, _runTime);

  _mdvx2->clearReadFields();
  _mdvx2->setReadTime(Mdvx::READ_FIRST_BEFORE, _params->input2_url,  _params->input2_max_valid_age, _runTime);

  vector<string>& input1FieldNames = _blenderFieldNames[bix]->input1FieldNames;
  vector<string>& input2FieldNames = _blenderFieldNames[bix]->input2FieldNames;

  vector<string>::const_iterator six;
  for(six = input1FieldNames.begin(); six != input1FieldNames.end(); six++) {
    _mdvx1->addReadField(*six);
    if(_params->debug >= Params::DEBUG_VERBOSE) {
      cout << "adding " << *six << endl;
    }
  }

  if(_blenderFieldNames[bix]->input1WeightName != "")  {
    _mdvx1->addReadField(_blenderFieldNames[bix]->input1WeightName);
  }

  for(six = input2FieldNames.begin(); six != input2FieldNames.end(); six++) {
    _mdvx2->addReadField(*six);
  }

  if(_blenderFieldNames[bix]->input2WeightName != "")  {
    _mdvx2->addReadField(_blenderFieldNames[bix]->input2WeightName);
  }

  _mdvx1->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvx1->setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _mdvx1->setReadScalingType(Mdvx::SCALING_NONE);

  _mdvx2->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvx2->setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _mdvx2->setReadScalingType(Mdvx::SCALING_NONE);


  if(_mdvx1->readVolume() < 0) {  
    cerr << "ERROR: failed to read input1. " << endl;
    cerr << _mdvx1->getErrStr() << endl;
    cerr.flush();
    return false;
  }
  if(_mdvx2->readVolume() < 0) {  
    cerr << "ERROR: failed to read input2. " << endl;
    cerr << _mdvx2->getErrStr() << endl;
    cerr.flush();
    return false;
  }
  
  cerr << "File found for input 1 (" <<  _params->input1_url << "): " << _mdvx1->getPathInUse() << endl;
  cerr << "File found for input 2 (" <<  _params->input2_url << "): " << _mdvx2->getPathInUse() << endl;

  if(!_initializeOutputs()) {
    return false;
  }
  return true;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MdvBlender::_initializeOutputs
//
// Description:	Initializes the class.
//
// Returns:	none
//
// Notes:	use isOK method to test success
//
//
bool MdvBlender::_initializeOutputs()
{
  switch (_params->output_projection) {      
    case Params::OUTPUT_PROJ_INPUT1: {
      _outMasterHdr = _mdvx1->getMasterHeader();
      _outFieldHdr = _mdvx1->getFieldByNum(0)->getFieldHeader();
      _outVlevelHdr = _mdvx1->getFieldByNum(0)->getVlevelHeader();
      _numX = _outFieldHdr.nx;
      _numY = _outFieldHdr.ny;
      _numZ = _outFieldHdr.nz;
      break;
    }

  default:
    cerr << "ERROR: Only OUTPUT_PROJ_INPUT1 has been implemented." << endl;
    return false;
  }

  return true;
}


bool MdvBlender::_allocateOutputData(int dix, int fix) {
  vector<string>& outputFieldNames = _blenderFieldNames[dix]->outputFieldNames;

  string inName = _blenderFieldNames[dix]->input1FieldNames[fix];
  _outVlevelHdr = _mdvx1->getFieldByName(inName)->getVlevelHeader();
  Mdvx::field_header_t fh = _mdvx1->getFieldByName(inName)->getFieldHeader();
  unsigned int numX = fh.nx;
  unsigned int numY = fh.ny;
  unsigned int numZ = fh.nz;
  if(numX != _numX ||
     numY != _numY ||
     numZ != _numZ) {
	  cerr << "Field " << inName << " dimensions do not match first grid" << endl;
	  return false;
  }
  _numElem = _numX*_numY*_numZ;
  float* vol = new float[_numElem];
  for(unsigned int i = 0; i < _numElem; ++i) {
    vol[i] = Constants::MISSING_DATA_VALUE;
  }
  string outName = outputFieldNames[fix];
  map<string, float*>::iterator it = _vols.find(outName);
  if(it == _vols.end()) {
    _vols[outName] = vol;
  }
  else { // check this in another place?
    cerr << "ERROR: Field " << outName << " already read." << endl;
    return false;
  }
	
  return true;
}

void MdvBlender::printBlenderField(blenderFieldNames_t* bft)
{
  cerr << "Added blender field:" << endl;

  cerr << "  Input 1 Names:" << endl;
  for(unsigned int i=0; i< bft->input1FieldNames.size(); i++) {
    cerr << "    " << bft->input1FieldNames[i] << endl;
  }

  if(bft->input1WeightName == "") { 
    cerr << "  Input 1 Constant Weight: " << endl;
    cerr << "    " << bft->input1ConstantWeight << endl;
  }
  else {
    cerr << "  Input 1 Weight Field: " << endl;
    cerr << "    " << bft->input1WeightName << endl;
  }

  cerr << "  Input 2 Names:" << endl;
  for(unsigned int i=0; i< bft->input2FieldNames.size(); i++) {
    cerr << "    " << bft->input2FieldNames[i] << endl;
  }

  if(bft->input2WeightName == "") { 
    cerr << "  Input 2 Constant Weight: " << endl;
    cerr << "    " << bft->input2ConstantWeight << endl;
  }
  else {
    cerr << "  Input 2 Weight Field: " << endl;
    cerr << "    " << bft->input2WeightName << endl;
  }

  cerr << "  Out Names:" << endl;
  for(unsigned int i=0; i< bft->outputFieldNames.size(); i++) {
    cerr << "    " << bft->outputFieldNames[i] << endl;
  }
}

void MdvBlender::_addFieldToOutput(string name, string longName, string units) {
  Mdvx::encoding_type_t encodingType =
    static_cast<Mdvx::encoding_type_t>(_params->encoding_type);
  Mdvx::compression_type_t compressionType =
    static_cast<Mdvx::compression_type_t>(_params->compression_type);

  Mdvx::field_header_t header;
  _setFieldHeader(header, name, longName, units, 0, _numX, _numY, _numZ);

  MdvxField* field = new MdvxField(header, _outVlevelHdr, 
                                   static_cast<const void*>(_vols[name]));
  field->convertType(encodingType, compressionType);
  _out->addField(field);
}

bool MdvBlender::_writeOutput()
{
  if(_params->debug >= Params::DEBUG_VERBOSE) {
    _out->setDebug(1);
  }

  if(_params->write_ldata) {
  _out->setWriteLdataInfo();
 }

  Mdvx::master_header_t masterHeader;
  _setMasterHeader(masterHeader);
  _out->setMasterHeader(masterHeader);

  if(_out->writeToDir(_params->output_url) != 0) {
    cerr << "ERROR: " << endl;
    cerr << "Error writing data for time " 
         << utimstr(_outMasterHdr.time_centroid) << " to URL " 
         << _params->output_url << ":" << _out->getErrStr() << endl;
    return false;
  } else {
    string outputPathMsg = string( "Writing to " ) + string( _out->getPathInUse() );
    cerr << outputPathMsg << endl;
    PMU_auto_register( outputPathMsg.c_str() );
  } 

  return true;
}


bool MdvBlender::_addPassThroughFields()
{
  _mdvx1->clearReadFields();
  _mdvx1->setReadTime(Mdvx::READ_FIRST_BEFORE, _params->input1_url,  _params->input1_max_valid_age, _runTime);
  _mdvx2->clearReadFields();
  _mdvx2->setReadTime(Mdvx::READ_FIRST_BEFORE, _params->input2_url,  _params->input2_max_valid_age, _runTime);

  if(_params->input1_pass_throughs_n != 0) {
    for(int i=0; i<_params->input1_pass_throughs_n; i++) {
      if(strlen(_params->_input1_pass_throughs[i].inFieldName) != 0) {
        _mdvx1->addReadField(_params->_input1_pass_throughs[i].inFieldName);
      }
    }

    _mdvx1->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    _mdvx1->setReadCompressionType(Mdvx::COMPRESSION_NONE);
    _mdvx1->setReadScalingType(Mdvx::SCALING_NONE);

    if (_mdvx1->readVolume() < 0) {  
      cerr << "ERROR: failed to read input1. " << endl;
      cerr << _mdvx1->getErrStr() << endl;
      cerr.flush();
      return false;
    }

    for(int i=0; i<_params->input1_pass_throughs_n; i++) {
      if(strlen(_params->_input1_pass_throughs[i].inFieldName) == 0) { continue; }
      string fname = _params->_input1_pass_throughs[i].inFieldName;
      MdvxField* fi = _mdvx1->getFieldByName(fname);
      MdvxField* field = new MdvxField(*fi);
      if(field == NULL) {
        cerr << "ERROR: Pass through field " 
             << _params->_input1_pass_throughs[i].inFieldName 
             << " not found in Input 1" << endl;
        return false;
      }
      Mdvx::field_header_t fh = field->getFieldHeader();
      STRcopy(fh.field_name_long, _params->_input1_pass_throughs[i].outFieldLongName, MDV_LONG_FIELD_LEN);
      STRcopy(fh.field_name, _params->_input1_pass_throughs[i].outFieldName, MDV_SHORT_FIELD_LEN);
      field->setFieldHeader(fh);
      _out->addField(field);
      if(_params->debug >= Params::DEBUG_NORM) {
        cerr << "Added " << field->getFieldName() << " to output" << endl;
      }
    }
  }

  if(_params->input2_pass_throughs_n != 0) {
    for(int i=0; i<_params->input2_pass_throughs_n; i++) {
      if(strlen(_params->_input2_pass_throughs[i].inFieldName) != 0) {
        _mdvx2->addReadField(_params->_input2_pass_throughs[i].inFieldName);
      }
    }

    _mdvx2->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    _mdvx2->setReadCompressionType(Mdvx::COMPRESSION_NONE);
    _mdvx2->setReadScalingType(Mdvx::SCALING_NONE);

    if (_mdvx2->readVolume() < 0) {  
      cerr << "ERROR: failed to read input2. " << endl;
      cerr << _mdvx2->getErrStr() << endl;
      cerr.flush();
      return false;
    }

    for(int i=0; i<_params->input2_pass_throughs_n; i++) {
      if(strlen(_params->_input2_pass_throughs[i].inFieldName) == 0) { continue; }
      MdvxField* field = new MdvxField(*_mdvx2->getFieldByName(_params->_input2_pass_throughs[i].inFieldName));
      if(field == NULL) {
        cerr << "ERROR: Pass through field " 
             << _params->_input2_pass_throughs[i].inFieldName 
             << " not found in Input 2" << endl;
        return false;
      }
      Mdvx::field_header_t fh = field->getFieldHeader();
      STRcopy(fh.field_name_long, _params->_input2_pass_throughs[i].outFieldLongName, MDV_LONG_FIELD_LEN);
      STRcopy(fh.field_name, _params->_input2_pass_throughs[i].outFieldName, MDV_SHORT_FIELD_LEN);
      field->setFieldHeader(fh);
      _out->addField(field);
    }
  }

  return true;
}


bool MdvBlender::_doBlend(int dix) {
  bool useInput1Constant;
  bool useInput2Constant;
  double input1Constant;
  double input2Constant;
  MdvxField* input1WeightField;
  MdvxField* input2WeightField;
  const float* input1WeightData;
  const float* input2WeightData;
  double w1;
  double w2;

  if(_blenderFieldNames[dix]->input1WeightName == "") {
    useInput1Constant = true;
    input1Constant = _blenderFieldNames[dix]->input1ConstantWeight;
  }
  else {
    useInput1Constant = false;
    input1WeightField = _mdvx1->getFieldByName(_blenderFieldNames[dix]->input1WeightName);
    input1WeightData = static_cast<const float*>(input1WeightField->getVol());
  }

  if(_blenderFieldNames[dix]->input2WeightName == "") {
    useInput2Constant = true;
    input2Constant = _blenderFieldNames[dix]->input2ConstantWeight;
  }
  else {
    useInput2Constant = false;
    input2WeightField = _mdvx2->getFieldByName(_blenderFieldNames[dix]->input2WeightName);
    input2WeightData = static_cast<const float*>(input2WeightField->getVol());
  }

  // if using DITHER mode, loop over grid and determine which input to use
  if(_params->blend_method == Params::BLEND_DITHER) {
	  //	  _useInputs.clear();
    _numElem = _numX*_numY*_numZ;
    _useInputs = new float[_numElem];
    
	  for(unsigned int j = 0; j < _numElem; j++) {
      if(useInput1Constant) {
        w1 = input1Constant;
      }
      else {
        w1 = input1WeightData[j];
      }

      if(useInput2Constant) {
        w2 = input2Constant;
      }
      else {
        w2 = input2WeightData[j];
      }
      //     _useInputs.push_back(_evaluateDither(w1,w2));
      _useInputs[j] = _evaluateDither(w1,w2);
    }
	  if(_params->output_decision) {
		  _vols["decision"] = _useInputs;
		  _addFieldToOutput("decision", "dither decision", "1 or 2");
	  }
  }

  for(unsigned int fix=0; fix<_blenderFieldNames[dix]->input1FieldNames.size(); fix++) {
    if(!_allocateOutputData(dix, fix)) {
	    continue;
    }

    string input1Name = _blenderFieldNames[dix]->input1FieldNames[fix];
    MdvxField* input1Field = _mdvx1->getFieldByName(input1Name);
    const float* input1Data = static_cast<const float*>(input1Field->getVol());
    double miss1 = input1Field->getFieldHeader().missing_data_value;

    string input2Name = _blenderFieldNames[dix]->input2FieldNames[fix];
    MdvxField* input2Field = _mdvx2->getFieldByName(input2Name);
    const float* input2Data = static_cast<const float*>(input2Field->getVol());
    double miss2 = input2Field->getFieldHeader().missing_data_value;
 
    // verify dimensions are the same
    if(!_verifyDimensions(input1Field,input2Field)) {
      return false;
    }

    if(!useInput1Constant) {
      if(!_verifyDimensions(input1Field,input1WeightField)) {
        return false;
      } 
    }

    if(!useInput2Constant) {
      if(!_verifyDimensions(input2Field,input2WeightField)) {
        return false;
      } 
    }

    //    _blender->setRound(_blenderFieldNames[dix]->outputRounding[fix]);    

    for(unsigned int j=0; j<_numElem; j++) {
      if(_params->debug >= Params::DEBUG_GARRULOUS) {
        int z = j / (_numX*_numY);
        int temp = j % (_numX*_numY);
        int y = temp / _numX;
        int x = temp % _numX;
        cerr << "[" << x << "," << y << "," << z << "]:";
      }
      if(_params->blend_method == Params::BLEND_AVERAGE) {
        if(useInput1Constant) {
          w1 = input1Constant;
        }
        else {
          w1 = input1WeightData[j];
        }

        if(useInput2Constant) {
          w2 = input2Constant;
        }
        else {
          w2 = input2WeightData[j];
        }
	      _vols[input1Name][j] = _evaluateAverage(input1Data[j], input2Data[j], w1, w2, miss1, miss2, _blenderFieldNames[dix]->outputRounding[fix]);
      }
      else { // if(_params->blend_method == Params::BLEND_DITHER) {
	      if(_useInputs[j] == 1) {
		      if(input1Data[j] != miss1) {
  		      _vols[input1Name][j] = input1Data[j];
		      }
		      else if(input2Data[j] != miss2) {
   		      _vols[input1Name][j] = input2Data[j];
		      }
		      else {
            _vols[input1Name][j] = Constants::MISSING_DATA_VALUE;
		      }
	      }
	      else { // if useInputs[j] == 2) {
		      if(input2Data[j] != miss2) {
  		      _vols[input1Name][j] = input2Data[j];
		      }
		      else if(input1Data[j] != miss1) {
   		      _vols[input1Name][j] = input1Data[j];
		      }
		      else {
            _vols[input1Name][j] = Constants::MISSING_DATA_VALUE;
		      }
	      }
      }
    }

    _addFieldToOutput(_blenderFieldNames[dix]->outputFieldNames[fix],_blenderFieldNames[dix]->outputFieldLongNames[fix], _blenderFieldNames[dix]->outputFieldUnits[fix]);
  }
  return true;
}


void MdvBlender::_cleanupOutputData() {
  typedef std::map<string, float*>::iterator it_type;
  for(it_type iterator =_vols.begin(); iterator != _vols.end(); iterator++) {
    delete [] iterator->second;
  }
  _vols.clear();
  _out->clearFields();
}


void MdvBlender::_setFieldHeader(Mdvx::field_header_t& hdr, const string& name,
          const string& longName, const string& unit, int fCode, int nx,
          int ny, int nz)
{
  memset(&hdr, 0, sizeof(Mdvx::field_header_t));

  hdr.nx = nx;
  hdr.ny = ny;
  hdr.nz = nz;
  hdr.forecast_time = 0; //_outFieldHdr.forecast_time;
  hdr.forecast_delta = 0; //_outFieldHdr.forecast_delta;
  hdr.field_code = fCode;
  hdr.proj_type = _outFieldHdr.proj_type;
  hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  hdr.data_element_nbytes = FLOAT32_SIZE;
  hdr.volume_size = nx*ny*nz*hdr.data_element_nbytes;
  hdr.compression_type = Mdvx::COMPRESSION_NONE;
  hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  hdr.scaling_type = Mdvx::SCALING_NONE;
  hdr.native_vlevel_type = _outFieldHdr.native_vlevel_type;

  if (nz > 1 ){
    hdr.vlevel_type = _outFieldHdr.vlevel_type;
  }
  else{
    hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  }
  hdr.dz_constant = _outFieldHdr.dz_constant;

  if (nz == 1){
    hdr.data_dimension = 2;
  }
  else{
    hdr.data_dimension = 3;
  }	

  hdr.proj_origin_lat = _outFieldHdr.proj_origin_lat;
  hdr.proj_origin_lon = _outFieldHdr.proj_origin_lon;

  for(int i = 0; i <  MDV_MAX_PROJ_PARAMS; ++i) {
    hdr.proj_param[i] = _outFieldHdr.proj_param[i];
  }

  hdr.grid_dx =  _outFieldHdr.grid_dx;
  hdr.grid_dy = _outFieldHdr.grid_dy;
  hdr.grid_dz = _outFieldHdr.grid_dz;
  hdr.grid_minx = _outFieldHdr.grid_minx;
  hdr.grid_miny = _outFieldHdr.grid_miny;
  hdr.grid_minz = _outFieldHdr.grid_minz;
  hdr.scale = 1.0;
  hdr.bias = 0.0;
  hdr.bad_data_value = Constants::BAD_DATA_VALUE;
  hdr.missing_data_value = Constants::MISSING_DATA_VALUE;

  STRcopy(hdr.field_name_long, longName.c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(hdr.field_name, name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(hdr.units, unit.c_str(), MDV_UNITS_LEN);
  STRcopy(hdr.transform, "none", MDV_TRANSFORM_LEN);
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputUrl::_setMasterHeader
//
void MdvBlender::_setMasterHeader(Mdvx::master_header_t& hdr)
{
  memset(&hdr, 0, sizeof(Mdvx::master_header_t));

  hdr.time_written = time(0);
  hdr.time_gen = _runTime;
  hdr.time_begin = _runTime;
  hdr.time_end = _runTime;
  hdr.time_centroid = _runTime;
  hdr.time_expire = _runTime + _params->expire_offset;
  hdr.num_data_times = _outMasterHdr.num_data_times;
  hdr.data_dimension = _outMasterHdr.data_dimension;
  hdr.data_collection_type = _outMasterHdr.data_collection_type;
  hdr.native_vlevel_type = _outMasterHdr.native_vlevel_type;
  if (hdr.max_nz > 1 ){
    hdr.vlevel_type = _outMasterHdr.vlevel_type;
  }
  else{
    hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  }
  hdr.vlevel_included = 1;
  hdr.grid_orientation = _outMasterHdr.grid_orientation;
  hdr.data_ordering = _outMasterHdr.data_ordering;
  hdr.n_fields = 0;
  hdr.max_nx = _outMasterHdr.max_nx;
  hdr.max_ny = _outMasterHdr.max_ny;
  hdr.max_nz = _outMasterHdr.max_nz;
  hdr.n_chunks = 0;
  hdr.field_grids_differ = 0;
 
  STRcopy(hdr.data_set_info,
          "Produced by MdvBlender", MDV_INFO_LEN);
  STRcopy(hdr.data_set_name,
          "MdvBlender", MDV_NAME_LEN);
}


bool MdvBlender::_verifyDimensions(MdvxField* input1Field, MdvxField* input2Field)
{
  Mdvx::field_header_t fh1 = input1Field->getFieldHeader();
  Mdvx::field_header_t fh2 = input2Field->getFieldHeader();

  double delta = _params->grid_delta_tol;

  if(fh1.nz != 1 || fh2.nz != 1) {
    if( fh1.nz != fh2.nz ||
        fabs(fh1.grid_dz - fh2.grid_dz) > delta ||
        fabs(fh1.grid_minz - fh2.grid_minz) > delta ) {
      cerr << "ERROR: Vertical grid dimenions (nz, dz, or minz) differ between fields" << endl;
      return false;
    }
  }

  if(fh1.nx != fh2.nx ||
     fh1.ny != fh2.ny) {
    cerr << "ERROR: Grid dimenions (nx/ny) differ between fields" << endl;
    return false;
  }

  if(fabs(fh1.grid_dx - fh2.grid_dx) > delta ||
     fabs(fh1.grid_dy - fh2.grid_dy) > delta ) {
    cerr << "ERROR: Grid deltas (dx/dy) differ between fields" << endl;
    return false;
  }

  if(fabs(fh1.grid_minx - fh2.grid_minx) > delta ||
     fabs(fh1.grid_miny - fh2.grid_miny) > delta ) {
    cerr << "ERROR: Grid minimums (minx/miny) differ between fields" << endl;
    return false;
  }

  return true;
}


int MdvBlender::_evaluateDither(double w1, double w2)
{
	/*
	if(d1 == miss1 && d2 == miss2) {
		return Constants::MISSING_DATA_VALUE;
	}
  if(d1 == miss1) {
    return 2;
  }
  if(d2 == miss2) {
		return 1;
  }
	*/
	double random = ((double)rand() / RAND_MAX) * (w1+w2);
	if(_params->debug >= Params::DEBUG_VERBOSE) {
	  std::cerr << "W1: " << w1 << " W2: " << w2 << " Value: " << random;
  }
	if(random < w1) {
                if(_params->debug >= Params::DEBUG_VERBOSE) {
			std::cerr << " - Using input 1 data" << std::endl;
    }
		return 1;
	}
	else {
		if(_params->debug >= Params::DEBUG_VERBOSE) {
			std::cerr << " - Using input 2 data" << std::endl;
    }
		return 2;
	}
}

double MdvBlender::_evaluateAverage(double d1, double d2, double w1, double w2, double miss1, double miss2, Params::round_t round) {
  if(d1 == miss1 && d2 == miss2) {
    return Constants::MISSING_DATA_VALUE;
  }
  if(d1 == miss1) {
    return d2;
  }

  if(d2 == miss2) {
    return d1;
  }
  if(_params->debug) {
    std::cerr << d1 << " * " << w1 << " + " << d2 << " * " << w2 << std::endl;
  }
  // add 0.5 to round to closest integer
  if(round == Params::ROUND_UP) {	
    return ceil( floor(d1 * w1 + 0.5) + floor(d2 * w2 + 0.5)  );
  }
  else if(round == Params::ROUND_DOWN) {	
    return floor( floor(d1 * w1 + 0.5) + floor(d2 * w2 + 0.5)  );
  }
  else if(round == Params::ROUND_CLOSEST) {	
    return floor( d1 * w1 + d2 * w2 + 0.5 );
  }
  return d1 * w1 + d2 * w2;
}
