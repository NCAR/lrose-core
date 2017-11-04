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
 *  @version $Id: MdvBlender.cc,v 1.30 2017/10/17 19:16:03 mccabe Exp $
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
  _mdvs()
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
  delete _blenderFieldNames;
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

  //  bool moreDataTimes = true;

  //  while(moreDataTimes){
  while(!_inputTrigger->endOfData()) {
    PMU_auto_register("In main loop");
    // GET NEXT RUN TIME
    //    if((_params->run_mode == Params::REALTIME) ||
    //      (_params->run_mode == Params::ARCHIVE)) {
    
      if(_inputTrigger->getTimeNext(_runTime) == -1) {
        cerr << "getTimeNext failed." << endl;
        cerr << _inputTrigger->getErrStr() << endl;
        return false;
      }
      //    }    
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
    if(!_readInputFiles()) {
      //      _cleanUp();
      continue;
      //      return -1;
    }

    if(!_verifyAllDimensions()) {
	    continue;
    }
    
    if(!_initializeOutputs()) {
      continue;
    }
    // TODO: re add verifyDimensions to compare fields across files 
    if(!_doBlend()) {
      //      _cleanUp();
      _cleanupOutputData();
      continue;
      //      return -3;
    }   

    if(!_addPassThroughFields()) {
      cerr << "ERROR: Could not add pass through fields" << endl;
      _cleanupOutputData();
      //      _cleanUp();
      continue;      
      //      return -4;
    } 

    _writeOutput();

    _cleanupOutputData();

    //if given a specific runtime, just run once & exit.
    //    if(_params->run_mode == Params::RUNTIME){
    //      moreDataTimes  = false;
    //    }		 
    //    else{
    //      moreDataTimes = !_inputTrigger->endOfData();
    //    }
  }

  _cleanUp();

  return 0;
}



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void MdvBlender::_cleanUp(){
  vector<unsigned int>::iterator it;
  for(it =_indices.begin(); it < _indices.end(); it++) {
    unsigned int i = *it;
    //  for(unsigned int i = 0; i < _mdvs.size(); i++) {
    delete _mdvs[i];
  }
  _mdvs.clear();
  _indices.clear();

  delete _out;
}

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
    cerr << "WARNING: RUNTIME mode has not been throughly tested" << endl;
    _runTime = DateTime::parseDateTime(_params->run_time);
    
    if(_inputTrigger->setArchive(_params->trigger_url, _runTime, _runTime)) {
      cerr << "ERROR - " << endl;
      cerr << "  setup for runtimee mode failed." << endl;
      cerr << _inputTrigger->getErrStr() << endl;
      return false;
    }  

  }

  //initialize the inputs
  for(int i = 0; i < _params->inputs_n; i++) {
    DsMdvx* mdv = new DsMdvx();
    mdv->setHeartbeatFunction((DsMdvx::heartbeat_t)PMU_auto_register);    
    if(_params->debug >= Params::DEBUG_GARRULOUS) {
      mdv->setDebug(true);
    }
    else {
      mdv->setDebug(false);
    }
    mdv->clearRead();
    _mdvs.push_back(mdv);
  }

  _out = new DsMdvx();
  _out->setHeartbeatFunction((DsMdvx::heartbeat_t)PMU_auto_register);

  if(_params->random_seed < 0) {
    srand(time(NULL));
  }
  else {
    srand(_params->random_seed);
  }

  // to check if grid information has not been set
  _maxZ = -1; 
  
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
  blenderFieldNames_t* bft = new blenderFieldNames_t();
  vector<string> roundingTemp;
  vector<string> blendTemp;  
  //output field names
  _fillVector(_params->output.field_names, bft->outputFieldNames);
  _fillVector(_params->output.field_long_names, bft->outputFieldLongNames,false);
  _fillVector(_params->output.field_units, bft->outputFieldUnits,false);
  _fillVector(_params->output.rounding, roundingTemp);
  _fillVector(_params->output.blend_methods, blendTemp);  

  for (int ix=0; ix < _params->inputs_n; ix++){
    vector<pass_through_t> pts;
    vector<string> inputs;
    // input file names
    _fillVector(_params->_inputs[ix].field_names, inputs);
    bft->inputFieldNames.push_back(inputs);

   
    for(int pix = 0; pix < _params->input_pass_throughs_n2; pix++) {
      if(strlen(_params->__input_pass_throughs[ix][pix]) == 0) {
	continue;
      }
      vector<string> items;

      pass_through_t pt; // = new pass_through_t();
      _fillVector(_params->__input_pass_throughs[ix][pix], items);
      pt.inputName = items[0];
      pt.outputName = items[1];
      pt.outputLongName = items[2];      
      pts.push_back(pt);
    }

    bft->inputPassThroughs.push_back(pts);
      
    bft->inputWeightNames.push_back(_params->_inputs[ix].weight_field);
    bft->inputConstantWeights.push_back(_params->_inputs[ix].constant_weight);
  }
  
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

  for(unsigned int bix = 0; bix < blendTemp.size(); bix++) {
    if(blendTemp[bix] == "BLEND_DITHER") { 
      bft->blendMethods.push_back(Params::BLEND_DITHER);
    }
    else if(blendTemp[bix] == "BLEND_AVERAGE") { 
      bft->blendMethods.push_back(Params::BLEND_AVERAGE);
    }
    else {
      cerr << "Invalid parameter: blendMethods"  << endl;
      return false;
    }
  }  
  
  unsigned int numFields = bft->outputFieldNames.size();

  if(bft->outputFieldLongNames.size() != numFields) {
    cerr << "ERROR: Incorrect number of output field long names" << endl;
    _isOK = false;
    return false;
  }

  if(bft->outputFieldUnits.size() != numFields) {
    cerr << "ERROR: Incorrect number of output field units" << endl;
    _isOK = false;
    return false;
  }

  if(bft->outputRounding.size() != numFields) {
    cerr << "ERROR: Incorrect number of output rounding" << endl;
    _isOK = false;
    return false;
  }

  for(unsigned int i = 0; i < bft->inputFieldNames.size(); i++) {
    if(bft->inputFieldNames[i].size() != numFields) {
      int ip1 = i+1;
      cerr << "ERROR: Incorrect number of input field "
	   << ip1 << " names" << endl;
      _isOK = false;
      return false;
    }
  }

  if(bft->inputPassThroughs.size() != 0 &&
     bft->inputPassThroughs.size() != bft->inputFieldNames.size()) {
    cerr << "ERROR: Number of input pass through sections should match " <<
      "number of inputs" << endl;
    _isOK = false;
    return false;
  }
  
  _blenderFieldNames  = bft;

  if(_params->debug >= Params::DEBUG_NORM) {
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
bool MdvBlender::_readInputFiles()
{
  vector< vector<string> > inputFieldNames;
  
  PMU_force_register("Reading Inputs");
  if(_params->debug >= Params::DEBUG_NORM) {
    cout << "Reading Inputs at " << ctime(&_runTime) << endl;
  }

  _indices.clear();
  
  for(int i = 0; i < _params->inputs_n; i++) {
    int ip1 = i+1;
    _mdvs[i]->clearReadFields();
    _mdvs[i]->setReadTime(Mdvx::READ_FIRST_BEFORE, _params->_inputs[i].url,  _params->_inputs[i].max_valid_age, _runTime);
    
    vector<string> inputFieldNames = _blenderFieldNames->inputFieldNames[i];
    vector<string>::const_iterator six;
    for(six = inputFieldNames.begin(); six != inputFieldNames.end(); six++) {
      _mdvs[i]->addReadField(*six);
      if(_params->debug >= Params::DEBUG_VERBOSE) {
        cout << "adding " << *six << endl;
      }
    }
    
    if(_blenderFieldNames->inputWeightNames[i] != "")  {
      _mdvs[i]->addReadField(_blenderFieldNames->inputWeightNames[i]);
    }

    _mdvs[i]->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    _mdvs[i]->setReadCompressionType(Mdvx::COMPRESSION_NONE);
    _mdvs[i]->setReadScalingType(Mdvx::SCALING_NONE);
    
    if(_mdvs[i]->readVolume() < 0) {
      if(_params->_inputs[i].required) {
        cerr << "ERROR: failed to read input " << ip1 << " (" << _params->_inputs[i].url << ")" << endl;
        cerr << _mdvs[i]->getErrStr() << endl;
        cerr.flush();
        return false;
      } else {
        cerr << "WARNING: failed to read input " << ip1 << "." << endl;
	cerr << "Data is not required, so skipping..." << endl;
      }
    }
    else {
      _indices.push_back(i);
      cout << "File found for input " << ip1 << " (" <<  _params->_inputs[i].url << "): " << _mdvs[i]->getPathInUse() << endl;
    }
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
      unsigned int i = *_indices.begin();
      _outMasterHdr = _mdvs[i]->getMasterHeader();      
      // loop fields and save a 3D field's info if available
      for(unsigned int j = 0; j < _blenderFieldNames->inputFieldNames[i].size(); j++) {
        string name = _blenderFieldNames->inputFieldNames[i][j];
        Mdvx::field_header_t fhdr = _mdvs[i]->getFieldByName(name)->getFieldHeader();

        if(_maxZ == -1 || fhdr.nz > _maxZ) {
          _outFieldHdr = _mdvs[i]->getFieldByName(name)->getFieldHeader();
          _outVlevelHdr = _mdvs[i]->getFieldByName(name)->getVlevelHeader();
          _numX = _outFieldHdr.nx;
          _numY = _outFieldHdr.ny;
          _maxZ = _outFieldHdr.nz;
        }
      }
      break;
    }

  default:
    cerr << "ERROR: Only OUTPUT_PROJ_INPUT1 has been implemented." << endl;
    return false;
  }

  return true;
}

bool MdvBlender::_allocateOutputData(int fix) {
  vector<string>& outputFieldNames = _blenderFieldNames->outputFieldNames;
  unsigned int i = *(_indices.begin());
  string inName = _blenderFieldNames->inputFieldNames[i][fix];
  _outVlevelHdr = _mdvs[0]->getFieldByName(inName)->getVlevelHeader();
  Mdvx::field_header_t fh = _mdvs[i]->getFieldByName(inName)->getFieldHeader();
  unsigned int numX = fh.nx;
  unsigned int numY = fh.ny;
  unsigned int numZ = fh.nz;
  if(numX != _numX ||
     numY != _numY) {
	  cerr << "ERROR: Field " << inName << " dimensions do not match first grid " << numX << " " << _numX << " and " << numY << " " << _numY << endl;
	  return false;
  }
  _numElem = _numX*_numY*numZ;
  float* vol = new float[_numElem];
  for(unsigned int i = 0; i < _numElem; ++i) {
    vol[i] = Constants::MISSING_DATA_VALUE;
  }
  string outName = outputFieldNames[fix];
  map<string, float*>::iterator it = _vols.find(outName);
  if(it == _vols.end()) {
    _vols[outName] = vol;
    delete [] vol;
  }
  else { // check this in another place?
    cerr << "ERROR: Field " << outName << " already read." << endl;
    return false;
  }
	
  return true;
}

void MdvBlender::printBlenderField(blenderFieldNames_t* bft)
{
  cout << "Added blender field:" << endl;

  for(unsigned int i = 0; i < bft->inputFieldNames.size(); i++) {
    int ip1 = i+1;
    cout << "  Input " << ip1 <<" Names:" << endl;
    
    for(unsigned int j=0; j< bft->inputFieldNames[i].size(); j++) {
      cout << "    " << bft->inputFieldNames[i][j] << endl;
    }
    if(bft->inputWeightNames[i] == "") { 
      cout << "  Input " << ip1 <<" Constant Weight: " << endl;
      cout << "    " << bft->inputConstantWeights[i] << endl;
    }
    else {
      cout << "  Input " << ip1 << " Weight Field: " << endl;
      cout << "    " << bft->inputWeightNames[i] << endl;
    }
  }

  cout << "  Out Names:" << endl;
  for(unsigned int i=0; i< bft->outputFieldNames.size(); i++) {
    cout << "    " << bft->outputFieldNames[i] << endl;
  }
}

void MdvBlender::_addFieldToOutput(string name, string longName, string units) {
  Mdvx::encoding_type_t encodingType =
    static_cast<Mdvx::encoding_type_t>(_params->encoding_type);
  Mdvx::compression_type_t compressionType =
    static_cast<Mdvx::compression_type_t>(_params->compression_type);

  int numZ;
  if(name == "decision") {
	  numZ = _maxZ;
  } else {
    numZ = _mdvs[0]->getFieldByName(name)->getFieldHeader().nz;
  }
  Mdvx::field_header_t header;
  _setFieldHeader(header, name, longName, units, 0, _numX, _numY, numZ);

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

  PMU_force_register( "Writing output" );

  if(_out->writeToDir(_params->output_url) != 0) {
    cerr << "ERROR: " << endl;
    cerr << "Error writing data for time " 
         << utimstr(_outMasterHdr.time_centroid) << " to URL " 
         << _params->output_url << ":" << _out->getErrStr() << endl;
    return false;
  } else {
    string outputPathMsg = string( "Wrote to " ) + string( _out->getPathInUse() );
    cerr << outputPathMsg << endl << endl;
    PMU_auto_register( outputPathMsg.c_str() );
  } 

  _cleanupOutputData();
  return true;
}


bool MdvBlender::_addPassThroughFields()
{
  PMU_force_register("Adding pass through fields");
  //  for(unsigned int i = 0; i < _params->input_pass_throughs_n1; i++) {
  //  for(unsigned int i = 0; i < _blenderFieldNames->inputPassThroughs.size(); i++) {
  vector<unsigned int>::iterator it;
  for(it=_indices.begin(); it < _indices.end(); it++) {
    unsigned int i = *it;
    _mdvs[i]->clearReadFields();
    _mdvs[i]->setReadTime(Mdvx::READ_FIRST_BEFORE, _params->_inputs[i].url,  _params->_inputs[i].max_valid_age, _runTime);

    //    for(int j=0; j<_params->input_pass_throughs_n2; j++) {
    for(unsigned int j=0; j<_blenderFieldNames->inputPassThroughs[i].size(); j++) {      
      if(_blenderFieldNames->inputPassThroughs[i][j].inputName.length() != 0) {
        _mdvs[i]->addReadField(_blenderFieldNames->inputPassThroughs[i][j].inputName);
      }
    }

    _mdvs[i]->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    _mdvs[i]->setReadCompressionType(Mdvx::COMPRESSION_NONE);
    _mdvs[i]->setReadScalingType(Mdvx::SCALING_NONE);

    if (_mdvs[i]->readVolume() < 0) {  
      cerr << "ERROR: failed to read input " << i << ". " << endl;
      cerr << _mdvs[i]->getErrStr() << endl;
      cerr.flush();
      return false;
    }

    for(unsigned int j=0; j<_blenderFieldNames->inputPassThroughs[i].size(); j++) {
      string fname = _blenderFieldNames->inputPassThroughs[i][j].inputName;
      if(fname.length() == 0) {
	continue;
      }

      MdvxField* fi = _mdvs[i]->getFieldByName(fname);
      MdvxField* field = new MdvxField(*fi);
      if(field == NULL) {
        cerr << "ERROR: Pass through field " 
             << _blenderFieldNames->inputPassThroughs[i][j].inputName 
             << " not found in Input " << i << endl;
        return false;
      }
      Mdvx::field_header_t fh = field->getFieldHeader();
      STRcopy(fh.field_name_long, _blenderFieldNames->inputPassThroughs[i][j].outputLongName.c_str(), MDV_LONG_FIELD_LEN);
      STRcopy(fh.field_name, _blenderFieldNames->inputPassThroughs[i][j].outputName.c_str(), MDV_SHORT_FIELD_LEN);
      field->setFieldHeader(fh);
      _out->addField(field);
      if(_params->debug >= Params::DEBUG_VERBOSE) {
        cerr << "Added " << field->getFieldName() << " to output" << endl;
      }
    }
  }

  return true;
}


float* MdvBlender::_ditherFields(vector<const float*> dataVols, vector<float> miss, vector<float> bad) {
  float* out = new float[_numElem];
  // loop grid and select data from vols based on useInputs
  for(unsigned int j = 0; j < _numElem; j++) {
    vector<unsigned int>::iterator it;
    int count = 0;
    for(it=_indices.begin(); it < _indices.end(); it++, count++) {
      unsigned int i = *it;
      if(_useInputs[j] == i) {

        if(dataVols[count][j] != miss[count] && dataVols[count][j] != bad[count]) {
          out[j] = dataVols[count][j];
        }
        else {
          out[j] = Constants::MISSING_DATA_VALUE;
        }
      }
    }
  }
  return out;
}

float* MdvBlender::_averageFields(vector<const float*> dataVols, vector<float> constants, vector<const float*> weightVols, vector<float> miss, vector<float> bad){
  float* out = new float[_numElem];
  for(unsigned int j = 0; j < _numElem; j++) {
    float numerator = 0.0;
    float denominator = 0.0;
    vector<unsigned int>::iterator it;
    int count = 0;
    for(it=_indices.begin(); it < _indices.end(); it++, count++) {      
      if( dataVols[count][j] != miss[count] && dataVols[count][j] != bad[count]) {
        if(constants[count] == 0) {
          numerator += dataVols[count][j] * weightVols[count][j];
	  denominator += weightVols[count][j];
	}
	else {
          numerator += dataVols[count][j] * constants[count];
	  denominator += constants[count];
	}
      }
    }
    
    if(denominator != 0.0) {
      out[j] = numerator / denominator;
    }
    else {
      out[j] = Constants::MISSING_DATA_VALUE;
    }   
  }
  
  return out;
}

bool MdvBlender::_doBlend() {
  vector<bool> useConstant;
  vector<double> constants;
  vector<MdvxField*> weightFields;
  MdvxField* weightField;  
  vector<const float*> weightData;
  vector<double> w;
  vector<double> dataPts;

  PMU_force_register("Blending data");

  _numElem = _numX*_numY*_maxZ;
  // loop over indices
  //   check if any dithering is done, set bool
  bool doDither = false;
  vector<unsigned int>::iterator it;
  for(it=_indices.begin(); it < _indices.end(); it++) {
    unsigned int i = *it;
    if(_blenderFieldNames->blendMethods[i] == Params::BLEND_DITHER) {
      doDither = true;
    }
  }
  // if dithering will be done, populate _useInputs  
  if(doDither) {
    _useInputs = new float[_numElem];
   
    for(unsigned int j = 0; j < _numElem; j++) {
      w.clear();
      for(it=_indices.begin(); it < _indices.end(); it++) {
        int i = *it;
        if(_blenderFieldNames->inputWeightNames[i] == "") {
          w.push_back(_blenderFieldNames->inputConstantWeights[i]);
	}
        else {
	  weightField = _mdvs[i]->getFieldByName(_blenderFieldNames->inputWeightNames[i]);
          w.push_back(static_cast<const float*>(weightField->getVol())[j]);
        }
      }
      _useInputs[j] = _indices[_evaluateDither(w)];
    }
  }

  // loop over fields
  for(unsigned int fix=0; fix<_blenderFieldNames->inputFieldNames[0].size(); fix++) {    
  //   allocate output data
    string pmuStr = string("Blending field: ") + _blenderFieldNames->inputFieldNames[0][fix];
    PMU_force_register(pmuStr.c_str());
    
    if(!_allocateOutputData(fix)) {
      continue;
    }

    string name = _blenderFieldNames->outputFieldNames[fix];      
    // if dithering, (function _ditherFields(useInputs, vector of data* vols))
    if(_blenderFieldNames->blendMethods[fix] == Params::BLEND_DITHER) {
      vector<const float*> dataVols;
      vector<float> miss;
      vector<float> bad;      
      for(it=_indices.begin(); it < _indices.end(); it++) {
        int i = *it;	
        dataVols.push_back(static_cast<const float*>(_mdvs[i]->getFieldByName(_blenderFieldNames->inputFieldNames[i][fix])->getVol()));
        miss.push_back(_mdvs[i]->getFieldByName(_blenderFieldNames->inputFieldNames[i][fix])->getFieldHeader().missing_data_value);
        bad.push_back(_mdvs[i]->getFieldByName(_blenderFieldNames->inputFieldNames[i][fix])->getFieldHeader().bad_data_value);		
      }
      _vols[name] = _ditherFields(dataVols,miss,bad);
    }
    else if(_blenderFieldNames->blendMethods[fix] == Params::BLEND_AVERAGE) {
      vector<const float*> dataVols;
      vector<float> constants;
      vector<const float*> weightVols;
      vector<float> miss;
      vector<float> bad;
      for(it=_indices.begin(); it < _indices.end(); it++) {
        int i = *it;	
        dataVols.push_back(static_cast<const float*>(_mdvs[i]->getFieldByName(_blenderFieldNames->inputFieldNames[i][fix])->getVol()));
	if(_blenderFieldNames->inputWeightNames[i] == "") {
          weightVols.push_back(NULL);
          constants.push_back(_blenderFieldNames->inputConstantWeights[i]);
	} else {
          weightVols.push_back(static_cast<const float*>(_mdvs[i]->getFieldByName(_blenderFieldNames->inputWeightNames[i])->getVol()));
          constants.push_back(0);
	}
        miss.push_back(_mdvs[i]->getFieldByName(_blenderFieldNames->inputFieldNames[i][fix])->getFieldHeader().missing_data_value);
        bad.push_back(_mdvs[i]->getFieldByName(_blenderFieldNames->inputFieldNames[i][fix])->getFieldHeader().bad_data_value);	
      }
      _vols[name] = _averageFields(dataVols, constants, weightVols, miss, bad);
    }
    //  }
  /*
  // if output decision is set, loop over _useInputs and add 1, then add to output 
  
  //  unsigned int numInputs = _params->inputs_n;
  //  for(unsigned int i = 0; i < numInputs; i++) {
  //  vector<unsigned int>::iterator it;
  unsigned int count = 0;
  for(it=_indices.begin(); it < _indices.end(); it++, count++) {
    unsigned int i = *it;
    if(_blenderFieldNames->inputWeightNames[i] == "") {
      useConstant.push_back(true);
      constants.push_back(_blenderFieldNames->inputConstantWeights[i]);
      weightFields.push_back(NULL);
      weightData.push_back(NULL);
    }
    else {
      useConstant.push_back(false);
      weightFields.push_back(_mdvs[i]->getFieldByName(_blenderFieldNames->inputWeightNames[i]));
      constants.push_back(-1);
      weightData.push_back(static_cast<const float*>(weightFields[count]->getVol())); 
    }
  }

  // if using DITHER mode, loop over grid and determine which input to use
  if(_params->blend_method == Params::BLEND_DITHER) {
    _numElem = _numX*_numY*_numZ;
    _useInputs = new float[_numElem];
   
    for(unsigned int j = 0; j < _numElem; j++) {
      w.clear();
      for(it=_indices.begin(); it < _indices.end(); it++) {
        int i = *it;
        if(useConstant[i]) {
          w.push_back(constants[i]);
	}
        else {
          w.push_back(weightData[i][j]);
        }
      }
      _useInputs[j] = _indices[_evaluateDither(w)];
    }
    // TODO: Move to bottom of function in another params blend metho == DITHER
    if(_params->output_decision) {
      //    for(unsigned int j = 0; j < _numElem; j++) {
      //           float* 
    //	    }
      _vols["decision"] = _useInputs;
      _addFieldToOutput("decision", "dither decision", "1-N");
    }
  }
  
  for(unsigned int fix=0; fix<_blenderFieldNames->inputFieldNames[0].size(); fix++) {
    string pmuStr = string("Blending field: ") + _blenderFieldNames->inputFieldNames[0][fix];
    PMU_force_register(pmuStr.c_str());
    
    if(!_allocateOutputData(fix)) {
      continue;
    }
    vector<string> inputNames;
    vector<MdvxField*> inputFields;
    vector<const float*> inputData;
    vector<double> miss;
    vector<double> bad;

    //    for(int i = 0 ; i < _params->inputs_n; i++) {
    count = 0;
    for(it=_indices.begin(); it < _indices.end(); it++, count++) {
      unsigned int i = *it;
      inputNames.push_back(_blenderFieldNames->inputFieldNames[i][fix]);
      inputFields.push_back(_mdvs[i]->getFieldByName(inputNames[count]));
      inputData.push_back(static_cast<const float*>(inputFields[count]->getVol()));
      miss.push_back(inputFields[count]->getFieldHeader().missing_data_value);
      bad.push_back(inputFields[count]->getFieldHeader().bad_data_value);

      // verify dimensions are the same
      if(i != 0) {
        if(!_verifyDimensions(inputFields[0], inputFields[count])) {
          return false;
        }
      }

      if(!useConstant[count]) {
        if(!_verifyDimensions(inputFields[count],weightFields[count])) {
          return false;
        } 
      }
    }

    for(unsigned int j=0; j<_numElem; j++) {
      if(_params->debug >= Params::DEBUG_GARRULOUS) {
        int z = j / (_numX*_numY);
        int temp = j % (_numX*_numY);
        int y = temp / _numX;
        int x = temp % _numX;
        if(_params->debug >= Params::DEBUG_GARRULOUS) {
          cout << "[" << x << "," << y << "," << z << "]:";
        }
      }
      
      if(_params->blend_method == Params::BLEND_AVERAGE) {
        w.clear();
        dataPts.clear();
	      //	miss.clear();
	
	      //	for(int i = 0; i < _params->inputs_n; i++) {
        count = 0;
        for(it=_indices.begin(); it < _indices.end(); it++, count++) {
          unsigned int i = *it;
          if(useConstant[count]) {
            w.push_back(constants[count]);
	  }
	  else {
	    w.push_back(weightData[count][j]);
	  }
	  dataPts.push_back(inputData[count][j]);
		      //          miss.push_back(inputFields[i]->getFieldHeader().missing_data_value);
        }
 
        string name = _blenderFieldNames->outputFieldNames[fix];
        _vols[name][j] = _evaluateAverage(dataPts, w, miss, bad, _blenderFieldNames->outputRounding[fix]);
      }
      else { // if(_params->blend_method == Params::BLEND_DITHER) {
	      //        for(int i = 0; i < _params->inputs_n; i++) {
        count = 0;
        for(it=_indices.begin(); it < _indices.end(); it++, count++) {
          unsigned int i = *it;
          if(_useInputs[j] == i) {
            string name = _blenderFieldNames->outputFieldNames[fix];
            if(inputData[count][j] != miss[count] && inputData[count][j] != bad[count]) {
              _vols[name][j] = inputData[count][j];
	    }
	    else {
	      _vols[name][j] = Constants::MISSING_DATA_VALUE;
	    }
	  }
        }
      }
    }
*/
    _addFieldToOutput(_blenderFieldNames->outputFieldNames[fix],_blenderFieldNames->outputFieldLongNames[fix], _blenderFieldNames->outputFieldUnits[fix]);
   
  }
  
  if(_params->output_decision) {
    // increment all values of _useInputs so that the decision output starts with 1, not 0
    for(unsigned int j = 0; j < _numElem; j++) {
      _useInputs[j]++; 
    }
      _vols["decision"] = _useInputs;
      _addFieldToOutput("decision", "dither decision", "1-N");
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

bool MdvBlender::_verifyAllDimensions()
{
  // loop over fields
  for(unsigned int fix=0; fix<_blenderFieldNames->inputFieldNames[0].size(); fix++) {
	  // loop each file and verify dimensions are the same
	  unsigned int first = *_indices.begin();
    string name1 = _blenderFieldNames->inputFieldNames[first][fix];
	  MdvxField* field1 = _mdvs[first]->getFieldByName(name1);
    
	  for(vector<unsigned int>::iterator it = _indices.begin()+1; it < _indices.end(); it++) {
      unsigned int i = *it;
      string name2 = _blenderFieldNames->inputFieldNames[i][fix];      
      MdvxField* field2 = _mdvs[i]->getFieldByName(name2);
  	  if(!_verifyDimensions(field1, field2)) {
  		  return false;
	    }
	  }
  }
  return true;
}

bool MdvBlender::_verifyDimensions(MdvxField* input1Field, MdvxField* input2Field)
{
  Mdvx::field_header_t fh1 = input1Field->getFieldHeader();
  Mdvx::field_header_t fh2 = input2Field->getFieldHeader();

  double delta = _params->grid_delta_tol;

  if(fh1.nz != 1 && fh2.nz != 1) {
    if( fh1.nz != fh2.nz ||
        fabs(fh1.grid_dz - fh2.grid_dz) > delta ||
        fabs(fh1.grid_minz - fh2.grid_minz) > delta ) {
      cerr << "ERROR: Vertical grid dimenions (nz, dz, or minz) differ between fields" << endl;
      return false;
    }
  }

  if(fh1.nx != fh2.nx ||
     fh1.ny != fh2.ny || 
     fh1.nz != fh2.nz) {	  
    cerr << "ERROR: Grid dimenions (nx/ny/nz) differ between fields" << endl;
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


 int MdvBlender::_evaluateDither(vector<double> w)
{
  double weightSum = 0;
  double runningWeight = 0;
  for(unsigned int i = 0; i < w.size(); i++) {
    weightSum += w[i];
  }
  double random = ((double)rand() / RAND_MAX) * (weightSum);
  if(_params->debug >= Params::DEBUG_VERBOSE) {
    for(unsigned int i = 0; i < w.size(); i++) {
      unsigned int ip1 = i+1;
      cout << "W" << ip1 << ": " << w[i] << " ";
    }
    cout << " Value: " << random;
  }
  for(unsigned int i = 0; i < w.size(); i++) {
    unsigned int ip1 = i+1;
    runningWeight += w[i];
    if(random < runningWeight) {
      if(_params->debug >= Params::DEBUG_VERBOSE) {
        cout << " - Using input " << ip1 << " data" << std::endl;
      }
      return i;
    }
  }
  return w.size()-1;
}

double MdvBlender::_evaluateAverage(vector<double> d, vector<double> w, vector<double> miss, vector<double> bad, Params::round_t round) {
  double weightedData = 0;
  double totalWeights = 0;
  for(unsigned int i = 0; i < d.size(); i++) {
    if(d[i] != miss[i] && d[i] != bad[i]) {
      weightedData += d[i] * w[i];
      totalWeights += w[i];
    }
  }

  if(totalWeights == 0) {
    return Constants::MISSING_DATA_VALUE;
  }
 
  // add 0.5 to round to closest integer
  double total = 0;
  if(round == Params::ROUND_UP) {
    return ceil(weightedData / totalWeights);
    //    return ceil( floor(d1 * w1 + 0.5) + floor(d2 * w2 + 0.5)  );
  }
  else if(round == Params::ROUND_DOWN) {
    return floor(weightedData / totalWeights);
    //    return floor( floor(d1 * w1 + 0.5) + floor(d2 * w2 + 0.5)  );
  }
  else if(round == Params::ROUND_CLOSEST) {
    total = (weightedData + 0.5) / totalWeights;
    return floor(total);
    //    return floor( d1 * w1 + d2 * w2 + 0.5 );
  }
  return weightedData / totalWeights;
}
