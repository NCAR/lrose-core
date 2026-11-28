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
 * @file MdvHowSimlar.cc
 *
 * A simple app to do a quick comparison between two Mdv files.
 * This app is based on MdvQuickCompare and MdvBlender but is used for ACES, where MdvQuickCompare is used by the IFI test harness.
 # MdvQuickCompare is entirely command line based, but this app will take a param file.
 *
 * @author P J Prestopnik
 * @version $Id $
 */

// SYSTEM INCLUDES
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>


// System/RAP include files
#include "os_config.h"
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>

#include <toolsa/str.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxTimes.hh>
#include <Mdv/MdvxField.hh>    


// Local include files
#include "MdvHowSimilar.hh"
#include "Params.hh"
#include "Args.hh"

using namespace std;

// the singleton itself
MdvHowSimilar *MdvHowSimilar::_instance = 0;

// define any constants

const float EPSILON = 1e-6;  // used for comparing floats


const float MdvHowSimilar::MISSING_DATA_VALUE = -9999;
const float MdvHowSimilar::SIG_DIFF_DEFAULT = 0.01;
const float MdvHowSimilar::RMS_SIG_DIFF_DEFAULT = 0.1;

MdvHowSimilar::MdvHowSimilar() : 
  _isOK(true)
{

  // Make sure the singleton wasn't already created.
  assert(_instance == 0);

  // Set the singleton instance pointer
  _instance = this;
}


MdvHowSimilar::~MdvHowSimilar()
{

  // Free contained objects
  delete _params;
  delete _args;

}


MdvHowSimilar* MdvHowSimilar::instance(int argc, char **argv)
{
  if (_instance == 0) {
    new MdvHowSimilar();

    if(_instance->_isOK) {
      _instance->_initialize(argc, argv);
    }
  }
  return(_instance);
}

bool MdvHowSimilar::_initialize(int argc, char **argv)
{

  // set programe name
  Path pathParts(argv[0]);
  _progName = pathParts.getBase();

  //ucopyright(_progName.c_str());

  // get command line args
  _args = new Args(argc, argv, _progName);

  _isOK = _args->isOK();
	  

  // get TDRP params
  _params = new Params();

  if(_params->loadFromArgs(argc, argv, _args->getOverride().list, NULL)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    _isOK = false;
  } 

  
  for (int ix = 0; ix < _params->field_comparison_info_n; ix++){
	  field_comparison_info_t fci;
	  fci.field_name = _params->_field_comparison_info[ix].field_name;
		fci.significant_difference = _params->_field_comparison_info[ix].significant_difference;
	  fci.rms_sig_diff = _params->_field_comparison_info[ix].rms_sig_diff;
	  fieldComparisonInfo.push_back(fci);
  }
  if (_args->fieldInfo.size() != 0){
    fieldComparisonInfo.clear();
    for(unsigned int x=0; x < _args->fieldInfo.size(); x++) {
      field_comparison_info_t fci;
      // split comma separated string into strings
      vector<string> items;
      _fillVector(_args->fieldInfo[x], items);
      fci.field_name = items[0];
      if(items.size() > 1) {
        fci.significant_difference = atof(items[1].c_str());
      }
      else {
        fci.significant_difference = SIG_DIFF_DEFAULT;
      }
      if(items.size() > 2) {
        fci.rms_sig_diff = atof(items[2].c_str());
      }
      else {
        fci.rms_sig_diff = RMS_SIG_DIFF_DEFAULT;
      }

      fieldComparisonInfo.push_back(fci);
    }
  }
	
  if (_args->reportFile != ""){
    frout.open(_args->reportFile.c_str());
    rout = &frout;
  }
  else{
    rout = &cout;
  }
  return true;
}
 
bool MdvHowSimilar::_allocateOutputData() {

  //still need this for temporary storage for calculations, even if we aren't creating an output file.
  for (fciIx_t fciIx = fieldComparisonInfo.begin(); fciIx != fieldComparisonInfo.end(); fciIx++){
    field_comparison_info_t& fci = *fciIx;

    MdvxField *field1 = _mdvx1.getFieldByName(fci.field_name );
    if (field1 == NULL){
      cerr << "ERROR: Field " << fci.field_name  << " not found in input 1." << endl;
      return false;
    }
    Mdvx::field_header_t fh1 = field1->getFieldHeader();

    MdvxField *field2 = _mdvx2.getFieldByName(fci.field_name );
    if (field2 == NULL){
      cerr << "ERROR: Field " << fci.field_name  << " not found in input 2." << endl;
      return false;
    }
    Mdvx::field_header_t fh2 = field2->getFieldHeader();

    _checkFieldDimensions(fh1,fh2);

    Mdvx::field_header_t fh = field1->getFieldHeader();

    size_t numX = fh.nx;
    size_t numY = fh.ny;
    size_t numZ = fh.nz;

    size_t numElem = numX * numY * numZ;

    fl32* vol = new fl32[numElem];

    for(size_t i = 0; i < numElem; ++i) {
      vol[i] = MISSING_DATA_VALUE;
    }

    string outName = fci.field_name;
    map<string, fl32*>::iterator it = _outVols.find(outName);
    if(it == _outVols.end()) {
      _outVols[outName] = vol;
    }
    else { 
      cerr << "ERROR: Field " << outName << " already read." << endl;
      return false;
    }
  }
  return true;
}

int MdvHowSimilar::run()
{
  if(_params->debug != Params::DEBUG_OFF) {
    cerr << "Running:" << endl;
  } 

  _readInputFiles();
  _allocateOutputData();
  _compareInputFiles();
  _writeOutput();
  _cleanupOutputData();

  return EXIT_SUCCESS;
}



void MdvHowSimilar::_writeOutput(){
  if (_args->outFile == ""){
    return;
  }

  if(_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing to output file: " << _args->outFile << endl;
    _mdvxOut.setDebug(1);
  }

  Mdvx::master_header_t masterHeader;
  _setMasterHeader(masterHeader);
  _mdvxOut.setMasterHeader(masterHeader);

  if(_mdvxOut.writeToPath(_args->outFile) != 0) {
    cerr << "ERROR: " << endl;
    cerr << "Error writing data for time " 
	 << utimstr(_outMasterHdr.time_centroid) << " to path " 
	 << _args->outFile << ":" << _mdvxOut.getErrStr() << endl;
    return;
  } else {
    string outputPathMsg = string( "Writing to " ) + string( _mdvxOut.getPathInUse() );
    cerr << outputPathMsg << endl;
    //PMU_auto_register( outputPathMsg.c_str() );
  } 
}

void MdvHowSimilar::_cleanupOutputData() {
  typedef std::map<string, float*>::iterator it_type;
  for(it_type iterator =_outVols.begin(); iterator != _outVols.end(); iterator++) {
    delete [] iterator->second;
  }
  _outVols.clear();
  if (_args->outFile == ""){
    return;
  }

  _mdvxOut.clearFields();
}

void MdvHowSimilar::_addFieldToOutput(string name) {

  string fname = name;
  Mdvx::encoding_type_t encodingType =
    static_cast<Mdvx::encoding_type_t>(_params->encoding_type);

  Mdvx::compression_type_t compressionType =
    static_cast<Mdvx::compression_type_t>(_params->compression_type);

  Mdvx::field_header_t header;
  _setFieldHeader(header, fname);

  MdvxField* field = new MdvxField(header, _outVlevelHdr, 
                                   static_cast<const void*>(_outVols[name]));
  field->convertType(encodingType, compressionType);
  _mdvxOut.addField(field);
}

void MdvHowSimilar::_setFieldHeader(Mdvx::field_header_t& hdr, const string& name)
{
  memset(&hdr, 0, sizeof(Mdvx::field_header_t));

  hdr.nx = _nx;
  hdr.ny = _ny;
  hdr.nz = _nz;
  hdr.forecast_time = 0; //_outFieldHdr.forecast_time;
  hdr.forecast_delta = 0; //_outFieldHdr.forecast_delta;
  hdr.field_code = 0; //fCode;
  hdr.proj_type = _outFieldHdr.proj_type;
  hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  hdr.data_element_nbytes = FLOAT32_SIZE;
  hdr.volume_size = _nx*_ny*_nz*hdr.data_element_nbytes;
  hdr.compression_type = Mdvx::COMPRESSION_NONE;
  hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  hdr.scaling_type = Mdvx::SCALING_NONE;
  hdr.native_vlevel_type = _outFieldHdr.native_vlevel_type;

  if (_nz > 1 ){
    hdr.vlevel_type = _outFieldHdr.vlevel_type;
  }
  else{
    hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  }
  hdr.dz_constant = _outFieldHdr.dz_constant;

  if (_nz == 1){
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
  hdr.bad_data_value = MISSING_DATA_VALUE;
  hdr.missing_data_value = MISSING_DATA_VALUE;
  STRcopy(hdr.field_name_long, (name+string(" Difference")).c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(hdr.field_name, name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(hdr.units, _outFieldHdr.units, MDV_UNITS_LEN);
  STRcopy(hdr.transform, "none", MDV_TRANSFORM_LEN);
}


void MdvHowSimilar::_readInputFiles(){
  if(_params->debug >= Params::DEBUG_NORM) {
    cout << "Reading Inputs at " <<  _args->file1 << " and " << _args->file2 << endl;
  }	
  _mdvx1.setReadPath(_args->file1);
  _mdvx2.setReadPath(_args->file2);
	
  for (fciIx_t fciIx = fieldComparisonInfo.begin(); fciIx != fieldComparisonInfo.end(); fciIx++){
    field_comparison_info_t& fci = *fciIx;
    _mdvx1.addReadField(fci.field_name);
    _mdvx2.addReadField(fci.field_name);
  }

  _mdvx1.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvx1.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _mdvx2.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvx2.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (_mdvx1.readVolume()){
    cerr << "Failed to read from file " << _args->file1 << endl;
    cerr << _mdvx1.getErrStr();
    exit (-1);
  }

  if (_mdvx2.readVolume()){
    cerr << "Failed to read from file " << _args->file2 << endl;
    cerr << _mdvx2.getErrStr();
    exit (-1);
  }

  _outMasterHdr = _mdvx1.getMasterHeader();
	
}

void MdvHowSimilar::_printExplanationHeader()
{
	//nDiffPts: %10i   TotalDiff: %10.4f   TotalAbsDiff: %10.4f   MeanDiff: %10.4f   STD: %10.4f   RMS: %10.4f Bad1: %i Bad2: %i\n",

	*rout << "nDiffPts - The number of points (i,j) on the current k-level that had a difference more than the \"significant_difference\" parameter in the param file. This uses the absolute value of the difference.\n";
	
	*rout << "TotalDiff - The sum of the differences at each point (when neither point is missing or bad).\n";

	*rout << "TotalAbsDiff - The sum of the absolute value of the differences at each point (when neither point is missing or bad).\n";

	*rout << "MeanDiff - TotalDiff divide by the total # of points in the field that are not missing or bad.\n";

	*rout << "STD -  STD of the TotalDiff population\n";

	*rout << "RMS - root mean square difference of the TotalDiff population.\n";

	*rout << "Bad1 - # of points where input 1 is bad/missing, but input 2 is valid data.\n";

	*rout << "Bad2 - # of points where input 2 is bad/missing, but input 1 is valid data. \n";

}

void MdvHowSimilar::_compareInputFiles()
{
	_printExplanationHeader();

  for (fciIx_t fciIx = fieldComparisonInfo.begin(); fciIx != fieldComparisonInfo.end(); fciIx++){
    field_comparison_info_t& fci = *fciIx;

    MdvxField *field1 = _mdvx1.getFieldByName(fci.field_name );
    fl32 *data1 = (fl32 *) field1->getVol();
    Mdvx::field_header_t fh1 = field1->getFieldHeader();

    MdvxField *field2 = _mdvx2.getFieldByName(fci.field_name );
    fl32 *data2 = (fl32 *) field2->getVol();
    Mdvx::field_header_t fh2 = field2->getFieldHeader();

    _setFieldDimensionsAndHeaders(field1);	

    float* outData = _outVols[fci.field_name];

    int vDiffs = 0; //# of vertical levels with significant differences
    float rmsTotal = 0;
    float diffTotal = 0.0;
    float absDiffTotal = 0.0;

    for (int iz=0; iz < fh1.nz; iz++){
      string header(fci.field_name );
      header += " : k = ";
      stringstream ss;
      ss << setw(2) << setfill(' ') << iz;
      header += ss.str();

      size_t vLevelOffset = iz * _nx * _ny;

      fl32 *outDataLevel = outData + vLevelOffset;
      fl32 *d1level = data1 + vLevelOffset;
      fl32 *d2level = data2 + vLevelOffset;
  
      float totalDiffLevel = 0.0;
      float absDiffLevel = 0.0;

      float rms = _doStatisticsForOneLevel(header, fh1, d1level, fh2, d2level, fci.significant_difference, outDataLevel, totalDiffLevel, absDiffLevel);
      rmsTotal += rms;
      diffTotal += totalDiffLevel;
      absDiffTotal += absDiffLevel;
      if (rms > fci.rms_sig_diff){
	vDiffs++;
      }				
    }
    _printFieldSummary(fci.field_name, vDiffs, rmsTotal/fh1.nz, diffTotal, absDiffTotal);

    if (_args->outFile != ""){
      _addFieldToOutput(fci.field_name);
    }
  }

}	


void MdvHowSimilar::_printFieldSummary(const string fieldName, const int vDiffs, const float avgRMS, const float diffTotal, const float absDiffTotal)
{
  *rout << "\nSUMMARY for " << fieldName << " -- AVG RMS: " << avgRMS << " | " << vDiffs << " level(s) have significant differences | DIFF TOTAL: " << diffTotal << " | ABS DIFF TOTAL: " << absDiffTotal << "\n\n";
}


float MdvHowSimilar::_doStatisticsForOneLevel(const string header, const Mdvx::field_header_t& fh1,
                                              const fl32* data1, const Mdvx::field_header_t& fh2,
                                              const fl32* data2, const float sigDiff, fl32* outData,
                                              float& totalDiff, float& totalAbsDiff)
{
  int nGoodPoints=0;
  double mean=0;
  double STD = 0;
  double RMS = 0;
  // double totalDiff = 0.0;
  totalDiff = 0.0;
  totalAbsDiff = 0.0;
  int nDiffPoints = 0;
  int totalGridPoints = _nx * _ny;

  int bad1 = 0; //count of input1 is bad/missing, but 2 is not.
  int bad2 = 0; //opposite count for input2

	
  for (int ix=0; ix < totalGridPoints; ix++){

    if (! isValid(fh1,data1[ix]) && ! isValid(fh2,data2[ix])){
      //everythings cool - they are both invalid				
      continue;
    }
    if(! isValid(fh1,data1[ix]) ){
      bad1++;
      continue;
    }
    if(! isValid(fh2,data2[ix])){
      bad2++;
      continue;
    }
    outData[ix] = data1[ix]-data2[ix];
    if ( fabs(outData[ix]) > sigDiff){
      nDiffPoints++;
    }

    totalDiff += outData[ix];				 
    totalAbsDiff += fabs(outData[ix]);
    nGoodPoints++;
  }

  if (nGoodPoints > 0){
    //mean
    mean = totalDiff / (double)nGoodPoints;
		
    //standard deviation
    int npCheck =0;
    double d=0.0;
    for (int i=0; i< totalGridPoints; i++){
      if (!isEqual(MISSING_DATA_VALUE,outData[i])){
	d=d+(outData[i]-mean)*(outData[i]-mean);
	npCheck++;
      }	
    }
	
    if (npCheck != nGoodPoints){
      cerr << "WARNING - npcheck " << npCheck <<  " != nGoodPoints " << nGoodPoints << endl;
    }
    STD = sqrt( d/nGoodPoints );

    // rms error
    double s=0.0;
    for (int i=0; i< totalGridPoints; i++){
      if (!isEqual(MISSING_DATA_VALUE,outData[i])){
	s=s+outData[i]*outData[i];
      }
    }
    RMS = sqrt( s / nGoodPoints );	
  }	

  char tmpBuff[1024];
  sprintf(tmpBuff,"%s   nDiffPts: %10i   TotalDiff: %10.4f   TotalAbsDiff: %10.4f   MeanDiff: %10.4f   STD: %10.4f   RMS: %10.4f Bad1: %i Bad2: %i\n",
	  header.c_str(), nDiffPoints, totalDiff, totalAbsDiff, mean, STD, RMS, bad1, bad2);
  *rout << tmpBuff;
  return RMS;

}

void MdvHowSimilar::_checkFieldDimensions(const Mdvx::field_header_t& fh1, const Mdvx::field_header_t& fh2)
{
  if (
      (fh1.nx != fh2.nx) ||
      (fh1.ny != fh2.ny) ||
      (fh1.nz != fh2.nz)
      ){
    cerr << "FATAL: Input files have different grid sizes.";
    cerr << "Program terminated." << endl;
    exit(EXIT_FAILURE);
  }


}
void MdvHowSimilar::_setFieldDimensionsAndHeaders(const MdvxField* mf)
{

  Mdvx::field_header_t fh = mf->getFieldHeader();
  _nx = fh.nx;
  _ny = fh.ny;
  _nz = fh.nz;

  _outFieldHdr = fh;
  _outVlevelHdr = mf->getVlevelHeader();
}

bool MdvHowSimilar::isValid(const Mdvx::field_header_t& fh, float value)
{
  return ! (isEqual(value, fh.missing_data_value) ||
	    isEqual(value, fh.bad_data_value));	    
}

bool MdvHowSimilar::isEqual(float a, float b)
{
  return ( fabs(a-b) < EPSILON);
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputUrl::_setMasterHeader
//
void MdvHowSimilar::_setMasterHeader(Mdvx::master_header_t& hdr)
{
  memset(&hdr, 0, sizeof(Mdvx::master_header_t));

  hdr.time_written = time(0);
  hdr.time_gen = _outMasterHdr.time_gen;
  hdr.time_begin = _outMasterHdr.time_begin;
  hdr.time_end = _outMasterHdr.time_end;
  hdr.time_centroid = _outMasterHdr.time_centroid;
  hdr.time_expire = _outMasterHdr.time_expire;
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
          "Produced by MdvHowSimilar", MDV_INFO_LEN);
  STRcopy(hdr.data_set_name,
          "MdvHowSimilar", MDV_NAME_LEN);
}

void MdvHowSimilar::_fillVector(string commaSep, vector<string>& vec, bool removeWhite/*=true*/)
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
